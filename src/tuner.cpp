// H11: autotuning the crown knob space, with the winning profile cached to
// disk (results/tuning.json) so later runs and the `chudnovsky_bs_crown_tuned`
// benchmark variant reuse it. Coordinate descent over one knob at a time,
// two passes, median-of-k wall time as the objective; an evaluation only
// counts if the exact-prefix verification passes.

#include "satox/benchmark.hpp"

#include "satox/crown.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace satox {
namespace {

struct KnobChoice {
    std::string name;
    std::vector<long> values;
    void (*apply)(CrownTuning &, long);
    long (*get)(const CrownTuning &);
};

const std::vector<KnobChoice> &knob_space() {
    static const std::vector<KnobChoice> knobs = {
        {"leaf_block_terms",
         {4, 8, 16, 32},
         [](CrownTuning &t, long v) { t.leaf_block_terms = static_cast<unsigned long>(v); },
         [](const CrownTuning &t) { return static_cast<long>(t.leaf_block_terms); }},
        {"min_chunk_terms",
         {128, 256, 512},
         [](CrownTuning &t, long v) { t.min_chunk_terms = static_cast<unsigned long>(v); },
         [](const CrownTuning &t) { return static_cast<long>(t.min_chunk_terms); }},
        {"max_crown_depth",
         {6, 7, 8, 9},
         [](CrownTuning &t, long v) { t.max_crown_depth = static_cast<unsigned int>(v); },
         [](const CrownTuning &t) { return static_cast<long>(t.max_crown_depth); }},
        {"max_parallel_levels",
         {3, 4, 5, 6},
         [](CrownTuning &t, long v) { t.max_parallel_levels = static_cast<unsigned int>(v); },
         [](const CrownTuning &t) { return static_cast<long>(t.max_parallel_levels); }},
        {"intra_node_parallel_bits",
         {1l << 18, 1l << 19, 1l << 20},
         [](CrownTuning &t, long v) { t.intra_node_parallel_bits = v; },
         [](const CrownTuning &t) { return static_cast<long>(t.intra_node_parallel_bits); }},
        {"root_split_sixteenths",
         {8, 9, 10, 11},
         [](CrownTuning &t, long v) {
             t.root_split_sixteenths = static_cast<unsigned int>(v);
         },
         [](const CrownTuning &t) { return static_cast<long>(t.root_split_sixteenths); }},
    };
    return knobs;
}

double median_of(std::vector<double> values) {
    std::sort(values.begin(), values.end());
    const size_t mid = values.size() / 2;
    if ((values.size() & 1u) != 0u) {
        return values[mid];
    }
    return (values[mid - 1] + values[mid]) / 2.0;
}

// Evaluates a tuning candidate through the exact code path the tuned
// benchmark variant uses: profile on disk, algorithm loads it per compute.
double evaluate_tuning(const CrownTuning &tuning, const std::string &tuning_path, int digits,
                       int guard_digits, int trials) {
    if (!save_crown_tuning(tuning_path, tuning)) {
        return std::numeric_limits<double>::infinity();
    }
    const std::unique_ptr<PiAlgorithm> algorithm = make_chudnovsky_crown_tuned_algorithm();
    std::vector<double> wall;
    for (int i = 0; i < std::max(1, trials); ++i) {
        const ComputeResult result = algorithm->compute(digits, guard_digits);
        if (!result.supported || !result.verified) {
            return std::numeric_limits<double>::infinity();
        }
        wall.push_back(result.wall_ms);
    }
    return median_of(std::move(wall));
}

std::string tuning_to_string(const CrownTuning &tuning) {
    std::string out;
    for (const KnobChoice &knob : knob_space()) {
        out += knob.name + "=" + std::to_string(knob.get(tuning)) + " ";
    }
    return out;
}

} // namespace

int run_tuner(const TunerOptions &options) {
    std::filesystem::create_directories(options.output_dir);
    const std::string tuning_path = options.output_dir + "/tuning.json";
    const std::string log_path = options.output_dir + "/tuning-log.csv";

    std::ofstream log(log_path);
    log << "pass,knob,value,median_wall_ms,accepted\n";

    CrownTuning best;
    std::cout << "Tuning crown knobs at " << options.digits << " digits, " << options.trials
              << " trials per evaluation\n";
    std::cout << "  defaults: " << tuning_to_string(best) << "\n";
    double best_wall =
        evaluate_tuning(best, tuning_path, options.digits, options.guard_digits,
                        options.trials);
    const double default_wall = best_wall;
    std::cout << "  default median wall: " << best_wall << " ms\n";
    log << "0,defaults," << 0 << ',' << best_wall << ",true\n";

    for (int pass = 1; pass <= options.passes; ++pass) {
        bool improved = false;
        for (const KnobChoice &knob : knob_space()) {
            for (long value : knob.values) {
                if (value == knob.get(best)) {
                    continue;
                }
                CrownTuning candidate = best;
                knob.apply(candidate, value);
                const double wall = evaluate_tuning(candidate, tuning_path, options.digits,
                                                    options.guard_digits, options.trials);
                const bool accept = wall < best_wall;
                std::cout << "  pass " << pass << " " << knob.name << "=" << value << ": "
                          << wall << " ms" << (accept ? "  <-- accepted" : "") << "\n";
                log << pass << ',' << knob.name << ',' << value << ',' << wall << ','
                    << (accept ? "true" : "false") << "\n";
                if (accept) {
                    best = candidate;
                    best_wall = wall;
                    improved = true;
                }
            }
        }
        if (!improved) {
            break;
        }
    }

    save_crown_tuning(tuning_path, best);
    std::cout << "Best profile: " << tuning_to_string(best) << "\n";
    std::cout << "Median wall: " << default_wall << " -> " << best_wall << " ms ("
              << (default_wall > 0 ? 100.0 * (default_wall - best_wall) / default_wall : 0.0)
              << "% change). Wrote " << tuning_path << " and " << log_path << "\n";
    return 0;
}

} // namespace satox
