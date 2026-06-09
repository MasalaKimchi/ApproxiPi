#include "satox/crown.hpp"

#include "satox/timer.hpp"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <fstream>
#include <future>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

namespace satox {
namespace {

constexpr mpfr_prec_t kCrownGuardBits = 64;
constexpr mpfr_prec_t kMinNodePrecision = 64;

struct CrownNode {
    mpfr_t p;
    mpfr_t q;
    mpfr_t t;
    bool has_p = false;
    bool initialized_q = false;
    bool initialized_t = false;

    CrownNode() = default;
    CrownNode(const CrownNode &) = delete;
    CrownNode &operator=(const CrownNode &) = delete;

    ~CrownNode() {
        if (has_p) {
            mpfr_clear(p);
        }
        if (initialized_q) {
            mpfr_clear(q);
        }
        if (initialized_t) {
            mpfr_clear(t);
        }
    }
};

unsigned int choose_crown_depth(unsigned long terms, const CrownTuning &tuning) {
    unsigned int depth = 0;
    while (depth < tuning.max_crown_depth &&
           (terms >> (depth + 1u)) >= std::max(1ul, tuning.min_chunk_terms)) {
        ++depth;
    }
    return depth;
}

mpfr_prec_t clamp_precision(mpfr_prec_t wanted) {
    return std::max(kMinNodePrecision, wanted);
}

// Precision actually required for any value belonging to the subtree whose
// first term index has contribution offset `shift_bits` below the leading
// chunk: bits beyond (target + guard - shift) cannot influence the final
// rounded answer.
mpfr_prec_t needed_precision(mpfr_prec_t target_bits, long shift_bits) {
    const long wanted = static_cast<long>(target_bits) + kCrownGuardBits - shift_bits;
    return clamp_precision(static_cast<mpfr_prec_t>(std::max(wanted, 1l)));
}

struct CrownContext {
    const HypergeometricBsSpec *spec = nullptr;
    mpfr_prec_t target_bits = 0;
    const std::function<bool(mpfr_srcptr, mpfr_srcptr, mpfr_srcptr, long)> *root_inputs_hook =
        nullptr;
    CrownTuning tuning;
    bool root_q_elided = false;
    unsigned long chunk_count = 0;
    std::vector<HypergeometricBsResult> chunks;
    // Conservative lower bound on the binary downshift of chunk i's
    // contribution relative to the full sum.
    std::vector<long> shift_lb;
    std::atomic<unsigned long long> crown_merges{0};
    std::atomic<unsigned long long> truncated_values{0};
    std::atomic<unsigned long long> skipped_p_products{0};
    std::atomic<unsigned long long> crown_mul_count{0};
    std::atomic<unsigned long long> crown_mul_bits{0};
};

void count_crown_mul(CrownContext &ctx, mpfr_srcptr a, mpfr_srcptr b) {
    ctx.crown_mul_count.fetch_add(1ull, std::memory_order_relaxed);
    ctx.crown_mul_bits.fetch_add(
        static_cast<unsigned long long>(mpfr_get_prec(a)) +
            static_cast<unsigned long long>(mpfr_get_prec(b)),
        std::memory_order_relaxed);
}

void note_truncation(CrownContext &ctx, mpfr_prec_t exact_bits, mpfr_prec_t used_bits) {
    if (used_bits < exact_bits) {
        ctx.truncated_values.fetch_add(1ull, std::memory_order_relaxed);
    }
}

// P(a, b) is only ever consumed in tail products P(a, b) * T(b, c), whose
// results are capped at the precision needed at offset b (the node's right
// edge). So P may be truncated to the right-edge precision, which is much
// smaller than the left-edge precision used for Q and T.
long p_shift_index(const CrownContext &ctx, unsigned long hi) {
    return ctx.shift_lb[std::min<size_t>(hi, ctx.shift_lb.size() - 1)];
}

void convert_chunk(CrownContext &ctx, unsigned long index, bool need_p, CrownNode &out) {
    const HypergeometricBsResult &chunk = ctx.chunks[index];
    const mpfr_prec_t needed = needed_precision(ctx.target_bits, ctx.shift_lb[index]);

    const mpfr_prec_t q_exact = static_cast<mpfr_prec_t>(mpz_sizeinbase(chunk.q, 2) + 2);
    const mpfr_prec_t q_prec = clamp_precision(std::min(q_exact, needed));
    mpfr_init2(out.q, q_prec);
    out.initialized_q = true;
    mpfr_set_z(out.q, chunk.q, MPFR_RNDN);
    note_truncation(ctx, q_exact, q_prec);

    const mpfr_prec_t t_exact = static_cast<mpfr_prec_t>(mpz_sizeinbase(chunk.t, 2) + 2);
    const mpfr_prec_t t_prec = clamp_precision(std::min(t_exact, needed));
    mpfr_init2(out.t, t_prec);
    out.initialized_t = true;
    mpfr_set_z(out.t, chunk.t, MPFR_RNDN);
    note_truncation(ctx, t_exact, t_prec);

    if (need_p) {
        const mpfr_prec_t needed_p =
            needed_precision(ctx.target_bits, p_shift_index(ctx, index + 1));
        const mpfr_prec_t p_exact = static_cast<mpfr_prec_t>(mpz_sizeinbase(chunk.p, 2) + 2);
        const mpfr_prec_t p_prec = clamp_precision(std::min(p_exact, needed_p));
        mpfr_init2(out.p, p_prec);
        out.has_p = true;
        mpfr_set_z(out.p, chunk.p, MPFR_RNDN);
        note_truncation(ctx, p_exact, p_prec);
    } else {
        ctx.skipped_p_products.fetch_add(1ull, std::memory_order_relaxed);
    }
}

void crown_merge(CrownContext &ctx, unsigned long lo, unsigned long hi, bool need_p,
                 unsigned int parallel_levels, CrownNode &out) {
    if (hi - lo == 1) {
        convert_chunk(ctx, lo, need_p, out);
        return;
    }

    const bool is_root = (lo == 0 && hi == ctx.chunk_count);
    // The root splits asymmetrically (default 9/16 left) so the leading T
    // product overlaps the final T beyond half the target precision (see
    // root_t_main_hook contract).
    const unsigned long root_num = std::min(15u, std::max(1u, ctx.tuning.root_split_sixteenths));
    const unsigned long mid =
        is_root ? std::min(hi - 1, lo + ((hi - lo) * root_num + 15ul) / 16ul)
                : lo + (hi - lo) / 2;
    CrownNode left;
    CrownNode right;
    if (parallel_levels > 0) {
        auto left_future = std::async(std::launch::async, [&]() {
            crown_merge(ctx, lo, mid, true, parallel_levels - 1, left);
        });
        crown_merge(ctx, mid, hi, need_p, parallel_levels - 1, right);
        left_future.get();
    } else {
        crown_merge(ctx, lo, mid, true, 0, left);
        crown_merge(ctx, mid, hi, need_p, 0, right);
    }

    const mpfr_prec_t needed_out = needed_precision(ctx.target_bits, ctx.shift_lb[lo]);
    const mpfr_prec_t needed_tail = needed_precision(ctx.target_bits, ctx.shift_lb[mid]);
    ctx.crown_merges.fetch_add(1ull, std::memory_order_relaxed);

    // Q = Ql * Qr, truncated to what the final answer can observe.
    const mpfr_prec_t q_exact =
        mpfr_get_prec(left.q) + mpfr_get_prec(right.q) + 2;
    const mpfr_prec_t q_prec = clamp_precision(std::min(q_exact, needed_out));
    mpfr_init2(out.q, q_prec);
    out.initialized_q = true;
    note_truncation(ctx, q_exact, q_prec);

    // T = Tl * Qr + Pl * Tr. The right-hand product only carries the tail
    // contribution, so it is computed at the tail's (smaller) precision.
    const mpfr_prec_t t_exact =
        mpfr_get_prec(left.t) + mpfr_get_prec(right.q) + 2;
    const mpfr_prec_t t_prec = clamp_precision(std::min(t_exact, needed_out));
    mpfr_init2(out.t, t_prec);
    out.initialized_t = true;
    note_truncation(ctx, t_exact, t_prec);

    const mpfr_prec_t tail_exact =
        mpfr_get_prec(left.p) + mpfr_get_prec(right.t) + 2;
    const mpfr_prec_t tail_prec = clamp_precision(std::min(tail_exact, needed_tail));
    note_truncation(ctx, tail_exact, tail_prec);
    mpfr_t tail;
    mpfr_init2(tail, tail_prec);

    // Rigorous lower bound on how many leading bits Tl * Qr shares with the
    // final T: the tail Pl * Tr enters that many bits further down.
    const long t_main_agreement =
        static_cast<long>(ctx.shift_lb[mid]) - static_cast<long>(ctx.shift_lb[lo]) - 2;
    bool skip_q = false;
    if (is_root && ctx.root_inputs_hook != nullptr && *ctx.root_inputs_hook &&
        t_main_agreement > 0) {
        skip_q = (*ctx.root_inputs_hook)(left.t, left.q, right.q, t_main_agreement);
        ctx.root_q_elided = skip_q;
    }
    if (skip_q) {
        mpfr_set_ui(out.q, 1ul, MPFR_RNDN);
    }

    if (!skip_q) {
        count_crown_mul(ctx, left.q, right.q);
    }
    count_crown_mul(ctx, left.t, right.q);
    count_crown_mul(ctx, left.p, right.t);
    if (q_prec >= ctx.tuning.intra_node_parallel_bits) {
        std::future<void> q_future;
        if (!skip_q) {
            q_future = std::async(std::launch::async, [&]() {
                mpfr_mul(out.q, left.q, right.q, MPFR_RNDN);
            });
        }
        auto tail_future = std::async(std::launch::async, [&]() {
            mpfr_mul(tail, left.p, right.t, MPFR_RNDN);
        });
        mpfr_mul(out.t, left.t, right.q, MPFR_RNDN);
        tail_future.get();
        mpfr_add(out.t, out.t, tail, MPFR_RNDN);
        if (q_future.valid()) {
            q_future.get();
        }
    } else {
        if (!skip_q) {
            mpfr_mul(out.q, left.q, right.q, MPFR_RNDN);
        }
        mpfr_mul(out.t, left.t, right.q, MPFR_RNDN);
        mpfr_mul(tail, left.p, right.t, MPFR_RNDN);
        mpfr_add(out.t, out.t, tail, MPFR_RNDN);
    }
    mpfr_clear(tail);

    if (need_p) {
        const mpfr_prec_t needed_p =
            needed_precision(ctx.target_bits, p_shift_index(ctx, hi));
        const mpfr_prec_t p_exact =
            mpfr_get_prec(left.p) + mpfr_get_prec(right.p) + 2;
        const mpfr_prec_t p_prec = clamp_precision(std::min(p_exact, needed_p));
        mpfr_init2(out.p, p_prec);
        out.has_p = true;
        count_crown_mul(ctx, left.p, right.p);
        mpfr_mul(out.p, left.p, right.p, MPFR_RNDN);
        note_truncation(ctx, p_exact, p_prec);
    } else {
        ctx.skipped_p_products.fetch_add(1ull, std::memory_order_relaxed);
    }
}

void compute_chunks_parallel(const HypergeometricBsSpec &spec,
                             const std::vector<std::pair<unsigned long, unsigned long>> &ranges,
                             std::vector<HypergeometricBsResult> &chunks,
                             BinarySplittingStats *stats) {
    const unsigned int workers = std::min<unsigned int>(
        std::max(1u, std::thread::hardware_concurrency()),
        static_cast<unsigned int>(ranges.size()));

    std::vector<BinarySplittingStats> chunk_stats(ranges.size());
    std::atomic<size_t> next{0};
    auto worker = [&]() {
        while (true) {
            const size_t index = next.fetch_add(1, std::memory_order_relaxed);
            if (index >= ranges.size()) {
                return;
            }
            binary_split_hypergeometric(spec, ranges[index].first, ranges[index].second,
                                        chunks[index], &chunk_stats[index], 0);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(workers > 0 ? workers - 1 : 0);
    for (unsigned int i = 1; i < workers; ++i) {
        threads.emplace_back(worker);
    }
    worker();
    for (std::thread &thread : threads) {
        thread.join();
    }

    if (stats != nullptr) {
        for (const BinarySplittingStats &cs : chunk_stats) {
            stats->gcd_reductions += cs.gcd_reductions;
            stats->cancelled_bits += cs.cancelled_bits;
            stats->max_operand_bits = std::max(stats->max_operand_bits, cs.max_operand_bits);
            stats->mul_count += cs.mul_count;
            stats->mul_bit_volume += cs.mul_bit_volume;
        }
    }
}

} // namespace

void binary_split_crown(const HypergeometricBsSpec &spec, unsigned long terms,
                        mpfr_t q_out, mpfr_t t_out, CrownStats *stats,
                        const std::function<bool(mpfr_srcptr, mpfr_srcptr, mpfr_srcptr, long)>
                            *root_inputs_hook,
                        const CrownTuning *tuning) {
    const CrownTuning active_tuning = (tuning != nullptr) ? *tuning : CrownTuning{};
    HypergeometricBsSpec tuned_spec = spec;
    tuned_spec.leaf_block_terms = active_tuning.leaf_block_terms;
    const unsigned int depth = choose_crown_depth(terms, active_tuning);

    if (depth == 0) {
        HypergeometricBsResult node;
        binary_split_hypergeometric(tuned_spec, 0, terms, node,
                                    stats != nullptr ? &stats->split : nullptr, 0);
        mpfr_set_z(q_out, node.q, MPFR_RNDN);
        mpfr_set_z(t_out, node.t, MPFR_RNDN);
        return;
    }

    // Chunk boundaries follow the same midpoint recursion as the exact tree.
    std::vector<std::pair<unsigned long, unsigned long>> ranges{{0ul, terms}};
    for (unsigned int level = 0; level < depth; ++level) {
        std::vector<std::pair<unsigned long, unsigned long>> split_ranges;
        split_ranges.reserve(ranges.size() * 2);
        for (const auto &range : ranges) {
            const unsigned long mid = (range.first + range.second) / 2ul;
            split_ranges.emplace_back(range.first, mid);
            split_ranges.emplace_back(mid, range.second);
        }
        ranges.swap(split_ranges);
    }

    CrownContext ctx;
    ctx.spec = &tuned_spec;
    ctx.target_bits = std::max(mpfr_get_prec(q_out), mpfr_get_prec(t_out));
    ctx.root_inputs_hook = root_inputs_hook;
    ctx.tuning = active_tuning;
    ctx.chunk_count = static_cast<unsigned long>(ranges.size());
    ctx.chunks = std::vector<HypergeometricBsResult>(ranges.size());

    const Timer chunk_timer;
    compute_chunks_parallel(tuned_spec, ranges, ctx.chunks,
                            stats != nullptr ? &stats->split : nullptr);
    const double chunk_ms = chunk_timer.wall_ms();

    // Rigorous lower bound on the downshift of chunk i's contribution:
    // log2|Q_j| >= sizeinbase(Q_j) - 1 and log2|P_j| <= sizeinbase(P_j), so
    // sum(size(Q_j) - size(P_j)) - i underestimates sum(log2|Q_j/P_j|).
    ctx.shift_lb.assign(ranges.size(), 0l);
    long running = 0;
    for (size_t i = 1; i < ranges.size(); ++i) {
        const long q_bits = static_cast<long>(mpz_sizeinbase(ctx.chunks[i - 1].q, 2));
        const long p_bits = static_cast<long>(mpz_sizeinbase(ctx.chunks[i - 1].p, 2));
        running += q_bits - p_bits - 1;
        ctx.shift_lb[i] = std::max(0l, running);
    }

    const unsigned int parallel_levels = std::min(depth, active_tuning.max_parallel_levels);
    const Timer merge_timer;
    CrownNode root;
    crown_merge(ctx, 0, static_cast<unsigned long>(ranges.size()), false, parallel_levels,
                root);

    mpfr_set(q_out, root.q, MPFR_RNDN);
    mpfr_set(t_out, root.t, MPFR_RNDN);

    if (stats != nullptr) {
        stats->crown_depth = depth;
        stats->split.parallel_depth = depth;
        stats->crown_merges = ctx.crown_merges.load();
        stats->truncated_values = ctx.truncated_values.load();
        stats->skipped_p_products = ctx.skipped_p_products.load();
        stats->chunk_ms = chunk_ms;
        stats->merge_ms = merge_timer.wall_ms();
        stats->crown_mul_count = ctx.crown_mul_count.load();
        stats->crown_mul_bit_volume = static_cast<double>(ctx.crown_mul_bits.load());
    }
}

namespace {

long parse_tuning_field(const std::string &text, const std::string &key, long fallback) {
    const std::string needle = "\"" + key + "\"";
    const size_t pos = text.find(needle);
    if (pos == std::string::npos) {
        return fallback;
    }
    const size_t colon = text.find(':', pos + needle.size());
    if (colon == std::string::npos) {
        return fallback;
    }
    return std::strtol(text.c_str() + colon + 1, nullptr, 10);
}

} // namespace

bool load_crown_tuning(const std::string &path, CrownTuning &tuning) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    const std::string text = buffer.str();
    if (text.find("min_chunk_terms") == std::string::npos) {
        return false;
    }
    CrownTuning defaults;
    tuning.min_chunk_terms = static_cast<unsigned long>(parse_tuning_field(
        text, "min_chunk_terms", static_cast<long>(defaults.min_chunk_terms)));
    tuning.max_crown_depth = static_cast<unsigned int>(parse_tuning_field(
        text, "max_crown_depth", static_cast<long>(defaults.max_crown_depth)));
    tuning.max_parallel_levels = static_cast<unsigned int>(parse_tuning_field(
        text, "max_parallel_levels", static_cast<long>(defaults.max_parallel_levels)));
    tuning.intra_node_parallel_bits = static_cast<mpfr_prec_t>(parse_tuning_field(
        text, "intra_node_parallel_bits",
        static_cast<long>(defaults.intra_node_parallel_bits)));
    tuning.root_split_sixteenths = static_cast<unsigned int>(parse_tuning_field(
        text, "root_split_sixteenths", static_cast<long>(defaults.root_split_sixteenths)));
    tuning.leaf_block_terms = static_cast<unsigned long>(parse_tuning_field(
        text, "leaf_block_terms", static_cast<long>(defaults.leaf_block_terms)));
    return true;
}

bool save_crown_tuning(const std::string &path, const CrownTuning &tuning) {
    std::ofstream out(path);
    if (!out) {
        return false;
    }
    out << "{\n"
        << "  \"min_chunk_terms\": " << tuning.min_chunk_terms << ",\n"
        << "  \"max_crown_depth\": " << tuning.max_crown_depth << ",\n"
        << "  \"max_parallel_levels\": " << tuning.max_parallel_levels << ",\n"
        << "  \"intra_node_parallel_bits\": " << tuning.intra_node_parallel_bits << ",\n"
        << "  \"root_split_sixteenths\": " << tuning.root_split_sixteenths << ",\n"
        << "  \"leaf_block_terms\": " << tuning.leaf_block_terms << "\n"
        << "}\n";
    return static_cast<bool>(out);
}

} // namespace satox
