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

HypergeometricBsSpec chudnovsky_spec(bool leaf_pq_cancellation) {
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
    spec.leaf_pq_cancellation = leaf_pq_cancellation;
    return spec;
}

class ChudnovskyAlgorithm final : public PiAlgorithm {
  public:
    explicit ChudnovskyAlgorithm(bool leaf_pq_cancellation = false)
        : leaf_pq_cancellation_(leaf_pq_cancellation) {}

    AlgorithmMetadata metadata() const override {
        if (leaf_pq_cancellation_) {
            return {"chudnovsky_bs_valuation",
                    "Chudnovsky binary splitting with leaf valuation cancellation", 1,
                    1000000, true, false};
        }
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
        BinarySplittingStats bs_stats{};
        const unsigned int parallel_depth = recommended_parallel_depth(terms);
        const HypergeometricBsSpec spec = chudnovsky_spec(leaf_pq_cancellation_);
        const Timer split_timer;
        binary_split_hypergeometric(spec, 0, terms, node, &bs_stats, parallel_depth);
        result.split_ms = split_timer.wall_ms();
        result.gcd_reductions = bs_stats.gcd_reductions;
        result.cancelled_bits = bs_stats.cancelled_bits;
        result.max_operand_bits = bs_stats.max_operand_bits;
        result.parallel_depth = bs_stats.parallel_depth;

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
        mpfr_sqrt_ui(sqrt_10005, 10005ul, MPFR_RNDN);
        mpfr_mul(q, q, sqrt_10005, MPFR_RNDN);
        mpfr_set_z(t, node.t, MPFR_RNDN);
        mpfr_div(pi, q, t, MPFR_RNDN);
        result.finalize_ms = finalize_timer.wall_ms();

        const Timer format_timer;
        result.decimal_prefix = mpfr_to_decimal_prefix(pi, decimal_digits);
        result.format_ms = format_timer.wall_ms();
        result.wall_ms = timer.wall_ms();
        result.cpu_ms = timer.cpu_ms();
        const Timer verify_timer;
        result.verified = decimal_prefix_matches_pi(result.decimal_prefix, decimal_digits,
                                                    effective_guard_digits);
        result.verify_ms = verify_timer.wall_ms();
        result.verification_method = "MPFR const_pi prefix";

        mpfr_clear(q);
        mpfr_clear(t);
        mpfr_clear(sqrt_10005);
        mpfr_clear(pi);
        return result;
    }

  private:
    bool leaf_pq_cancellation_;
};

} // namespace

std::unique_ptr<PiAlgorithm> make_chudnovsky_algorithm() {
    return std::make_unique<ChudnovskyAlgorithm>();
}

std::unique_ptr<PiAlgorithm> make_chudnovsky_valuation_algorithm() {
    return std::make_unique<ChudnovskyAlgorithm>(true);
}

} // namespace satox
