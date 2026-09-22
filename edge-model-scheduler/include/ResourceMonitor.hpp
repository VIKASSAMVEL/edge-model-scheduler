#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>

namespace edge_scheduler {

struct SystemMetrics {
    float gpu_utilization;      // 0.0 to 100.0
    float thermal_temperature;  // Celsius
    bool is_throttling;
};

class ResourceMonitor {
public:
    ResourceMonitor();
    ~ResourceMonitor();

    // Start background polling
    void start(int polling_interval_ms = 100);
    // Stop background polling
    void stop();

    // Get the latest metrics
    SystemMetrics get_current_metrics() const;

private:
    void polling_loop(int interval_ms);
    float read_thermal_zone() const;
    float read_gpu_utilization() const;

    std::atomic<bool> running_{false};
    std::thread monitor_thread_;
    
    mutable std::mutex metrics_mutex_;
    SystemMetrics current_metrics_;
};

} // namespace edge_scheduler
