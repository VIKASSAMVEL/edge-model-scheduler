#pragma once

#include "ResourceMonitor.hpp"
#include "PinnedMemoryPool.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <functional>
#include <string>
#include <chrono>
#include <algorithm>

namespace edge_scheduler {

enum class Priority {
    HIGH = 1,     // Collision avoidance / safety (strict <40ms SLO)
    MEDIUM = 2,   // Visual tracking / SLAM
    LOW = 3       // Background logging / analytics
};

struct InferenceTask {
    std::string model_id;
    Priority priority;
    std::function<void(void* buffer)> payload;
    uint64_t timestamp_ms;
    uint32_t aging_boost = 0; // Dynamic aging counter to prevent starvation

    // Effective priority: smaller value means dispatched first
    int effective_priority() const {
        int base = static_cast<int>(priority);
        int effective = base - static_cast<int>(aging_boost);
        return std::max(1, effective); // Never higher than 1
    }
};

class AdvancedScheduler {
public:
    AdvancedScheduler(std::shared_ptr<ResourceMonitor> monitor, 
                      std::shared_ptr<PinnedMemoryPool> mem_pool);
    ~AdvancedScheduler();

    void start();
    void stop();

    void submit_task(const std::string& model_id, Priority priority, 
                     std::function<void(void* buffer)> payload);

    // Dynamic aging threshold in milliseconds (promotes starving LOW tasks if headroom exists)
    void set_starvation_threshold_ms(uint64_t threshold_ms) { starvation_threshold_ms_ = threshold_ms; }

private:
    void worker_loop();
    void check_aging_and_starvation(uint64_t now);

    std::shared_ptr<ResourceMonitor> monitor_;
    std::shared_ptr<PinnedMemoryPool> mem_pool_;

    std::vector<InferenceTask> task_list_; // Custom sorted queue for dynamic priority aging
    std::mutex queue_mutex_;
    std::condition_variable cv_;

    std::atomic<bool> running_{false};
    std::vector<std::thread> workers_;

    uint64_t starvation_threshold_ms_ = 250; // 250ms threshold
};

} // namespace edge_scheduler
