#pragma once

#include <ctime>
#include <chrono>

namespace satox {

class Timer {
  public:
    Timer() : wall_start_(std::chrono::steady_clock::now()), cpu_start_(std::clock()) {}

    double wall_ms() const {
        const auto elapsed = std::chrono::steady_clock::now() - wall_start_;
        return std::chrono::duration<double, std::milli>(elapsed).count();
    }

    double cpu_ms() const {
        return 1000.0 * static_cast<double>(std::clock() - cpu_start_) /
               static_cast<double>(CLOCKS_PER_SEC);
    }

  private:
    std::chrono::steady_clock::time_point wall_start_;
    std::clock_t cpu_start_;
};

} // namespace satox
