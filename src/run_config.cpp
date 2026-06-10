#include "satox/run_config.hpp"

#include <sstream>
#include <stdexcept>

namespace satox {

RunConfig &global_run_config() {
    static RunConfig config;
    return config;
}

void apply_ablation(const std::string &ablation) {
    RunConfig &cfg = global_run_config();
    cfg.ablation_tag = ablation;

    if (ablation == "no_binary_split") {
        cfg.split_mode_override = SplitMode::BlockedLeaf;
        cfg.split_mode_set = true;
        return;
    }
    if (ablation == "no_gcd") {
        cfg.disable_gcd = true;
        return;
    }
    if (ablation == "no_checkpoint") {
        cfg.enable_checkpoint = false;
        return;
    }
    if (ablation == "no_residues") {
        cfg.enable_residues = false;
        return;
    }
    if (ablation == "storage_memory") {
        cfg.storage_backend = StorageBackend::Memory;
        return;
    }
    if (ablation == "storage_mmap") {
        cfg.storage_backend = StorageBackend::Mmap;
        return;
    }
    if (ablation == "storage_file") {
        cfg.storage_backend = StorageBackend::File;
        return;
    }
    if (ablation.rfind("leaf_block_", 0) == 0) {
        cfg.leaf_block_override = std::stoi(ablation.substr(11));
        return;
    }
    if (ablation.rfind("threads_", 0) == 0) {
        cfg.threads_override = std::stoi(ablation.substr(8));
        return;
    }
    throw std::invalid_argument("unknown ablation: " + ablation);
}

} // namespace satox
