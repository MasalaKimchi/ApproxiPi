#include "satox/checkpoint.hpp"

#include "satox/resource_monitor.hpp"
#include "satox/run_config.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace satox {
namespace {

std::string checkpoint_dir(const std::string &run_id) {
    return "results/checkpoints/" + run_id;
}

} // namespace

CheckpointWriter::CheckpointWriter(std::string run_id) : run_id_(std::move(run_id)) {
    if (global_run_config().storage_backend == StorageBackend::Memory) {
        return;
    }
    std::filesystem::create_directories(checkpoint_dir(run_id_));
}

void CheckpointWriter::write_chunk(unsigned long chunk_index, const std::string &payload) {
    if (!global_run_config().enable_checkpoint ||
        global_run_config().storage_backend == StorageBackend::Memory) {
        return;
    }
    const std::string path =
        checkpoint_dir(run_id_) + "/chunk_" + std::to_string(chunk_index) + ".bin";
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("checkpoint write failed: " + path);
    }
    out.write(payload.data(), static_cast<std::streamsize>(payload.size()));
    bytes_written_ += payload.size();
    add_io_bytes(0, payload.size());
}

} // namespace satox
