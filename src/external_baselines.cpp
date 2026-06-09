// External-library baselines, run through the exact same digit-request,
// formatting and verification pipeline as the in-repo algorithms. These are
// installable, citable references (MPFR, FLINT/Arb) that replace the
// unavailable y-cruncher as the "is this actually fast?" control.

#include "satox/algorithm.hpp"

#include "satox/format.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <mpfr.h>

#ifdef SATOX_HAVE_FLINT
#include <flint/arb.h>
#include <flint/flint.h>
#endif

#include <memory>
#include <thread>

namespace satox {
namespace {

class MpfrConstPiAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"mpfr_const_pi", "MPFR mpfr_const_pi (cold cache, single-threaded)", 1,
                1000000, true, false};
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
        const int precision_bits =
            bits_for_decimal_digits(decimal_digits, effective_guard_digits);

        // MPFR caches const_pi internally; clear it so each trial measures a
        // cold computation rather than a memcpy of the previous trial.
        mpfr_free_cache();

        const Timer timer;
        mpfr_t pi;
        mpfr_init2(pi, static_cast<mpfr_prec_t>(precision_bits));
        const Timer finalize_timer;
        mpfr_const_pi(pi, MPFR_RNDN);
        result.finalize_ms = finalize_timer.wall_ms();
        result.terms_or_iterations = 1;

        const Timer format_timer;
        result.decimal_prefix = mpfr_to_decimal_prefix(pi, decimal_digits);
        result.format_ms = format_timer.wall_ms();
        result.wall_ms = timer.wall_ms();
        result.cpu_ms = timer.cpu_ms();

        const Timer verify_timer;
        result.verified = decimal_prefix_matches_pi(result.decimal_prefix, decimal_digits,
                                                    effective_guard_digits);
        result.verify_ms = verify_timer.wall_ms();
        result.verification_method = "MPFR const_pi prefix (self-consistency; "
                                     "cross-checked via prefix hash against other rows)";
        mpfr_clear(pi);
        return result;
    }
};

#ifdef SATOX_HAVE_FLINT

class ArbConstPiAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"arb_const_pi", "FLINT/Arb arb_const_pi (cold cache)", 1, 1000000, true,
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

        const int effective_guard_digits = guard_digits + 128;
        const int precision_bits =
            bits_for_decimal_digits(decimal_digits, effective_guard_digits);

        // Give FLINT the same hardware budget as the in-repo parallel code
        // and clear its constant caches so every trial is cold.
        flint_set_num_threads(
            static_cast<int>(std::max(1u, std::thread::hardware_concurrency())));
        flint_cleanup();

        const Timer timer;
        arb_t pi;
        arb_init(pi);
        const Timer finalize_timer;
        arb_const_pi(pi, static_cast<slong>(precision_bits));
        result.finalize_ms = finalize_timer.wall_ms();
        result.terms_or_iterations = 1;

        mpfr_t pi_mpfr;
        mpfr_init2(pi_mpfr, static_cast<mpfr_prec_t>(precision_bits));
        arf_get_mpfr(pi_mpfr, arb_midref(pi), MPFR_RNDN);

        const Timer format_timer;
        result.decimal_prefix = mpfr_to_decimal_prefix(pi_mpfr, decimal_digits);
        result.format_ms = format_timer.wall_ms();
        result.wall_ms = timer.wall_ms();
        result.cpu_ms = timer.cpu_ms();

        const Timer verify_timer;
        result.verified = decimal_prefix_matches_pi(result.decimal_prefix, decimal_digits,
                                                    effective_guard_digits);
        result.verify_ms = verify_timer.wall_ms();
        result.verification_method = "MPFR const_pi prefix";

        mpfr_clear(pi_mpfr);
        arb_clear(pi);
        return result;
    }
};

#endif // SATOX_HAVE_FLINT

class ArbUnavailableAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"arb_const_pi", "FLINT/Arb arb_const_pi (not built)", 1, 1000000, true,
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
