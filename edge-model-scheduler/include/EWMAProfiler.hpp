#pragma once

#include <vector>
#include <chrono>
#include <numeric>
#include <iostream>
#include <atomic>
#include <cstdint>

namespace edge_scheduler {

/**
 * EWMAExecutionProfiler:
 * 
 * Tracks real-time exponential moving average of inference latency per model.
 * If a model's running latency + queued waiting time exceeds its declared SLO,
 * the scheduler predicts deadline misses BEFORE dispatching CUDA kernels.
 * 
 * Highly optimized for ultra-low latency & lock-free thread safety:
 * Uses atomic fixed-point microseconds (1 us resolution) with CAS loop
 * to ensure 100% wait-free, mutexless execution even under concurrent dispatches.
 */
class EWMAExecutionProfiler {
public:
    explicit EWMAExecutionProfiler(float alpha = 0.2f) 
        : alpha_scaled_(static_cast<uint32_t>(alpha * 1024.0f)),
          avg_latency_us_(0),
          sample_count_(0) {}

    void record_execution_time(float latency_ms) {
        uint64_t new_sample_us = static_cast<uint64_t>(latency_ms * 1000.0f);
        uint64_t cur_avg = avg_latency_us_.load(std::memory_order_relaxed);
        uint64_t new_avg = 0;

        uint64_t cur_samples = sample_count_.fetch_add(1, std::memory_order_relaxed);
        if (cur_samples == 0) {
            avg_latency_us_.store(new_sample_us, std::memory_order_release);
            return;
        }

        // Lock-free CAS update using fixed-point math:
        // new_avg = (alpha * sample) + ((1 - alpha) * cur_avg)
        do {
            new_avg = (alpha_scaled_ * new_sample_us + (1024 - alpha_scaled_) * cur_avg) / 1024;
        } while (!avg_latency_us_.compare_exchange_weak(
            cur_avg, new_avg, std::memory_order_release, std::memory_order_relaxed));
    }

    float get_expected_latency_ms() const noexcept {
        return static_cast<float>(avg_latency_us_.load(std::memory_order_acquire)) / 1000.0f;
    }

    // Returns true if dispatching this task will miss its target deadline
    bool will_exceed_deadline(float slo_target_ms, float queue_delay_ms) const noexcept {
        if (sample_count_.load(std::memory_order_relaxed) < 3) return false; // Need calibration samples
        float projected_total = queue_delay_ms + get_expected_latency_ms();
        return projected_total > slo_target_ms;
    }

    void reset() noexcept {
        avg_latency_us_.store(0, std::memory_order_release);
        sample_count_.store(0, std::memory_order_release);
    }

private:
    uint32_t alpha_scaled_; // 10-bit fixed-point (alpha * 1024)
    alignas(64) std::atomic<uint64_t> avg_latency_us_;
    alignas(64) std::atomic<uint64_t> sample_count_;
};

} // namespace edge_scheduler

