#include "satox/chudnovsky_common.hpp"

#include "satox/format.hpp"
#include "satox/run_config.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <mpfr.h>

#include <cmath>
#include <future>

namespace satox {
namespace {

constexpr int kSharedIntegerVerifyMaxDigits = 1000000;

} // namespace

HypergeometricBsSpec make_chudnovsky_spec(const std::string &id, unsigned long leaf_block_terms,
                                          bool leaf_pq_cancellation) {
    HypergeometricBsSpec spec;
    spec.id = id;
    spec.p_factors = {{6, -5}, {2, -1}, {6, -1}};
    spec.q_factors = {{1, 0}, {1, 0}, {1, 0}};
    spec.q_constant = 10939058860032000ul;
    spec.linear_a = 545140134l;
    spec.linear_b = 13591409l;
    spec.alternating = true;
    spec.unit_first_p = true;
    spec.unit_first_q = true;
    spec.leaf_t_uses_q = false;
    spec.leaf_block_terms = leaf_block_terms;
    const bool gcd = !global_run_config().disable_gcd;
    spec.leaf_pq_cancellation = leaf_pq_cancellation && gcd;
    spec.gcd_cancellation = gcd;
    return spec;
}

unsigned long chudnovsky_term_count(int decimal_digits, int effective_guard_digits) {
    return static_cast<unsigned long>(
               std::ceil((decimal_digits + effective_guard_digits) / kChudnovskyDigitsPerTerm)) +
           1ul;
}

ChudnovskyFinalizeResult finalize_chudnovsky_pi(const HypergeometricBsResult &node,
                                                int decimal_digits, int effective_guard_digits,
                                                bool streaming_format) {
    ChudnovskyFinalizeResult out;
    const int precision_bits = bits_for_decimal_digits(decimal_digits, effective_guard_digits);
    const Timer finalize_timer;
    std::future<void> reference_warm_future;
    if (decimal_digits > kSharedIntegerVerifyMaxDigits) {
        reference_warm_future = std::async(std::launch::async, [=]() {
            warm_pi_reference_cache(decimal_digits, effective_guard_digits);
        });
    }
    mpfr_t q;
    mpfr_t t;
    mpfr_t sqrt_10005;
    mpfr_t pi;
    mpfr_init2(q, static_cast<mpfr_prec_t>(precision_bits));
    mpfr_init2(t, static_cast<mpfr_prec_t>(precision_bits));
    mpfr_init2(sqrt_10005, static_cast<mpfr_prec_t>(precision_bits));
    mpfr_init2(pi, static_cast<mpfr_prec_t>(precision_bits));

    mpfr_set_z(q, node.q, MPFR_RNDN);
    mpfr_mul_ui(q, q, 426880ul, MPFR_RNDN);
    const Timer sqrt_timer;
    mpfr_sqrt_ui(sqrt_10005, 10005ul, MPFR_RNDN);
    mpfr_mul(q, q, sqrt_10005, MPFR_RNDN);
    out.sqrt_div_ms = sqrt_timer.wall_ms();
    mpfr_set_z(t, node.t, MPFR_RNDN);
    mpfr_div(pi, q, t, MPFR_RNDN);
    out.sqrt_div_ms += finalize_timer.wall_ms() - sqrt_timer.wall_ms();
    out.finalize_ms = finalize_timer.wall_ms();

    DecimalPowerCache power_cache;
    build_decimal_power_cache(decimal_digits, power_cache);
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
                verify_scaled_pi_mpz(verify_int, decimal_digits, effective_guard_digits,
                                     &out.verify_ms);
            mpz_clear(verify_int);
            return ok;
        });
    } else {
        mpz_clear(verify_int);
        mpfr_init2(verify_scaled, static_cast<mpfr_prec_t>(precision_bits));
        mpfr_set(verify_scaled, pi_scaled, MPFR_RNDN);
        if (reference_warm_future.valid()) {
            reference_warm_future.get();
        }
        verify_future = std::async(std::launch::async, [&]() {
            const bool ok =
                verify_scaled_pi_mpfr(verify_scaled, decimal_digits, effective_guard_digits,
                                      &out.verify_ms);
            mpfr_clear(verify_scaled);
            return ok;
        });
    }

    const Timer format_timer;
    (void)streaming_format;
    out.decimal_prefix =
        shared_integer_snapshot
            ? scaled_pi_integer_to_decimal_parallel(scaled_int, decimal_digits, power_cache)
            : scaled_pi_to_decimal_parallel(pi_scaled, decimal_digits, power_cache);
    out.format_ms = format_timer.wall_ms();
    out.verified = verify_future.get();
    out.verification_method = "MPFR pre-format scaled-integer check (shared integer snapshot)";

    if (shared_integer_snapshot) {
        mpz_clear(scaled_int);
    }
    mpfr_clear(pi_scaled);
    mpfr_clear(q);
    mpfr_clear(t);
    mpfr_clear(sqrt_10005);
    mpfr_clear(pi);
    return out;
}

} // namespace satox
