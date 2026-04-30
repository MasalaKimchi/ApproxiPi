#include "satox/verification.hpp"

#include "satox/format.hpp"

namespace satox {

bool valid_digit_request(int decimal_digits, int guard_digits, std::string *error) {
    if (decimal_digits <= 0) {
        if (error != nullptr) {
            *error = "decimal_digits must be positive";
        }
        return false;
    }
    if (guard_digits < 0) {
        if (error != nullptr) {
            *error = "guard_digits must be non-negative";
        }
        return false;
    }
    return true;
}

bool decimal_prefix_matches_pi(const std::string &candidate, int digits_after_decimal,
                               int guard_digits) {
    return candidate == mpfr_pi_prefix(digits_after_decimal, guard_digits);
}

bool decimal_prefix_matches_reference(const std::string &candidate,
                                      const std::string &reference) {
    return candidate == reference;
}

std::string pi_known_prefix() {
    return "3.14159265358979323846264338327950288419716939937510"
           "58209749445923078164062862089986280348253421170679";
}

} // namespace satox
