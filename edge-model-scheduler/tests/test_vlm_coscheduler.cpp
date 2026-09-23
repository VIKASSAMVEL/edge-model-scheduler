#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>

#include "VlmCoScheduler.hpp"

using namespace edge_scheduler;

// ═══════════════════════════════════════════════════════════════
// Test Suite: VlmCoScheduler Multimodal Cooperative Arbitration
// ═══════════════════════════════════════════════════════════════

void test_nominal_vlm_generation() {
    std::cout << "[VLM Test 1] Testing Nominal Autoregressive Decode (20 Tokens)... " << std::flush;

    VlmCoScheduler scheduler(204.8f, 0.65f); // AGX Orin 204.8 GB/s, 65% cap

    uint32_t tokens_executed = 0;
    for (int i = 0; i < 20; ++i) {
        scheduler.generate_token_step(1, [&]() {
            tokens_executed++;
            std::this_thread::sleep_for(std::chrono::microseconds(200)); // Simulated decode
        });
    }

    assert(tokens_executed == 20);
    assert(scheduler.total_tokens() == 20);
    assert(scheduler.total_yields() == 0); // No contention

    auto tele = scheduler.get_telemetry();
    assert(tele.cooperative_yields == 0);
    assert(tele.total_tokens_generated == 20);
    (void)tele;

    std::cout << "PASSED. (Tokens: " << scheduler.total_tokens() 
              << ", Yields: " << scheduler.total_yields() << ")\n";
}

void test_safety_inter_token_preemption() {
    std::cout << "[VLM Test 2] Testing Cooperative Inter-Token Preemption Under Safety Frame Ingress... " << std::flush;

    VlmCoScheduler scheduler(204.8f, 0.65f);

    std::atomic<bool> vlm_running{true};
    std::atomic<uint64_t> safety_frames_cleared{0};

    // Thread 1: Background VLM streaming generation (e.g. "Explain navigation path...")
    std::thread vlm_thread([&]() {
        for (int i = 0; i < 40 && vlm_running.load(); ++i) {
            scheduler.generate_token_step(1, [&]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(2)); // KV cache decode
            });
        }
    });

    // Thread 2: High-priority 60 FPS Safety Ingress (YOLOv8 Emergency Obstacle)
    std::thread safety_thread([&]() {
        for (int i = 0; i < 5; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            auto t0 = std::chrono::steady_clock::now();

            // Notify safety frame arrived
            scheduler.notify_safety_ingress();

            // Simulate high-priority GPU execution
            std::this_thread::sleep_for(std::chrono::milliseconds(3));

            // Clear safety frame
            scheduler.notify_safety_complete();

            auto dur_ms = std::chrono::duration<float, std::milli>(
                std::chrono::steady_clock::now() - t0).count();
            assert(dur_ms < 15.0f); // Safety latency strictly bounded!
            safety_frames_cleared++;
        }
    });

    safety_thread.join();
    vlm_running.store(false);
    vlm_thread.join();

    assert(safety_frames_cleared == 5);
    assert(scheduler.total_yields() > 0); // Proof of cooperative yielding!

    std::cout << "PASSED. (Safety Frames: " << safety_frames_cleared 
              << ", Cooperative Yields: " << scheduler.total_yields() << ")\n";
}

void test_emc_memory_bus_throttle() {
    std::cout << "[VLM Test 3] Testing EMC Unified Memory Controller Bus Pacing... " << std::flush;

    VlmCoScheduler scheduler(204.8f, 0.65f); // Cap = 204.8 * 0.65 = 133.12 GB/s

    // 1. Under safe bus load (80 GB/s)
    scheduler.update_emc_telemetry(80.0f);
    assert(scheduler.should_vlm_yield() == false);

    // 2. Under heavy bus contention (160 GB/s > 133.12 GB/s cap)
    scheduler.update_emc_telemetry(160.0f);
    assert(scheduler.should_vlm_yield() == true); // Must throttle to protect real-time pipeline!

    // 3. Headroom restored (60 GB/s)
    scheduler.update_emc_telemetry(60.0f);
    assert(scheduler.should_vlm_yield() == false);

    std::cout << "PASSED. (Bus Cap: " << scheduler.vlm_bw_cap_gbps() << " GB/s)\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "🤖 VlmCoScheduler Multimodal Arbitration Suite   \n";
    std::cout << "==================================================\n";

    test_nominal_vlm_generation();
    test_safety_inter_token_preemption();
    test_emc_memory_bus_throttle();

    std::cout << "==================================================\n";
    std::cout << "🎉 ALL VLM CO-SCHEDULER TESTS PASSED!\n";
    std::cout << "==================================================\n";
    return 0;
}
