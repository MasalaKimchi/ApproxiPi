#pragma once

#include <cstdint>
#include <string>

namespace satox {

class CheckpointWriter {
  public:
    explicit CheckpointWriter(std::string run_id);
    void write_chunk(unsigned long chunk_index, const std::string &payload);

  private:
    std::string run_id_;
    std::uint64_t bytes_written_ = 0;
};

} // namespace satox
