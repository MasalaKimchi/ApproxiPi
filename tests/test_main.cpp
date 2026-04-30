#include "satox/algorithm.hpp"
#include "satox/bbp.hpp"
#include "satox/benchmark.hpp"
#include "satox/binary_splitting.hpp"
#include "satox/candidate.hpp"
#include "satox/formula_spec.hpp"
#include "satox/verification.hpp"

#include <cassert>
#include <iostream>

namespace {

void test_known_prefixes() {
    auto algorithms = satox::make_default_algorithms();
    for (const auto &algorithm : algorithms) {
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
    const std::vector<satox::FormulaSpec> formulas = satox::load_formula_specs("candidates");
    assert(formulas.size() >= 2);
    const std::string report = satox::formula_score_report(formulas, 1000);
    assert(report.find("C-163") != std::string::npos);
    assert(report.find("R-396") != std::string::npos);
}

} // namespace

int main() {
    test_known_prefixes();
    test_invalid_inputs();
    test_bbp_spots();
    test_candidate_metadata();
    test_output_schema_helpers();
    test_generic_binary_splitting();
    test_formula_specs();
    std::cout << "satox-tests: all tests passed\n";
    return 0;
}
