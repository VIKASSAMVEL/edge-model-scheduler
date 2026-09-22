#include "ResourceMonitor.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <stdexcept>

namespace edge_scheduler {

ResourceMonitor::ResourceMonitor() {
    current_metrics_ = {0.0f, 0.0f, false};
}

ResourceMonitor::~ResourceMonitor() {
    stop();
}

void ResourceMonitor::start(int polling_interval_ms) {
    if (running_) return;
    running_ = true;
    monitor_thread_ = std::thread(&ResourceMonitor::polling_loop, this, polling_interval_ms);
}

void ResourceMonitor::stop() {
    running_ = false;
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
}

SystemMetrics ResourceMonitor::get_current_metrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    return current_metrics_;
}

float ResourceMonitor::read_thermal_zone() const {
    float max_temp = 0.0f;
    bool found_sensor = false;

    // 1. Scan Linux / Tegra sysfs thermal zones (thermal_zone0 through thermal_zone15)
    for (int i = 0; i < 16; ++i) {
        std::string zone_path = "/sys/class/thermal/thermal_zone" + std::to_string(i) + "/temp";
        std::ifstream temp_file(zone_path);
        if (temp_file.is_open()) {
            long temp_raw = 0;
            if (temp_file >> temp_raw && temp_raw > 0) {
                float temp_c = static_cast<float>(temp_raw) / 1000.0f;
                if (temp_c > max_temp && temp_c < 125.0f) { // Sanity filter
                    max_temp = temp_c;
                    found_sensor = true;
                }
            }
        }
    }

    // 2. Scan Linux hwmon sensors (/sys/class/hwmon/hwmon*/temp*_input)
    if (!found_sensor) {
        for (int h = 0; h < 4; ++h) {
            for (int t = 1; t <= 4; ++t) {
                std::string hwmon_path = "/sys/class/hwmon/hwmon" + std::to_string(h) + "/temp" + std::to_string(t) + "_input";
                std::ifstream hwmon_file(hwmon_path);
                if (hwmon_file.is_open()) {
                    long temp_raw = 0;
                    if (hwmon_file >> temp_raw && temp_raw > 0) {
                        float temp_c = static_cast<float>(temp_raw) / 1000.0f;
                        if (temp_c > max_temp && temp_c < 125.0f) {
                            max_temp = temp_c;
                            found_sensor = true;
                        }
                    }
                }
            }
        }
    }

    if (found_sensor) {
        return max_temp;
    }

    // Container fallback: return nominal ambient temperature (42.0°C)
    return 42.0f;
}

float ResourceMonitor::read_gpu_utilization() const {
    // 1. Scan known Jetson / Tegra sysfs GPU load entries
    static const char* const sysfs_gpu_paths[] = {
        "/sys/devices/gpu.0/load",
        "/sys/devices/platform/17000000.ga10b/load",
        "/sys/devices/17000000.ga10b/load",
        "/sys/kernel/debug/actmon/gpu_load",
        "/sys/class/devfreq/17000000.ga10b/load"
    };

    for (const char* path : sysfs_gpu_paths) {
        std::ifstream load_file(path);
        if (load_file.is_open()) {
            float load = 0.0f;
            if (load_file >> load) {
                // Tegra load values are typically 0-1000 (representing 0.0% to 100.0%)
                if (load > 100.0f) {
                    return load / 10.0f;
                }
                return load;
            }
        }
    }

    // 2. Fallback: Parse /proc/loadavg or /proc/stat to derive active system utilization
    std::ifstream loadavg("/proc/loadavg");
    if (loadavg.is_open()) {
        float l1 = 0.0f;
        if (loadavg >> l1) {
            // Normalize load average against standard 4-core SoC
            float util = (l1 / 4.0f) * 100.0f;
            return std::min(100.0f, std::max(0.0f, util));
        }
    }

    return 5.0f; // Minimal idle baseline
}

void ResourceMonitor::polling_loop(int interval_ms) {
    while (running_) {
        float temp = read_thermal_zone();
        float gpu_util = read_gpu_utilization();
        bool throttling = (temp > 85.0f); // Arbitrary threshold for thermal throttling

        {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            current_metrics_.thermal_temperature = temp;
            current_metrics_.gpu_utilization = gpu_util;
            current_metrics_.is_throttling = throttling;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
}

} // namespace edge_scheduler
