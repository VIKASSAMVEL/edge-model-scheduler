#pragma once

#include <string>
#include <chrono>

#if defined(__has_include)
  #if __has_include(<nvToolsExt.h>)
    #include <nvToolsExt.h>
    #define EDGE_SCHEDULER_HAS_NVTX 1
  #elif __has_include(<nvtx3/nvToolsExt.h>)
    #include <nvtx3/nvToolsExt.h>
    #define EDGE_SCHEDULER_HAS_NVTX 1
  #endif
#endif

namespace edge_scheduler {

class NvtxProfiler {
public:
    static inline void push_range(const char* name) {
#ifdef EDGE_SCHEDULER_HAS_NVTX
        nvtxRangePushA(name);
#else
        (void)name;
#endif
    }

    static inline void pop_range() {
#ifdef EDGE_SCHEDULER_HAS_NVTX
        nvtxRangePop();
#endif
    }

    static inline void mark(const char* message) {
#ifdef EDGE_SCHEDULER_HAS_NVTX
        nvtxMarkA(message);
#else
        (void)message;
#endif
    }
};

class ScopedNvtxRange {
public:
    explicit ScopedNvtxRange(const char* name) {
        NvtxProfiler::push_range(name);
    }
    ~ScopedNvtxRange() {
        NvtxProfiler::pop_range();
    }
};

#define NVTX_SCOPED_ZONE(name) edge_scheduler::ScopedNvtxRange _nvtx_scoped_##__LINE__(name)
#define NVTX_MARK(msg) edge_scheduler::NvtxProfiler::mark(msg)

} // namespace edge_scheduler
