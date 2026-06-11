// H14: scale-aware router — crown below the arb crossover, FLINT/Arb at and above.

#include "satox/algorithm.hpp"

#include <memory>
#include <string>

namespace satox {

// Empirical crossover from results/summary.md and results/trials.csv: crown wins
// at 10^5–10^7 wall time; arb_const_pi wins at 10^8 on this machine.
constexpr int kHybridArbCrossoverDigits = 100'000'000;

const char *hybrid_delegate_name(int decimal_digits) {
#ifdef SATOX_HAVE_FLINT
    if (decimal_digits >= kHybridArbCrossoverDigits) {
        return "arb_const_pi";
    }
#endif
    return "chudnovsky_bs_crown";
}

namespace {

class ChudnovskyHybridAlgorithm final : public PiAlgorithm {
  public:
    ChudnovskyHybridAlgorithm()
        : crown_(make_chudnovsky_crown_algorithm()),
          arb_(make_arb_const_pi_algorithm()) {}

    AlgorithmMetadata metadata() const override {
        AlgorithmMetadata meta;
        meta.name = "chudnovsky_hybrid";
        meta.family =
            "Hybrid router: chudnovsky_bs_crown below "
            + std::to_string(kHybridArbCrossoverDigits) +
            " digits, arb_const_pi at and above (when FLINT built)";
        meta.min_digits = 1;
        meta.max_digits = crown_->metadata().max_digits;
        meta.full_prefix = true;
        meta.verification_only = false;
        return meta;
    }

    ComputeResult compute(int decimal_digits, int guard_digits) const override {
        const char *delegate = hybrid_delegate_name(decimal_digits);
        ComputeResult result;
        if (std::string(delegate) == "arb_const_pi") {
            result = arb_->compute(decimal_digits, guard_digits);
        } else {
            result = crown_->compute(decimal_digits, guard_digits);
        }
        result.metadata = metadata();
        result.notes = std::string("delegate=") + delegate;
        return result;
    }

  private:
    std::unique_ptr<PiAlgorithm> crown_;
    std::unique_ptr<PiAlgorithm> arb_;
};

} // namespace

std::unique_ptr<PiAlgorithm> make_chudnovsky_hybrid_algorithm() {
    return std::make_unique<ChudnovskyHybridAlgorithm>();
}

} // namespace satox
