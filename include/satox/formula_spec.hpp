#pragma once

#include "satox/binary_splitting.hpp"
#include "satox/candidate.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace satox {

struct FormulaSpec {
    CandidateFormula candidate;
    HypergeometricBsSpec bs_spec;
    std::string constant_expression;
};

FormulaSpec load_formula_spec(const std::string &path);
std::vector<FormulaSpec> load_formula_specs(const std::string &directory);
std::string formula_score_report(const std::vector<FormulaSpec> &formulas, int decimal_digits);

} // namespace satox
