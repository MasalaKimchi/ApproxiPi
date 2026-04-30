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
};

int run_benchmark(const BenchmarkOptions &options);

std::string csv_header();
std::string result_to_csv(const ComputeResult &result, const std::string &baseline_name,
                          double baseline_wall_ms);
std::string result_to_json(const ComputeResult &result, const std::string &baseline_name,
                           double baseline_wall_ms);

} // namespace satox
