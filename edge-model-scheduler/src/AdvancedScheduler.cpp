#include "AdvancedScheduler.hpp"
#include <iostream>

namespace edge_scheduler {

static uint64_t get_time_now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

AdvancedScheduler::AdvancedScheduler(std::shared_ptr<ResourceMonitor> monitor, 
                                     std::shared_ptr<PinnedMemoryPool> mem_pool)
    : monitor_(monitor), mem_pool_(mem_pool) {}

AdvancedScheduler::~AdvancedScheduler() {
    stop();
}

void AdvancedScheduler::start() {
    if (running_) return;
    running_ = true;
    workers_.emplace_back(&AdvancedScheduler::worker_loop, this);
}

void AdvancedScheduler::stop() {
    running_ = false;
    cv_.notify_all();
    for (auto& w : workers_) {
        if (w.joinable()) {
            w.join();
        }
    }
    workers_.clear();
}

void AdvancedScheduler::submit_task(const std::string& model_id, Priority priority, 
                                   std::function<void(void* buffer)> payload) {
    if (!running_) return;
    InferenceTask task;
    task.model_id = model_id;
    task.priority = priority;
    task.payload = payload;
    task.timestamp_ms = get_time_now_ms();
    task.aging_boost = 0;

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_list_.push_back(std::move(task));
    }
    cv_.notify_one();
}

void AdvancedScheduler::check_aging_and_starvation(uint64_t now) {
    for (auto& task : task_list_) {
        if (task.priority == Priority::LOW && (now - task.timestamp_ms) > starvation_threshold_ms_) {
            task.aging_boost = 1;
        }
    }
}

void AdvancedScheduler::worker_loop() {
    while (running_) {
        InferenceTask current_task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] { return !task_list_.empty() || !running_; });

            if (!running_ && task_list_.empty()) {
                return;
            }

            if (task_list_.empty()) {
                continue;
            }

            uint64_t now = get_time_now_ms();
            check_aging_and_starvation(now);

            std::stable_sort(task_list_.begin(), task_list_.end(), 
                [](const InferenceTask& a, const InferenceTask& b) {
                    if (a.effective_priority() == b.effective_priority()) {
                        return a.timestamp_ms < b.timestamp_ms;
                    }
                    return a.effective_priority() < b.effective_priority();
                });

            current_task = std::move(task_list_.front());
            task_list_.erase(task_list_.begin());
        }

        if (monitor_) {
            auto metrics = monitor_->get_current_metrics();
            if ((metrics.is_throttling || metrics.gpu_utilization > 95.0f) && 
                current_task.priority == Priority::LOW && current_task.aging_boost == 0) {
                continue;
            }
        }

        void* buffer = nullptr;
        if (mem_pool_) {
            buffer = mem_pool_->acquire();
        }

        try {
            if (current_task.payload) {
                current_task.payload(buffer);
            }
        } catch (...) {
        }

        if (mem_pool_ && buffer) {
            mem_pool_->release(buffer);
        }
    }
}

} // namespace edge_scheduler
