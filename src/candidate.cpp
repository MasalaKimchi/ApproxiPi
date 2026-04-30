#include "satox/candidate.hpp"

#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace satox {
namespace {

std::vector<std::string> split_pipe(const std::string &line) {
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, '|')) {
        parts.push_back(item);
    }
    return parts;
}

} // namespace

bool CandidateFormula::has_required_metadata() const {
    return !id.empty() && !family.empty() && !recurrence.empty() &&
           estimated_digits_per_term > 0.0 && !implementation_notes.empty() &&
           proof_status != ProofStatus::Unknown;
}

std::vector<CandidateFormula> load_candidate_formulas(const std::string &path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("could not open candidate formula file: " + path);
    }

    std::vector<CandidateFormula> candidates;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const std::vector<std::string> parts = split_pipe(line);
        if (parts.size() != 14) {
            throw std::runtime_error("candidate line must have 14 pipe-delimited fields");
        }
        CandidateFormula candidate;
        candidate.id = parts[0];
        candidate.family = parts[1];
        candidate.recurrence = parts[2];
        candidate.estimated_digits_per_term = std::stod(parts[3]);
        candidate.discriminant = parts[4];
        candidate.class_invariant = parts[5];
        candidate.algebraic_height_bits = std::stod(parts[6]);
        candidate.polynomial_degree = std::stoi(parts[7]);
        candidate.numerator_degree = std::stoi(parts[8]);
        candidate.denominator_degree = std::stoi(parts[9]);
        candidate.binary_splitting_ready = parts[10] == "yes" || parts[10] == "true";
        candidate.proof_status = proof_status_from_string(parts[11]);
        candidate.implementation_notes = parts[12];
        if (candidate.id.empty()) {
            throw std::runtime_error("candidate id must be non-empty");
        }
        if (!parts[13].empty()) {
            candidate.implementation_notes += " Source: " + parts[13];
        }
        candidates.push_back(candidate);
    }
    return candidates;
}

std::string candidate_decision(const CandidateFormula &candidate, double chudnovsky_wall_ms,
                               bool benchmarked_and_verified) {
    if (!candidate.has_required_metadata()) {
        return "rejected: missing required formula metadata";
    }
    if (!benchmarked_and_verified) {
        return "not_claimed: metadata stored, but candidate has not passed the benchmark pipeline";
    }
    if (chudnovsky_wall_ms <= 0.0) {
        return "not_claimed: missing Chudnovsky baseline timing";
    }
    return "eligible_for_comparison: verified benchmark data available";
}

ProofStatus proof_status_from_string(const std::string &text) {
    if (text == "metadata" || text == "metadata_only") {
        return ProofStatus::MetadataOnly;
    }
    if (text == "numeric" || text == "numeric_verified") {
        return ProofStatus::NumericVerified;
    }
    if (text == "symbolic" || text == "symbolic_certified") {
        return ProofStatus::SymbolicCertified;
    }
    return ProofStatus::Unknown;
}

std::string proof_status_to_string(ProofStatus status) {
    switch (status) {
    case ProofStatus::MetadataOnly:
        return "metadata_only";
    case ProofStatus::NumericVerified:
        return "numeric_verified";
    case ProofStatus::SymbolicCertified:
        return "symbolic_certified";
    case ProofStatus::Unknown:
    default:
        return "unknown";
    }
}

CandidateScore score_candidate(const CandidateFormula &candidate, int decimal_digits) {
    CandidateScore score;
    if (!candidate.has_required_metadata() || decimal_digits <= 0) {
        score.decision = "reject: incomplete metadata";
        return score;
    }

    const double degree = static_cast<double>(candidate.numerator_degree +
                                             candidate.denominator_degree +
                                             candidate.polynomial_degree);
    const double terms = std::ceil(static_cast<double>(decimal_digits) /
                                   candidate.estimated_digits_per_term);
    score.convergence_bonus = candidate.estimated_digits_per_term / 14.181647462725477;
    score.height_penalty = std::log2(1.0 + candidate.algebraic_height_bits) / 12.0;
    score.degree_penalty = degree / 24.0;
    score.multiplication_estimate = terms * std::max(1.0, degree);

    double proof_bonus = 0.0;
    if (candidate.proof_status == ProofStatus::NumericVerified) {
        proof_bonus = 0.15;
    } else if (candidate.proof_status == ProofStatus::SymbolicCertified) {
        proof_bonus = 0.35;
    }
    const double bs_bonus = candidate.binary_splitting_ready ? 0.35 : -0.50;
    score.score = score.convergence_bonus + proof_bonus + bs_bonus - score.height_penalty -
                  score.degree_penalty;

    if (!candidate.binary_splitting_ready) {
        score.decision = "hold: not binary-splitting ready";
    } else if (candidate.proof_status != ProofStatus::SymbolicCertified) {
        score.decision = "hold: needs proof certificate";
    } else if (score.score > 1.0) {
        score.decision = "promote: implement benchmark kernel";
    } else {
        score.decision = "hold: score below Chudnovsky replacement threshold";
    }
    return score;
}

} // namespace satox
