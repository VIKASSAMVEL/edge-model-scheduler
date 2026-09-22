#pragma once

#include <iostream>
#include <vector>
#include <deque>
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

    float compute_slack_ms(std::chrono::steady_clock::time_point now) const {
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
        : max_batch_size_(max_batch_size), max_coalesce_window_ms_(max_coalesce_window_ms) {}

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

    // Attempt to form the next optimal batch or shed any expired frames in queue
    bool try_form_batch(BatchJob& out_batch, float current_time_budget_ms = 100.0f) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pending_queue_.empty()) return false;

        auto now = std::chrono::steady_clock::now();

        // 1. Prune expired frames from pending queue (Pre-execution shedding)
        auto it = pending_queue_.begin();
        while (it != pending_queue_.end()) {
            float slack = it->compute_slack_ms(now);
            if (slack < 0.0f) {
                stats_.stale_frames_shed.fetch_add(1, std::memory_order_relaxed);
                if (it->callback) {
                    it->callback(false, std::chrono::duration<float, std::milli>(now - it->ingress_time).count());
                }
                it = pending_queue_.erase(it);
            } else {
                ++it;
            }
        }

        if (pending_queue_.empty()) return false;

        // 2. Select highest priority / lowest slack anchor request
        std::sort(pending_queue_.begin(), pending_queue_.end(),
            [&now](const SlackRequest& a, const SlackRequest& b) {
                if (a.priority != b.priority) return a.priority < b.priority;
                return a.compute_slack_ms(now) < b.compute_slack_ms(now);
            });

        auto& anchor = pending_queue_.front();
        std::string target_model = anchor.model_name;
        float anchor_slack = anchor.compute_slack_ms(now);

        out_batch.model_name = target_model;
        out_batch.request_ids.clear();
        out_batch.buffers.clear();
        out_batch.min_slack_ms = anchor_slack;
        out_batch.dispatch_time = now;

        // 3. Coalesce compatible requests into batch
        it = pending_queue_.begin();
        while (it != pending_queue_.end() && out_batch.request_ids.size() < max_batch_size_) {
            if (it->model_name == target_model) {
                out_batch.request_ids.push_back(it->request_id);
                out_batch.buffers.push_back(it->buffer);
                it = pending_queue_.erase(it);
            } else {
                ++it;
            }
        }

        out_batch.batch_size = static_cast<uint32_t>(out_batch.request_ids.size());
        stats_.total_dispatched.fetch_add(out_batch.batch_size, std::memory_order_relaxed);
        if (out_batch.batch_size > 1) {
            stats_.batches_created.fetch_add(1, std::memory_order_relaxed);
        } else {
            stats_.single_frame_dispatches.fetch_add(1, std::memory_order_relaxed);
        }

        return true;
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
    std::deque<SlackRequest> pending_queue_;
    SlackBatcherStats stats_;
};

} // namespace edge_scheduler
