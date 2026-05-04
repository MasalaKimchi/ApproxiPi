#include "satox/algorithm.hpp"

#include "satox/format.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <mpfr.h>

#include <cmath>
#include <memory>

namespace satox {
namespace {

void set_precision(mpfr_prec_t precision, mpfr_t y, mpfr_t a, mpfr_t sqrt2, mpfr_t y4,
                   mpfr_t root, mpfr_t next_y, mpfr_t one_plus_y, mpfr_t y_poly,
                   mpfr_t term, mpfr_t pow2, mpfr_t pi) {
    mpfr_prec_round(y, precision, MPFR_RNDN);
    mpfr_prec_round(a, precision, MPFR_RNDN);
    mpfr_prec_round(sqrt2, precision, MPFR_RNDN);
    mpfr_prec_round(y4, precision, MPFR_RNDN);
    mpfr_prec_round(root, precision, MPFR_RNDN);
    mpfr_prec_round(next_y, precision, MPFR_RNDN);
    mpfr_prec_round(one_plus_y, precision, MPFR_RNDN);
    mpfr_prec_round(y_poly, precision, MPFR_RNDN);
    mpfr_prec_round(term, precision, MPFR_RNDN);
    mpfr_prec_round(pow2, precision, MPFR_RNDN);
    mpfr_prec_round(pi, precision, MPFR_RNDN);
}

class BorweinQuarticAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"borwein_quartic", "Borwein quartic convergence", 1, 1000000, true, false};
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
        const int effective_guard_digits = guard_digits + 128;
        const Timer timer;
        const Timer finalize_timer;
        double format_ms = 0.0;
        double verify_ms = 0.0;
        const int precision_bits = bits_for_decimal_digits(decimal_digits, effective_guard_digits);

        mpfr_t y;
        mpfr_t a;
        mpfr_t sqrt2;
        mpfr_t y4;
        mpfr_t root;
        mpfr_t next_y;
        mpfr_t one_plus_y;
        mpfr_t y_poly;
        mpfr_t term;
        mpfr_t pow2;
        mpfr_t pi;
        mpfr_inits2(static_cast<mpfr_prec_t>(precision_bits), y, a, sqrt2, y4, root, next_y,
                    one_plus_y, y_poly, term, pow2, pi, (mpfr_ptr)nullptr);

        mpfr_sqrt_ui(sqrt2, 2ul, MPFR_RNDN);
        mpfr_sub_ui(y, sqrt2, 1ul, MPFR_RNDN);

        mpfr_mul_ui(a, sqrt2, 4ul, MPFR_RNDN);
        mpfr_ui_sub(a, 6ul, a, MPFR_RNDN);

        const unsigned int iterations =
            static_cast<unsigned int>(
                std::ceil(std::log(static_cast<double>(decimal_digits + effective_guard_digits)) /
                          std::log(4.0))) +
            1u;
        result.terms_or_iterations = iterations;
        result.estimated_digits_per_term =
            static_cast<double>(decimal_digits) / static_cast<double>(iterations);

        for (unsigned int k = 0; k < iterations; ++k) {
            const unsigned int remaining = iterations - k - 1u;
            const double divisor = remaining > 2u ? std::pow(4.0, static_cast<double>(remaining - 2u))
                                                  : 1.0;
            const int step_digits =
                remaining > 2u
                    ? static_cast<int>(std::ceil(
                          static_cast<double>(decimal_digits + effective_guard_digits) /
                          divisor)) +
                          256
                    : decimal_digits + effective_guard_digits;
            const mpfr_prec_t step_bits =
                static_cast<mpfr_prec_t>(bits_for_decimal_digits(step_digits, 0));
            set_precision(step_bits, y, a, sqrt2, y4, root, next_y, one_plus_y, y_poly, term,
                          pow2, pi);

            // y_{k+1} = (1 - (1-y_k^4)^(1/4)) / (1 + (1-y_k^4)^(1/4))
            mpfr_mul(y4, y, y, MPFR_RNDN);
            mpfr_mul(y4, y4, y4, MPFR_RNDN);
            mpfr_ui_sub(root, 1ul, y4, MPFR_RNDN);
            mpfr_sqrt(root, root, MPFR_RNDN);
            mpfr_sqrt(root, root, MPFR_RNDN);

            mpfr_ui_sub(next_y, 1ul, root, MPFR_RNDN);
            mpfr_add_ui(term, root, 1ul, MPFR_RNDN);
            mpfr_div(next_y, next_y, term, MPFR_RNDN);

            // a_{k+1} = a_k(1+y)^4 - 2^(2k+3)y(1+y+y^2)
            mpfr_add_ui(one_plus_y, next_y, 1ul, MPFR_RNDN);
            mpfr_mul(term, one_plus_y, one_plus_y, MPFR_RNDN);
            mpfr_mul(term, term, term, MPFR_RNDN);
            mpfr_mul(a, a, term, MPFR_RNDN);

            mpfr_mul(y_poly, next_y, next_y, MPFR_RNDN);
            mpfr_add(y_poly, y_poly, next_y, MPFR_RNDN);
            mpfr_add_ui(y_poly, y_poly, 1ul, MPFR_RNDN);
            mpfr_mul(y_poly, y_poly, next_y, MPFR_RNDN);
            mpfr_set_ui_2exp(pow2, 1ul, 2ul * k + 3ul, MPFR_RNDN);
            mpfr_mul(y_poly, y_poly, pow2, MPFR_RNDN);
            mpfr_sub(a, a, y_poly, MPFR_RNDN);

            mpfr_set(y, next_y, MPFR_RNDN);
        }

