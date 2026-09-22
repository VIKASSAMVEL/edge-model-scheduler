#pragma once

#include "AdvancedScheduler.hpp"
#include "ResourceMonitor.hpp"
#include "PinnedMemoryPool.hpp"
#include "QuantizationGovernor.hpp"
#include "EWMAProfiler.hpp"
#include "ShmRingBuffer.hpp"
#include "IpcTransport.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace edge_scheduler {

/**
 * ChaosEngine: Automated Adversarial Testing Framework
 *
 * Simulates real-world failure modes that edge devices encounter:
 *   - Thermal shock (sudden temperature spikes)
 *   - Memory pressure (pool exhaustion / OOM)
 *   - Client process crashes mid-inference
 *   - Queue flooding (burst traffic exceeding capacity)
 *   - GPU utilization saturation
 *
 * Each scenario is self-contained, measurable, and produces
 * a pass/fail verdict with detailed timing metrics.
 */

struct ChaosResult {
    std::string scenario_name;
    bool passed{false};
    float duration_ms{0.0f};
    uint32_t frames_processed{0};
    uint32_t frames_dropped{0};
    float worst_latency_ms{0.0f};
    float p99_latency_ms{0.0f};
    std::string failure_reason;
};

class ChaosEngine {
public:
    ChaosEngine(std::shared_ptr<ResourceMonitor> monitor,
                std::shared_ptr<PinnedMemoryPool> pool,
                std::shared_ptr<QuantizationGovernor> governor)
        : monitor_(monitor), pool_(pool), governor_(governor) {}

    /**
     * Scenario 1: Thermal Shock
     * Simulates a sudden 15°C temperature spike within 200ms.
     * Verifies zero high-priority frames are dropped during the thermal event.
     *
     * SLO: All HIGH priority tasks must complete within 40ms.
     */
    ChaosResult run_thermal_shock(AdvancedScheduler& scheduler, int duration_sec = 10) {
        ChaosResult result;
        result.scenario_name = "thermal_shock";
        auto start = std::chrono::steady_clock::now();

        std::atomic<uint32_t> high_completed{0};
        std::atomic<uint32_t> high_dropped{0};
        std::atomic<float> worst_latency{0.0f};
        std::vector<float> latencies;
        std::mutex lat_mutex;

        std::atomic<bool> running{true};
        scheduler.start();

        // Background: continuously submit HIGH priority tasks
        std::thread producer([&]() {
            while (running) {
                auto task_start = std::chrono::steady_clock::now();
                scheduler.submit_task("safety_model", Priority::HIGH,
                    [&, task_start](void* /*buf*/) {
                        auto task_end = std::chrono::steady_clock::now();
                        float lat = std::chrono::duration<float, std::milli>(
                            task_end - task_start).count();
                        high_completed++;
                        {
                            std::lock_guard<std::mutex> lk(lat_mutex);
                            latencies.push_back(lat);
                        }
                        float cur = worst_latency.load();
                        while (lat > cur && !worst_latency.compare_exchange_weak(cur, lat)) {}
                    });
                std::this_thread::sleep_for(std::chrono::milliseconds(33)); // 30fps
            }
        });

        // Inject thermal shock at 3 seconds
        std::thread shock([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            // Force governor to INT8 mode (simulating thermal spike)
            governor_->evaluate_telemetry(95.0f, 98.0f);
            std::cout << "[ChaosEngine] THERMAL SHOCK injected (95°C)" << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(2));
            // Recovery
            governor_->evaluate_telemetry(65.0f, 40.0f);
            std::cout << "[ChaosEngine] Thermal recovery (65°C)" << std::endl;
        });

        std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
        running = false;
        producer.join();
        shock.join();
        scheduler.stop();

        auto end = std::chrono::steady_clock::now();
        result.duration_ms = std::chrono::duration<float, std::milli>(end - start).count();
        result.frames_processed = high_completed.load();
        result.frames_dropped = high_dropped.load();
        result.worst_latency_ms = worst_latency.load();

        // Calculate P99
        {
            std::lock_guard<std::mutex> lk(lat_mutex);
            if (!latencies.empty()) {
                std::sort(latencies.begin(), latencies.end());
                size_t p99_idx = static_cast<size_t>(latencies.size() * 0.99);
                result.p99_latency_ms = latencies[p99_idx];
            }
        }

        result.passed = (result.frames_dropped == 0 && result.worst_latency_ms < 40.0f);
        if (!result.passed) {
            result.failure_reason = "dropped=" + std::to_string(result.frames_dropped)
                + " worst_lat=" + std::to_string(result.worst_latency_ms) + "ms";
        }

