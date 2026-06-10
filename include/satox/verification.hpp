#pragma once

#include <string>

namespace satox {

bool valid_digit_request(int decimal_digits, int guard_digits, std::string *error);
bool decimal_prefix_matches_pi(const std::string &candidate, int digits_after_decimal,
                               int guard_digits);
std::string pi_known_prefix();

} // namespace satox
