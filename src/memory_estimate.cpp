#include "satox/memory_estimate.hpp"

#include "satox/limits.hpp"
#include "satox/resource_monitor.hpp"
#include "satox/run_config.hpp"

#include <cmath>
#include <sstream>

namespace satox {

std::uint64_t estimate_peak_bytes(int decimal_digits, const std::string &algorithm_name) {
    const double bits =
        static_cast<double>(decimal_digits + 200) * std::log2(10.0);
    const double terms =
        static_cast<double>(decimal_digits) / 14.18 + 8.0;
    const double log_terms = std::log2(std::max(2.0, terms));

    double operand_bits = bits * (log_terms + 2.0);
    if (algorithm_name.find("crown") != std::string::npos) {
        operand_bits = std::min(operand_bits, bits + 65536.0);
    }
    if (algorithm_name == "chudnovsky_naive" || algorithm_name == "chudnovsky_recurrence") {
        operand_bits = bits * terms * 0.25;
    }

    const std::uint64_t integer_bytes =
        static_cast<std::uint64_t>(operand_bits / 8.0) * 3ull;
    const std::uint64_t decimal_bytes =
        static_cast<std::uint64_t>(decimal_digits) + 64ull;
    return integer_bytes + decimal_bytes + 64ull * 1024ull * 1024ull;
}

bool memory_guard_allows(int decimal_digits, const std::string &algorithm_name,
                       std::string *error) {
    if (global_run_config().skip_memory_guard) {
        return true;
    }
    const std::uint64_t total = system_total_bytes();
    if (total == 0) {
        return true;
    }
    const std::uint64_t estimate = estimate_peak_bytes(decimal_digits, algorithm_name);
    if (estimate > total * 8 / 10) {
        if (error != nullptr) {
            std::ostringstream out;
            out << "estimated peak " << (estimate / (1024ull * 1024ull))
                << " MiB exceeds 80% of system RAM for " << algorithm_name;
            *error = out.str();
        }
        return false;
    }
    return true;
}

} // namespace satox
