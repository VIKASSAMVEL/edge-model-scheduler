#pragma once

#include "ResourceMonitor.hpp"
#include "TaskTypes.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace edge_scheduler {

struct LegacyInferenceTask : public BaseInferenceTask {
    std::function<void()> payload;

    bool operator<(const LegacyInferenceTask& other) const {
        if (priority == other.priority) {
            return timestamp_ms > other.timestamp_ms;
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

    void submit_task(const LegacyInferenceTask& task);

private:
    void worker_loop();

    std::shared_ptr<ResourceMonitor> monitor_;
    
    std::priority_queue<LegacyInferenceTask> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    
    std::atomic<bool> running_{false};
    std::vector<std::thread> workers_;
};

} // namespace edge_scheduler
