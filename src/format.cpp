#include "satox/format.hpp"

#include <gmp.h>

#include <cmath>
#include <cstdint>
#include <sstream>
#include <stdexcept>

namespace satox {

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
