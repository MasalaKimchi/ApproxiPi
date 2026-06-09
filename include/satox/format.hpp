#pragma once

#include <gmp.h>
#include <mpfr.h>

#include <string>

namespace satox {

int bits_for_decimal_digits(int decimal_digits, int guard_digits);
std::string mpfr_to_decimal_prefix(mpfr_t value, int digits_after_decimal);

// Power-of-ten cache for fixed-point decimal rendering. Building it is pure
// integer work that does not depend on the computed value, so callers can
// overlap it with the series evaluation. (A floating-point reciprocal-power
// variant was tried and rejected: the full-precision reciprocal cannot be
// hidden under the series evaluation because the chunk phase saturates all
// cores.)
struct DecimalPowerCache {
    mpz_t p10_full; // 10^digits
    mpz_t pows[3];  // 10^w, 10^(2w), 10^(4w) for the 8-way split width w
    int digits = 0;
    int part_width = 0;
    bool ready = false;

    DecimalPowerCache();
    ~DecimalPowerCache();
    DecimalPowerCache(const DecimalPowerCache &) = delete;
    DecimalPowerCache &operator=(const DecimalPowerCache &) = delete;
};

void build_decimal_power_cache(int digits_after_decimal, DecimalPowerCache &cache);

// Renders pi from the pre-scaled integer floor(pi * 10^digits) using an
// 8-way parallel divide-and-conquer base conversion. `value_scaled` must hold
// pi * 10^digits to at least integer accuracy. Falls back to the serial
// formatter when preconditions do not hold.
std::string scaled_pi_to_decimal_parallel(mpfr_t value_scaled, int digits_after_decimal,
                                          const DecimalPowerCache &cache);

// Renders a non-negative integer as exactly `width` decimal digits
// (zero-padded) with the cache's divide-and-conquer splitting; `level` 2 is
// the full 8-way split. Exposed so callers can pipeline prefix rendering
// (H12): render the carry-stable high half early, low half later.
std::string render_decimal_fixed(const mpz_t value, int width, int level,
                                 const DecimalPowerCache &cache);
std::string mpfr_pi_prefix(int digits_after_decimal, int guard_digits);
std::string short_hash(const std::string &text);
std::string escape_json(const std::string &text);

} // namespace satox
