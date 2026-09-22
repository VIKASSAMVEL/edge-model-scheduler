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
    // Jetson typical thermal zone for GPU/CPU. We read thermal_zone0 for simplicity.
    // Real implementation would parse /sys/devices/virtual/thermal/thermal_zone*/temp
    std::ifstream temp_file("/sys/devices/virtual/thermal/thermal_zone0/temp");
    if (temp_file.is_open()) {
        long temp;
        temp_file >> temp;
        return static_cast<float>(temp) / 1000.0f;
    }
    return 0.0f; // Mock if not running on Jetson
}

float ResourceMonitor::read_gpu_utilization() const {
    // To read GPU utilization natively on Jetson, we can parse sysfs nodes 
    // such as /sys/devices/gpu.0/load (depends on exact Jetpack version).
    std::ifstream load_file("/sys/devices/gpu.0/load");
    if (load_file.is_open()) {
        float load;
        load_file >> load;
        return load / 10.0f; // Scale depending on sysfs specific output
    }
    return 0.0f; // Mock if not on Jetson
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
