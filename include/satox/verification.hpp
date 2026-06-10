#pragma once

#include <mpfr.h>

#include <string>

namespace satox {

bool valid_digit_request(int decimal_digits, int guard_digits, std::string *error);
bool decimal_prefix_matches_pi(const std::string &candidate, int digits_after_decimal,
                               int guard_digits);
// H13: one mpfr_const_pi + integer compare (+ optional modular residues).
bool verify_pi_decimal_prefix(const std::string &candidate, int digits_after_decimal,
                              int guard_digits, bool check_residues, double *elapsed_ms);
// H13b: verify before decimal formatting using the computed scaled value pi * 10^D.
bool verify_scaled_pi_mpfr(mpfr_srcptr candidate_scaled, int digits_after_decimal,
                           int guard_digits, double *elapsed_ms);
bool verify_unscaled_pi_mpfr(mpfr_srcptr pi, int digits_after_decimal, int guard_digits,
                             double *elapsed_ms);
std::string pi_known_prefix();

} // namespace satox
