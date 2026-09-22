#include "Scheduler.hpp"
#include <iostream>
#include <chrono>

namespace edge_scheduler {

Scheduler::Scheduler(std::shared_ptr<ResourceMonitor> monitor) 
    : monitor_(monitor) {
}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    if (running_) return;
    running_ = true;

    // Start 1 worker thread for MVP. 
    // In TensorRT, you typically have 1 execution context per engine, 
    // or you manage multiple contexts. A single dispatcher thread ensures 
    // we serialize execution if the GPU is fully saturated.
    workers_.emplace_back(&Scheduler::worker_loop, this);
}

void Scheduler::stop() {
    running_ = false;
    condition_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers_.clear();
}

void Scheduler::submit_task(const InferenceTask& task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.push(task);
    }
    condition_.notify_one();
}

void Scheduler::worker_loop() {
    while (running_) {
        InferenceTask task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this] { return !task_queue_.empty() || !running_; });
            
            if (!running_ && task_queue_.empty()) {
                return;
            }

            task = task_queue_.top();
            task_queue_.pop();
        }

        // Before execution, check telemetry
        auto metrics = monitor_->get_current_metrics();
        if (metrics.is_throttling || metrics.gpu_utilization > 95.0f) {
            // Drop low priority tasks if throttling
            if (task.priority == Priority::LOW) {
                std::cout << "[Scheduler] Dropping LOW priority task for model " 
                          << task.model_id << " due to resource exhaustion." << std::endl;
                continue; 
            }
        }

        // Execute task
        try {
            if (task.payload) {
                task.payload();
            }
        } catch (const std::exception& e) {
            std::cerr << "[Scheduler] Task execution failed: " << e.what() << std::endl;
        }
    }
}

} // namespace edge_scheduler
