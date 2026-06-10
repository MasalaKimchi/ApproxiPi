#include "satox/resource_monitor.hpp"

#include "satox/run_config.hpp"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <sys/resource.h>
#include <thread>
#include <unistd.h>

#if defined(__APPLE__)
#include <mach/mach.h>
#include <sys/sysctl.h>
#endif

namespace satox {
namespace {

std::uint64_t g_extra_read = 0;
std::uint64_t g_extra_write = 0;

std::uint64_t ru_block_bytes() {
    struct rusage usage {};
    if (getrusage(RUSAGE_SELF, &usage) != 0) {
        return 0;
    }
    constexpr std::uint64_t kBlockSize = 512;
    return static_cast<std::uint64_t>(usage.ru_inblock + usage.ru_oublock) * kBlockSize;
}

#if defined(__APPLE__)
std::uint64_t task_rss_bytes() {
    mach_task_basic_info info {};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                  reinterpret_cast<task_info_t>(&info), &count) != KERN_SUCCESS) {
        return 0;
    }
    return static_cast<std::uint64_t>(info.resident_size);
}
#else
std::uint64_t task_rss_bytes() {
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            std::uint64_t kb = 0;
            std::sscanf(line.c_str(), "VmRSS: %llu kB", &kb);
            return kb * 1024ull;
        }
    }
    struct rusage usage {};
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return static_cast<std::uint64_t>(usage.ru_maxrss) * 1024ull;
    }
    return 0;
}
#endif

double read_rapl_joules() {
#if defined(__linux__)
    std::ifstream energy("/sys/class/powercap/intel-rapl:0/energy_uj");
    if (!energy) {
        return 0.0;
    }
    std::uint64_t microjoules = 0;
    energy >> microjoules;
    return static_cast<double>(microjoules) / 1e6;
#else
    return 0.0;
#endif
}

} // namespace

void add_io_bytes(std::uint64_t read_delta, std::uint64_t write_delta) {
    g_extra_read += read_delta;
    g_extra_write += write_delta;
}

std::uint64_t current_rss_bytes() {
    return task_rss_bytes();
}

std::uint64_t system_total_bytes() {
#if defined(__APPLE__)
    std::int64_t mem = 0;
    size_t len = sizeof(mem);
    if (sysctlbyname("hw.memsize", &mem, &len, nullptr, 0) == 0) {
        return static_cast<std::uint64_t>(mem);
    }
#else
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.rfind("MemTotal:", 0) == 0) {
            std::uint64_t kb = 0;
            std::sscanf(line.c_str(), "MemTotal: %llu kB", &kb);
            return kb * 1024ull;
        }
    }
#endif
    return 0;
}

std::string host_manifest_json(double electricity_usd_per_kwh, double instance_usd_per_hour) {
    char hostname[256] = "unknown";
    gethostname(hostname, sizeof(hostname));
    std::ostringstream out;
    out << "{\n"
        << "  \"hostname\": \"" << hostname << "\",\n"
        << "  \"total_ram_bytes\": " << system_total_bytes() << ",\n"
        << "  \"electricity_usd_per_kwh\": " << electricity_usd_per_kwh << ",\n"
        << "  \"instance_usd_per_hour\": " << instance_usd_per_hour << ",\n"
        << "  \"hardware_threads\": " << std::thread::hardware_concurrency() << "\n"
        << "}";
    return out.str();
}

ResourceMonitor::ResourceMonitor() {
    baseline_rss_ = current_rss_bytes();
    peak_rss_ = baseline_rss_;
    const std::uint64_t blocks = ru_block_bytes();
    baseline_read_ = blocks;
    baseline_write_ = blocks;
}

ResourceMonitor::~ResourceMonitor() = default;

void ResourceMonitor::sample() {
    peak_rss_ = std::max(peak_rss_, current_rss_bytes());
}

ResourceSnapshot ResourceMonitor::finish(double wall_seconds) {
    sample();
    ResourceSnapshot snap;
    snap.peak_rss_bytes = peak_rss_;
    const std::uint64_t blocks = ru_block_bytes();
    snap.bytes_read = (blocks >= baseline_read_ ? blocks - baseline_read_ : 0) + g_extra_read;
    snap.bytes_written = (blocks >= baseline_write_ ? blocks - baseline_write_ : 0) + g_extra_write;
    extra_read_ = snap.bytes_read;
    extra_write_ = snap.bytes_written;

    if (global_run_config().measure_energy) {
#if defined(__linux__)
        const double end_j = read_rapl_joules();
        if (end_j > 0.0) {
            snap.energy_joules = end_j;
            snap.energy_backend = "intel_rapl";
            if (wall_seconds > 0.0) {
                snap.mean_power_watts = snap.energy_joules / wall_seconds;
            }
        }
#else
        snap.energy_backend = "unavailable";
#endif
    }
    return snap;
}

} // namespace satox
