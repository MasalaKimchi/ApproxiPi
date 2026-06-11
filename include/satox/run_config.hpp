#pragma once

#include <string>

namespace satox {

enum class StorageBackend { Memory, Mmap, File };

enum class SplitMode { Binary, BlockedLeaf };

struct RunConfig {
    int timeout_sec = 0;
    int leaf_block_override = 0;
    int threads_override = -1;
    bool enable_checkpoint = true;
    bool enable_residues = true;
    bool disable_gcd = false;
    SplitMode split_mode_override = SplitMode::Binary;
    bool split_mode_set = false;
    StorageBackend storage_backend = StorageBackend::Memory;
    std::string ablation_tag;
    double electricity_usd_per_kwh = 0.12;
    double instance_usd_per_hour = 0.0;
    bool measure_energy = true;
    bool skip_memory_guard = false;
    std::string scratch_dir = "/tmp/satox";
};

RunConfig &global_run_config();
void apply_ablation(const std::string &ablation);

} // namespace satox
