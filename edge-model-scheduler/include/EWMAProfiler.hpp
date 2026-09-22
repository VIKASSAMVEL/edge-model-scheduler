#pragma once

#include <vector>
#include <chrono>
#include <numeric>
#include <iostream>

namespace edge_scheduler {

/**
 * EWMAExecutionProfiler:
 * 
 * Tracks real-time exponential moving average of inference latency per model.
 * If a model's running latency + queued waiting time exceeds its declared SLO,
 * the scheduler predicts deadline misses BEFORE dispatching CUDA kernels.
 */
class EWMAExecutionProfiler {
public:
    EWMAExecutionProfiler(float alpha = 0.2f) : alpha_(alpha), avg_latency_ms_(0.0f), sample_count_(0) {}

    void record_execution_time(float latency_ms) {
        if (sample_count_ == 0) {
            avg_latency_ms_ = latency_ms;
        } else {
            avg_latency_ms_ = (alpha_ * latency_ms) + ((1.0f - alpha_) * avg_latency_ms_);
        }
        sample_count_++;
    }

    float get_expected_latency_ms() const {
        return avg_latency_ms_;
    }

    // Returns true if dispatching this task will miss its target deadline
    bool will_exceed_deadline(float slo_target_ms, float queue_delay_ms) const {
        if (sample_count_ < 3) return false; // Need calibration samples
        float projected_total = queue_delay_ms + avg_latency_ms_;
        return projected_total > slo_target_ms;
    }

private:
    float alpha_;
    float avg_latency_ms_;
    size_t sample_count_;
};

} // namespace edge_scheduler
