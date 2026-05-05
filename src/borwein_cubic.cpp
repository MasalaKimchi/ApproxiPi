#include "satox/algorithm.hpp"

#include "satox/format.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <mpfr.h>

#include <cmath>
#include <memory>

namespace satox {
namespace {

class BorweinCubicAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"borwein_cubic", "Borwein cubic convergence", 1, 1000000, true, false};
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

        mpfr_t a;
        mpfr_t s;
        mpfr_t r;
        mpfr_t tmp;
        mpfr_t tmp2;
        mpfr_t pow3;
        mpfr_t pi;
        mpfr_inits2(static_cast<mpfr_prec_t>(precision_bits), a, s, r, tmp, tmp2, pow3, pi,
                    (mpfr_ptr)nullptr);

        mpfr_set_ui(a, 1ul, MPFR_RNDN);
        mpfr_div_ui(a, a, 3ul, MPFR_RNDN);
        mpfr_sqrt_ui(s, 3ul, MPFR_RNDN);
        mpfr_sub_ui(s, s, 1ul, MPFR_RNDN);
        mpfr_div_ui(s, s, 2ul, MPFR_RNDN);
        mpfr_set_ui(pow3, 1ul, MPFR_RNDN);

        const unsigned int iterations =
            static_cast<unsigned int>(
                std::ceil(std::log(static_cast<double>(decimal_digits + effective_guard_digits)) /
                          std::log(3.0))) +
            6u;
        result.terms_or_iterations = iterations;
        result.estimated_digits_per_term =
            static_cast<double>(decimal_digits) / static_cast<double>(iterations);

        for (unsigned int k = 0; k < iterations; ++k) {
            mpfr_mul(tmp, s, s, MPFR_RNDN);
            mpfr_mul(tmp, tmp, s, MPFR_RNDN);
            mpfr_ui_sub(tmp, 1ul, tmp, MPFR_RNDN);
            mpfr_rootn_ui(tmp, tmp, 3ul, MPFR_RNDN);

            mpfr_mul_ui(tmp2, tmp, 2ul, MPFR_RNDN);
            mpfr_add_ui(tmp2, tmp2, 1ul, MPFR_RNDN);
            mpfr_ui_div(r, 3ul, tmp2, MPFR_RNDN);

            mpfr_sub_ui(s, r, 1ul, MPFR_RNDN);
            mpfr_div_ui(s, s, 2ul, MPFR_RNDN);

            mpfr_mul(tmp, r, r, MPFR_RNDN);
            mpfr_mul(a, a, tmp, MPFR_RNDN);
            mpfr_sub_ui(tmp2, tmp, 1ul, MPFR_RNDN);
            mpfr_mul(tmp2, tmp2, pow3, MPFR_RNDN);
            mpfr_sub(a, a, tmp2, MPFR_RNDN);
            mpfr_mul_ui(pow3, pow3, 3ul, MPFR_RNDN);
        }

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
        result.verification_method = "full-precision MPFR const_pi prefix";

        mpfr_clears(a, s, r, tmp, tmp2, pow3, pi, (mpfr_ptr)nullptr);
        return result;
    }
};

} // namespace

std::unique_ptr<PiAlgorithm> make_borwein_cubic_algorithm() {
    return std::make_unique<BorweinCubicAlgorithm>();
}

} // namespace satox
