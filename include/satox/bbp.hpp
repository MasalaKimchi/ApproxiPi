#pragma once

#include <string>
#include <vector>

namespace satox {

struct BbpCheck {
    int offset;
    std::string hex_digits;
};

std::string bbp_hex_digits(int offset, int count);
std::vector<BbpCheck> default_bbp_checks();

} // namespace satox
