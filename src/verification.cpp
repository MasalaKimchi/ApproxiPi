#include "satox/verification.hpp"

#include "satox/format.hpp"
#include "satox/run_config.hpp"
#include "satox/timer.hpp"

#include <algorithm>
#include <gmp.h>
#include <map>
#include <mpfr.h>
#include <mutex>
#include <string>

namespace satox {
namespace {

constexpr int kSampleVerifyCap = 1000000;

std::mutex &reference_cache_mutex() {
    static std::mutex mutex;
    return mutex;
}

std::map<std::pair<int, int>, std::string> &reference_cache() {
    static std::map<std::pair<int, int>, std::string> cache;
    return cache;
}

std::string compute_reference_pi_scaled_string(int verify_digits, int guard_digits) {
    const int precision_bits = bits_for_decimal_digits(verify_digits, guard_digits);
    mpfr_t pi;
    mpfr_t scaled;
    mpfr_init2(pi, static_cast<mpfr_prec_t>(precision_bits));
    mpfr_init2(scaled, static_cast<mpfr_prec_t>(precision_bits));
    mpfr_const_pi(pi, MPFR_RNDN);

    mpz_t pow10;
    mpz_t reference;
    mpz_init(pow10);
    mpz_init(reference);
    mpz_ui_pow_ui(pow10, 10ul, static_cast<unsigned long>(verify_digits));
    mpfr_mul_z(scaled, pi, pow10, MPFR_RNDN);
    mpfr_get_z(reference, scaled, MPFR_RNDZ);

    char *raw = mpz_get_str(nullptr, 10, reference);
    std::string out(raw);
    void (*free_func)(void *, size_t) = nullptr;
    mp_get_memory_functions(nullptr, nullptr, &free_func);
    free_func(raw, out.size() + 1);

    mpz_clear(reference);
    mpz_clear(pow10);
    mpfr_clear(scaled);
    mpfr_clear(pi);
    return out;
}

bool reference_pi_scaled_mpz_cached(int verify_digits, int guard_digits, mpz_t out) {
    const std::pair<int, int> key{verify_digits, guard_digits};
    {
        std::lock_guard<std::mutex> lock(reference_cache_mutex());
        const auto found = reference_cache().find(key);
        if (found != reference_cache().end()) {
            return mpz_set_str(out, found->second.c_str(), 10) == 0;
        }
    }

    const std::string computed =
        compute_reference_pi_scaled_string(verify_digits, guard_digits);
    {
        std::lock_guard<std::mutex> lock(reference_cache_mutex());
        reference_cache().emplace(key, computed);
    }
    return mpz_set_str(out, computed.c_str(), 10) == 0;
}

bool parse_decimal_prefix_scaled(const std::string &prefix, int verify_digits, mpz_t out) {
    if (verify_digits < 0) {
        return false;
    }
    const size_t dot = prefix.find('.');
    if (dot == std::string::npos) {
        return false;
    }

    std::string digits;
    digits.reserve(dot + static_cast<size_t>(verify_digits));
    for (size_t i = 0; i < dot; ++i) {
        const char c = prefix[i];
        if (c >= '0' && c <= '9') {
            digits.push_back(c);
        }
    }

    int frac_digits = 0;
    for (size_t i = dot + 1; i < prefix.size() && frac_digits < verify_digits; ++i) {
        const char c = prefix[i];
        if (c >= '0' && c <= '9') {
            digits.push_back(c);
            ++frac_digits;
        }
    }
    digits.append(static_cast<size_t>(verify_digits - frac_digits), '0');
    return mpz_set_str(out, digits.c_str(), 10) == 0;
}

bool verify_scaled_pi_mpfr_impl(mpfr_srcptr candidate_scaled, int digits_after_decimal,
                                int guard_digits, double *elapsed_ms) {
    const Timer timer;
    const int verify_digits =
        digits_after_decimal > kSampleVerifyCap ? kSampleVerifyCap : digits_after_decimal;
    const int reference_bits = bits_for_decimal_digits(verify_digits, guard_digits);

    mpfr_t candidate_sample;
    mpfr_init2(candidate_sample, static_cast<mpfr_prec_t>(reference_bits));

    mpfr_set(candidate_sample, candidate_scaled, MPFR_RNDN);
    if (digits_after_decimal > verify_digits) {
        mpz_t p10_drop;
        mpz_init(p10_drop);
        mpz_ui_pow_ui(p10_drop, 10ul,
                      static_cast<unsigned long>(digits_after_decimal - verify_digits));
        mpfr_div_z(candidate_sample, candidate_sample, p10_drop, MPFR_RNDZ);
        mpz_clear(p10_drop);
    }

    mpz_t candidate_z;
    mpz_t reference_z;
    mpz_init(candidate_z);
    mpz_init(reference_z);
    mpfr_get_z(candidate_z, candidate_sample, MPFR_RNDZ);
    reference_pi_scaled_mpz_cached(verify_digits, guard_digits, reference_z);
    const bool ok = mpz_cmp(candidate_z, reference_z) == 0;

    mpz_clear(reference_z);
    mpz_clear(candidate_z);
    mpfr_clear(candidate_sample);

    if (elapsed_ms != nullptr) {
        *elapsed_ms = timer.wall_ms();
    }
    return ok;
}

} // namespace

bool verify_scaled_pi_mpfr(mpfr_srcptr candidate_scaled, int digits_after_decimal,
                           int guard_digits, double *elapsed_ms) {
    return verify_scaled_pi_mpfr_impl(candidate_scaled, digits_after_decimal, guard_digits,
                                      elapsed_ms);
}

bool verify_unscaled_pi_mpfr(mpfr_srcptr pi, int digits_after_decimal, int guard_digits,
                             double *elapsed_ms) {
    const int precision_bits = bits_for_decimal_digits(digits_after_decimal, guard_digits);
    mpfr_t scaled;
    mpfr_init2(scaled, static_cast<mpfr_prec_t>(precision_bits));
    mpz_t p10;
    mpz_init(p10);
    mpz_ui_pow_ui(p10, 10ul, static_cast<unsigned long>(digits_after_decimal));
    mpfr_mul_z(scaled, pi, p10, MPFR_RNDN);
    const bool ok =
        verify_scaled_pi_mpfr_impl(scaled, digits_after_decimal, guard_digits, elapsed_ms);
    mpz_clear(p10);
    mpfr_clear(scaled);
    return ok;
}

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

bool verify_pi_decimal_prefix(const std::string &candidate, int digits_after_decimal,
                              int guard_digits, bool check_residues, double *elapsed_ms) {
    const Timer timer;
    const int verify_digits =
        digits_after_decimal > kSampleVerifyCap ? kSampleVerifyCap : digits_after_decimal;

    mpz_t reference;
    mpz_t parsed;
    mpz_init(reference);
    mpz_init(parsed);

    bool ok = reference_pi_scaled_mpz_cached(verify_digits, guard_digits, reference) &&
              parse_decimal_prefix_scaled(candidate, verify_digits, parsed) &&
              mpz_cmp(parsed, reference) == 0;

    (void)check_residues;

    mpz_clear(parsed);
    mpz_clear(reference);

    if (elapsed_ms != nullptr) {
        *elapsed_ms = timer.wall_ms();
    }
    return ok;
}

bool decimal_prefix_matches_pi(const std::string &candidate, int digits_after_decimal,
                               int guard_digits) {
    return verify_pi_decimal_prefix(candidate, digits_after_decimal, guard_digits, false,
                                    nullptr);
}

std::string pi_known_prefix() {
    return "3.14159265358979323846264338327950288419716939937510"
           "58209749445923078164062862089986280348253421170679";
}

} // namespace satox
