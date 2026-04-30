#pragma once

#include <string>
#include <vector>

namespace satox {

enum class ProofStatus {
    Unknown,
    MetadataOnly,
    NumericVerified,
    SymbolicCertified,
};

struct CandidateFormula {
    std::string id;
    std::string family;
    std::string recurrence;
    double estimated_digits_per_term = 0.0;
    std::string implementation_notes;
    std::string discriminant;
    std::string class_invariant;
    double algebraic_height_bits = 0.0;
    int polynomial_degree = 0;
    int numerator_degree = 0;
    int denominator_degree = 0;
    bool binary_splitting_ready = false;
    ProofStatus proof_status = ProofStatus::Unknown;

    bool has_required_metadata() const;
};

struct CandidateScore {
    double score = 0.0;
    double convergence_bonus = 0.0;
    double height_penalty = 0.0;
    double degree_penalty = 0.0;
    double multiplication_estimate = 0.0;
    std::string decision;
};

std::vector<CandidateFormula> load_candidate_formulas(const std::string &path);
std::string candidate_decision(const CandidateFormula &candidate, double chudnovsky_wall_ms,
                               bool benchmarked_and_verified);
ProofStatus proof_status_from_string(const std::string &text);
std::string proof_status_to_string(ProofStatus status);
CandidateScore score_candidate(const CandidateFormula &candidate, int decimal_digits);

} // namespace satox
