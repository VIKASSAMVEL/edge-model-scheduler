#pragma once

#include <cstdint>
#include <string>
#include <functional>

namespace edge_scheduler {

enum class Priority {
    HIGH = 1,     // Collision avoidance / safety (strict <40ms SLO)
    MEDIUM = 2,   // Visual tracking / SLAM
    LOW = 3       // Background logging / analytics
};

struct BaseInferenceTask {
    std::string model_id;
    Priority priority;
    uint64_t timestamp_ms;
    uint32_t aging_boost = 0;

    int effective_priority() const {
        int base = static_cast<int>(priority);
        int effective = base - static_cast<int>(aging_boost);
        return (effective < 1) ? 1 : effective;
    }
};

} // namespace edge_scheduler
