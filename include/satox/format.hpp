#pragma once

#include <mpfr.h>

#include <string>

namespace satox {

int bits_for_decimal_digits(int decimal_digits, int guard_digits);
std::string mpfr_to_decimal_prefix(mpfr_t value, int digits_after_decimal);
std::string mpfr_pi_prefix(int digits_after_decimal, int guard_digits);
std::string short_hash(const std::string &text);
std::string escape_json(const std::string &text);

} // namespace satox
