#include "satox/formula_spec.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace satox {
namespace {

std::string trim(const std::string &text) {
    size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin])) != 0) {
        ++begin;
    }
    size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
        --end;
    }
    return text.substr(begin, end - begin);
}

bool to_bool(const std::string &text) {
    return text == "yes" || text == "true" || text == "1";
}

long parse_long(const std::string &text, long fallback = 0) {
    if (text.empty()) {
        return fallback;
    }
    return std::stol(text);
}

unsigned long parse_ulong(const std::string &text, unsigned long fallback = 1) {
    if (text.empty()) {
        return fallback;
    }
    return std::stoul(text);
}

LinearFactor parse_factor(std::string factor) {
    factor.erase(std::remove_if(factor.begin(), factor.end(), [](unsigned char c) {
                     return std::isspace(c) != 0 || c == '*';
                 }),
                 factor.end());
    if (factor.empty()) {
        throw std::runtime_error("empty factor in formula spec");
    }

    const size_t n_pos = factor.find('n');
    if (n_pos == std::string::npos) {
        return {0, std::stol(factor)};
    }

    std::string slope_text = factor.substr(0, n_pos);
    long slope = 1;
    if (slope_text == "-") {
        slope = -1;
    } else if (!slope_text.empty() && slope_text != "+") {
        slope = std::stol(slope_text);
    }

    long intercept = 0;
    if (n_pos + 1 < factor.size()) {
        intercept = std::stol(factor.substr(n_pos + 1));
    }
    return {slope, intercept};
}

std::vector<LinearFactor> parse_factors(const std::string &text) {
    std::vector<LinearFactor> factors;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = trim(item);
        if (!item.empty()) {
            factors.push_back(parse_factor(item));
        }
    }
    return factors;
}

std::unordered_map<std::string, std::string> read_kv(const std::string &path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("could not open formula spec: " + path);
    }
    std::unordered_map<std::string, std::string> kv;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            throw std::runtime_error("formula spec line must be key=value: " + line);
        }
        kv[trim(line.substr(0, eq))] = trim(line.substr(eq + 1));
    }
    return kv;
}

std::string get(const std::unordered_map<std::string, std::string> &kv, const std::string &key,
                const std::string &fallback = "") {
    const auto it = kv.find(key);
    return it == kv.end() ? fallback : it->second;
}

} // namespace

FormulaSpec load_formula_spec(const std::string &path) {
    const auto kv = read_kv(path);
    FormulaSpec formula;
    formula.candidate.id = get(kv, "id");
    formula.candidate.family = get(kv, "family");
    formula.candidate.recurrence = get(kv, "recurrence");
    formula.candidate.estimated_digits_per_term =
        std::stod(get(kv, "estimated_digits_per_term", "0"));
    formula.candidate.discriminant = get(kv, "discriminant");
    formula.candidate.class_invariant = get(kv, "class_invariant");
    formula.candidate.algebraic_height_bits =
        std::stod(get(kv, "algebraic_height_bits", "0"));
    formula.candidate.polynomial_degree = static_cast<int>(parse_long(get(kv, "polynomial_degree")));
    formula.candidate.numerator_degree = static_cast<int>(parse_long(get(kv, "numerator_degree")));
    formula.candidate.denominator_degree =
        static_cast<int>(parse_long(get(kv, "denominator_degree")));
    formula.candidate.binary_splitting_ready = to_bool(get(kv, "binary_splitting_ready"));
    formula.candidate.proof_status = proof_status_from_string(get(kv, "proof_status"));
    formula.candidate.implementation_notes = get(kv, "implementation_notes");

    formula.bs_spec.id = formula.candidate.id;
    formula.bs_spec.p_factors = parse_factors(get(kv, "p_factors"));
    formula.bs_spec.q_factors = parse_factors(get(kv, "q_factors"));
    formula.bs_spec.q_constant = parse_ulong(get(kv, "q_constant"), 1ul);
    formula.bs_spec.linear_a = parse_long(get(kv, "linear_a"));
    formula.bs_spec.linear_b = parse_long(get(kv, "linear_b"));
    formula.bs_spec.alternating = to_bool(get(kv, "alternating"));
    formula.bs_spec.unit_first_p = to_bool(get(kv, "unit_first_p"));
    formula.bs_spec.unit_first_q = to_bool(get(kv, "unit_first_q"));
    formula.bs_spec.leaf_t_uses_q = get(kv, "leaf_t_multiplier", "p") == "q";
    formula.bs_spec.gcd_cancellation = to_bool(get(kv, "gcd_cancellation"));
    formula.constant_expression = get(kv, "constant_expression");

    if (!formula.candidate.has_required_metadata()) {
        throw std::runtime_error("formula spec missing required metadata: " + path);
    }
    return formula;
}

std::vector<FormulaSpec> load_formula_specs(const std::string &directory) {
    std::vector<FormulaSpec> formulas;
    if (!std::filesystem::exists(directory)) {
        return formulas;
    }
    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".formula") {
            formulas.push_back(load_formula_spec(entry.path().string()));
        }
    }
    std::sort(formulas.begin(), formulas.end(),
              [](const FormulaSpec &a, const FormulaSpec &b) {
                  return a.candidate.id < b.candidate.id;
              });
    return formulas;
}

std::string formula_score_report(const std::vector<FormulaSpec> &formulas, int decimal_digits) {
    std::ostringstream out;
    out << "# SATO-X Candidate Score Report\n\n";
    out << "Target digits: `" << decimal_digits << "`\n\n";
    out << "| Candidate | Family | D | Invariant | Proof | BS-ready | Digits/term | Score | "
           "Estimated multiplies | Decision |\n";
    out << "|---|---|---:|---|---|---:|---:|---:|---:|---|\n";
    std::vector<FormulaSpec> ranked = formulas;
    std::sort(ranked.begin(), ranked.end(), [decimal_digits](const FormulaSpec &a,
                                                             const FormulaSpec &b) {
        return score_candidate(a.candidate, decimal_digits).score >
               score_candidate(b.candidate, decimal_digits).score;
    });

    for (const FormulaSpec &formula : ranked) {
        const CandidateScore score = score_candidate(formula.candidate, decimal_digits);
        out << "| `" << formula.candidate.id << "` | " << formula.candidate.family << " | "
            << formula.candidate.discriminant << " | `" << formula.candidate.class_invariant
            << "` | " << proof_status_to_string(formula.candidate.proof_status) << " | "
            << (formula.candidate.binary_splitting_ready ? "yes" : "no") << " | "
            << formula.candidate.estimated_digits_per_term << " | " << score.score << " | "
            << score.multiplication_estimate << " | " << score.decision << " |\n";
    }
    return out.str();
}

} // namespace satox
