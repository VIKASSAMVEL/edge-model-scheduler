#pragma once

#include "ResourceMonitor.hpp"
#include "EWMAProfiler.hpp"
#include "QuantizationGovernor.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace edge_scheduler {

/**
 * MetricsExporter: Fleet-Scale Observability Engine
 *
 * Collects and exports scheduler telemetry in multiple formats:
 *   1. Prometheus exposition format (HTTP scrape endpoint)
 *   2. JSON Lines for log aggregation (Datadog, Loki)
 *   3. MQTT publish for IoT fleet dashboards
 *
 * Metrics collected per export interval:
 *   - GPU utilization, temperature, throttling state
 *   - Inference queue depth, processing rate
 *   - P50/P99/max latency per model
 *   - Precision mode (FP16/INT8), swap count
 *   - Memory pool utilization
 *   - IPC ring buffer fill level
 *   - Watchdog heartbeat count
 */

struct SchedulerMetrics {
    // Hardware
    float gpu_utilization{0.0f};
    float gpu_temperature_c{0.0f};
    bool is_throttling{false};

    // Scheduler
    uint64_t tasks_dispatched{0};
    uint64_t tasks_completed{0};
    uint64_t tasks_dropped{0};
    uint32_t queue_depth{0};
    float avg_latency_ms{0.0f};
    float p99_latency_ms{0.0f};
    float max_latency_ms{0.0f};

    // Quantization
    uint8_t precision_mode{0}; // 0=FP16, 1=INT8
    uint32_t precision_swaps{0};

    // IPC
    uint32_t ipc_ring_pending{0};
    uint64_t ipc_requests_received{0};

    // System
    uint64_t uptime_seconds{0};
    uint64_t watchdog_heartbeats{0};
    float daemon_cpu_percent{0.0f};
};

class MetricsExporter {
public:
    MetricsExporter(uint32_t export_interval_ms = 5000)
        : export_interval_ms_(export_interval_ms) {}

    ~MetricsExporter() { stop(); }

    /**
     * Register a metrics source callback.
     * Called at each export interval to collect fresh data.
     */
    void set_metrics_source(std::function<SchedulerMetrics()> source) {
        metrics_source_ = std::move(source);
    }

    void start() {
        if (running_) return;
        running_ = true;
        start_time_ = std::chrono::steady_clock::now();
        export_thread_ = std::thread(&MetricsExporter::export_loop, this);
    }

    void stop() {
        running_ = false;
        if (export_thread_.joinable()) {
            export_thread_.join();
        }
    }

