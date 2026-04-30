#include "satox/algorithm.hpp"

#include "satox/binary_splitting.hpp"
#include "satox/format.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <gmp.h>
#include <mpfr.h>

#include <cmath>
#include <memory>
#include <string>

namespace satox {
namespace {

constexpr double kChudnovskyDigitsPerTerm = 14.181647462725477;

HypergeometricBsSpec chudnovsky_spec() {
    HypergeometricBsSpec spec;
    spec.id = "chudnovsky_bs";
    spec.p_factors = {{6, -5}, {2, -1}, {6, -1}};
    spec.q_factors = {{1, 0}, {1, 0}, {1, 0}};
    spec.q_constant = 10939058860032000ul;
    spec.linear_a = 545140134l;
    spec.linear_b = 13591409l;
    spec.alternating = true;
    spec.unit_first_p = true;
    spec.unit_first_q = true;
    spec.leaf_t_uses_q = false;
    return spec;
}

class ChudnovskyAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"chudnovsky_bs", "Chudnovsky binary splitting", 1, 1000000, true, false};
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

        result.supported = true;
        const int effective_guard_digits = guard_digits + 128;
        const Timer timer;
        const unsigned long terms =
            static_cast<unsigned long>(std::ceil((decimal_digits + effective_guard_digits) /
                                                 kChudnovskyDigitsPerTerm)) +
            1ul;
        result.terms_or_iterations = terms;

        HypergeometricBsResult node;
        BinarySplittingStats bs_stats;
        binary_split_hypergeometric(chudnovsky_spec(), 0, terms, node, &bs_stats);
        result.gcd_reductions = bs_stats.gcd_reductions;
        result.cancelled_bits = bs_stats.cancelled_bits;

        const int precision_bits = bits_for_decimal_digits(decimal_digits, effective_guard_digits);
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
        mpfr_sqrt_ui(sqrt_10005, 10005ul, MPFR_RNDN);
        mpfr_mul(q, q, sqrt_10005, MPFR_RNDN);
        mpfr_set_z(t, node.t, MPFR_RNDN);
        mpfr_div(pi, q, t, MPFR_RNDN);

        result.decimal_prefix = mpfr_to_decimal_prefix(pi, decimal_digits);
        result.wall_ms = timer.wall_ms();
        result.cpu_ms = timer.cpu_ms();
        result.verified = decimal_prefix_matches_pi(result.decimal_prefix, decimal_digits,
                                                    effective_guard_digits);
        result.verification_method = "MPFR const_pi prefix";

        mpfr_clear(q);
        mpfr_clear(t);
        mpfr_clear(sqrt_10005);
        mpfr_clear(pi);
        return result;
    }
};

} // namespace

std::unique_ptr<PiAlgorithm> make_chudnovsky_algorithm() {
    return std::make_unique<ChudnovskyAlgorithm>();
}

} // namespace satox
