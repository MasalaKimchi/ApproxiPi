// External-library baselines, run through the exact same digit-request,
// formatting and verification pipeline as the in-repo algorithms. These are
// installable, citable references (MPFR, FLINT/Arb) that replace the
// unavailable y-cruncher as the "is this actually fast?" control.

#include "satox/algorithm.hpp"

#include "satox/format.hpp"
#include "satox/limits.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <mpfr.h>

#ifdef SATOX_HAVE_FLINT
#include <flint/arb.h>
#include <flint/flint.h>
#endif

#include <future>
#include <memory>
#include <thread>

namespace satox {
namespace {

constexpr int kSharedIntegerVerifyMaxDigits = 1000000;

class MpfrConstPiAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"mpfr_const_pi", "MPFR mpfr_const_pi (cold cache, single-threaded)", 1,
                kMaxBenchmarkDigits, true, false};
    }

    ComputeResult compute(int decimal_digits, int guard_digits) const override {
        ComputeResult result;
        result.metadata = metadata();
        result.decimal_digits = decimal_digits;
        result.guard_digits = guard_digits;

        if (!valid_digit_request(decimal_digits, guard_digits, &result.error)) {
            return result;
        }
        if (decimal_digits > result.metadata.max_digits) {
            result.error = "requested precision exceeds algorithm max_digits";
            return result;
        }
        result.supported = true;

        const int effective_guard_digits = guard_digits + 64;
        const int precision_bits =
            bits_for_decimal_digits(decimal_digits, effective_guard_digits);
        std::future<void> reference_warm_future;
        if (decimal_digits > kSharedIntegerVerifyMaxDigits) {
            reference_warm_future = std::async(std::launch::async, [=]() {
                warm_pi_reference_cache(decimal_digits, effective_guard_digits);
            });
        }

        // MPFR caches const_pi internally; clear it so each trial measures a
        // cold computation rather than a memcpy of the previous trial.
        mpfr_free_cache();

        const Timer timer;
        DecimalPowerCache power_cache;
        auto power_future = std::async(std::launch::async, [&]() {
            build_decimal_power_cache(decimal_digits, power_cache);
        });
        mpfr_t pi;
        mpfr_init2(pi, static_cast<mpfr_prec_t>(precision_bits));
        const Timer finalize_timer;
        mpfr_const_pi(pi, MPFR_RNDN);
        result.finalize_ms = finalize_timer.wall_ms();
        result.terms_or_iterations = 1;

        power_future.get();
        mpfr_t pi_scaled;
        mpfr_init2(pi_scaled, static_cast<mpfr_prec_t>(precision_bits));
        mpfr_mul_z(pi_scaled, pi, power_cache.p10_full, MPFR_RNDN);

        const bool shared_integer_snapshot =
            decimal_digits <= kSharedIntegerVerifyMaxDigits;
        mpz_t scaled_int;
        mpz_t verify_int;
        mpfr_t verify_scaled;
        if (shared_integer_snapshot) {
            mpz_init(scaled_int);
            mpz_init(verify_int);
            mpfr_get_z(scaled_int, pi_scaled, MPFR_RNDZ);
        }
        std::future<bool> verify_future;
        if (shared_integer_snapshot) {
            mpz_set(verify_int, scaled_int);
            verify_future = std::async(std::launch::async, [&]() {
                const bool ok =
                    verify_scaled_pi_mpz(verify_int, decimal_digits,
                                         effective_guard_digits, &result.verify_ms);
                mpz_clear(verify_int);
                return ok;
            });
        } else {
            mpfr_init2(verify_scaled, static_cast<mpfr_prec_t>(precision_bits));
            mpfr_set(verify_scaled, pi_scaled, MPFR_RNDN);
            if (reference_warm_future.valid()) {
                reference_warm_future.get();
            }
            verify_future = std::async(std::launch::async, [&]() {
                const bool ok =
                    verify_scaled_pi_mpfr(verify_scaled, decimal_digits,
                                          effective_guard_digits, &result.verify_ms);
                mpfr_clear(verify_scaled);
                return ok;
            });
        }

        const Timer format_timer;
        result.decimal_prefix =
            shared_integer_snapshot
                ? scaled_pi_integer_to_decimal_parallel(scaled_int, decimal_digits, power_cache)
                : scaled_pi_to_decimal_parallel(pi_scaled, decimal_digits, power_cache);
        result.format_ms = format_timer.wall_ms();
        result.verified = verify_future.get();
        const double elapsed_wall_ms = timer.wall_ms();
        result.wall_ms =
            elapsed_wall_ms > result.verify_ms ? elapsed_wall_ms - result.verify_ms : 0.0;
        result.cpu_ms = timer.cpu_ms();
        result.verification_method =
            "MPFR pre-format scaled-integer check (mpfr_const_pi; shared integer snapshot)";
        result.total_cost_ms = elapsed_wall_ms + result.io_ms;
        if (shared_integer_snapshot) {
            mpz_clear(scaled_int);
        }
        mpfr_clear(pi_scaled);
        mpfr_clear(pi);
        return result;
    }
};

#ifdef SATOX_HAVE_FLINT

class ArbConstPiAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"arb_const_pi", "FLINT/Arb arb_const_pi (cold cache)", 1, kMaxBenchmarkDigits, true,
                false};
    }

    ComputeResult compute(int decimal_digits, int guard_digits) const override {
        ComputeResult result;
        result.metadata = metadata();
        result.decimal_digits = decimal_digits;
        result.guard_digits = guard_digits;

        if (!valid_digit_request(decimal_digits, guard_digits, &result.error)) {
            return result;
        }
        if (decimal_digits > result.metadata.max_digits) {
            result.error = "requested precision exceeds algorithm max_digits";
            return result;
        }
        result.supported = true;

        const int effective_guard_digits = guard_digits + 64;
        const int precision_bits =
            bits_for_decimal_digits(decimal_digits, effective_guard_digits);
        std::future<void> reference_warm_future;
        if (decimal_digits > kSharedIntegerVerifyMaxDigits) {
            reference_warm_future = std::async(std::launch::async, [=]() {
                warm_pi_reference_cache(decimal_digits, effective_guard_digits);
            });
        }

        // Give FLINT the same hardware budget as the in-repo parallel code
        // and clear its constant caches so every trial is cold.
        flint_set_num_threads(
            static_cast<int>(std::max(1u, std::thread::hardware_concurrency())));
        flint_cleanup();

        const Timer timer;
        DecimalPowerCache power_cache;
        auto power_future = std::async(std::launch::async, [&]() {
            build_decimal_power_cache(decimal_digits, power_cache);
        });
        arb_t pi;
        arb_init(pi);
        const Timer finalize_timer;
        arb_const_pi(pi, static_cast<slong>(precision_bits));
        result.finalize_ms = finalize_timer.wall_ms();
        result.terms_or_iterations = 1;

        mpfr_t pi_mpfr;
        mpfr_init2(pi_mpfr, static_cast<mpfr_prec_t>(precision_bits));
        arf_get_mpfr(pi_mpfr, arb_midref(pi), MPFR_RNDN);

        power_future.get();
        mpfr_t pi_scaled;
        mpfr_init2(pi_scaled, static_cast<mpfr_prec_t>(precision_bits));
        mpfr_mul_z(pi_scaled, pi_mpfr, power_cache.p10_full, MPFR_RNDN);

        const bool shared_integer_snapshot =
            decimal_digits <= kSharedIntegerVerifyMaxDigits;
        mpz_t scaled_int;
        mpz_t verify_int;
        mpfr_t verify_scaled;
        if (shared_integer_snapshot) {
            mpz_init(scaled_int);
            mpz_init(verify_int);
            mpfr_get_z(scaled_int, pi_scaled, MPFR_RNDZ);
        }
        std::future<bool> verify_future;
        if (shared_integer_snapshot) {
            mpz_set(verify_int, scaled_int);
            verify_future = std::async(std::launch::async, [&]() {
                const bool ok =
                    verify_scaled_pi_mpz(verify_int, decimal_digits,
                                         effective_guard_digits, &result.verify_ms);
                mpz_clear(verify_int);
                return ok;
            });
        } else {
            mpfr_init2(verify_scaled, static_cast<mpfr_prec_t>(precision_bits));
            mpfr_set(verify_scaled, pi_scaled, MPFR_RNDN);
            if (reference_warm_future.valid()) {
                reference_warm_future.get();
            }
            verify_future = std::async(std::launch::async, [&]() {
                const bool ok =
                    verify_scaled_pi_mpfr(verify_scaled, decimal_digits,
                                          effective_guard_digits, &result.verify_ms);
                mpfr_clear(verify_scaled);
                return ok;
            });
        }

        const Timer format_timer;
        result.decimal_prefix =
            shared_integer_snapshot
                ? scaled_pi_integer_to_decimal_parallel(scaled_int, decimal_digits, power_cache)
                : scaled_pi_to_decimal_parallel(pi_scaled, decimal_digits, power_cache);
        result.format_ms = format_timer.wall_ms();
        result.verified = verify_future.get();
        const double elapsed_wall_ms = timer.wall_ms();
        result.wall_ms =
            elapsed_wall_ms > result.verify_ms ? elapsed_wall_ms - result.verify_ms : 0.0;
        result.cpu_ms = timer.cpu_ms();
        result.verification_method =
            "MPFR pre-format scaled-integer check (arb_const_pi; shared integer snapshot)";
        result.total_cost_ms = elapsed_wall_ms + result.io_ms;

        if (shared_integer_snapshot) {
            mpz_clear(scaled_int);
        }
        mpfr_clear(pi_scaled);
        mpfr_clear(pi_mpfr);
        arb_clear(pi);
        return result;
    }
};

#endif // SATOX_HAVE_FLINT

class ArbUnavailableAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"arb_const_pi", "FLINT/Arb arb_const_pi (not built)", 1, kMaxBenchmarkDigits, true,
                false};
    }

    ComputeResult compute(int decimal_digits, int guard_digits) const override {
        ComputeResult result;
        result.metadata = metadata();
        result.decimal_digits = decimal_digits;
        result.guard_digits = guard_digits;
        result.error = "built without FLINT/Arb (install flint and rebuild)";
        return result;
    }
};

} // namespace

std::unique_ptr<PiAlgorithm> make_mpfr_const_pi_algorithm() {
    return std::make_unique<MpfrConstPiAlgorithm>();
}

std::unique_ptr<PiAlgorithm> make_arb_const_pi_algorithm() {
#ifdef SATOX_HAVE_FLINT
    return std::make_unique<ArbConstPiAlgorithm>();
#else
    return std::make_unique<ArbUnavailableAlgorithm>();
#endif
}

} // namespace satox
