#include "satox/algorithm.hpp"

#include "satox/binary_splitting.hpp"
#include "satox/checkpoint.hpp"
#include "satox/chudnovsky_common.hpp"
#include "satox/limits.hpp"
#include "satox/memory_estimate.hpp"
#include "satox/run_config.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <memory>

namespace satox {
namespace {

class ChudnovskyAlgorithm final : public PiAlgorithm {
  public:
    explicit ChudnovskyAlgorithm(bool leaf_pq_cancellation = false)
        : leaf_pq_cancellation_(leaf_pq_cancellation) {}

    AlgorithmMetadata metadata() const override {
        if (leaf_pq_cancellation_) {
            return {"chudnovsky_bs_valuation",
                    "Chudnovsky binary splitting with leaf valuation cancellation", 1,
                    kMaxBenchmarkDigits, true, false};
        }
        return {"chudnovsky_bs", "Chudnovsky binary splitting", 1, kMaxBenchmarkDigits, true,
                false};
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
        if (!memory_guard_allows(decimal_digits, result.metadata.name, &result.error)) {
            return result;
        }

        result.supported = true;
        result.notes = global_run_config().ablation_tag;
        const int effective_guard_digits = guard_digits + 64;
        const Timer timer;
        const unsigned long terms = chudnovsky_term_count(decimal_digits, effective_guard_digits);
        result.terms_or_iterations = terms;

        HypergeometricBsResult node;
        BinarySplittingStats bs_stats{};
        const unsigned int parallel_depth = recommended_parallel_depth(terms);
        const std::string spec_id = leaf_pq_cancellation_ ? "chudnovsky_bs_valuation" : "chudnovsky_bs";
        const HypergeometricBsSpec spec =
            make_chudnovsky_spec(spec_id, 8, leaf_pq_cancellation_);
        const Timer split_timer;
        binary_split_hypergeometric(spec, 0, terms, node, &bs_stats, parallel_depth);
        result.split_ms = split_timer.wall_ms();
        result.series_ms = bs_stats.series_ms;
        result.bigint_ms = bs_stats.bigint_ms;
        result.gcd_reductions = bs_stats.gcd_reductions;
        result.cancelled_bits = bs_stats.cancelled_bits;
        result.max_operand_bits = bs_stats.max_operand_bits;
        result.parallel_depth = bs_stats.parallel_depth;
        result.mul_count = bs_stats.mul_count;
        result.mul_bit_volume = bs_stats.mul_bit_volume;

        if (global_run_config().enable_checkpoint &&
            global_run_config().storage_backend != StorageBackend::Memory) {
            const Timer io_timer;
            CheckpointWriter writer("chudnovsky_bs_" + std::to_string(decimal_digits));
            writer.write_chunk(0, std::to_string(terms) + ":" + std::to_string(bs_stats.mul_count));
            result.io_ms = io_timer.wall_ms();
        }

        const bool streaming_format = decimal_digits >= 10000000;
        const ChudnovskyFinalizeResult fin = finalize_chudnovsky_pi(
            node, decimal_digits, effective_guard_digits, streaming_format);
        result.decimal_prefix = fin.decimal_prefix;
        result.finalize_ms = fin.finalize_ms;
        result.sqrt_div_ms = fin.sqrt_div_ms;
        result.format_ms = fin.format_ms;
        result.verify_ms = fin.verify_ms;
        result.verified = fin.verified;
        result.verification_method = fin.verification_method;
        const double elapsed_wall_ms = timer.wall_ms();
        result.wall_ms =
            elapsed_wall_ms > result.verify_ms ? elapsed_wall_ms - result.verify_ms : 0.0;
        result.cpu_ms = timer.cpu_ms();
        result.total_cost_ms = result.wall_ms + result.verify_ms + result.io_ms;
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
