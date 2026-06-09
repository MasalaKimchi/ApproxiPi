#pragma once

#include "satox/algorithm.hpp"
#include "satox/candidate.hpp"

#include <string>
#include <vector>

namespace satox {

struct BenchmarkOptions {
    std::vector<int> digits = {1000, 10000, 100000, 1000000};
    int guard_digits = 25;
    int trials = 3;
    int warmups = 0;
    std::string output_dir = "results";
    std::string candidate_file;
    std::string formula_dir = "candidates";
    // Empty means "run every default algorithm"; otherwise only the named ones.
    std::vector<std::string> algorithms;
};

int run_benchmark(const BenchmarkOptions &options);

struct TunerOptions {
    int digits = 1000000;
    int guard_digits = 25;
    int trials = 3;
    int passes = 2;
    std::string output_dir = "results";
};

// H11 autotuner: coordinate descent over the crown knob space; caches the
// winning profile to <output_dir>/tuning.json and logs every evaluation.
int run_tuner(const TunerOptions &options);

std::string csv_header();
std::string result_to_csv(const ComputeResult &result, const std::string &baseline_name,
                          double baseline_wall_ms);
std::string result_to_json(const ComputeResult &result, const std::string &baseline_name,
                           double baseline_wall_ms);

} // namespace satox
