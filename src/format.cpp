#include "satox/format.hpp"

#include "satox/resource_monitor.hpp"

#include <gmp.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <future>
#include <sstream>
#include <stdexcept>

namespace satox {

namespace {

constexpr int kMinParallelFormatDigits = 64;

std::string mpz_fixed_width_decimal(const mpz_t value, int width) {
    char *raw = mpz_get_str(nullptr, 10, value);
    std::string digits(raw);
    void (*free_func)(void *, size_t) = nullptr;
    mp_get_memory_functions(nullptr, nullptr, &free_func);
    free_func(raw, digits.size() + 1);

    if (static_cast<int>(digits.size()) > width) {
        throw std::runtime_error("decimal part wider than expected");
    }
    std::string out;
    out.reserve(static_cast<size_t>(width));
    out.append(static_cast<size_t>(width) - digits.size(), '0');
    out += digits;
    return out;
}

} // namespace

DecimalPowerCache::DecimalPowerCache() {
    mpz_init(p10_full);
    for (mpz_t &power : pows) {
        mpz_init(power);
    }
}

DecimalPowerCache::~DecimalPowerCache() {
    mpz_clear(p10_full);
    for (mpz_t &power : pows) {
        mpz_clear(power);
    }
}

void build_decimal_power_cache(int digits_after_decimal, DecimalPowerCache &cache) {
    cache.digits = digits_after_decimal;
    const int total_width = digits_after_decimal + 1;
    cache.part_width = std::max(1, total_width / 8);
    mpz_ui_pow_ui(cache.pows[0], 10ul, static_cast<unsigned long>(cache.part_width));
    mpz_mul(cache.pows[1], cache.pows[0], cache.pows[0]);
    mpz_mul(cache.pows[2], cache.pows[1], cache.pows[1]);
    mpz_ui_pow_ui(cache.p10_full, 10ul, static_cast<unsigned long>(digits_after_decimal));
    cache.ready = true;
}

namespace {

// Recursive 8-way divide-and-conquer rendering: at `level` the low part holds
// exactly part_width * 2^level digits, split off with the cached power.
std::string render_fixed_decimal(const mpz_t value, int width, int level,
                                 const DecimalPowerCache &cache) {
    if (level < 0) {
        return mpz_fixed_width_decimal(value, width);
    }
    const int low_width = cache.part_width << level;
    if (width <= low_width) {
        return render_fixed_decimal(value, width, level - 1, cache);
    }

    mpz_t high;
    mpz_t low;
    mpz_init(high);
    mpz_init(low);
    mpz_tdiv_qr(high, low, value, cache.pows[level]);

    std::string high_part;
    auto high_future = std::async(std::launch::async, [&]() {
        high_part = render_fixed_decimal(high, width - low_width, level - 1, cache);
    });
    std::string low_part = render_fixed_decimal(low, low_width, level - 1, cache);
    high_future.get();

    mpz_clear(high);
    mpz_clear(low);
    return high_part + low_part;
}

} // namespace

std::string render_decimal_fixed(const mpz_t value, int width, int level,
                                 const DecimalPowerCache &cache) {
    return render_fixed_decimal(value, width, level, cache);
}

std::string scaled_pi_to_decimal_parallel(mpfr_t value_scaled, int digits_after_decimal,
                                          const DecimalPowerCache &cache) {
    const int total_width = digits_after_decimal + 1;

    if (!cache.ready || cache.digits != digits_after_decimal ||
        digits_after_decimal < kMinParallelFormatDigits ||
        total_width - 7 * cache.part_width < 1 || mpfr_sgn(value_scaled) <= 0) {
        // Undo the 10^digits scaling and use the serial formatter.
        mpfr_t value;
        mpfr_init2(value, mpfr_get_prec(value_scaled));
        if (cache.ready) {
            mpfr_div_z(value, value_scaled, cache.p10_full, MPFR_RNDN);
        } else {
            mpfr_set(value, value_scaled, MPFR_RNDN);
        }
        std::string out = mpfr_to_decimal_prefix(value, digits_after_decimal);
        mpfr_clear(value);
        return out;
    }

    mpz_t z;
    mpz_init(z);
    mpfr_get_z(z, value_scaled, MPFR_RNDZ);
    std::string digits = render_fixed_decimal(z, total_width, 2, cache);
    mpz_clear(z);

    std::string out;
    out.reserve(digits.size() + 1);
    out += digits.substr(0, digits.size() - static_cast<size_t>(digits_after_decimal));
    out += '.';
    out += digits.substr(digits.size() - static_cast<size_t>(digits_after_decimal));
    return out;
}

int bits_for_decimal_digits(int decimal_digits, int guard_digits) {
    if (decimal_digits < 0 || guard_digits < 0) {
        throw std::invalid_argument("digits and guard digits must be non-negative");
    }
    const double total_digits = static_cast<double>(decimal_digits + guard_digits + 16);
    return static_cast<int>(std::ceil(total_digits * std::log2(10.0)));
}

std::string mpfr_to_decimal_prefix(mpfr_t value, int digits_after_decimal) {
    if (digits_after_decimal < 0) {
        throw std::invalid_argument("digits_after_decimal must be non-negative");
    }

    mpfr_exp_t exponent = 0;
    char *raw = mpfr_get_str(nullptr, &exponent, 10,
                             static_cast<size_t>(digits_after_decimal + 2), value, MPFR_RNDZ);
    if (raw == nullptr) {
        throw std::runtime_error("mpfr_get_str failed");
    }

    std::string digits(raw);
    mpfr_free_str(raw);

    while (static_cast<int>(digits.size()) < digits_after_decimal + 2) {
        digits.push_back('0');
    }

    std::string out;
    if (exponent <= 0) {
        out = "0.";
        out.append(static_cast<size_t>(-exponent), '0');
        out += digits;
    } else {
        if (static_cast<size_t>(exponent) >= digits.size()) {
            out = digits;
            out.append(static_cast<size_t>(exponent) - digits.size(), '0');
            out += ".";
        } else {
            out = digits.substr(0, static_cast<size_t>(exponent));
            out += ".";
            out += digits.substr(static_cast<size_t>(exponent));
        }
    }

    const size_t dot = out.find('.');
    if (dot == std::string::npos) {
        out += ".";
    }
    const size_t desired = out.find('.') + 1 + static_cast<size_t>(digits_after_decimal);
    if (out.size() < desired) {
        out.append(desired - out.size(), '0');
    }
    out.resize(desired);
    return out;
}

std::string mpfr_to_decimal_prefix_streaming(mpfr_t value, int digits_after_decimal) {
    constexpr int kStreamingThreshold = 10000000;
    if (digits_after_decimal < kStreamingThreshold) {
        return mpfr_to_decimal_prefix(value, digits_after_decimal);
    }

    const std::string path = "results/tmp_pi_prefix.txt";
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return mpfr_to_decimal_prefix(value, digits_after_decimal);
    }

