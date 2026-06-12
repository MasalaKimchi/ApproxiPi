#include "satox/algorithm.hpp"

#include "satox/binary_splitting.hpp"
#include "satox/chudnovsky_common.hpp"
#include "satox/limits.hpp"
#include "satox/memory_estimate.hpp"
#include "satox/run_config.hpp"
#include "satox/timer.hpp"
#include "satox/verification.hpp"

#include <cmath>
#include <memory>

namespace satox {
namespace {

class ChudnovskyNaiveAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"chudnovsky_naive", "baseline / naive Chudnovsky summation", 1, kNaiveMaxDigits,
                true, false};
    }

    ComputeResult compute(int decimal_digits, int guard_digits) const override {
        ComputeResult result;
        result.metadata = metadata();
        result.decimal_digits = decimal_digits;
        result.guard_digits = guard_digits;
        result.estimated_digits_per_term = kChudnovskyDigitsPerTerm;
        result.notes = global_run_config().ablation_tag;

        if (!valid_digit_request(decimal_digits, guard_digits, &result.error)) {
            return result;
        }
        if (decimal_digits > result.metadata.max_digits) {
            result.error =
                "naive summation capped at " + std::to_string(kNaiveMaxDigits) + " digits";
            return result;
        }
        if (!memory_guard_allows(decimal_digits, result.metadata.name, &result.error)) {
            return result;
        }

        result.supported = true;
        const int effective_guard_digits = guard_digits + 64;
        const Timer timer;
        const unsigned long terms = chudnovsky_term_count(decimal_digits, effective_guard_digits);
        result.terms_or_iterations = terms;

        HypergeometricBsResult node;
        BinarySplittingStats bs_stats{};
        const HypergeometricBsSpec spec = make_chudnovsky_spec("chudnovsky_naive", 1);
        const Timer split_timer;
        blocked_leaf_hypergeometric(spec, 0, terms, node, &bs_stats);
        result.split_ms = split_timer.wall_ms();
        result.series_ms = bs_stats.series_ms;
        result.bigint_ms = bs_stats.bigint_ms;
        result.gcd_reductions = bs_stats.gcd_reductions;
        result.cancelled_bits = bs_stats.cancelled_bits;
        result.max_operand_bits = bs_stats.max_operand_bits;
        result.mul_count = bs_stats.mul_count;
        result.mul_bit_volume = bs_stats.mul_bit_volume;

        const ChudnovskyFinalizeResult fin =
            finalize_chudnovsky_pi(node, decimal_digits, effective_guard_digits);
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
};

class ChudnovskyRecurrenceAlgorithm final : public PiAlgorithm {
  public:
    AlgorithmMetadata metadata() const override {
        return {"chudnovsky_recurrence", "baseline / blocked recurrence summation", 1,
                kRecurrenceMaxDigits, true, false};
    }

    ComputeResult compute(int decimal_digits, int guard_digits) const override {
        ComputeResult result;
        result.metadata = metadata();
        result.decimal_digits = decimal_digits;
        result.guard_digits = guard_digits;
        result.estimated_digits_per_term = kChudnovskyDigitsPerTerm;
        result.notes = global_run_config().ablation_tag;

        if (!valid_digit_request(decimal_digits, guard_digits, &result.error)) {
            return result;
        }
        if (decimal_digits > result.metadata.max_digits) {
            result.error = "recurrence summation capped at " + std::to_string(kRecurrenceMaxDigits) +
                           " digits";
            return result;
        }
        if (!memory_guard_allows(decimal_digits, result.metadata.name, &result.error)) {
            return result;
        }

        result.supported = true;
        const int effective_guard_digits = guard_digits + 64;
        const Timer timer;
        const unsigned long terms = chudnovsky_term_count(decimal_digits, effective_guard_digits);
        result.terms_or_iterations = terms;

        HypergeometricBsSpec spec = make_chudnovsky_spec("chudnovsky_recurrence", 8);
        if (global_run_config().leaf_block_override > 0) {
            spec.leaf_block_terms =
                static_cast<unsigned long>(global_run_config().leaf_block_override);
        }

        HypergeometricBsResult node;
        BinarySplittingStats bs_stats{};
        const Timer split_timer;
        blocked_leaf_hypergeometric(spec, 0, terms, node, &bs_stats);
        result.split_ms = split_timer.wall_ms();
        result.series_ms = bs_stats.series_ms;
        result.bigint_ms = bs_stats.bigint_ms;
        result.gcd_reductions = bs_stats.gcd_reductions;
        result.cancelled_bits = bs_stats.cancelled_bits;
        result.max_operand_bits = bs_stats.max_operand_bits;
        result.mul_count = bs_stats.mul_count;
        result.mul_bit_volume = bs_stats.mul_bit_volume;

        const ChudnovskyFinalizeResult fin =
            finalize_chudnovsky_pi(node, decimal_digits, effective_guard_digits);
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
};

} // namespace

std::unique_ptr<PiAlgorithm> make_chudnovsky_naive_algorithm() {
    return std::make_unique<ChudnovskyNaiveAlgorithm>();
}

std::unique_ptr<PiAlgorithm> make_chudnovsky_recurrence_algorithm() {
    return std::make_unique<ChudnovskyRecurrenceAlgorithm>();
}

} // namespace satox