        set_precision(static_cast<mpfr_prec_t>(precision_bits), y, a, sqrt2, y4, root, next_y,
                      one_plus_y, y_poly, term, pow2, pi);
        mpfr_ui_div(pi, 1ul, a, MPFR_RNDN);
        result.finalize_ms = finalize_timer.wall_ms();

        const Timer format_timer;
        result.decimal_prefix = mpfr_to_decimal_prefix(pi, decimal_digits);
        format_ms += format_timer.wall_ms();
        result.wall_ms = timer.wall_ms();
        result.cpu_ms = timer.cpu_ms();
        const Timer verify_timer;
        result.verified = decimal_prefix_matches_pi(result.decimal_prefix, decimal_digits,
                                                    effective_guard_digits);
        verify_ms += verify_timer.wall_ms();
        result.format_ms = format_ms;
        result.verify_ms = verify_ms;
        result.verification_method = "adaptive MPFR const_pi prefix";

        if (!result.verified) {
            set_precision(static_cast<mpfr_prec_t>(precision_bits), y, a, sqrt2, y4, root,
                          next_y, one_plus_y, y_poly, term, pow2, pi);
            mpfr_sqrt_ui(sqrt2, 2ul, MPFR_RNDN);
            mpfr_sub_ui(y, sqrt2, 1ul, MPFR_RNDN);
            mpfr_mul_ui(a, sqrt2, 4ul, MPFR_RNDN);
            mpfr_ui_sub(a, 6ul, a, MPFR_RNDN);

            for (unsigned int k = 0; k < iterations; ++k) {
                mpfr_mul(y4, y, y, MPFR_RNDN);
                mpfr_mul(y4, y4, y4, MPFR_RNDN);
                mpfr_ui_sub(root, 1ul, y4, MPFR_RNDN);
                mpfr_sqrt(root, root, MPFR_RNDN);
                mpfr_sqrt(root, root, MPFR_RNDN);

                mpfr_ui_sub(next_y, 1ul, root, MPFR_RNDN);
                mpfr_add_ui(term, root, 1ul, MPFR_RNDN);
                mpfr_div(next_y, next_y, term, MPFR_RNDN);

                mpfr_add_ui(one_plus_y, next_y, 1ul, MPFR_RNDN);
                mpfr_mul(term, one_plus_y, one_plus_y, MPFR_RNDN);
                mpfr_mul(term, term, term, MPFR_RNDN);
                mpfr_mul(a, a, term, MPFR_RNDN);

                mpfr_mul(y_poly, next_y, next_y, MPFR_RNDN);
                mpfr_add(y_poly, y_poly, next_y, MPFR_RNDN);
                mpfr_add_ui(y_poly, y_poly, 1ul, MPFR_RNDN);
                mpfr_mul(y_poly, y_poly, next_y, MPFR_RNDN);
                mpfr_set_ui_2exp(pow2, 1ul, 2ul * k + 3ul, MPFR_RNDN);
                mpfr_mul(y_poly, y_poly, pow2, MPFR_RNDN);
                mpfr_sub(a, a, y_poly, MPFR_RNDN);

                mpfr_set(y, next_y, MPFR_RNDN);
            }

            mpfr_ui_div(pi, 1ul, a, MPFR_RNDN);
            result.finalize_ms = finalize_timer.wall_ms();
            const Timer fallback_format_timer;
            result.decimal_prefix = mpfr_to_decimal_prefix(pi, decimal_digits);
            format_ms += fallback_format_timer.wall_ms();
            result.wall_ms = timer.wall_ms();
            result.cpu_ms = timer.cpu_ms();
            const Timer fallback_verify_timer;
            result.verified = decimal_prefix_matches_pi(result.decimal_prefix, decimal_digits,
                                                        effective_guard_digits);
            verify_ms += fallback_verify_timer.wall_ms();
            result.format_ms = format_ms;
            result.verify_ms = verify_ms;
            result.verification_method = "adaptive attempted; full-precision MPFR fallback";
        }

        mpfr_clears(y, a, sqrt2, y4, root, next_y, one_plus_y, y_poly, term, pow2, pi,
                    (mpfr_ptr)nullptr);
        return result;
    }
};

} // namespace

std::unique_ptr<PiAlgorithm> make_borwein_quartic_algorithm() {
    return std::make_unique<BorweinQuarticAlgorithm>();
}

} // namespace satox
