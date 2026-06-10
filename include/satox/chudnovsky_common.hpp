#pragma once

#include "satox/binary_splitting.hpp"

#include <string>

namespace satox {

constexpr double kChudnovskyDigitsPerTerm = 14.181647462725477;

HypergeometricBsSpec make_chudnovsky_spec(const std::string &id,
                                          unsigned long leaf_block_terms = 8,
                                          bool leaf_pq_cancellation = false);

unsigned long chudnovsky_term_count(int decimal_digits, int effective_guard_digits);

struct ChudnovskyFinalizeResult {
    std::string decimal_prefix;
    double finalize_ms = 0.0;
    double sqrt_div_ms = 0.0;
    double format_ms = 0.0;
    double verify_ms = 0.0;
    bool verified = false;
    std::string verification_method;
};

ChudnovskyFinalizeResult finalize_chudnovsky_pi(const HypergeometricBsResult &node,
                                                int decimal_digits,
                                                int effective_guard_digits,
                                                bool streaming_format = false);

} // namespace satox
