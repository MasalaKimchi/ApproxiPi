#include "satox/bbp.hpp"

#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>

namespace satox {
namespace {

std::uint64_t mod_pow(std::uint64_t base, std::uint64_t exponent, std::uint64_t modulus) {
    if (modulus == 1) {
        return 0;
    }
    std::uint64_t result = 1 % modulus;
    base %= modulus;
    while (exponent > 0) {
        if ((exponent & 1u) != 0) {
            result = (result * base) % modulus;
        }
        base = (base * base) % modulus;
        exponent >>= 1u;
    }
    return result;
}

double bbp_sum(int j, int n) {
    double s = 0.0;
    for (int k = 0; k <= n; ++k) {
        const std::uint64_t denom = static_cast<std::uint64_t>(8 * k + j);
        const std::uint64_t pow = mod_pow(16, static_cast<std::uint64_t>(n - k), denom);
        s += static_cast<double>(pow) / static_cast<double>(denom);
        s -= std::floor(s);
    }
    for (int k = n + 1; k <= n + 100; ++k) {
        const double term = std::pow(16.0, static_cast<double>(n - k)) /
                            static_cast<double>(8 * k + j);
        if (term < 1e-17) {
            break;
        }
        s += term;
        s -= std::floor(s);
    }
    return s;
}

} // namespace

std::string bbp_hex_digits(int offset, int count) {
    if (offset < 0 || count <= 0) {
        return "";
    }

    double x = 4.0 * bbp_sum(1, offset) - 2.0 * bbp_sum(4, offset) -
               bbp_sum(5, offset) - bbp_sum(6, offset);
    x -= std::floor(x);
    if (x < 0.0) {
        x += 1.0;
    }

    std::ostringstream out;
    for (int i = 0; i < count; ++i) {
        x *= 16.0;
        const int digit = static_cast<int>(x);
        out << std::hex << std::nouppercase << digit;
        x -= digit;
    }
    return out.str();
}

std::vector<BbpCheck> default_bbp_checks() {
    return {{0, bbp_hex_digits(0, 8)}, {10, bbp_hex_digits(10, 8)},
            {100, bbp_hex_digits(100, 8)}};
}

} // namespace satox
