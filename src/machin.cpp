#include "satox/algorithm.hpp"

#include "satox/format.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <mpfr.h>

#include <cmath>
#include <memory>

namespace satox {
namespace {

constexpr double kMachinDigitsPerTerm = 1.3979400086720377;

class MachinAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"machin_arctan", "Machin arctangent formula", 1, 100000, true, false};
    }

    ComputeResult compute(int decimal_digits, int guard_digits) const override {
        ComputeResult result;
        result.metadata = metadata();
        result.decimal_digits = decimal_digits;
        result.guard_digits = guard_digits;
        result.estimated_digits_per_term = kMachinDigitsPerTerm;

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
        const int precision_bits = bits_for_decimal_digits(decimal_digits, effective_guard_digits);

        mpfr_t x;
        mpfr_t y;
        mpfr_t pi;
        mpfr_inits2(static_cast<mpfr_prec_t>(precision_bits), x, y, pi, (mpfr_ptr)nullptr);

        mpfr_set_ui(x, 5ul, MPFR_RNDN);
        mpfr_ui_div(x, 1ul, x, MPFR_RNDN);
        mpfr_atan(x, x, MPFR_RNDN);
        mpfr_mul_ui(x, x, 16ul, MPFR_RNDN);

        mpfr_set_ui(y, 239ul, MPFR_RNDN);
        mpfr_ui_div(y, 1ul, y, MPFR_RNDN);
        mpfr_atan(y, y, MPFR_RNDN);
        mpfr_mul_ui(y, y, 4ul, MPFR_RNDN);

        mpfr_sub(pi, x, y, MPFR_RNDN);
        result.finalize_ms = finalize_timer.wall_ms();

        result.terms_or_iterations =
            static_cast<std::uint64_t>(std::ceil((decimal_digits + effective_guard_digits) /
                                                 kMachinDigitsPerTerm));

        const Timer format_timer;
        result.decimal_prefix = mpfr_to_decimal_prefix(pi, decimal_digits);
        result.format_ms = format_timer.wall_ms();
        result.wall_ms = timer.wall_ms();
        result.cpu_ms = timer.cpu_ms();

        const Timer verify_timer;
        result.verified = decimal_prefix_matches_pi(result.decimal_prefix, decimal_digits,
                                                    effective_guard_digits);
        result.verify_ms = verify_timer.wall_ms();
        result.verification_method = "Machin 16 atan(1/5) - 4 atan(1/239)";

        mpfr_clears(x, y, pi, (mpfr_ptr)nullptr);
        return result;
    }
};

} // namespace

std::unique_ptr<PiAlgorithm> make_machin_algorithm() {
    return std::make_unique<MachinAlgorithm>();
}

} // namespace satox
