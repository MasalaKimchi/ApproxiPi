#include "satox/benchmark.hpp"

#include "satox/bbp.hpp"
#include "satox/formula_spec.hpp"
#include "satox/format.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace satox {

std::vector<std::unique_ptr<PiAlgorithm>> make_default_algorithms() {
    std::vector<std::unique_ptr<PiAlgorithm>> algorithms;
    algorithms.push_back(make_chudnovsky_algorithm());
    algorithms.push_back(make_chudnovsky_valuation_algorithm());
    algorithms.push_back(make_ramanujan_algorithm());
    algorithms.push_back(make_machin_algorithm());
    algorithms.push_back(make_agm_algorithm());
    algorithms.push_back(make_borwein_cubic_algorithm());
    algorithms.push_back(make_borwein_quartic_algorithm());
    return algorithms;
}

std::string csv_header() {
    return "algorithm,family,digits,guard_digits,trials,supported,verified,wall_ms,min_wall_ms,"
           "max_wall_ms,stddev_wall_ms,cpu_ms,split_ms,finalize_ms,format_ms,verify_ms,"
           "terms_or_iterations,estimated_digits_per_term,gcd_reductions,cancelled_bits,"
           "max_operand_bits,parallel_depth,verification_method,"
           "baseline,relative_wall_time,prefix_hash,error\n";
}

std::string result_to_csv(const ComputeResult &result, const std::string &baseline_name,
                          double baseline_wall_ms) {
    const double relative =
        (baseline_wall_ms > 0.0 && result.wall_ms > 0.0) ? result.wall_ms / baseline_wall_ms : 0.0;
    std::ostringstream out;
    out << result.metadata.name << ',' << '"' << result.metadata.family << '"' << ','
        << result.decimal_digits << ',' << result.guard_digits << ',' << 1 << ','
        << (result.supported ? "true" : "false") << ',' << (result.verified ? "true" : "false")
        << ',' << std::fixed << std::setprecision(3) << result.wall_ms << ','
        << result.wall_ms << ',' << result.wall_ms << ',' << 0.0 << ',' << result.cpu_ms << ','
        << result.split_ms << ',' << result.finalize_ms << ',' << result.format_ms << ','
        << result.verify_ms << ',' << result.terms_or_iterations << ',' << std::setprecision(6)
        << result.estimated_digits_per_term << ',' << result.gcd_reductions << ','
        << std::setprecision(3) << result.cancelled_bits << ',' << result.max_operand_bits << ','
        << result.parallel_depth << ',' << '"'
        << result.verification_method << '"'
        << ',' << baseline_name << ',' << std::setprecision(6) << relative << ','
        << short_hash(result.decimal_prefix) << ',' << '"' << result.error << '"' << '\n';
    return out.str();
}

namespace {

struct ResultStats {
    ComputeResult representative;
    int trials = 0;
    double median_wall_ms = 0.0;
    double min_wall_ms = 0.0;
    double max_wall_ms = 0.0;
    double stddev_wall_ms = 0.0;
    double median_cpu_ms = 0.0;
    double median_split_ms = 0.0;
    double median_finalize_ms = 0.0;
    double median_format_ms = 0.0;
    double median_verify_ms = 0.0;
};

double median(std::vector<double> values) {
    if (values.empty()) {
        return 0.0;
    }
    std::sort(values.begin(), values.end());
    const size_t mid = values.size() / 2;
    if ((values.size() & 1u) != 0u) {
        return values[mid];
    }
    return (values[mid - 1] + values[mid]) / 2.0;
}

double stddev(const std::vector<double> &values) {
    if (values.size() < 2) {
        return 0.0;
    }
    const double mean = std::accumulate(values.begin(), values.end(), 0.0) /
                        static_cast<double>(values.size());
    double accum = 0.0;
    for (double value : values) {
        const double delta = value - mean;
        accum += delta * delta;
    }
    return std::sqrt(accum / static_cast<double>(values.size() - 1));
}

ResultStats summarize_results(std::vector<ComputeResult> results) {
    ResultStats stats;
    stats.trials = static_cast<int>(results.size());
    if (results.empty()) {
        return stats;
    }
    stats.representative = results.front();
    std::vector<double> wall;
    std::vector<double> cpu;
    std::vector<double> split;
    std::vector<double> finalize;
    std::vector<double> format;
    std::vector<double> verify;
    for (const ComputeResult &result : results) {
        wall.push_back(result.wall_ms);
        cpu.push_back(result.cpu_ms);
        split.push_back(result.split_ms);
        finalize.push_back(result.finalize_ms);
        format.push_back(result.format_ms);
        verify.push_back(result.verify_ms);
        if (result.verified && (!stats.representative.verified ||
                                result.wall_ms < stats.representative.wall_ms)) {
            stats.representative = result;
        }
    }
    stats.median_wall_ms = median(wall);
    stats.min_wall_ms = *std::min_element(wall.begin(), wall.end());
    stats.max_wall_ms = *std::max_element(wall.begin(), wall.end());
    stats.stddev_wall_ms = stddev(wall);
    stats.median_cpu_ms = median(cpu);
    stats.median_split_ms = median(split);
    stats.median_finalize_ms = median(finalize);
    stats.median_format_ms = median(format);
    stats.median_verify_ms = median(verify);
    stats.representative.wall_ms = stats.median_wall_ms;
    stats.representative.cpu_ms = stats.median_cpu_ms;
    stats.representative.split_ms = stats.median_split_ms;
    stats.representative.finalize_ms = stats.median_finalize_ms;
    stats.representative.format_ms = stats.median_format_ms;
    stats.representative.verify_ms = stats.median_verify_ms;
    return stats;
}

std::string stats_to_csv(const ResultStats &stats, const std::string &baseline_name,
                         double baseline_wall_ms) {
    const ComputeResult &result = stats.representative;
    const double relative =
        (baseline_wall_ms > 0.0 && stats.median_wall_ms > 0.0)
            ? stats.median_wall_ms / baseline_wall_ms
            : 0.0;
    std::ostringstream out;
    out << result.metadata.name << ',' << '"' << result.metadata.family << '"' << ','
        << result.decimal_digits << ',' << result.guard_digits << ',' << stats.trials << ','
        << (result.supported ? "true" : "false") << ','
        << (result.verified ? "true" : "false") << ',' << std::fixed
        << std::setprecision(3) << stats.median_wall_ms << ',' << stats.min_wall_ms << ','
        << stats.max_wall_ms << ',' << stats.stddev_wall_ms << ',' << stats.median_cpu_ms << ','
        << stats.median_split_ms << ',' << stats.median_finalize_ms << ','
        << stats.median_format_ms << ',' << stats.median_verify_ms << ','
        << result.terms_or_iterations << ',' << std::setprecision(6)
        << result.estimated_digits_per_term << ',' << result.gcd_reductions << ','
        << std::setprecision(3) << result.cancelled_bits << ',' << result.max_operand_bits << ','
        << result.parallel_depth << ',' << '"'
        << result.verification_method << '"'
        << ',' << baseline_name << ',' << std::setprecision(6) << relative << ','
        << short_hash(result.decimal_prefix) << ',' << '"' << result.error << '"' << '\n';
    return out.str();
}

} // namespace

