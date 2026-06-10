#pragma once

#include <string>

namespace satox {

bool verify_decimal_prefix_residues(const std::string &decimal_prefix, int digits_after_decimal,
                                    double *elapsed_ms);

} // namespace satox
