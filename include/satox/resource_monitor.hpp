#pragma once

#include <cstdint>
#include <string>

namespace satox {

struct ResourceSnapshot {
    std::uint64_t peak_rss_bytes = 0;
    std::uint64_t bytes_read = 0;
    std::uint64_t bytes_written = 0;
    std::uint64_t alloc_count = 0;
    double energy_joules = 0.0;
    double mean_power_watts = 0.0;
    std::string energy_backend;
};

class ResourceMonitor {
  public:
    ResourceMonitor();
    ~ResourceMonitor();

    void sample();
    ResourceSnapshot finish(double wall_seconds);

  private:
    std::uint64_t baseline_rss_ = 0;
    std::uint64_t peak_rss_ = 0;
    std::uint64_t baseline_read_ = 0;
    std::uint64_t baseline_write_ = 0;
    std::uint64_t extra_read_ = 0;
    std::uint64_t extra_write_ = 0;
};

std::uint64_t current_rss_bytes();
std::uint64_t system_total_bytes();
std::string host_manifest_json(double electricity_usd_per_kwh, double instance_usd_per_hour);

void add_io_bytes(std::uint64_t read_delta, std::uint64_t write_delta);

} // namespace satox
