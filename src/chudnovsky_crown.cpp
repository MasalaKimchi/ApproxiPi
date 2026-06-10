#include "satox/algorithm.hpp"

#include "satox/chudnovsky_common.hpp"
#include "satox/crown.hpp"
#include "satox/format.hpp"
#include "satox/limits.hpp"
#include "satox/memory_estimate.hpp"
#include "satox/run_config.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <mpfr.h>

#include <cmath>
#include <future>
#include <iomanip>
#include <memory>
#include <sstream>

namespace satox {
namespace {

HypergeometricBsSpec chudnovsky_crown_spec() {
    return make_chudnovsky_spec("chudnovsky_bs_crown", 8, true);
}

class ChudnovskyCrownAlgorithm final : public PiAlgorithm {
  public:
    ChudnovskyCrownAlgorithm() = default;
    ChudnovskyCrownAlgorithm(std::string name, std::string family, std::string tuning_path)
        : name_(std::move(name)), family_(std::move(family)),
          tuning_path_(std::move(tuning_path)) {}

    AlgorithmMetadata metadata() const override {
        return {name_, family_, 1, kMaxBenchmarkDigits, true, false};
    }

    ComputeResult compute(int decimal_digits, int guard_digits) const override {
        ComputeResult result;
        result.metadata = metadata();
        result.decimal_digits = decimal_digits;
        result.guard_digits = guard_digits;
        result.estimated_digits_per_term = kChudnovskyDigitsPerTerm;

        if (!valid_digit_request(decimal_digits, guard_digits, &result.error)) {
            return result;
        }
        if (decimal_digits > result.metadata.max_digits) {
            result.error = "requested precision exceeds algorithm max_digits";
            return result;
        }
        if (!memory_guard_allows(decimal_digits, result.metadata.name, &result.error)) {
            return result;
        }

        CrownTuning tuning;
        const bool tuned = !tuning_path_.empty();
        if (tuned && !load_crown_tuning(tuning_path_, tuning)) {
            result.error = "no tuning profile at " + tuning_path_ +
                           " (run satox-bench --tune first)";
            return result;
        }

        result.supported = true;
        result.notes = global_run_config().ablation_tag;
        int crown_guard_bonus = 128;
        if (decimal_digits >= 100000000) {
            crown_guard_bonus = 512;
        } else if (decimal_digits >= 10000000) {
            crown_guard_bonus = 256;
        }
        const int effective_guard_digits = guard_digits + crown_guard_bonus;
        const Timer timer;
        const unsigned long terms =
            static_cast<unsigned long>(std::ceil((decimal_digits + effective_guard_digits) /
                                                 kChudnovskyDigitsPerTerm)) +
            1ul;
        result.terms_or_iterations = terms;

        const int precision_bits = bits_for_decimal_digits(decimal_digits, effective_guard_digits);
        mpfr_t q;
        mpfr_t t;
        mpfr_t scale_constant;
        mpfr_t reciprocal;
        mpfr_t pi_scaled;
        mpfr_init2(q, static_cast<mpfr_prec_t>(precision_bits));
        mpfr_init2(t, static_cast<mpfr_prec_t>(precision_bits));
        mpfr_init2(scale_constant, static_cast<mpfr_prec_t>(precision_bits));
        mpfr_init2(reciprocal, static_cast<mpfr_prec_t>(precision_bits));
        mpfr_init2(pi_scaled, static_cast<mpfr_prec_t>(precision_bits));

        // Below this size, thread spawn and the 10^digits pipeline cost more
        // than they save; use the plain finalize/format path instead.
        const bool pipelined = decimal_digits >= 20000;

        // Value-independent work overlaps the series evaluation: the decimal
        // power table and the fused constant 426880 * sqrt(10005) * 10^digits.
        DecimalPowerCache power_cache;
        std::shared_future<void> constant_ready;
        if (pipelined) {
            constant_ready =
                std::async(std::launch::async,
                           [&]() {
                               build_decimal_power_cache(decimal_digits, power_cache);
                               mpfr_sqrt_ui(scale_constant, 10005ul, MPFR_RNDN);
                               mpfr_mul_ui(scale_constant, scale_constant, 426880ul, MPFR_RNDN);
                               mpfr_mul_z(scale_constant, scale_constant, power_cache.p10_full,
                                          MPFR_RNDN);
                           })
                    .share();
        }

        // Root-Q elision + warm-started Newton reciprocal: pi only needs
        // constant * Ql * Qr / T, so the root Q product is never formed.
        // The numerator and both half-precision reciprocals (1/T ~=
        // (1/Tl) * (1/Qr) to the tail's downshift) run concurrently with the
        // root T merge; one half-width Newton correction reaches full
        // precision afterwards.
        mpfr_t t_left_copy;
        mpfr_t q_left_copy;
        mpfr_t q_right_copy;
        mpfr_t numerator;
        bool warm_copies_initialized = false;
        bool warm_started = false;
        mpfr_prec_t warm_bits = 0;
        std::future<void> warm_future;
        std::future<void> numerator_future;
        const std::function<bool(mpfr_srcptr, mpfr_srcptr, mpfr_srcptr, long)> warm_hook =
            [&](mpfr_srcptr t_left, mpfr_srcptr q_left, mpfr_srcptr q_right,
                long agreement_bits) {
                const long s0 = agreement_bits - 64;
                // Gate: one correction step must reach full precision.
                if (s0 < 4096 || 2l * s0 < static_cast<long>(precision_bits) + 256l) {
                    return false;
                }
                warm_bits = static_cast<mpfr_prec_t>(s0);
                mpfr_init2(t_left_copy, warm_bits + 64);
                mpfr_init2(q_left_copy, mpfr_get_prec(q_left));
                mpfr_init2(q_right_copy, mpfr_get_prec(q_right));
                mpfr_init2(numerator, static_cast<mpfr_prec_t>(precision_bits));
                warm_copies_initialized = true;
                mpfr_set(t_left_copy, t_left, MPFR_RNDN);
                mpfr_set(q_left_copy, q_left, MPFR_RNDN);
                mpfr_set(q_right_copy, q_right, MPFR_RNDN);
                mpfr_set_prec(reciprocal, warm_bits);
                warm_started = true;
                warm_future = std::async(std::launch::async, [&]() {
                    mpfr_t recip_q;
                    mpfr_init2(recip_q, warm_bits);
                    auto recip_q_future = std::async(std::launch::async, [&]() {
                        mpfr_ui_div(recip_q, 1ul, q_right_copy, MPFR_RNDN);
                    });
                    mpfr_t recip_t;
                    mpfr_init2(recip_t, warm_bits);
                    mpfr_ui_div(recip_t, 1ul, t_left_copy, MPFR_RNDN);
                    recip_q_future.get();
                    mpfr_mul(reciprocal, recip_t, recip_q, MPFR_RNDN);
                    mpfr_clear(recip_t);
                    mpfr_clear(recip_q);
                });
                numerator_future = std::async(std::launch::async, [&]() {
                    constant_ready.get();
                    mpfr_mul(numerator, scale_constant, q_left_copy, MPFR_RNDN);
                    mpfr_mul(numerator, numerator, q_right_copy, MPFR_RNDN);
                });
                return true;
            };

        CrownStats crown_stats{};
        const HypergeometricBsSpec spec = chudnovsky_crown_spec();
        const Timer split_timer;
        binary_split_crown(spec, terms, q, t, &crown_stats, pipelined ? &warm_hook : nullptr,
                           tuned ? &tuning : nullptr);
        result.split_ms = split_timer.wall_ms();
        result.gcd_reductions = crown_stats.split.gcd_reductions;
        result.cancelled_bits = crown_stats.split.cancelled_bits;
        result.max_operand_bits = crown_stats.split.max_operand_bits;
        result.parallel_depth = crown_stats.crown_depth;
        result.mul_count = crown_stats.split.mul_count + crown_stats.crown_mul_count;
        result.mul_bit_volume =
            crown_stats.split.mul_bit_volume + crown_stats.crown_mul_bit_volume;

        if (!pipelined) {
            // Small sizes: plain serial finalize and format, no 10^digits
            // scaling and no helper threads.
            const Timer small_finalize_timer;
            mpfr_sqrt_ui(scale_constant, 10005ul, MPFR_RNDN);
            mpfr_mul_ui(scale_constant, scale_constant, 426880ul, MPFR_RNDN);
            mpfr_mul(q, q, scale_constant, MPFR_RNDN);
            mpfr_div(pi_scaled, q, t, MPFR_RNDN);
            result.finalize_ms = small_finalize_timer.wall_ms();

            mpz_t p10;
            mpz_init(p10);
            mpz_ui_pow_ui(p10, 10ul, static_cast<unsigned long>(decimal_digits));
            mpfr_mul_z(pi_scaled, pi_scaled, p10, MPFR_RNDN);

            result.verified = verify_scaled_pi_mpfr(pi_scaled, decimal_digits,
                                                    effective_guard_digits, &result.verify_ms);
            result.verification_method = "MPFR pre-format scaled-integer check (truncated crown)";

            const Timer small_format_timer;
            mpfr_div_z(pi_scaled, pi_scaled, p10, MPFR_RNDN);
            mpz_clear(p10);
            result.decimal_prefix = mpfr_to_decimal_prefix(pi_scaled, decimal_digits);
            result.format_ms = small_format_timer.wall_ms();
            result.wall_ms = timer.wall_ms();
            result.cpu_ms = timer.cpu_ms();
            mpfr_clear(q);
            mpfr_clear(t);
            mpfr_clear(scale_constant);
            mpfr_clear(reciprocal);
            mpfr_clear(pi_scaled);
            return result;
        }

        // H12 pipelined prefix rendering: after pi0 = numerator * r0, the
        // Newton correction only perturbs bits ~warm_bits below the leading
        // bit, so the top half of the decimal expansion is already final.
        // Render it concurrently with the residual/correction products and
        // splice the corrected low half in afterwards; a rigorous range check
        // on the recovered low half proves the splice is exact (rare carry
        // crossings fall back to the full re-render).
        const int total_width = decimal_digits + 1;
        mpz_t z0;
        mpz_t low0;
        mpz_t high0;
        mpz_t shifted_high;
        std::string high_prefix;
        bool prefix_pipeline = false;
        std::promise<void> low0_ready;
        std::future<void> low0_ready_future;
        std::future<void> prefix_future;

        const Timer finalize_timer;
        if (warm_started) {
            warm_future.get();
            // pi0 = numerator * r0 and the Newton residual T * r0 - 1 are
            // independent full-precision products; the correction product
            // only needs the missing low bits.
            mpfr_t residual;
            mpfr_init2(residual, static_cast<mpfr_prec_t>(precision_bits));
            auto residual_future = std::async(std::launch::async, [&]() {
                mpfr_mul(residual, t, reciprocal, MPFR_RNDN);
                mpfr_sub_ui(residual, residual, 1ul, MPFR_RNDN);
            });
            numerator_future.get();
            // Root-Q elision moved the two numerator products out of the
            // split phase; keep them in the work metric for fairness.
            result.mul_count += 2;
            result.mul_bit_volume +=
                static_cast<double>(mpfr_get_prec(scale_constant)) +
                static_cast<double>(mpfr_get_prec(q_left_copy)) +
                static_cast<double>(mpfr_get_prec(numerator)) +
                static_cast<double>(mpfr_get_prec(q_right_copy));
            mpfr_mul(pi_scaled, numerator, reciprocal, MPFR_RNDN);

            // The high half is carry-stable only when the correction is far
            // smaller than the low-half modulus 10^(4w); warm_bits exceeds
            // half the precision by the H4/H7 gate, giving a wide margin.
            if (power_cache.ready && power_cache.digits == decimal_digits &&
                total_width - 7 * power_cache.part_width >= 1 && mpfr_sgn(pi_scaled) > 0) {
                mpz_init(z0);
                mpz_init(low0);
                mpz_init(high0);
                mpz_init(shifted_high);
                mpfr_get_z(z0, pi_scaled, MPFR_RNDZ);
                prefix_pipeline = true;
                low0_ready_future = low0_ready.get_future();
                prefix_future = std::async(std::launch::async, [&]() {
                    // The split divmod runs under the correction products;
                    // low0 is released to the main thread as soon as it
                    // exists so the corrected low half can render in
                    // parallel with the high half.
                    mpz_tdiv_qr(high0, low0, z0, power_cache.pows[2]);
                    low0_ready.set_value();
                    const int low_width = power_cache.part_width << 2;
                    high_prefix = render_decimal_fixed(high0, total_width - low_width, 1,
                                                       power_cache);
                });
            }

            residual_future.get();
            mpfr_t correction;
            mpfr_init2(correction,
                       std::max<mpfr_prec_t>(static_cast<mpfr_prec_t>(precision_bits) -
                                                 warm_bits + 64,
                                             64));
            mpfr_mul(correction, pi_scaled, residual, MPFR_RNDN);
            mpfr_sub(pi_scaled, pi_scaled, correction, MPFR_RNDN);
            mpfr_clear(residual);
            mpfr_clear(correction);
        } else {
            // Fallback: plain full-precision reciprocal against the kernel's
            // root Q.
            constant_ready.get();
            auto recip_future = std::async(std::launch::async, [&]() {
                mpfr_ui_div(reciprocal, 1ul, t, MPFR_RNDN);
            });
            mpfr_mul(q, q, scale_constant, MPFR_RNDN);
            recip_future.get();
            mpfr_mul(pi_scaled, q, reciprocal, MPFR_RNDN);
        }
        result.finalize_ms = finalize_timer.wall_ms();

        result.verified = verify_scaled_pi_mpfr(pi_scaled, decimal_digits, effective_guard_digits,
                                                &result.verify_ms);

        const Timer format_timer;
        bool spliced = false;
        if (prefix_pipeline) {
            // Recover the corrected low half without re-dividing: the high
            // half's contribution z0 - low0 = high0 * 10^(4w) is already
            // known, so low_corr = z_corr - (z0 - low0). The range check
            // 0 <= low_corr < 10^(4w) proves high0 is still the exact high
            // half of the corrected value.
            mpz_t z_corr;
            mpz_init(z_corr);
            mpfr_get_z(z_corr, pi_scaled, MPFR_RNDZ);
            low0_ready_future.wait();
            mpz_sub(shifted_high, z0, low0);
            mpz_sub(z_corr, z_corr, shifted_high);
            const int low_width = power_cache.part_width << 2;
            if (mpz_sgn(z_corr) >= 0 && mpz_cmp(z_corr, power_cache.pows[2]) < 0) {
                // Low half renders here while the prefix thread finishes the
                // high half.
                const std::string low_digits =
                    render_decimal_fixed(z_corr, low_width, 1, power_cache);
                prefix_future.get();
                const std::string digits = high_prefix + low_digits;
                const size_t int_width = digits.size() - static_cast<size_t>(decimal_digits);
                result.decimal_prefix = digits.substr(0, int_width) + "." +
                                        digits.substr(int_width);
                spliced = true;
            } else {
                prefix_future.get();
            }
            mpz_clear(z_corr);
        }
        if (!spliced) {
            result.decimal_prefix =
                scaled_pi_to_decimal_parallel(pi_scaled, decimal_digits, power_cache);
        }
        result.format_ms = format_timer.wall_ms();
        result.wall_ms = timer.wall_ms();
        result.cpu_ms = timer.cpu_ms();
        result.total_cost_ms = result.wall_ms + result.verify_ms;
        {
            std::ostringstream method;
            method << "MPFR pre-format scaled-integer check (truncated crown; chunks "
                   << std::fixed << std::setprecision(3) << crown_stats.chunk_ms
                   << "ms, merge " << crown_stats.merge_ms << "ms)";
            result.verification_method = method.str();
        }

        mpfr_clear(q);
        mpfr_clear(t);
        mpfr_clear(scale_constant);
        mpfr_clear(reciprocal);
        mpfr_clear(pi_scaled);
        if (prefix_pipeline) {
            mpz_clear(z0);
            mpz_clear(low0);
            mpz_clear(high0);
            mpz_clear(shifted_high);
        }
        if (warm_copies_initialized) {
            mpfr_clear(t_left_copy);
            mpfr_clear(q_left_copy);
            mpfr_clear(q_right_copy);
            mpfr_clear(numerator);
        }
        return result;
    }

  private:
    std::string name_ = "chudnovsky_bs_crown";
    std::string family_ = "Chudnovsky binary splitting with truncated MPFR crown";
    std::string tuning_path_;
};

} // namespace

std::unique_ptr<PiAlgorithm> make_chudnovsky_crown_algorithm() {
    return std::make_unique<ChudnovskyCrownAlgorithm>(
        "chudnovsky_bs_crown", "Chudnovsky binary splitting with truncated MPFR crown", "");
}

std::unique_ptr<PiAlgorithm> make_chudnovsky_crown_tuned_algorithm() {
    return std::make_unique<ChudnovskyCrownAlgorithm>(
        "chudnovsky_bs_crown_tuned",
        "Truncated crown with autotuned knob profile (results/tuning.json)",
        "results/tuning.json");
}

} // namespace satox
