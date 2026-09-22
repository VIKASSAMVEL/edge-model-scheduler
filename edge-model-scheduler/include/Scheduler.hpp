#pragma once

#include "ResourceMonitor.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace edge_scheduler {

enum class Priority {
    HIGH = 1,
    MEDIUM = 2,
    LOW = 3
};

struct InferenceTask {
    std::string model_id;
    Priority priority;
    std::function<void()> payload;
    uint64_t timestamp_ms;

    bool operator<(const InferenceTask& other) const {
        // Higher priority value means lower actual priority in enum.
        // We want HIGH (1) to come first.
        if (priority == other.priority) {
            return timestamp_ms > other.timestamp_ms; // FIFO for same priority
        }
        return priority > other.priority; 
    }
};

class Scheduler {
public:
    Scheduler(std::shared_ptr<ResourceMonitor> monitor);
    ~Scheduler();

    void start();
    void stop();

    void submit_task(const InferenceTask& task);

private:
    void worker_loop();

    std::shared_ptr<ResourceMonitor> monitor_;
    
    std::priority_queue<InferenceTask> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    
    std::atomic<bool> running_{false};
    std::vector<std::thread> workers_;
};

} // namespace edge_scheduler
