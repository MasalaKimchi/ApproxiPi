#include "satox/algorithm.hpp"

#include "satox/bbp.hpp"
#include "satox/timer.hpp"

#include <cmath>
#include <memory>
#include <vector>

namespace satox {
namespace {

class BbpHexExtractAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"bbp_hex_extract", "baseline / BBP hex digit extraction", 1, 100000000, false,
                true};
    }

    ComputeResult compute(int decimal_digits, int guard_digits) const override {
        ComputeResult result;
        result.metadata = metadata();
        result.decimal_digits = decimal_digits;
        result.guard_digits = guard_digits;
        result.estimated_digits_per_term = 0.0;

        if (decimal_digits <= 0) {
            result.error = "decimal_digits must be positive";
            return result;
        }

        result.supported = true;
        const Timer timer;

        std::vector<int> offsets = {0, 10, 100, 1000};
        if (decimal_digits >= 10000) {
            offsets.push_back(10000);
        }
        if (decimal_digits >= 100000) {
            offsets.push_back(100000);
        }

        const Timer split_timer;
        std::string combined;
        for (int offset : offsets) {
            if (offset * 4 > decimal_digits) {
                break;
            }
            combined += bbp_hex_digits(offset, 8);
        }
        result.split_ms = split_timer.wall_ms();
        result.terms_or_iterations = offsets.size();
        result.decimal_prefix = "3." + combined;
        result.format_ms = 0.0;
        result.finalize_ms = 0.0;

        const Timer verify_timer;
        bool ok = true;
        for (const BbpCheck &check : default_bbp_checks()) {
            if (check.offset * 4 > decimal_digits) {
                continue;
            }
            if (bbp_hex_digits(check.offset, 8) != check.hex_digits) {
                ok = false;
                break;
            }
        }
        result.verified = ok;
        result.verify_ms = verify_timer.wall_ms();
        result.wall_ms = timer.wall_ms();
        result.cpu_ms = timer.cpu_ms();
        result.total_cost_ms = result.wall_ms + result.verify_ms;
        result.verification_method = "BBP hex self-consistency";
        return result;
    }
};

} // namespace

std::unique_ptr<PiAlgorithm> make_bbp_hex_extract_algorithm() {
    return std::make_unique<BbpHexExtractAlgorithm>();
}

} // namespace satox
