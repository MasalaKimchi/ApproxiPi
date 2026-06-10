#pragma once

#include <string>

namespace satox {

bool verify_decimal_prefix_residues(const std::string &decimal_prefix, int digits_after_decimal,
                                    int guard_digits, double *elapsed_ms);

} // namespace satox
