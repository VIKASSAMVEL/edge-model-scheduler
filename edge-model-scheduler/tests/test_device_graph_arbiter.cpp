#include <iostream>
#include <cassert>
#include "DeviceGraphArbiter.hpp"

using namespace edge_scheduler;

void test_device_branch_selection() {
    std::cout << "[DeviceGraph Test 1] Testing Device Conditional Branch Selection... " << std::flush;

    DeviceGraphArbiter arbiter;

    // 1. Emergency safety frame (Priority 0, Slack = 20ms)
    auto b1 = arbiter.evaluate_and_dispatch(20.0f, 0);
    assert(b1 == DeviceGraphBranch::BRANCH_SAFETY_EMERGENCY);

    // 2. Expired stale frame (Slack = -5.0ms) -> Instant skip
    auto b2 = arbiter.evaluate_and_dispatch(-5.0f, 1);
    assert(b2 == DeviceGraphBranch::BRANCH_PRE_EXECUTION_SHED);

    // 3. Nominal background perception frame (Priority 1, Slack = 15ms)
    auto b3 = arbiter.evaluate_and_dispatch(15.0f, 1);
    assert(b3 == DeviceGraphBranch::BRANCH_STANDARD_STREAM);

    std::cout << "PASSED. (All 3 Silicon Branches Routed Deterministically)\n";
}

void test_sub_microsecond_dispatch_latency() {
    std::cout << "[DeviceGraph Test 2] Benchmarking On-Silicon Dispatch Latency (100,000 evaluations)... " << std::flush;

    DeviceGraphArbiter arbiter;

    for (int i = 0; i < 100000; ++i) {
        float slack = (i % 5 == 0) ? -2.0f : 12.0f;
        uint32_t prio = (i % 3 == 0) ? 0 : 1;
        arbiter.evaluate_and_dispatch(slack, prio);
    }

    auto tele = arbiter.get_telemetry();
    assert(tele.total_dispatches == 100000);
    assert(tele.avg_dispatch_latency_ns < 500.0); // Strict requirement: < 500 nanoseconds!

    std::cout << "PASSED. (Average Latency: " << tele.avg_dispatch_latency_ns 
              << " ns / dispatch, Target: < 500 ns)\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "⚡ Device-Side Conditional CUDA Graph Test Suite   \n";
    std::cout << "==================================================\n";

    test_device_branch_selection();
    test_sub_microsecond_dispatch_latency();

    std::cout << "==================================================\n";
    std::cout << "🎉 ALL DEVICE GRAPH ARBITER TESTS PASSED!\n";
    std::cout << "==================================================\n";
    return 0;
}
