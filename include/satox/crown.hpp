#pragma once

#include "satox/binary_splitting.hpp"

#include <mpfr.h>

#include <functional>

namespace satox {

struct CrownStats {
    BinarySplittingStats split;
    unsigned int crown_depth = 0;
    unsigned long long crown_merges = 0;
    unsigned long long truncated_values = 0;
    unsigned long long skipped_p_products = 0;
    double chunk_ms = 0.0;
    double merge_ms = 0.0;
    // MPFR crown-merge multiplications, in the same metric as the exact
    // kernel: each product contributes prec(a) + prec(b) bits.
    unsigned long long crown_mul_count = 0;
    double crown_mul_bit_volume = 0.0;
};

// Performance-tuning knobs for the crown kernel. Defaults reproduce the
// hand-chosen H1-H9 configuration; the H11 autotuner searches this space and
// caches the per-machine winner (results/tuning.json).
struct CrownTuning {
    unsigned long min_chunk_terms = 256;   // smallest exact chunk
    unsigned int max_crown_depth = 7;      // 2^depth chunks
    unsigned int max_parallel_levels = 5;  // crown-merge fan-out levels
    mpfr_prec_t intra_node_parallel_bits = 1l << 19; // threshold for 3-way node
    unsigned int root_split_sixteenths = 9; // root left subtree weight (n/16)
    unsigned long leaf_block_terms = 8;    // iterative leaf block in chunks
};

bool load_crown_tuning(const std::string &path, CrownTuning &tuning);
bool save_crown_tuning(const std::string &path, const CrownTuning &tuning);

// Truncated-crown binary splitting (TCBS).
//
// Evaluates the hypergeometric sum for `spec` over [0, terms) and writes the
// root Q and T rounded to the precision of `q_out`/`t_out`. The bottom of the
// tree is the existing exact integer binary splitting, evaluated as 2^k
// independent chunks. The top k levels (the "crown") are merged in MPFR
// fixed-point arithmetic instead of exact integers:
//
//   - values are truncated to the precision actually needed for the final
//     answer, so the root merge uses short products instead of full ones;
//   - P products are skipped on the rightmost spine where they are provably
//     never consumed (including the root, whose P is dead in sum form);
//   - the per-node precision budget is reduced by each subtree's contribution
//     offset, derived rigorously from the exact bit sizes of the chunk P/Q
//     values (right-hand subtrees only influence low-order digits).
//
// Rounding error is bounded by a few ulps per merge at >= 64 guard bits below
// target precision, far inside the caller's decimal guard band. Callers must
// still run exact-prefix verification; this kernel makes no unverified claim.
// `root_inputs_hook`, when provided, is invoked on the merge thread as soon
// as both root children are merged and before the root products start. It
// receives the left child's T and Q, the right child's Q, and a rigorous
// lower bound on the number of leading bits in which Tl * Qr agrees with the
// final T (the root tail's downshift). The root split is intentionally
// asymmetric (9/16 of the terms on the left) so this overlap exceeds half
// the target precision with a wide margin. This lets callers:
//
//   - warm-start a Newton reciprocal 1/T ~= (1/Tl) * (1/Qr) concurrently
//     with the entire root merge, finishing with one half-width correction;
//   - assemble the full numerator (constant * Ql * Qr) on their own threads,
//     since pi never needs the root Q as a single value.
//
// If the hook returns true, the caller takes ownership of the numerator and
// the kernel skips the root Q product entirely; `q_out` is then left
// untouched. The hook must copy what it needs and return quickly.
void binary_split_crown(
    const HypergeometricBsSpec &spec, unsigned long terms, mpfr_t q_out, mpfr_t t_out,
    CrownStats *stats,
    const std::function<bool(mpfr_srcptr, mpfr_srcptr, mpfr_srcptr, long)> *root_inputs_hook =
        nullptr,
    const CrownTuning *tuning = nullptr);

} // namespace satox
