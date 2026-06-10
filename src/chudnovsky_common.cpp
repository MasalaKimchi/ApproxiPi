#include "satox/chudnovsky_common.hpp"

#include "satox/format.hpp"
#include "satox/run_config.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <mpfr.h>

#include <cmath>

namespace satox {

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

    out.verified = verify_unscaled_pi_mpfr(pi, decimal_digits, effective_guard_digits,
                                           &out.verify_ms);

    const Timer format_timer;
    out.decimal_prefix =
        streaming_format ? mpfr_to_decimal_prefix_streaming(pi, decimal_digits)
                         : mpfr_to_decimal_prefix(pi, decimal_digits);
    out.format_ms = format_timer.wall_ms();
    out.verification_method = "MPFR pre-format scaled-integer check";

    mpfr_clear(q);
    mpfr_clear(t);
    mpfr_clear(sqrt_10005);
    mpfr_clear(pi);
    return out;
}

} // namespace satox
