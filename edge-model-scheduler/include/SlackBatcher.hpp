#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <algorithm>
#include <memory>
#include <cstdint>
#include <atomic>

namespace edge_scheduler {

struct SlackRequest {
    uint64_t request_id{0};
    std::string model_name;
    uint32_t priority{1};              // 0=Highest, 1=High, 2=Medium, 3=Low
    float deadline_ms{35.0f};           // Target max latency from ingress
    float expected_exec_ms{10.0f};      // Estimated execution time from EWMA profiler
    std::chrono::steady_clock::time_point ingress_time;
    void* buffer{nullptr};
    size_t buffer_size{0};
    std::function<void(bool success, float latency_ms)> callback;

    inline float compute_slack_ms(std::chrono::steady_clock::time_point now) const {
        float elapsed_ms = std::chrono::duration<float, std::milli>(now - ingress_time).count();
        return deadline_ms - elapsed_ms - expected_exec_ms;
    }
};

struct BatchJob {
    std::string model_name;
    uint32_t batch_size{0};
    std::vector<uint64_t> request_ids;
    std::vector<void*> buffers;
    float min_slack_ms{0.0f};
    std::chrono::steady_clock::time_point dispatch_time;
};

struct SlackBatcherStats {
    std::atomic<uint64_t> total_submitted{0};
    std::atomic<uint64_t> total_dispatched{0};
    std::atomic<uint64_t> stale_frames_shed{0};
    std::atomic<uint64_t> batches_created{0};
    std::atomic<uint64_t> single_frame_dispatches{0};
};

class SlackBatcher {
public:
    explicit SlackBatcher(uint32_t max_batch_size = 4, float max_coalesce_window_ms = 5.0f)
        : max_batch_size_(max_batch_size), max_coalesce_window_ms_(max_coalesce_window_ms)
    {
        pending_queue_.reserve(128); // Pre-allocate contiguous memory to avoid heap reallocation
    }

    // Submit a request. If slack is already negative (impossible to meet SLO), sheds instantly!
    bool submit(SlackRequest req) {
        stats_.total_submitted.fetch_add(1, std::memory_order_relaxed);
        auto now = std::chrono::steady_clock::now();
        float slack = req.compute_slack_ms(now);

        if (slack < 0.0f) {
            // Instant pre-execution shedding: frame is already expired!
            stats_.stale_frames_shed.fetch_add(1, std::memory_order_relaxed);
            if (req.callback) {
                req.callback(false, std::chrono::duration<float, std::milli>(now - req.ingress_time).count());
            }
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        pending_queue_.push_back(std::move(req));
        cv_.notify_one();
        return true;
    }

    // Attempt to form the next optimal batch with O(N) single-pass selection (No std::sort)
    bool try_form_batch(BatchJob& out_batch, float /*current_time_budget_ms*/ = 100.0f) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pending_queue_.empty()) return false;

        auto now = std::chrono::steady_clock::now();

        // 1. Single-pass pre-execution shedding & prune expired frames
        size_t write_pos = 0;
        for (size_t i = 0; i < pending_queue_.size(); ++i) {
            float slack = pending_queue_[i].compute_slack_ms(now);
            if (slack < 0.0f) {
                stats_.stale_frames_shed.fetch_add(1, std::memory_order_relaxed);
                if (pending_queue_[i].callback) {
                    pending_queue_[i].callback(false, std::chrono::duration<float, std::milli>(now - pending_queue_[i].ingress_time).count());
                }
            } else {
                if (write_pos != i) {
                    pending_queue_[write_pos] = std::move(pending_queue_[i]);
                }
                write_pos++;
            }
        }
        pending_queue_.resize(write_pos);

        if (pending_queue_.empty()) return false;

        // 2. O(N) single-pass minimum scan for anchor request (highest priority / lowest slack)
        size_t anchor_idx = 0;
        float min_slack = pending_queue_[0].compute_slack_ms(now);
        uint32_t best_priority = pending_queue_[0].priority;

        for (size_t i = 1; i < pending_queue_.size(); ++i) {
            uint32_t prio = pending_queue_[i].priority;
            float slack = pending_queue_[i].compute_slack_ms(now);
            if (prio < best_priority || (prio == best_priority && slack < min_slack)) {
                best_priority = prio;
                min_slack = slack;
                anchor_idx = i;
            }
        }

        std::string target_model = pending_queue_[anchor_idx].model_name;

        out_batch.model_name = target_model;
        out_batch.request_ids.clear();
        out_batch.buffers.clear();
        out_batch.min_slack_ms = min_slack;
        out_batch.dispatch_time = now;

        // 3. Coalesce matching requests in contiguous pass
        std::vector<bool> consumed(pending_queue_.size(), false);
        for (size_t i = 0; i < pending_queue_.size() && out_batch.request_ids.size() < max_batch_size_; ++i) {
            if (pending_queue_[i].model_name == target_model) {
                out_batch.request_ids.push_back(pending_queue_[i].request_id);
                out_batch.buffers.push_back(pending_queue_[i].buffer);
                consumed[i] = true;
            }
        }

        // 4. In-place compacting
        write_pos = 0;
        for (size_t i = 0; i < pending_queue_.size(); ++i) {
            if (!consumed[i]) {
                if (write_pos != i) {
                    pending_queue_[write_pos] = std::move(pending_queue_[i]);
                }
                write_pos++;
            }
        }
        pending_queue_.resize(write_pos);

        out_batch.batch_size = static_cast<uint32_t>(out_batch.request_ids.size());
        stats_.total_dispatched.fetch_add(out_batch.batch_size, std::memory_order_relaxed);
        if (out_batch.batch_size > 1) {
            stats_.batches_created.fetch_add(1, std::memory_order_relaxed);
        } else {
            stats_.single_frame_dispatches.fetch_add(1, std::memory_order_relaxed);
        }

        return true;
    }

    // Drain and form all available batches until queue is empty
    void flush_all() {
        BatchJob b;
        while (try_form_batch(b)) {}
    }

    size_t pending_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pending_queue_.size();
    }

    uint64_t total_submitted() const { return stats_.total_submitted.load(std::memory_order_relaxed); }
    uint64_t total_dispatched() const { return stats_.total_dispatched.load(std::memory_order_relaxed); }
    uint64_t stale_frames_shed() const { return stats_.stale_frames_shed.load(std::memory_order_relaxed); }
    uint64_t batches_created() const { return stats_.batches_created.load(std::memory_order_relaxed); }
    uint32_t max_batch_size() const { return max_batch_size_; }

    float slo_compliance_rate() const {
        uint64_t sub = stats_.total_submitted.load(std::memory_order_relaxed);
        uint64_t shed = stats_.stale_frames_shed.load(std::memory_order_relaxed);
        if (sub == 0) return 100.0f;
        return (static_cast<float>(sub - shed) / static_cast<float>(sub)) * 100.0f;
    }

private:
    uint32_t max_batch_size_{4};
    float max_coalesce_window_ms_{5.0f};
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<SlackRequest> pending_queue_;
    SlackBatcherStats stats_;
};

} // namespace edge_scheduler
