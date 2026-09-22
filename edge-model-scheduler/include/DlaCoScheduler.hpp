#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <chrono>

namespace edge_scheduler {

enum class AccelDevice {
    GPU = 0,
    DLA0 = 1,
    DLA1 = 2,
    HYBRID = 3
};

struct AccelStats {
    std::atomic<uint64_t> gpu_tasks{0};
    std::atomic<uint64_t> dla0_tasks{0};
    std::atomic<uint64_t> dla1_tasks{0};
    std::atomic<uint64_t> hybrid_tasks{0};
    std::atomic<uint64_t> bus_stalls_avoided{0};
};

struct RoutingDecision {
    AccelDevice target_device{AccelDevice::GPU};
    std::string reason;
    float estimated_latency_ms{0.0f};
    bool needs_memory_barrier{false};
};

class DlaCoScheduler {
public:
    explicit DlaCoScheduler(uint32_t available_dla_cores = 2, float max_lpddr5_bandwidth_gbps = 204.8f)
        : available_dla_cores_(available_dla_cores), max_bus_bandwidth_gbps_(max_lpddr5_bandwidth_gbps) {}

    // Dynamic routing decision for incoming inference tasks
    RoutingDecision route_task(
        const std::string& model_name,
        uint32_t priority,
        size_t input_bytes,
        bool has_custom_ops = false,
        bool is_pure_cnn = true)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        RoutingDecision decision;

        // Rule 1: Priority 0 (Emergency Braking / Safety Perception) ALWAYS runs on GPU Stream 0
        // for sub-millisecond preemption guarantees
        if (priority == 0) {
            decision.target_device = AccelDevice::GPU;
            decision.reason = "Safety-critical (Priority 0) bound to preemption-capable GPU SMs";
            decision.estimated_latency_ms = 8.5f;
            stats_.gpu_tasks.fetch_add(1, std::memory_order_relaxed);
            return decision;
        }

        // Rule 2: If model contains custom plugins or transformer attention layers not supported by DLA hardware
        if (has_custom_ops || !is_pure_cnn || available_dla_cores_ == 0) {
            decision.target_device = AccelDevice::GPU;
            decision.reason = "Contains custom/transformer ops unsupported by DLA; routed to GPU to avoid fallback stalls";
            decision.estimated_latency_ms = 14.2f;
            stats_.gpu_tasks.fetch_add(1, std::memory_order_relaxed);
            return decision;
        }

        // Rule 3: Memory bus arbitration
        // If GPU is currently under heavy memory bandwidth pressure, interleave or serialize DLA memory load
        float current_bandwidth = current_bus_load_gbps_.load(std::memory_order_relaxed);
        if (current_bandwidth > 0.75f * max_bus_bandwidth_gbps_) {
            decision.needs_memory_barrier = true;
            stats_.bus_stalls_avoided.fetch_add(1, std::memory_order_relaxed);
        }

        // Rule 4: Balanced distribution between DLA0 and DLA1
        if (available_dla_cores_ >= 2) {
            if (dla0_inflight_ <= dla1_inflight_) {
                decision.target_device = AccelDevice::DLA0;
                decision.reason = "Pure CNN offloaded to DLA0 (balanced load)";
                decision.estimated_latency_ms = 11.8f;
                dla0_inflight_++;
                stats_.dla0_tasks.fetch_add(1, std::memory_order_relaxed);
            } else {
                decision.target_device = AccelDevice::DLA1;
                decision.reason = "Pure CNN offloaded to DLA1 (balanced load)";
                decision.estimated_latency_ms = 11.8f;
                dla1_inflight_++;
                stats_.dla1_tasks.fetch_add(1, std::memory_order_relaxed);
            }
        } else if (available_dla_cores_ == 1) {
            decision.target_device = AccelDevice::DLA0;
            decision.reason = "Pure CNN offloaded to single available DLA0";
            decision.estimated_latency_ms = 12.0f;
            dla0_inflight_++;
            stats_.dla0_tasks.fetch_add(1, std::memory_order_relaxed);
        } else {
            decision.target_device = AccelDevice::GPU;
            decision.reason = "DLA unavailable; routed to GPU";
            stats_.gpu_tasks.fetch_add(1, std::memory_order_relaxed);
        }

        return decision;
    }

    void notify_task_complete(AccelDevice device) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (device == AccelDevice::DLA0 && dla0_inflight_ > 0) dla0_inflight_--;
        if (device == AccelDevice::DLA1 && dla1_inflight_ > 0) dla1_inflight_--;
    }

    void update_memory_bus_load(float current_gbps) {
        current_bus_load_gbps_.store(current_gbps, std::memory_order_relaxed);
    }

    uint32_t available_dla_cores() const { return available_dla_cores_; }
    uint64_t gpu_tasks() const { return stats_.gpu_tasks.load(std::memory_order_relaxed); }
    uint64_t dla0_tasks() const { return stats_.dla0_tasks.load(std::memory_order_relaxed); }
    uint64_t dla1_tasks() const { return stats_.dla1_tasks.load(std::memory_order_relaxed); }
    uint64_t bus_stalls_avoided() const { return stats_.bus_stalls_avoided.load(std::memory_order_relaxed); }

    std::string device_to_string(AccelDevice dev) const {
        switch (dev) {
            case AccelDevice::GPU: return "GPU";
            case AccelDevice::DLA0: return "DLA0";
            case AccelDevice::DLA1: return "DLA1";
            case AccelDevice::HYBRID: return "HYBRID";
        }
        return "UNKNOWN";
    }

private:
    uint32_t available_dla_cores_{2};
    float max_bus_bandwidth_gbps_{204.8f};
    std::atomic<float> current_bus_load_gbps_{24.5f};
    uint32_t dla0_inflight_{0};
    uint32_t dla1_inflight_{0};
    mutable std::mutex mutex_;
    AccelStats stats_;
};

} // namespace edge_scheduler