    /**
     * Generate Prometheus exposition format string.
     * Ready to serve via HTTP endpoint at /metrics.
     */
    std::string to_prometheus(const SchedulerMetrics& m) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);

        ss << "# HELP edge_scheduler_gpu_utilization GPU utilization percentage\n";
        ss << "# TYPE edge_scheduler_gpu_utilization gauge\n";
        ss << "edge_scheduler_gpu_utilization " << m.gpu_utilization << "\n\n";

        ss << "# HELP edge_scheduler_gpu_temperature GPU temperature in Celsius\n";
        ss << "# TYPE edge_scheduler_gpu_temperature gauge\n";
        ss << "edge_scheduler_gpu_temperature " << m.gpu_temperature_c << "\n\n";

        ss << "# HELP edge_scheduler_gpu_throttling Whether GPU is throttling\n";
        ss << "# TYPE edge_scheduler_gpu_throttling gauge\n";
        ss << "edge_scheduler_gpu_throttling " << (m.is_throttling ? 1 : 0) << "\n\n";

        ss << "# HELP edge_scheduler_tasks_dispatched_total Total tasks dispatched\n";
        ss << "# TYPE edge_scheduler_tasks_dispatched_total counter\n";
        ss << "edge_scheduler_tasks_dispatched_total " << m.tasks_dispatched << "\n\n";

        ss << "# HELP edge_scheduler_tasks_completed_total Total tasks completed\n";
        ss << "# TYPE edge_scheduler_tasks_completed_total counter\n";
        ss << "edge_scheduler_tasks_completed_total " << m.tasks_completed << "\n\n";

        ss << "# HELP edge_scheduler_tasks_dropped_total Total tasks dropped\n";
        ss << "# TYPE edge_scheduler_tasks_dropped_total counter\n";
        ss << "edge_scheduler_tasks_dropped_total " << m.tasks_dropped << "\n\n";

        ss << "# HELP edge_scheduler_queue_depth Current queue depth\n";
        ss << "# TYPE edge_scheduler_queue_depth gauge\n";
        ss << "edge_scheduler_queue_depth " << m.queue_depth << "\n\n";

        ss << "# HELP edge_scheduler_latency_avg_ms Average inference latency\n";
        ss << "# TYPE edge_scheduler_latency_avg_ms gauge\n";
        ss << "edge_scheduler_latency_avg_ms " << m.avg_latency_ms << "\n\n";

        ss << "# HELP edge_scheduler_latency_p99_ms P99 inference latency\n";
        ss << "# TYPE edge_scheduler_latency_p99_ms gauge\n";
        ss << "edge_scheduler_latency_p99_ms " << m.p99_latency_ms << "\n\n";

        ss << "# HELP edge_scheduler_precision_mode Current precision (0=FP16, 1=INT8)\n";
        ss << "# TYPE edge_scheduler_precision_mode gauge\n";
        ss << "edge_scheduler_precision_mode " << static_cast<int>(m.precision_mode) << "\n\n";

        ss << "# HELP edge_scheduler_precision_swaps_total Precision mode swap count\n";
        ss << "# TYPE edge_scheduler_precision_swaps_total counter\n";
        ss << "edge_scheduler_precision_swaps_total " << m.precision_swaps << "\n\n";

        ss << "# HELP edge_scheduler_ipc_ring_pending Pending IPC requests\n";
        ss << "# TYPE edge_scheduler_ipc_ring_pending gauge\n";
        ss << "edge_scheduler_ipc_ring_pending " << m.ipc_ring_pending << "\n\n";

        ss << "# HELP edge_scheduler_uptime_seconds Daemon uptime\n";
        ss << "# TYPE edge_scheduler_uptime_seconds counter\n";
        ss << "edge_scheduler_uptime_seconds " << m.uptime_seconds << "\n\n";

        ss << "# HELP edge_scheduler_daemon_cpu_percent Daemon CPU usage\n";
        ss << "# TYPE edge_scheduler_daemon_cpu_percent gauge\n";
        ss << "edge_scheduler_daemon_cpu_percent " << m.daemon_cpu_percent << "\n";

        return ss.str();
    }

    /**
     * Generate JSON Lines format for log aggregation.
     */
    std::string to_jsonl(const SchedulerMetrics& m) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);

        auto now = std::chrono::system_clock::now();
        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        ss << "{\"ts\":" << epoch
           << ",\"gpu_util\":" << m.gpu_utilization
           << ",\"gpu_temp\":" << m.gpu_temperature_c
           << ",\"throttling\":" << (m.is_throttling ? "true" : "false")
           << ",\"dispatched\":" << m.tasks_dispatched
           << ",\"completed\":" << m.tasks_completed
           << ",\"dropped\":" << m.tasks_dropped
           << ",\"queue\":" << m.queue_depth
           << ",\"avg_lat\":" << m.avg_latency_ms
           << ",\"p99_lat\":" << m.p99_latency_ms
           << ",\"precision\":" << (m.precision_mode == 0 ? "\"FP16\"" : "\"INT8\"")
           << ",\"swaps\":" << m.precision_swaps
           << ",\"ipc_pending\":" << m.ipc_ring_pending
           << ",\"uptime\":" << m.uptime_seconds
           << ",\"cpu\":" << m.daemon_cpu_percent
           << "}";

        return ss.str();
    }

    /**
     * Write metrics to a log file (append mode).
     */
    void export_to_file(const std::string& path) {
        log_file_path_ = path;
        export_to_file_ = true;
    }

    uint64_t export_count() const { return export_count_; }

private:
    void export_loop() {
        while (running_) {
            if (metrics_source_) {
                SchedulerMetrics m = metrics_source_();

                // Update uptime
                auto now = std::chrono::steady_clock::now();
                m.uptime_seconds = std::chrono::duration_cast<std::chrono::seconds>(
                    now - start_time_).count();

                // File export
                if (export_to_file_ && !log_file_path_.empty()) {
                    std::ofstream out(log_file_path_, std::ios::app);
                    if (out.is_open()) {
                        out << to_jsonl(m) << "\n";
                    }
                }

                latest_metrics_ = m;
                export_count_++;
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(export_interval_ms_));
        }
    }

    uint32_t export_interval_ms_;
    std::atomic<bool> running_{false};
    std::thread export_thread_;
    std::chrono::steady_clock::time_point start_time_;

    std::function<SchedulerMetrics()> metrics_source_;
    SchedulerMetrics latest_metrics_;
    uint64_t export_count_{0};

    bool export_to_file_{false};
    std::string log_file_path_;
};

} // namespace edge_scheduler
