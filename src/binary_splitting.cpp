#include "satox/binary_splitting.hpp"

#include <algorithm>
#include <future>
#include <stdexcept>
#include <thread>

namespace satox {
namespace {

constexpr unsigned long kLeafBlockTerms = 8;
constexpr unsigned long kMinTermsPerParallelTask = 4096;
constexpr unsigned int kMaxParallelDepth = 4;

void set_linear_product(mpz_t out, const std::vector<LinearFactor> &factors, unsigned long n) {
    mpz_set_ui(out, 1ul);
    for (const LinearFactor &factor : factors) {
        const long value = factor.slope * static_cast<long>(n) + factor.intercept;
        if (value < 0) {
            mpz_mul_si(out, out, value);
        } else {
            mpz_mul_ui(out, out, static_cast<unsigned long>(value));
        }
    }
}

long linear_value(long a, long b, unsigned long n) {
    return a * static_cast<long>(n) + b;
}

void reduce_common_pqt(HypergeometricBsResult &node, BinarySplittingStats *stats) {
    mpz_t g;
    mpz_t abs_t;
    mpz_init(g);
    mpz_init(abs_t);

    mpz_gcd(g, node.p, node.q);
    if (mpz_cmp_ui(g, 1ul) > 0) {
        mpz_abs(abs_t, node.t);
        mpz_gcd(g, g, abs_t);
    }

    if (mpz_cmp_ui(g, 1ul) > 0) {
        if (stats != nullptr) {
            ++stats->gcd_reductions;
            stats->cancelled_bits += static_cast<double>(mpz_sizeinbase(g, 2));
        }
        mpz_divexact(node.p, node.p, g);
        mpz_divexact(node.q, node.q, g);
        mpz_divexact(node.t, node.t, g);
    }

    mpz_clear(abs_t);
    mpz_clear(g);
}

void update_max_operand_bits(const HypergeometricBsResult &node, BinarySplittingStats *stats) {
    if (stats == nullptr) {
        return;
    }
    const unsigned long long p_bits = static_cast<unsigned long long>(mpz_sizeinbase(node.p, 2));
    const unsigned long long q_bits = static_cast<unsigned long long>(mpz_sizeinbase(node.q, 2));
    const unsigned long long t_bits = static_cast<unsigned long long>(mpz_sizeinbase(node.t, 2));
    stats->max_operand_bits = std::max(stats->max_operand_bits, std::max({p_bits, q_bits, t_bits}));
}

void merge_stats(BinarySplittingStats *target, const BinarySplittingStats &source) {
    if (target == nullptr) {
        return;
    }
    target->gcd_reductions += source.gcd_reductions;
    target->cancelled_bits += source.cancelled_bits;
    target->max_operand_bits = std::max(target->max_operand_bits, source.max_operand_bits);
    target->parallel_depth = std::max(target->parallel_depth, source.parallel_depth);
}

void set_leaf(const HypergeometricBsSpec &spec, unsigned long n, HypergeometricBsResult &out,
              BinarySplittingStats *stats) {
    if (n == 0 && spec.unit_first_p) {
        mpz_set_ui(out.p, 1ul);
    } else {
        set_linear_product(out.p, spec.p_factors, n);
    }

    if (n == 0 && spec.unit_first_q) {
        mpz_set_ui(out.q, 1ul);
    } else {
        set_linear_product(out.q, spec.q_factors, n);
    }
    if (!(n == 0 && spec.unit_first_q) && spec.q_constant != 1ul) {
        mpz_mul_ui(out.q, out.q, spec.q_constant);
    }

    mpz_set(out.t, spec.leaf_t_uses_q ? out.q : out.p);
    const long lin = linear_value(spec.linear_a, spec.linear_b, n);
    if (lin < 0) {
        mpz_mul_si(out.t, out.t, lin);
    } else {
        mpz_mul_ui(out.t, out.t, static_cast<unsigned long>(lin));
    }
    if (spec.alternating && (n & 1ul) != 0) {
        mpz_neg(out.t, out.t);
    }
    if (spec.gcd_cancellation) {
        reduce_common_pqt(out, stats);
    }
    update_max_operand_bits(out, stats);
}

void combine_nodes(const HypergeometricBsSpec &spec, const HypergeometricBsResult &left,
                   const HypergeometricBsResult &right, HypergeometricBsResult &out,
                   BinarySplittingStats *stats) {
    mpz_mul(out.p, left.p, right.p);
    mpz_mul(out.q, left.q, right.q);
    mpz_mul(out.t, left.t, right.q);
    mpz_addmul(out.t, left.p, right.t);
    if (spec.gcd_cancellation) {
        reduce_common_pqt(out, stats);
    }
    update_max_operand_bits(out, stats);
}

void append_leaf(const HypergeometricBsSpec &spec, HypergeometricBsResult &out,
                 const HypergeometricBsResult &leaf, BinarySplittingStats *stats) {
    mpz_mul(out.t, out.t, leaf.q);
    mpz_addmul(out.t, out.p, leaf.t);
    mpz_mul(out.p, out.p, leaf.p);
    mpz_mul(out.q, out.q, leaf.q);
    if (spec.gcd_cancellation) {
        reduce_common_pqt(out, stats);
    }
    update_max_operand_bits(out, stats);
}

void binary_split_hypergeometric_impl(const HypergeometricBsSpec &spec, unsigned long a,
                                      unsigned long b, HypergeometricBsResult &out,
                                      BinarySplittingStats *stats,
                                      unsigned int parallel_depth) {
    if (b <= a) {
        throw std::invalid_argument("binary_split_hypergeometric requires b > a");
    }

    if (b - a <= kLeafBlockTerms) {
        set_leaf(spec, a, out, stats);
        if (b - a > 1) {
            HypergeometricBsResult leaf;
            for (unsigned long n = a + 1; n < b; ++n) {
                set_leaf(spec, n, leaf, stats);
                append_leaf(spec, out, leaf, stats);
            }
        }
        return;
    }

    const unsigned long m = (a + b) / 2ul;
    HypergeometricBsResult left;
    HypergeometricBsResult right;
    if (parallel_depth > 0 && b - a >= kMinTermsPerParallelTask * 2ul) {
        BinarySplittingStats left_stats;
        BinarySplittingStats right_stats;
        left_stats.parallel_depth = parallel_depth;
        right_stats.parallel_depth = parallel_depth;

        auto left_future = std::async(std::launch::async, [&spec, a, m, &left, &left_stats,
                                                           parallel_depth]() {
            binary_split_hypergeometric_impl(spec, a, m, left, &left_stats, parallel_depth - 1);
        });
        binary_split_hypergeometric_impl(spec, m, b, right, &right_stats, parallel_depth - 1);
        left_future.get();
        merge_stats(stats, left_stats);
        merge_stats(stats, right_stats);
    } else {
        binary_split_hypergeometric_impl(spec, a, m, left, stats, 0);
        binary_split_hypergeometric_impl(spec, m, b, right, stats, 0);
    }
    combine_nodes(spec, left, right, out, stats);
}

} // namespace

HypergeometricBsResult::HypergeometricBsResult() {
    mpz_init(p);
    mpz_init(q);
    mpz_init(t);
}

HypergeometricBsResult::~HypergeometricBsResult() {
    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(t);
}

void binary_split_hypergeometric(const HypergeometricBsSpec &spec, unsigned long a,
                                 unsigned long b, HypergeometricBsResult &out,
                                 BinarySplittingStats *stats) {
    binary_split_hypergeometric(spec, a, b, out, stats, 0);
}

void binary_split_hypergeometric(const HypergeometricBsSpec &spec, unsigned long a,
                                 unsigned long b, HypergeometricBsResult &out,
                                 BinarySplittingStats *stats, unsigned int parallel_depth) {
    if (stats != nullptr) {
        stats->parallel_depth = parallel_depth;
    }
    binary_split_hypergeometric_impl(spec, a, b, out, stats, parallel_depth);
}

void binary_split_hypergeometric(const HypergeometricBsSpec &spec, unsigned long a,
                                 unsigned long b, HypergeometricBsResult &out) {
    binary_split_hypergeometric(spec, a, b, out, nullptr);
}

unsigned int recommended_parallel_depth(unsigned long terms) {
    const unsigned int workers = std::max(1u, std::thread::hardware_concurrency());
    unsigned int depth = 0;
    while (depth < kMaxParallelDepth && (1u << depth) < workers &&
           terms / (1ul << (depth + 1u)) >= kMinTermsPerParallelTask) {
        ++depth;
    }
    return depth;
}

} // namespace satox