std::string result_to_json(const ComputeResult &result, const std::string &baseline_name,
                           double baseline_wall_ms) {
    const double relative =
        (baseline_wall_ms > 0.0 && result.wall_ms > 0.0) ? result.wall_ms / baseline_wall_ms : 0.0;
    std::ostringstream out;
    out << "{"
        << "\"algorithm\":\"" << escape_json(result.metadata.name) << "\","
        << "\"family\":\"" << escape_json(result.metadata.family) << "\","
        << "\"digits\":" << result.decimal_digits << ','
        << "\"guard_digits\":" << result.guard_digits << ','
        << "\"supported\":" << (result.supported ? "true" : "false") << ','
        << "\"verified\":" << (result.verified ? "true" : "false") << ','
        << "\"wall_ms\":" << std::fixed << std::setprecision(3) << result.wall_ms << ','
        << "\"cpu_ms\":" << result.cpu_ms << ','
        << "\"split_ms\":" << result.split_ms << ','
        << "\"finalize_ms\":" << result.finalize_ms << ','
        << "\"format_ms\":" << result.format_ms << ','
        << "\"verify_ms\":" << result.verify_ms << ','
        << "\"terms_or_iterations\":" << result.terms_or_iterations << ','
        << "\"estimated_digits_per_term\":" << std::setprecision(6)
        << result.estimated_digits_per_term << ','
        << "\"gcd_reductions\":" << result.gcd_reductions << ','
        << "\"cancelled_bits\":" << result.cancelled_bits << ','
        << "\"max_operand_bits\":" << result.max_operand_bits << ','
        << "\"parallel_depth\":" << result.parallel_depth << ','
        << "\"verification_method\":\"" << escape_json(result.verification_method) << "\","
        << "\"baseline\":\"" << escape_json(baseline_name) << "\","
        << "\"relative_wall_time\":" << relative << ','
        << "\"prefix_hash\":\"" << short_hash(result.decimal_prefix) << "\","
        << "\"error\":\"" << escape_json(result.error) << "\""
        << "}";
    return out.str();
}

