#include "satox/residue_verify.hpp"

#include "satox/verification.hpp"

namespace satox {

bool verify_decimal_prefix_residues(const std::string &decimal_prefix, int digits_after_decimal,
                                    int guard_digits, double *elapsed_ms) {
    return verify_pi_decimal_prefix(decimal_prefix, digits_after_decimal, guard_digits, true,
                                    elapsed_ms);
}

} // namespace satox
