#include "satox/benchmark.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::vector<int> parse_digits(const std::string &text) {
    std::vector<int> digits;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, ',')) {
        digits.push_back(std::stoi(item));
    }
    return digits;
}

std::vector<std::string> parse_names(const std::string &text) {
    std::vector<std::string> names;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            names.push_back(item);
        }
    }
    return names;
}

void usage(const char *program) {
    std::cerr << "Usage: " << program
              << " [--digits 100000,1000000,10000000,100000000] [--guard 25]"
                 " [--trials 3] [--warmups 0] [--timeout-sec 0] [--out results]"
                 " [--candidates formulas/candidates.tsv] [--formula-dir formulas/specs]"
                 " [--algorithms name1,name2] [--ablation name]"
                 " [--electricity-usd-per-kwh 0.12] [--instance-usd-per-hour 0]"
                 " [--no-energy] [--skip-memory-guard] [--merge]\n"
              << "       " << program
              << " --tune [--digits 1000000] [--guard 25] [--trials 3] [--passes 2]"
                 " [--out results]\n";
}

} // namespace

int main(int argc, char **argv) {
    satox::BenchmarkOptions options;
    bool tune = false;
    int tune_passes = 2;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--tune") {
            tune = true;
        } else if (arg == "--passes" && i + 1 < argc) {
            tune_passes = std::stoi(argv[++i]);
        } else if (arg == "--digits" && i + 1 < argc) {
            options.digits = parse_digits(argv[++i]);
        } else if (arg == "--guard" && i + 1 < argc) {
            options.guard_digits = std::stoi(argv[++i]);
        } else if (arg == "--trials" && i + 1 < argc) {
            options.trials = std::stoi(argv[++i]);
        } else if (arg == "--warmups" && i + 1 < argc) {
            options.warmups = std::stoi(argv[++i]);
        } else if (arg == "--timeout-sec" && i + 1 < argc) {
            options.timeout_sec = std::stoi(argv[++i]);
        } else if (arg == "--out" && i + 1 < argc) {
            options.output_dir = argv[++i];
        } else if (arg == "--candidates" && i + 1 < argc) {
            options.candidate_file = argv[++i];
        } else if (arg == "--formula-dir" && i + 1 < argc) {
            options.formula_dir = argv[++i];
        } else if (arg == "--algorithms" && i + 1 < argc) {
            options.algorithms = parse_names(argv[++i]);
        } else if (arg == "--ablation" && i + 1 < argc) {
            options.ablation = argv[++i];
        } else if (arg == "--electricity-usd-per-kwh" && i + 1 < argc) {
            options.electricity_usd_per_kwh = std::stod(argv[++i]);
        } else if (arg == "--instance-usd-per-hour" && i + 1 < argc) {
            options.instance_usd_per_hour = std::stod(argv[++i]);
        } else if (arg == "--no-energy") {
            options.measure_energy = false;
        } else if (arg == "--skip-memory-guard") {
            options.skip_memory_guard = true;
        } else if (arg == "--merge") {
            options.merge = true;
        } else if (arg == "--help") {
            usage(argv[0]);
            return 0;
        } else {
            usage(argv[0]);
            return 2;
        }
    }

    try {
        if (tune) {
            satox::TunerOptions tuner_options;
            tuner_options.digits =
                options.digits.empty()
                    ? 1000000
                    : *std::max_element(options.digits.begin(), options.digits.end());
            tuner_options.guard_digits = options.guard_digits;
            tuner_options.trials = options.trials;
            tuner_options.passes = tune_passes;
            tuner_options.output_dir = options.output_dir;
            return satox::run_tuner(tuner_options);
        }
        return satox::run_benchmark(options);
    } catch (const std::exception &ex) {
        std::cerr << "satox-bench: " << ex.what() << '\n';
        return 1;
    }
}
