#include "satox/binary_splitting.hpp"

#include <stdexcept>

namespace satox {
namespace {

void set_linear_product(mpz_t out, const std::vector<LinearFactor> &factors, unsigned long n) {
    mpz_set_ui(out, 1ul);
    for (const LinearFactor &factor : factors) {
        const long value = factor.slope * static_cast<long>(n) + factor.intercept;
        if (value < 0) {
            mpz_mul_si(out, out, value);
        } else {
            mpz_mul_ui(out, out, static_cast<unsigned long>(value));
        }
    }
}

long linear_value(long a, long b, unsigned long n) {
    return a * static_cast<long>(n) + b;
}

void reduce_common_pqt(HypergeometricBsResult &node, BinarySplittingStats *stats) {
    mpz_t g;
    mpz_t abs_t;
    mpz_init(g);
    mpz_init(abs_t);

    mpz_gcd(g, node.p, node.q);
    if (mpz_cmp_ui(g, 1ul) > 0) {
        mpz_abs(abs_t, node.t);
        mpz_gcd(g, g, abs_t);
    }

    if (mpz_cmp_ui(g, 1ul) > 0) {
        if (stats != nullptr) {
            ++stats->gcd_reductions;
            stats->cancelled_bits += static_cast<double>(mpz_sizeinbase(g, 2));
        }
        mpz_divexact(node.p, node.p, g);
        mpz_divexact(node.q, node.q, g);
        mpz_divexact(node.t, node.t, g);
    }

    mpz_clear(abs_t);
    mpz_clear(g);
}

} // namespace

HypergeometricBsResult::HypergeometricBsResult() {
    mpz_init(p);
    mpz_init(q);
    mpz_init(t);
}

HypergeometricBsResult::~HypergeometricBsResult() {
    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(t);
}

void binary_split_hypergeometric(const HypergeometricBsSpec &spec, unsigned long a,
                                 unsigned long b, HypergeometricBsResult &out,
                                 BinarySplittingStats *stats) {
    if (b <= a) {
        throw std::invalid_argument("binary_split_hypergeometric requires b > a");
    }

    if (b - a == 1) {
        if (a == 0 && spec.unit_first_p) {
            mpz_set_ui(out.p, 1ul);
        } else {
            set_linear_product(out.p, spec.p_factors, a);
        }

        if (a == 0 && spec.unit_first_q) {
            mpz_set_ui(out.q, 1ul);
        } else {
            set_linear_product(out.q, spec.q_factors, a);
        }
        if (!(a == 0 && spec.unit_first_q) && spec.q_constant != 1ul) {
            mpz_mul_ui(out.q, out.q, spec.q_constant);
        }

        mpz_set(out.t, spec.leaf_t_uses_q ? out.q : out.p);
        const long lin = linear_value(spec.linear_a, spec.linear_b, a);
        if (lin < 0) {
            mpz_mul_si(out.t, out.t, lin);
        } else {
            mpz_mul_ui(out.t, out.t, static_cast<unsigned long>(lin));
        }
        if (spec.alternating && (a & 1ul) != 0) {
            mpz_neg(out.t, out.t);
        }
        if (spec.gcd_cancellation) {
            reduce_common_pqt(out, stats);
        }
        return;
    }

    const unsigned long m = (a + b) / 2ul;
    HypergeometricBsResult left;
    HypergeometricBsResult right;
    binary_split_hypergeometric(spec, a, m, left, stats);
    binary_split_hypergeometric(spec, m, b, right, stats);

    mpz_mul(out.p, left.p, right.p);
    mpz_mul(out.q, left.q, right.q);

    mpz_t left_scaled;
    mpz_t right_scaled;
    mpz_init(left_scaled);
    mpz_init(right_scaled);
    mpz_mul(left_scaled, left.t, right.q);
    mpz_mul(right_scaled, left.p, right.t);
    mpz_add(out.t, left_scaled, right_scaled);
    mpz_clear(left_scaled);
    mpz_clear(right_scaled);
    if (spec.gcd_cancellation) {
        reduce_common_pqt(out, stats);
    }
}

void binary_split_hypergeometric(const HypergeometricBsSpec &spec, unsigned long a,
                                 unsigned long b, HypergeometricBsResult &out) {
    binary_split_hypergeometric(spec, a, b, out, nullptr);
}

} // namespace satox
