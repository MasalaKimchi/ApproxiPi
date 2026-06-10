#pragma once

#include <cstdint>
#include <string>

namespace satox {

std::uint64_t estimate_peak_bytes(int decimal_digits, const std::string &algorithm_name);
bool memory_guard_allows(int decimal_digits, const std::string &algorithm_name,
                       std::string *error);

} // namespace satox