int run_benchmark(const BenchmarkOptions &options) {
    std::filesystem::create_directories(options.output_dir);

    std::ofstream csv(options.output_dir + "/benchmark.csv");
    std::ofstream json(options.output_dir + "/benchmark.json");
    std::ofstream md(options.output_dir + "/summary.md");
    if (!csv || !json || !md) {
        throw std::runtime_error("could not open benchmark output files");
    }

    csv << csv_header();
    json << "[\n";
    md << "# SATO-X Benchmark Summary\n\n";
    md << "Guard digits: `" << options.guard_digits << "`\n\n";
    md << "Trials per row: `" << options.trials << "`; warmups: `" << options.warmups << "`\n\n";
    md << "Optimization notes: shared binary splitting uses bounded parallel subtree "
          "evaluation, an `mpz_addmul` merge to avoid one temporary large-integer "
          "product per internal node, small 8-term leaf blocks before recursion, "
          "opt-in leaf valuation cancellation, and `log10(396^4 / 256)` for "
          "Ramanujan term-count estimation. Phase "
          "columns expose split/finalize/format/verify bottlenecks.\n\n";
    md << "| Digits | Algorithm | Supported | Verified | Median wall ms | Split | Finalize | Format | Verify | Terms/iterations | Max operand bits | Parallel depth | "
          "Relative to Chudnovsky | Notes |\n";
    md << "|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|\n";

    bool first_json = true;
    std::vector<std::unique_ptr<PiAlgorithm>> algorithms = make_default_algorithms();

    for (int digits : options.digits) {
        double baseline_wall_ms = 0.0;
        std::string baseline_name = "chudnovsky_bs";
        std::vector<ResultStats> results;

        for (const auto &algorithm : algorithms) {
            for (int i = 0; i < options.warmups; ++i) {
                (void)algorithm->compute(digits, options.guard_digits);
            }

            std::vector<ComputeResult> trials;
            for (int i = 0; i < std::max(1, options.trials); ++i) {
                trials.push_back(algorithm->compute(digits, options.guard_digits));
            }
            ResultStats stats = summarize_results(std::move(trials));
            if (stats.representative.metadata.name == baseline_name && stats.representative.verified) {
                baseline_wall_ms = stats.median_wall_ms;
            }
            results.push_back(std::move(stats));
        }

        for (const ResultStats &stats : results) {
            const ComputeResult &result = stats.representative;
            csv << stats_to_csv(stats, baseline_name, baseline_wall_ms);
            if (!first_json) {
                json << ",\n";
            }
            first_json = false;
            json << "  " << result_to_json(result, baseline_name, baseline_wall_ms);

            const double relative =
                (baseline_wall_ms > 0.0 && result.wall_ms > 0.0)
                    ? result.wall_ms / baseline_wall_ms
                    : 0.0;
            md << "| " << digits << " | `" << result.metadata.name << "` | "
               << (result.supported ? "yes" : "no") << " | "
               << (result.verified ? "yes" : "no") << " | " << std::fixed
               << std::setprecision(3) << stats.median_wall_ms << " | "
               << stats.median_split_ms << " | " << stats.median_finalize_ms << " | "
               << stats.median_format_ms << " | " << stats.median_verify_ms << " | "
               << result.terms_or_iterations << " | " << result.max_operand_bits << " | "
               << result.parallel_depth << " | "
               << std::setprecision(3) << relative
               << " | " << (result.error.empty() ? "" : result.error) << " |\n";
        }
    }

    const std::vector<BbpCheck> bbp_checks = default_bbp_checks();
    md << "\n## BBP Verification Spots\n\n";
    md << "| Hex offset | 8 hex digits |\n";
    md << "|---:|---|\n";
    for (const BbpCheck &check : bbp_checks) {
        md << "| " << check.offset << " | `" << check.hex_digits << "` |\n";
    }

    if (!options.candidate_file.empty()) {
        md << "\n## Candidate Formula Metadata\n\n";
        const std::vector<CandidateFormula> candidates =
            load_candidate_formulas(options.candidate_file);
        md << "| Candidate | Family | D | Invariant | Proof | Digits/term | Score | "
              "Estimated multiplies @100k | Decision |\n";
        md << "|---|---|---:|---|---|---:|---:|---:|---|\n";
        for (const CandidateFormula &candidate : candidates) {
            const CandidateScore score = score_candidate(candidate, 100000);
            md << "| `" << candidate.id << "` | " << candidate.family << " | "
               << candidate.discriminant << " | `" << candidate.class_invariant << "` | "
               << proof_status_to_string(candidate.proof_status) << " | "
               << candidate.estimated_digits_per_term << " | " << std::fixed
               << std::setprecision(3) << score.score << " | "
               << std::setprecision(0) << score.multiplication_estimate << " | "
               << score.decision << " |\n";
        }
    }

    const std::vector<FormulaSpec> formula_specs = load_formula_specs(options.formula_dir);
    if (!formula_specs.empty()) {
        const std::string report = formula_score_report(formula_specs, 100000);
        std::ofstream score_file(options.output_dir + "/satox-score.md");
        if (!score_file) {
            throw std::runtime_error("could not open satox-score output file");
        }
        score_file << report;
        md << "\n## Formula Spec Score Report\n\n";
        md << "Wrote `" << options.output_dir << "/satox-score.md` from `"
           << options.formula_dir << "/*.formula`.\n";
    }

    md << "\nNo SATO-X candidate is considered faster unless it is benchmarked, verified, and "
          "compared against the same Chudnovsky baseline.\n";
    json << "\n]\n";

    std::cout << "Wrote " << options.output_dir << "/benchmark.csv, benchmark.json, summary.md\n";
    return 0;
}

} // namespace satox
