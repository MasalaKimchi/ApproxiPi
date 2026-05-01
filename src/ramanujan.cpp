#include "satox/algorithm.hpp"

#include "satox/binary_splitting.hpp"
#include "satox/format.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <gmp.h>
#include <mpfr.h>

#include <cmath>
#include <memory>

namespace satox {
namespace {

constexpr double kRamanujanDigitsPerTerm = 7.9825407783902;

HypergeometricBsSpec ramanujan_spec() {
    HypergeometricBsSpec spec;
    spec.id = "ramanujan_classic_bs";
    spec.p_factors = {{4, 1}, {4, 2}, {4, 3}, {4, 4}};
    spec.q_factors = {{1, 1}, {1, 1}, {1, 1}, {1, 1}};
    spec.q_constant = 24591257856ul; // 396^4
    spec.linear_a = 26390l;
    spec.linear_b = 1103l;
    spec.leaf_t_uses_q = true;
    return spec;
}

class RamanujanAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"ramanujan_classic_bs", "Ramanujan classical binary splitting", 1, 1000000,
                true, false};
    }

    ComputeResult compute(int decimal_digits, int guard_digits) const override {
        ComputeResult result;
        result.metadata = metadata();
        result.decimal_digits = decimal_digits;
        result.guard_digits = guard_digits;
        result.estimated_digits_per_term = kRamanujanDigitsPerTerm;

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
                                                 kRamanujanDigitsPerTerm)) +
            8ul;
        result.terms_or_iterations = terms;

        HypergeometricBsResult node;
        BinarySplittingStats bs_stats{};
        binary_split_hypergeometric(ramanujan_spec(), 0, terms, node, &bs_stats);
        result.gcd_reductions = bs_stats.gcd_reductions;
        result.cancelled_bits = bs_stats.cancelled_bits;

        const int precision_bits = bits_for_decimal_digits(decimal_digits, effective_guard_digits);
        mpfr_t q;
        mpfr_t t;
        mpfr_t denominator;
        mpfr_t pi;
        mpfr_t sqrt2;
        mpfr_inits2(static_cast<mpfr_prec_t>(precision_bits), q, t, denominator, pi, sqrt2,
                    (mpfr_ptr)nullptr);

        mpfr_set_z(q, node.q, MPFR_RNDN);
        mpfr_mul_ui(q, q, 9801ul, MPFR_RNDN);
        mpfr_set_z(t, node.t, MPFR_RNDN);
        mpfr_sqrt_ui(sqrt2, 2ul, MPFR_RNDN);
        mpfr_mul_ui(denominator, sqrt2, 2ul, MPFR_RNDN);
        mpfr_mul(denominator, denominator, t, MPFR_RNDN);
        mpfr_div(pi, q, denominator, MPFR_RNDN);

        result.decimal_prefix = mpfr_to_decimal_prefix(pi, decimal_digits);
        result.wall_ms = timer.wall_ms();
        result.cpu_ms = timer.cpu_ms();
        result.verified = decimal_prefix_matches_pi(result.decimal_prefix, decimal_digits,
                                                    effective_guard_digits);
        result.verification_method = "MPFR const_pi prefix";

        mpfr_clears(q, t, denominator, pi, sqrt2, (mpfr_ptr)nullptr);
        return result;
    }
};

} // namespace

std::unique_ptr<PiAlgorithm> make_ramanujan_algorithm() {
    return std::make_unique<RamanujanAlgorithm>();
}

} // namespace satox
