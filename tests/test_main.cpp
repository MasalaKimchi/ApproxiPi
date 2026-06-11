#include "satox/algorithm.hpp"
#include "satox/bbp.hpp"
#include "satox/benchmark.hpp"
#include "satox/binary_splitting.hpp"
#include "satox/candidate.hpp"
#include "satox/formula_spec.hpp"
#include "satox/verification.hpp"

#include <cassert>
#include <iostream>
#include <string>

namespace {

void test_known_prefixes() {
    auto algorithms = satox::make_default_algorithms();
    for (const auto &algorithm : algorithms) {
        if (algorithm->metadata().verification_only) {
            continue;
        }
        for (int digits : {50, 100, 1000}) {
            const satox::ComputeResult result = algorithm->compute(digits, 25);
            assert(result.supported);
            assert(result.verified);
            assert(result.decimal_prefix.size() == static_cast<size_t>(digits + 2));
            assert(result.decimal_prefix.rfind(satox::pi_known_prefix().substr(0, 20), 0) == 0);
        }
    }
}

void test_invalid_inputs() {
    auto chudnovsky = satox::make_chudnovsky_algorithm();
    satox::ComputeResult zero = chudnovsky->compute(0, 25);
    assert(!zero.supported);
    assert(!zero.error.empty());

    satox::ComputeResult negative_guard = chudnovsky->compute(100, -1);
    assert(!negative_guard.supported);
    assert(!negative_guard.error.empty());
}

void test_unified_verification() {
    auto chudnovsky = satox::make_chudnovsky_algorithm();
    const satox::ComputeResult result = chudnovsky->compute(5000, 25);
    assert(result.verified);
    double elapsed = 0.0;
    assert(satox::verify_pi_decimal_prefix(result.decimal_prefix, 5000, 25, true, &elapsed));
    assert(satox::decimal_prefix_matches_pi(result.decimal_prefix, 5000, 25));
    assert(!satox::verify_pi_decimal_prefix("3.14", 5000, 25, false, nullptr));
}

void test_bbp_spots() {
    assert(satox::bbp_hex_digits(0, 8) == "243f6a88");
    assert(satox::bbp_hex_digits(10, 8) == "a308d313");
    assert(satox::bbp_hex_digits(100, 8) == "29b7c97c");
}

void test_candidate_metadata() {
    satox::CandidateFormula missing;
    missing.id = "bad";
    assert(!missing.has_required_metadata());
    assert(satox::candidate_decision(missing, 1.0, false).find("rejected") == 0);

    satox::CandidateFormula candidate;
    candidate.id = "C-TEST";
    candidate.family = "test";
    candidate.recurrence = "t[n+1]=r(n)t[n]";
    candidate.estimated_digits_per_term = 15.0;
    candidate.implementation_notes = "metadata only";
    candidate.discriminant = "-163";
    candidate.class_invariant = "j";
    candidate.algebraic_height_bits = 64.0;
    candidate.polynomial_degree = 3;
    candidate.numerator_degree = 3;
    candidate.denominator_degree = 3;
    candidate.binary_splitting_ready = true;
    candidate.proof_status = satox::ProofStatus::SymbolicCertified;
    assert(candidate.has_required_metadata());
    assert(satox::candidate_decision(candidate, 1.0, false).find("not_claimed") == 0);
    const satox::CandidateScore score = satox::score_candidate(candidate, 1000);
    assert(!score.decision.empty());
}

void test_output_schema_helpers() {
    auto chudnovsky = satox::make_chudnovsky_algorithm();
    satox::ComputeResult result = chudnovsky->compute(50, 25);
    const std::string csv = satox::result_to_csv(result, "chudnovsky_bs", result.wall_ms);
    const std::string json = satox::result_to_json(result, "chudnovsky_bs", result.wall_ms);
    assert(csv.find("chudnovsky_bs") != std::string::npos);
    assert(json.find("\"algorithm\":\"chudnovsky_bs\"") != std::string::npos);
    assert(satox::csv_header().find("relative_wall_time") != std::string::npos);
}

void test_generic_binary_splitting() {
    satox::HypergeometricBsSpec spec;
    spec.id = "toy";
    spec.q_factors.push_back({1, 1});
    spec.linear_a = 2;
    spec.linear_b = 1;

    satox::HypergeometricBsResult result;
    satox::binary_split_hypergeometric(spec, 0, 2, result);
    assert(mpz_cmp_ui(result.q, 2ul) == 0);
    assert(mpz_cmp_ui(result.t, 5ul) == 0);
}

void test_formula_specs() {
    const std::vector<satox::FormulaSpec> formulas =
        satox::load_formula_specs("formulas/specs");
    assert(formulas.size() >= 2);
    const std::string report = satox::formula_score_report(formulas, 1000);
    assert(report.find("C-163") != std::string::npos);
    assert(report.find("R-396") != std::string::npos);
}

void test_naive_matches_bs() {
    auto naive = satox::make_chudnovsky_naive_algorithm();
    auto bs = satox::make_chudnovsky_algorithm();
    const satox::ComputeResult naive_result = naive->compute(100, 25);
    const satox::ComputeResult bs_result = bs->compute(100, 25);
    assert(naive_result.verified);
    assert(bs_result.verified);
    assert(naive_result.decimal_prefix == bs_result.decimal_prefix);
}

void test_bbp_algorithm() {
    auto bbp = satox::make_bbp_hex_extract_algorithm();
    const satox::ComputeResult result = bbp->compute(1000, 25);
    assert(result.supported);
    assert(result.verified);
    assert(result.metadata.verification_only);
}

void test_csv_schema_extended() {
    assert(satox::csv_header().find("digits_per_joule") != std::string::npos);
    assert(satox::csv_header().find("peak_rss_bytes") != std::string::npos);
}

void test_hybrid_delegate_routing() {
    assert(std::string(satox::hybrid_delegate_name(1000)) == "chudnovsky_bs_crown");
    assert(std::string(satox::hybrid_delegate_name(100000)) == "chudnovsky_bs_crown");
#ifdef SATOX_HAVE_FLINT
    assert(std::string(satox::hybrid_delegate_name(1'000'000)) == "arb_const_pi");
    assert(std::string(satox::hybrid_delegate_name(10'000'000)) == "arb_const_pi");
#else
    assert(std::string(satox::hybrid_delegate_name(10'000'000)) == "chudnovsky_bs_crown");
#endif

    auto hybrid = satox::make_chudnovsky_hybrid_algorithm();
    assert(hybrid->metadata().name == "chudnovsky_hybrid");
    const satox::ComputeResult small = hybrid->compute(100, 25);
    assert(small.supported);
    assert(small.verified);
    assert(small.notes.find("delegate=chudnovsky_bs_crown") != std::string::npos);
}

} // namespace

int main() {
    test_known_prefixes();
    test_naive_matches_bs();
    test_bbp_algorithm();
    test_csv_schema_extended();
    test_unified_verification();
    test_invalid_inputs();
    test_bbp_spots();
    test_candidate_metadata();
    test_output_schema_helpers();
    test_generic_binary_splitting();
    test_formula_specs();
    test_hybrid_delegate_routing();
    std::cout << "satox-tests: all tests passed\n";
    return 0;
}