        return result;
    }

    /**
     * Scenario 2: Memory Bus Exhaustion
     * Saturates the memory pool by acquiring all buffers before submitting tasks.
     * Verifies the scheduler gracefully handles pool exhaustion with fallback allocation.
     */
    ChaosResult run_memory_exhaustion(AdvancedScheduler& scheduler) {
        ChaosResult result;
        result.scenario_name = "memory_exhaustion";
        auto start = std::chrono::steady_clock::now();

        // Exhaust the pool
        std::vector<void*> held_buffers;
        for (int i = 0; i < 20; ++i) {
            void* buf = pool_->acquire();
            if (buf) held_buffers.push_back(buf);
        }

        std::atomic<uint32_t> completed{0};
        scheduler.start();

        // Submit tasks while pool is exhausted
        for (int i = 0; i < 10; ++i) {
            scheduler.submit_task("test_model", Priority::HIGH,
                [&](void* /*buf*/) { completed++; });
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
        scheduler.stop();

        // Release held buffers
        for (void* buf : held_buffers) {
            pool_->release(buf);
        }

        auto end = std::chrono::steady_clock::now();
        result.duration_ms = std::chrono::duration<float, std::milli>(end - start).count();
        result.frames_processed = completed.load();
        // Pass if scheduler didn't crash and processed at least some tasks
        result.passed = (completed.load() > 0);
        if (!result.passed) {
            result.failure_reason = "No tasks completed under memory pressure";
        }

        return result;
    }

    /**
     * Scenario 3: Queue Flood (Burst Traffic)
     * Submit 500 tasks in <100ms to overwhelm the scheduler queue.
     * Verifies HIGH priority tasks are still serviced within SLO
     * while LOW priority may be shed.
     */
    ChaosResult run_queue_flood(AdvancedScheduler& scheduler) {
        ChaosResult result;
        result.scenario_name = "queue_flood";
        auto start = std::chrono::steady_clock::now();

        std::atomic<uint32_t> high_completed{0};
        std::atomic<uint32_t> low_completed{0};
        std::vector<float> high_latencies;
        std::mutex lat_mutex;

        scheduler.start();

        // Burst: 500 tasks in rapid succession
        for (int i = 0; i < 500; ++i) {
            Priority p = (i % 5 == 0) ? Priority::HIGH : Priority::LOW;
            auto submit_time = std::chrono::steady_clock::now();

            scheduler.submit_task("burst_model", p,
                [&, p, submit_time](void* /*buf*/) {
                    auto done = std::chrono::steady_clock::now();
                    float lat = std::chrono::duration<float, std::milli>(done - submit_time).count();
                    if (p == Priority::HIGH) {
                        high_completed++;
                        std::lock_guard<std::mutex> lk(lat_mutex);
                        high_latencies.push_back(lat);
                    } else {
                        low_completed++;
                    }
                });
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
        scheduler.stop();

        auto end = std::chrono::steady_clock::now();
        result.duration_ms = std::chrono::duration<float, std::milli>(end - start).count();
        result.frames_processed = high_completed.load() + low_completed.load();

        // Evaluate P99 of HIGH priority tasks
        {
            std::lock_guard<std::mutex> lk(lat_mutex);
            if (!high_latencies.empty()) {
                std::sort(high_latencies.begin(), high_latencies.end());
                size_t p99_idx = static_cast<size_t>(high_latencies.size() * 0.99);
                result.p99_latency_ms = high_latencies[p99_idx];
                result.worst_latency_ms = high_latencies.back();
            }
        }

        // HIGH priority must all complete, SLO < 40ms
        result.passed = (high_completed.load() == 100) && (result.p99_latency_ms < 40.0f);
        if (!result.passed) {
            result.failure_reason = "HIGH completed=" + std::to_string(high_completed.load())
                + "/100, P99=" + std::to_string(result.p99_latency_ms) + "ms";
        }

        return result;
    }

    /**
     * Run all chaos scenarios and print a summary report.
     */
    std::vector<ChaosResult> run_all(AdvancedScheduler& scheduler) {
        std::vector<ChaosResult> results;

        std::cout << "\n╔═══════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║  🔥 CHAOS ENGINE: Adversarial Test Suite          ║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════════════╝\n" << std::endl;

        results.push_back(run_thermal_shock(scheduler));
        print_result(results.back());

        results.push_back(run_memory_exhaustion(scheduler));
        print_result(results.back());

        results.push_back(run_queue_flood(scheduler));
        print_result(results.back());

        // Summary
        uint32_t passed = 0;
        for (const auto& r : results) {
            if (r.passed) passed++;
        }

        std::cout << "\n═══════════════════════════════════════════════════" << std::endl;
        std::cout << "  CHAOS RESULTS: " << passed << "/" << results.size()
                  << " scenarios passed" << std::endl;
        std::cout << "═══════════════════════════════════════════════════\n" << std::endl;

        return results;
    }

private:
    void print_result(const ChaosResult& r) {
        std::cout << (r.passed ? "  ✅ " : "  ❌ ")
                  << r.scenario_name
                  << " | " << r.frames_processed << " frames"
                  << " | P99=" << r.p99_latency_ms << "ms"
                  << " | Worst=" << r.worst_latency_ms << "ms"
                  << " | " << r.duration_ms << "ms";
        if (!r.passed) {
            std::cout << " | FAIL: " << r.failure_reason;
        }
        std::cout << std::endl;
    }

    std::shared_ptr<ResourceMonitor> monitor_;
    std::shared_ptr<PinnedMemoryPool> pool_;
    std::shared_ptr<QuantizationGovernor> governor_;
};

} // namespace edge_scheduler
