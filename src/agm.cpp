#include "satox/algorithm.hpp"

#include "satox/format.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <mpfr.h>

#include <cmath>
#include <memory>

namespace satox {
namespace {

void set_precision(mpfr_prec_t precision, mpfr_t a, mpfr_t b, mpfr_t t, mpfr_t p,
                   mpfr_t next_a, mpfr_t next_b, mpfr_t diff, mpfr_t tmp, mpfr_t pi) {
    mpfr_prec_round(a, precision, MPFR_RNDN);
    mpfr_prec_round(b, precision, MPFR_RNDN);
    mpfr_prec_round(t, precision, MPFR_RNDN);
    mpfr_prec_round(p, precision, MPFR_RNDN);
    mpfr_prec_round(next_a, precision, MPFR_RNDN);
    mpfr_prec_round(next_b, precision, MPFR_RNDN);
    mpfr_prec_round(diff, precision, MPFR_RNDN);
    mpfr_prec_round(tmp, precision, MPFR_RNDN);
    mpfr_prec_round(pi, precision, MPFR_RNDN);
}

class AgmAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"gauss_legendre_agm", "Gauss-Legendre AGM", 1, 100000, true, false};
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

        const int precision_bits = bits_for_decimal_digits(decimal_digits, effective_guard_digits);
        mpfr_t a;
        mpfr_t b;
        mpfr_t t;
        mpfr_t p;
        mpfr_t next_a;
        mpfr_t next_b;
        mpfr_t diff;
        mpfr_t tmp;
        mpfr_t pi;
        mpfr_inits2(static_cast<mpfr_prec_t>(precision_bits), a, b, t, p, next_a, next_b,
                    diff, tmp, pi, (mpfr_ptr)nullptr);

        mpfr_set_ui(a, 1ul, MPFR_RNDN);
        mpfr_sqrt_ui(b, 2ul, MPFR_RNDN);
        mpfr_ui_div(b, 1ul, b, MPFR_RNDN);
        mpfr_set_ui(t, 1ul, MPFR_RNDN);
        mpfr_div_ui(t, t, 4ul, MPFR_RNDN);
        mpfr_set_ui(p, 1ul, MPFR_RNDN);

        const unsigned int iterations =
            static_cast<unsigned int>(
                std::ceil(std::log2(decimal_digits + effective_guard_digits + 2))) +
            2u;
        result.terms_or_iterations = iterations;
        result.estimated_digits_per_term =
            static_cast<double>(decimal_digits) / static_cast<double>(iterations);

        for (unsigned int i = 0; i < iterations; ++i) {
            const unsigned int remaining = iterations - i - 1u;
            const double divisor = remaining > 2u ? std::ldexp(1.0, static_cast<int>(remaining - 2u))
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
            set_precision(step_bits, a, b, t, p, next_a, next_b, diff, tmp, pi);

            mpfr_add(next_a, a, b, MPFR_RNDN);
            mpfr_div_ui(next_a, next_a, 2ul, MPFR_RNDN);

            mpfr_mul(next_b, a, b, MPFR_RNDN);
            mpfr_sqrt(next_b, next_b, MPFR_RNDN);

            mpfr_sub(diff, a, next_a, MPFR_RNDN);
            mpfr_mul(diff, diff, diff, MPFR_RNDN);
            mpfr_mul(tmp, p, diff, MPFR_RNDN);
            mpfr_sub(t, t, tmp, MPFR_RNDN);
            mpfr_mul_ui(p, p, 2ul, MPFR_RNDN);

            mpfr_set(a, next_a, MPFR_RNDN);
            mpfr_set(b, next_b, MPFR_RNDN);
        }

        set_precision(static_cast<mpfr_prec_t>(precision_bits), a, b, t, p, next_a, next_b,
                      diff, tmp, pi);
        mpfr_add(pi, a, b, MPFR_RNDN);
        mpfr_mul(pi, pi, pi, MPFR_RNDN);
        mpfr_mul_ui(tmp, t, 4ul, MPFR_RNDN);
        mpfr_div(pi, pi, tmp, MPFR_RNDN);

        result.decimal_prefix = mpfr_to_decimal_prefix(pi, decimal_digits);
        result.wall_ms = timer.wall_ms();
        result.cpu_ms = timer.cpu_ms();
        result.verified = decimal_prefix_matches_pi(result.decimal_prefix, decimal_digits,
                                                    effective_guard_digits);
        result.verification_method = "adaptive MPFR const_pi prefix";

        if (!result.verified) {
            set_precision(static_cast<mpfr_prec_t>(precision_bits), a, b, t, p, next_a, next_b,
                          diff, tmp, pi);
            mpfr_set_ui(a, 1ul, MPFR_RNDN);
            mpfr_sqrt_ui(b, 2ul, MPFR_RNDN);
            mpfr_ui_div(b, 1ul, b, MPFR_RNDN);
            mpfr_set_ui(t, 1ul, MPFR_RNDN);
            mpfr_div_ui(t, t, 4ul, MPFR_RNDN);
            mpfr_set_ui(p, 1ul, MPFR_RNDN);

            for (unsigned int i = 0; i < iterations; ++i) {
                mpfr_add(next_a, a, b, MPFR_RNDN);
                mpfr_div_ui(next_a, next_a, 2ul, MPFR_RNDN);

                mpfr_mul(next_b, a, b, MPFR_RNDN);
                mpfr_sqrt(next_b, next_b, MPFR_RNDN);

                mpfr_sub(diff, a, next_a, MPFR_RNDN);
                mpfr_mul(diff, diff, diff, MPFR_RNDN);
                mpfr_mul(tmp, p, diff, MPFR_RNDN);
                mpfr_sub(t, t, tmp, MPFR_RNDN);
                mpfr_mul_ui(p, p, 2ul, MPFR_RNDN);

                mpfr_set(a, next_a, MPFR_RNDN);
                mpfr_set(b, next_b, MPFR_RNDN);
            }

            mpfr_add(pi, a, b, MPFR_RNDN);
            mpfr_mul(pi, pi, pi, MPFR_RNDN);
            mpfr_mul_ui(tmp, t, 4ul, MPFR_RNDN);
            mpfr_div(pi, pi, tmp, MPFR_RNDN);
            result.decimal_prefix = mpfr_to_decimal_prefix(pi, decimal_digits);
            result.wall_ms = timer.wall_ms();
            result.cpu_ms = timer.cpu_ms();
            result.verified = decimal_prefix_matches_pi(result.decimal_prefix, decimal_digits,
                                                        effective_guard_digits);
            result.verification_method = "adaptive attempted; full-precision MPFR fallback";
        }

        mpfr_clears(a, b, t, p, next_a, next_b, diff, tmp, pi, (mpfr_ptr)nullptr);
        return result;
    }
};

} // namespace

std::unique_ptr<PiAlgorithm> make_agm_algorithm() {
    return std::make_unique<AgmAlgorithm>();
}

} // namespace satox
