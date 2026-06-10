#include "satox/residue_verify.hpp"

#include "satox/format.hpp"
#include "satox/run_config.hpp"
#include "satox/timer.hpp"

#include <gmp.h>
#include <mpfr.h>

namespace satox {
namespace {

long long decimal_mod(const std::string &prefix, int digits_after_decimal, long prime) {
    long long value = 0;
    const size_t dot = prefix.find('.');
    if (dot == std::string::npos) {
        return 0;
    }
    const size_t end = std::min(prefix.size(), dot + 1 + static_cast<size_t>(digits_after_decimal));
    for (size_t i = 0; i < end; ++i) {
        const char c = prefix[i];
        if (c == '.') {
            continue;
        }
        value = (value * 10 + (c - '0')) % prime;
    }
    return value;
}

long long pi_mod_mpfr(int digits_after_decimal, long prime) {
    const int guard = 32;
    const int precision_bits = bits_for_decimal_digits(digits_after_decimal, guard);
    mpfr_t pi;
    mpfr_t scaled;
    mpfr_init2(pi, static_cast<mpfr_prec_t>(precision_bits));
    mpfr_init2(scaled, static_cast<mpfr_prec_t>(precision_bits));
    mpfr_const_pi(pi, MPFR_RNDN);
    mpz_t pow10;
    mpz_init(pow10);
    mpz_ui_pow_ui(pow10, 10ul, static_cast<unsigned long>(digits_after_decimal));
    mpfr_mul_z(scaled, pi, pow10, MPFR_RNDN);

    mpz_t scaled_z;
    mpz_init(scaled_z);
    mpfr_get_z(scaled_z, scaled, MPFR_RNDZ);
    mpz_clear(pow10);

    mpz_t mod;
    mpz_init(mod);
    mpz_mod_ui(mod, scaled_z, static_cast<unsigned long>(prime));
    const long long out = mpz_get_si(mod);
    mpz_clear(mod);
    mpz_clear(scaled_z);
    mpfr_clear(scaled);
    mpfr_clear(pi);
    return out;
}

} // namespace

bool verify_decimal_prefix_residues(const std::string &decimal_prefix, int digits_after_decimal,
                                    double *elapsed_ms) {
    if (!global_run_config().enable_residues) {
        if (elapsed_ms != nullptr) {
            *elapsed_ms = 0.0;
        }
        return true;
    }
    const Timer timer;
    constexpr long kPrimes[] = {1000000007, 1000000009, 1000000021, 1000000033};
    constexpr int kResidueSampleDigits = 1000000;
    const int residue_digits =
        digits_after_decimal > kResidueSampleDigits ? kResidueSampleDigits : digits_after_decimal;
    for (long prime : kPrimes) {
        const long long candidate = decimal_mod(decimal_prefix, residue_digits, prime);
        const long long reference = pi_mod_mpfr(residue_digits, prime);
        if (candidate != reference) {
            if (elapsed_ms != nullptr) {
                *elapsed_ms = timer.wall_ms();
            }
            return false;
        }
    }
    if (elapsed_ms != nullptr) {
        *elapsed_ms = timer.wall_ms();
    }
    return true;
}

} // namespace satox
