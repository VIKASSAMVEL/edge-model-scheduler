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
    InferenceTask task;
    task.model_id = model_id;
    task.priority = priority;
    task.payload = payload;
    task.timestamp_ms = get_time_now_ms();
    task.aging_boost = 0;

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_list_.push_back(task);
    }
    cv_.notify_one();
}

void AdvancedScheduler::check_aging_and_starvation(uint64_t now) {
    // Check if any low-priority task has waited longer than starvation_threshold_ms_
    for (auto& task : task_list_) {
        if (task.priority == Priority::LOW && (now - task.timestamp_ms) > starvation_threshold_ms_) {
            // Promote effective priority so it won't starve forever
            task.aging_boost = 1; // Effectively becomes MEDIUM
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

            uint64_t now = get_time_now_ms();
            check_aging_and_starvation(now);

            // Sort so highest priority (lowest effective_priority integer) comes first
            std::stable_sort(task_list_.begin(), task_list_.end(), 
                [](const InferenceTask& a, const InferenceTask& b) {
                    if (a.effective_priority() == b.effective_priority()) {
                        return a.timestamp_ms < b.timestamp_ms; // FIFO tie breaker
                    }
                    return a.effective_priority() < b.effective_priority();
                });

            current_task = task_list_.front();
            task_list_.erase(task_list_.begin());
        }

        // Check real-time hardware telemetry before allocating resources
        auto metrics = monitor_->get_current_metrics();
        
        // Dynamic Shedding Policy:
        // If thermal throttling (>85C) OR GPU saturation > 95%, shed unboosted LOW priority frames
        if ((metrics.is_throttling || metrics.gpu_utilization > 95.0f) && 
            current_task.priority == Priority::LOW && current_task.aging_boost == 0) {
            
            std::cout << "[AdaptiveScheduler] SHEDDING low-priority task '" 
                      << current_task.model_id << "' (Temp: " << metrics.thermal_temperature 
                      << "C, GPU: " << metrics.gpu_utilization << "%) to protect safety SLO." << std::endl;
            continue;
        }

        // Acquire Zero-Copy Pinned Memory Buffer from pool
        void* buffer = mem_pool_->acquire();

        try {
            if (current_task.payload) {
                current_task.payload(buffer);
            }
        } catch (const std::exception& e) {
            std::cerr << "[AdaptiveScheduler] Execution error in model " 
                      << current_task.model_id << ": " << e.what() << std::endl;
        }

        // Release buffer back to pool
        mem_pool_->release(buffer);
    }
}

} // namespace edge_scheduler