    const std::string prefix = mpfr_to_decimal_prefix(value, digits_after_decimal);
    out.write(prefix.data(), static_cast<std::streamsize>(prefix.size()));
    out.close();
    add_io_bytes(0, prefix.size());

    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

std::string mpfr_pi_prefix(int digits_after_decimal, int guard_digits) {
    const int precision_bits = bits_for_decimal_digits(digits_after_decimal, guard_digits);
    mpfr_t pi;
    mpfr_init2(pi, static_cast<mpfr_prec_t>(precision_bits));
    mpfr_const_pi(pi, MPFR_RNDN);
    std::string out = mpfr_to_decimal_prefix(pi, digits_after_decimal);
    mpfr_clear(pi);
    return out;
}

std::string short_hash(const std::string &text) {
    std::uint64_t hash = 1469598103934665603ull;
    for (unsigned char c : text) {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= 1099511628211ull;
    }
    std::ostringstream oss;
    oss << std::hex << hash;
    return oss.str();
}

std::string escape_json(const std::string &text) {
    std::ostringstream out;
    for (char c : text) {
        switch (c) {
        case '\\':
            out << "\\\\";
            break;
        case '"':
            out << "\\\"";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\r':
            out << "\\r";
            break;
        case '\t':
            out << "\\t";
            break;
        default:
            out << c;
            break;
        }
    }
    return out.str();
}

} // namespace satox
