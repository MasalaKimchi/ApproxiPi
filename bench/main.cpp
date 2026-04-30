#include "satox/benchmark.hpp"

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

void usage(const char *program) {
    std::cerr << "Usage: " << program
              << " [--digits 1000,10000,100000,1000000] [--guard 25]"
                 " [--trials 3] [--warmups 0] [--out results]"
                 " [--candidates formulas/candidates.tsv] [--formula-dir candidates]\n";
}

} // namespace

int main(int argc, char **argv) {
    satox::BenchmarkOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--digits" && i + 1 < argc) {
            options.digits = parse_digits(argv[++i]);
        } else if (arg == "--guard" && i + 1 < argc) {
            options.guard_digits = std::stoi(argv[++i]);
        } else if (arg == "--trials" && i + 1 < argc) {
            options.trials = std::stoi(argv[++i]);
        } else if (arg == "--warmups" && i + 1 < argc) {
            options.warmups = std::stoi(argv[++i]);
        } else if (arg == "--out" && i + 1 < argc) {
            options.output_dir = argv[++i];
        } else if (arg == "--candidates" && i + 1 < argc) {
            options.candidate_file = argv[++i];
        } else if (arg == "--formula-dir" && i + 1 < argc) {
            options.formula_dir = argv[++i];
        } else if (arg == "--help") {
            usage(argv[0]);
            return 0;
        } else {
            usage(argv[0]);
            return 2;
        }
    }

    try {
        return satox::run_benchmark(options);
    } catch (const std::exception &ex) {
        std::cerr << "satox-bench: " << ex.what() << '\n';
        return 1;
    }
}
