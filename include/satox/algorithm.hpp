#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace satox {

struct AlgorithmMetadata {
    std::string name;
    std::string family;
    int min_digits;
    int max_digits;
    bool full_prefix;
    bool verification_only;
};

struct ComputeResult {
    AlgorithmMetadata metadata;
    int decimal_digits = 0;
    int guard_digits = 0;
    bool supported = false;
    bool verified = false;
    std::string verification_method;
    std::string decimal_prefix;
    std::string error;
    std::uint64_t terms_or_iterations = 0;
    double estimated_digits_per_term = 0.0;
    double wall_ms = 0.0;
    double cpu_ms = 0.0;
    std::uint64_t gcd_reductions = 0;
    double cancelled_bits = 0.0;
};

class PiAlgorithm {
  public:
    virtual ~PiAlgorithm() = default;
    virtual AlgorithmMetadata metadata() const = 0;
    virtual ComputeResult compute(int decimal_digits, int guard_digits) const = 0;
};

std::vector<std::unique_ptr<PiAlgorithm>> make_default_algorithms();

std::unique_ptr<PiAlgorithm> make_chudnovsky_algorithm();
std::unique_ptr<PiAlgorithm> make_ramanujan_algorithm();
std::unique_ptr<PiAlgorithm> make_agm_algorithm();
std::unique_ptr<PiAlgorithm> make_borwein_quartic_algorithm();

} // namespace satox
