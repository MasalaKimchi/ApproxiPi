#pragma once

#include "satox/algorithm.hpp"
#include "satox/candidate.hpp"

#include <string>
#include <vector>

namespace satox {

struct BenchmarkOptions {
    std::vector<int> digits = {100000, 1000000, 10000000, 100000000};
    int guard_digits = 25;
    int trials = 3;
    int warmups = 0;
    int timeout_sec = 0;
    std::string output_dir = "results";
    std::string candidate_file;
    std::string formula_dir = "candidates";
    std::vector<std::string> algorithms;
    std::string ablation;
    double electricity_usd_per_kwh = 0.12;
    double instance_usd_per_hour = 0.0;
    bool measure_energy = true;
    bool skip_memory_guard = false;
    // When true, merge this run into existing results/benchmark.csv instead of replacing it.
    bool merge = false;
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
