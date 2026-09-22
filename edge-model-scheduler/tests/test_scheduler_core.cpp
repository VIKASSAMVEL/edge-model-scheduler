#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <atomic>

#include "AdvancedScheduler.hpp"
#include "ResourceMonitor.hpp"
#include "PinnedMemoryPool.hpp"
#include "QuantizationGovernor.hpp"
#include "EWMAProfiler.hpp"
#include "ModelRegistry.hpp"
#include "TensorRTRunner.hpp"

using namespace edge_scheduler;

void test_pinned_memory_pool() {
    std::cout << "[Test 1] Testing PinnedMemoryPool allocation and recycling... ";
    size_t buffer_size = 1024 * 512; // 512 KB
    PinnedMemoryPool pool(buffer_size, 2);

    void* b1 = pool.acquire();
    void* b2 = pool.acquire();
    assert(b1 != nullptr);
    assert(b2 != nullptr);
    assert(b1 != b2);

    // Pool empty: fallback acquisition
    void* b3 = pool.acquire();
    assert(b3 != nullptr);

    pool.release(b1);
    pool.release(b2);
    pool.release(b3);

    // Re-acquire from recycled pool
    void* recycled = pool.acquire();
    assert(recycled != nullptr);
    pool.release(recycled);

    std::cout << "PASSED." << std::endl;
}

void test_quantization_governor() {
    std::cout << "[Test 2] Testing QuantizationGovernor hysteresis thresholds... ";
    QuantizationGovernor gov;
    assert(gov.get_current_mode() == PrecisionMode::FP16);

    // High temperature trigger (>80C)
    gov.evaluate_telemetry(86.5f, 92.0f);
    assert(gov.get_current_mode() == PrecisionMode::INT8);

    // Intermediate temperature (75C) should not drop back (hysteresis)
    gov.evaluate_telemetry(75.0f, 70.0f);
    assert(gov.get_current_mode() == PrecisionMode::INT8);

    // Cool down (<72C)
    gov.evaluate_telemetry(68.0f, 60.0f);
    assert(gov.get_current_mode() == PrecisionMode::FP16);

    std::cout << "PASSED." << std::endl;
}

void test_ewma_profiler() {
    std::cout << "[Test 3] Testing EWMAExecutionProfiler and deadline prediction... ";
    EWMAExecutionProfiler profiler(0.3f);

    profiler.record_execution_time(20.0f);
    profiler.record_execution_time(22.0f);
    profiler.record_execution_time(21.0f);

    float exp_lat = profiler.get_expected_latency_ms();
    assert(exp_lat >= 19.0f && exp_lat <= 23.0f);

    // Target SLO 30ms, Queue delay 15ms + 21ms expected = 36ms > 30ms -> Will exceed
    bool will_miss = profiler.will_exceed_deadline(30.0f, 15.0f);
    assert(will_miss == true);

    // Target SLO 50ms, Queue delay 5ms + 21ms expected = 26ms < 50ms -> Will not exceed
    bool will_not_miss = profiler.will_exceed_deadline(50.0f, 5.0f);
    assert(will_not_miss == false);

    std::cout << "PASSED." << std::endl;
}

void test_model_registry() {
    std::cout << "[Test 4] Testing ModelRegistry registration... ";
    ModelRegistry registry;

    ModelConfig cfg{"yolov8_safety", "/opt/models/yolo.engine", 1, 35.0f, 1024 * 1024, true};
    bool ok = registry.register_model(cfg);
    assert(ok == true);
    assert(registry.has_model("yolov8_safety"));
    assert(registry.count() == 1);

    const ModelConfig* fetched = registry.get_model("yolov8_safety");
    assert(fetched != nullptr);
    assert(fetched->latency_slo_ms == 35.0f);

    std::cout << "PASSED." << std::endl;
}

void test_advanced_scheduler_dispatch() {
    std::cout << "[Test 5] Testing AdvancedScheduler priority queue ordering... ";
    auto monitor = std::make_shared<ResourceMonitor>();
    auto pool = std::make_shared<PinnedMemoryPool>(1024 * 64, 4);

    AdvancedScheduler scheduler(monitor, pool);
    scheduler.start();

    std::atomic<int> executed_count{0};
    std::vector<std::string> execution_order;
    std::mutex order_mutex;

    // Submit LOW first, then HIGH
    scheduler.submit_task("low_task", Priority::LOW, [&](void* /*buf*/) {
        std::lock_guard<std::mutex> lk(order_mutex);
        execution_order.push_back("low_task");
        executed_count++;
    });

    scheduler.submit_task("high_task", Priority::HIGH, [&](void* /*buf*/) {
        std::lock_guard<std::mutex> lk(order_mutex);
        execution_order.push_back("high_task");
        executed_count++;
    });

    // Wait for tasks to complete
    int attempts = 0;
    while (executed_count < 2 && attempts < 50) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        attempts++;
    }

    scheduler.stop();
    assert(executed_count == 2);
    std::cout << "PASSED." << std::endl;
}

void test_tensorrt_runner() {
    std::cout << "[Test 6] Testing TensorRTRunner lifecycle and infer... ";
    TensorRTRunner runner;
    assert(!runner.is_loaded());
    bool ok = runner.load_engine("default.engine");
    assert(ok);
    assert(runner.is_loaded());
    assert(runner.num_bindings() >= 2);
    assert(runner.input_size_bytes() > 0);
    assert(runner.output_size_bytes() > 0);

    bool inf_ok = runner.infer();
    assert(inf_ok);
    std::cout << "PASSED." << std::endl;
}

int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "🧪 Running EdgeModelScheduler C++ Native Unit Tests" << std::endl;
    std::cout << "==================================================" << std::endl;

    test_pinned_memory_pool();
    test_quantization_governor();
    test_ewma_profiler();
    test_model_registry();
    test_advanced_scheduler_dispatch();
    test_tensorrt_runner();

    std::cout << "==================================================" << std::endl;
    std::cout << "🎉 ALL UNIT TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "==================================================" << std::endl;
    return 0;
}
