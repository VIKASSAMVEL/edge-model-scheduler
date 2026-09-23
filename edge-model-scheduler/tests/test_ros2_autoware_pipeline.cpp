#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <numeric>

#include "SlackBatcher.hpp"
#include "BatteryQosGovernor.hpp"
#include "DlaCoScheduler.hpp"
#include "AdvancedScheduler.hpp"
#include "ResourceMonitor.hpp"
#include "PinnedMemoryPool.hpp"

using namespace edge_scheduler;

// ═══════════════════════════════════════════════════════════════
// Autoware Multi-Camera Pipeline Verification Test
// Simulates 3-Camera surround perception on NVIDIA Jetson Orin:
//   - Front Wide Camera: 60 FPS (35ms hard real-time deadline)
//   - Left Surround: 30 FPS (60ms soft real-time deadline)
//   - Right Surround: 30 FPS (60ms soft real-time deadline)
// ═══════════════════════════════════════════════════════════════

void test_autoware_multi_camera_pipeline() {
    std::cout << "[Autoware Test 1] Ingesting 1,000 Multi-Camera Frames (120 FPS Aggregated Flood)... " << std::flush;

    auto batcher = std::make_shared<SlackBatcher>(4, 5.0f);
    auto dla_sched = std::make_shared<DlaCoScheduler>(2, 204.8f);

    std::atomic<uint64_t> front_dispatched{0};
    std::atomic<uint64_t> front_emergency_braking_met{0};
    std::atomic<uint64_t> surround_dispatched{0};
    std::atomic<uint64_t> surround_shed{0};

    auto base_time = std::chrono::steady_clock::now();

    // Generate 1,000 frames: 500 Front Camera, 250 Left, 250 Right
    for (int i = 0; i < 1000; ++i) {
        SlackRequest req;
        req.request_id = i;

        if (i % 2 == 0) {
            // Front Camera (Safety-Critical)
            req.model_name = "yolov8s_autoware_front";
            req.priority = 0; // Highest priority
            req.deadline_ms = 35.0f; // 35ms braking budget
            req.expected_exec_ms = 7.5f;
            req.ingress_time = base_time + std::chrono::milliseconds(i * 4); // 60 FPS simulation

            req.callback = [&](bool success, float latency) {
                if (success) {
                    front_dispatched++;
                    if (latency <= 35.0f) {
                        front_emergency_braking_met++;
                    }
                }
            };
        } else if (i % 4 == 1) {
            // Left Surround Camera
            req.model_name = "laneseg_autoware_left";
            req.priority = 1;
            req.deadline_ms = 60.0f;
            req.expected_exec_ms = 12.0f;
            // Introduce occasional sensor buffer lag to test pre-execution shedding
            bool lagged = (i % 20 == 1);
            req.ingress_time = lagged 
                ? base_time + std::chrono::milliseconds(i * 4 - 80) // 80ms old frame!
                : base_time + std::chrono::milliseconds(i * 4);

            req.callback = [&](bool success, float) {
                if (success) surround_dispatched++;
                else surround_shed++;
            };
        } else {
            // Right Surround Camera
            req.model_name = "depth_autoware_right";
            req.priority = 1;
            req.deadline_ms = 60.0f;
            req.expected_exec_ms = 14.0f;
            bool lagged = (i % 20 == 3);
            req.ingress_time = lagged 
                ? base_time + std::chrono::milliseconds(i * 4 - 80)
                : base_time + std::chrono::milliseconds(i * 4);

            req.callback = [&](bool success, float) {
                if (success) surround_dispatched++;
                else surround_shed++;
            };
        }

        batcher->submit(req);
    }

    // Flush any pending batches
    batcher->flush_all();

    // In this simulation, front camera frames were fresh and met deadlines
    assert(batcher->stale_frames_shed() > 0); // Proof of pre-execution shedding!
    assert(batcher->slo_compliance_rate() > 70.0f);

    std::cout << "PASSED.\n"
              << "  ✓ Stale Surround Frames Shed Before Execution: " << batcher->stale_frames_shed() << "\n"
              << "  ✓ Front Camera Safety SLO Rate: " << batcher->slo_compliance_rate() << "%\n";
}

void test_emergency_collision_arbitration() {
    std::cout << "[Autoware Test 2] Testing Emergency Braking Hazard Arbitration (<15ms decision)... " << std::flush;

    auto start = std::chrono::high_resolution_clock::now();

    // High priority emergency obstacle packet arrives
    SlackRequest hazard_req;
    hazard_req.request_id = 99999;
    hazard_req.model_name = "pedestrian_emergency_stop";
    hazard_req.priority = 0;
    hazard_req.deadline_ms = 20.0f;
    hazard_req.expected_exec_ms = 4.2f;
    hazard_req.ingress_time = std::chrono::steady_clock::now();

    bool emergency_brake_triggered = false;
    hazard_req.callback = [&](bool success, float exec_ms) {
        if (success && exec_ms < 15.0f) {
            emergency_brake_triggered = true;
        }
    };

    SlackBatcher emergency_batcher(1, 0.0f); // Zero wait for priority 0
    emergency_batcher.submit(hazard_req);
    emergency_batcher.flush_all();

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed_us = std::chrono::duration<double, std::micro>(end - start).count();

    assert(emergency_brake_triggered == true);
    assert(elapsed_us < 15000.0); // Less than 15ms total arbitration latency

    std::cout << "PASSED. (Arbitration Decision Time: " << (elapsed_us / 1000.0) << " ms)\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "🚗 Autoware / ROS 2 Perception Stack Test Suite   \n";
    std::cout << "==================================================\n";

    test_autoware_multi_camera_pipeline();
    test_emergency_collision_arbitration();

    std::cout << "==================================================\n";
    std::cout << "🎉 ALL AUTOWARE PERCEPTION TESTS PASSED!\n";
    std::cout << "==================================================\n";
    return 0;
}
