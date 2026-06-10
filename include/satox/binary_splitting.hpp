#pragma once

#include "satox/run_config.hpp"

#include <gmp.h>

#include <string>
#include <vector>

namespace satox {

struct LinearFactor {
    long slope;
    long intercept;
};

struct HypergeometricBsSpec {
    std::string id;
    std::vector<LinearFactor> p_factors;
    std::vector<LinearFactor> q_factors;
    unsigned long q_constant = 1;
    long linear_a = 0;
    long linear_b = 0;
    bool alternating = false;
    bool unit_first_p = false;
    bool unit_first_q = false;
    bool leaf_t_uses_q = false;
    bool gcd_cancellation = false;
    bool leaf_pq_cancellation = false;
    // Terms accumulated iteratively before the recursion takes over.
    unsigned long leaf_block_terms = 8;
};

struct HypergeometricBsResult {
    mpz_t p;
    mpz_t q;
    mpz_t t;

    HypergeometricBsResult();
    ~HypergeometricBsResult();

    HypergeometricBsResult(const HypergeometricBsResult &) = delete;
    HypergeometricBsResult &operator=(const HypergeometricBsResult &) = delete;
};

struct BinarySplittingStats {
    unsigned long long gcd_reductions = 0;
    double cancelled_bits = 0.0;
    unsigned long long max_operand_bits = 0;
    unsigned int parallel_depth = 0;
    unsigned long long mul_count = 0;
    double mul_bit_volume = 0.0;
    double series_ms = 0.0;
    double bigint_ms = 0.0;
};

void binary_split_hypergeometric(const HypergeometricBsSpec &spec, unsigned long a,
                                 unsigned long b, HypergeometricBsResult &out,
                                 BinarySplittingStats *stats);

void binary_split_hypergeometric(const HypergeometricBsSpec &spec, unsigned long a,
                                 unsigned long b, HypergeometricBsResult &out,
                                 BinarySplittingStats *stats, unsigned int parallel_depth);

void binary_split_hypergeometric(const HypergeometricBsSpec &spec, unsigned long a,
                                 unsigned long b, HypergeometricBsResult &out);

void blocked_leaf_hypergeometric(const HypergeometricBsSpec &spec, unsigned long a,
                                 unsigned long b, HypergeometricBsResult &out,
                                 BinarySplittingStats *stats);

unsigned int recommended_parallel_depth(unsigned long terms);

} // namespace satox
