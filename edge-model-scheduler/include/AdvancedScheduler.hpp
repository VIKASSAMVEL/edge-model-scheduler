#pragma once

#include "ResourceMonitor.hpp"
#include "PinnedMemoryPool.hpp"
#include "TaskTypes.hpp"
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

struct AdvancedInferenceTask : public BaseInferenceTask {
    std::function<void(void* buffer)> payload;
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

    void set_starvation_threshold_ms(uint64_t threshold_ms) { starvation_threshold_ms_ = threshold_ms; }

private:
    void worker_loop();
    void check_aging_and_starvation(uint64_t now);

    std::shared_ptr<ResourceMonitor> monitor_;
    std::shared_ptr<PinnedMemoryPool> mem_pool_;

    std::vector<AdvancedInferenceTask> task_list_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;

    std::atomic<bool> running_{false};
    std::vector<std::thread> workers_;

    uint64_t starvation_threshold_ms_ = 250;
};

} // namespace edge_scheduler
