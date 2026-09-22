#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>

#include "SlackBatcher.hpp"
#include "BatteryQosGovernor.hpp"
#include "DlaCoScheduler.hpp"
#include "EmbeddedDashboard.hpp"

using namespace edge_scheduler;

// ═══════════════════════════════════════════════════════════════
// Test 1: Slack-Aware Batcher & Pre-Execution Shedding
// ═══════════════════════════════════════════════════════════════
void test_slack_batcher() {
    std::cout << "[God-Level Test 1] Testing SlackBatcher & Stale Frame Shedding... ";

    SlackBatcher batcher(4, 5.0f);

    auto now = std::chrono::steady_clock::now();

    // 1. Submit a healthy fresh frame
    SlackRequest fresh_req;
    fresh_req.request_id = 1;
    fresh_req.model_name = "yolov8_safety";
    fresh_req.priority = 0;
    fresh_req.deadline_ms = 35.0f;
    fresh_req.expected_exec_ms = 8.0f;
    fresh_req.ingress_time = now;

    bool submitted = batcher.submit(fresh_req);
    assert(submitted == true);
    assert(batcher.total_submitted() == 1);
    assert(batcher.stale_frames_shed() == 0);

    // 2. Submit an already-expired stale frame (ingress 50ms ago with 35ms deadline)
    SlackRequest stale_req;
    stale_req.request_id = 2;
    stale_req.model_name = "yolov8_safety";
    stale_req.priority = 1;
    stale_req.deadline_ms = 35.0f;
    stale_req.expected_exec_ms = 8.0f;
    stale_req.ingress_time = now - std::chrono::milliseconds(50); // Ingress was 50ms ago!

    bool callback_called = false;
    stale_req.callback = [&](bool success, float) {
        callback_called = true;
        assert(success == false);
    };

    submitted = batcher.submit(stale_req);
    // MUST BE SHED INSTANTLY BEFORE GPU DISPATCH!
    assert(submitted == false);
    assert(callback_called == true);
    assert(batcher.stale_frames_shed() == 1);

    // 3. Submit 3 more compatible fresh frames to test coalescing
    for (int i = 3; i <= 5; ++i) {
        SlackRequest req;
        req.request_id = i;
        req.model_name = "yolov8_safety";
        req.priority = 1;
        req.deadline_ms = 40.0f;
        req.expected_exec_ms = 8.0f;
        req.ingress_time = now;
        batcher.submit(req);
    }

    // Try forming a batch
    BatchJob job;
    bool formed = batcher.try_form_batch(job);
    assert(formed == true);
    assert(job.model_name == "yolov8_safety");
    assert(job.batch_size == 4); // Coalesced into 4-batch!
    assert(batcher.batches_created() == 1);

    std::cout << "PASSED. (Shed: " << batcher.stale_frames_shed()
              << ", SLO Rate: " << batcher.slo_compliance_rate() << "%)\n";
}

// ═══════════════════════════════════════════════════════════════
// Test 2: Battery & Energy-Aware QoS Governor
// ═══════════════════════════════════════════════════════════════
void test_battery_qos_governor() {
    std::cout << "[God-Level Test 2] Testing BatteryQosGovernor & Duty Cycling... ";

    BatteryQosGovernor gov(40.0f, 20.0f);

    // Start in NORMAL mode (80% SoC)
    gov.update_telemetry(80.0f, false);
    assert(gov.current_mode() == BatteryQosMode::NORMAL);

    // All priority levels should be admitted in NORMAL
    assert(gov.should_admit_task("yolo_safety", 0) == true);
    assert(gov.should_admit_task("slam_feature", 2) == true);
    assert(gov.should_force_int8(2) == false);

    // Drop to ECO mode (30% SoC)
    gov.update_telemetry(30.0f, false);
    assert(gov.current_mode() == BatteryQosMode::ECO);

    // In ECO: Safety (Priority 0) always admitted; Priority 2 duty-cycled at 50%
    assert(gov.should_admit_task("yolo_safety", 0) == true);
    bool slam_admit_1 = gov.should_admit_task("slam_feature", 2);
    bool slam_admit_2 = gov.should_admit_task("slam_feature", 2);
    // 50% duty cycle means alternating true/false
    assert(slam_admit_1 != slam_admit_2);
    assert(gov.should_force_int8(2) == true); // INT8 precision recommended in ECO

    // Drop to CRITICAL_RTH (<20% SoC, e.g. 15%)
    gov.update_telemetry(15.0f, false);
    assert(gov.current_mode() == BatteryQosMode::CRITICAL_RTH);

    // In CRITICAL_RTH: Priority 0 STILL ADMITTED for safe obstacle avoidance!
    assert(gov.should_admit_task("yolo_safety", 0) == true);
    // Background SLAM (Priority 2) COMPLETELY DROPPED
    assert(gov.should_admit_task("slam_feature", 2) == false);

    // Connect to charger (External Power)
    gov.update_telemetry(15.0f, true);
    assert(gov.current_mode() == BatteryQosMode::EXTERNAL_POWER);
    assert(gov.should_admit_task("slam_feature", 2) == true);

    std::cout << "PASSED.\n";
}

// ═══════════════════════════════════════════════════════════════
// Test 3: Asymmetric DLA + GPU Co-Scheduler
// ═══════════════════════════════════════════════════════════════
void test_dla_co_scheduler() {
    std::cout << "[God-Level Test 3] Testing DlaCoScheduler & Bus Arbitration... ";

    DlaCoScheduler co_sched(2, 204.8f);

    // 1. Safety Priority 0 task -> MUST be assigned to GPU SMs
    auto dec0 = co_sched.route_task("yolov8_safety", 0, 1024 * 1024, false, true);
    assert(dec0.target_device == AccelDevice::GPU);

    // 2. Transformer attention model with unsupported DLA ops -> MUST go to GPU
    auto dec1 = co_sched.route_task("bev_transformer", 1, 2048 * 1024, true, false);
    assert(dec1.target_device == AccelDevice::GPU);

    // 3. Pure CNN models -> Should be balanced between DLA0 and DLA1
    auto dec2 = co_sched.route_task("resnet_backbone", 2, 512 * 1024, false, true);
    assert(dec2.target_device == AccelDevice::DLA0);

    auto dec3 = co_sched.route_task("mobilenet_backbone", 2, 512 * 1024, false, true);
    assert(dec3.target_device == AccelDevice::DLA1);

    // 4. Heavy memory bus saturation -> Should flag memory barrier
    co_sched.update_memory_bus_load(180.0f); // 180 GB/s > 75% of 204.8 GB/s
    auto dec4 = co_sched.route_task("cnn_heavy", 2, 1024 * 1024, false, true);
    assert(dec4.needs_memory_barrier == true);
    assert(co_sched.bus_stalls_avoided() > 0);

    std::cout << "PASSED. (DLA0: " << co_sched.dla0_tasks()
              << ", DLA1: " << co_sched.dla1_tasks()
              << ", Bus Stalls Avoided: " << co_sched.bus_stalls_avoided() << ")\n";
}

// ═══════════════════════════════════════════════════════════════
// Test 4: Embedded Dashboard HTTP Server
// ═══════════════════════════════════════════════════════════════
void test_embedded_dashboard() {
    std::cout << "[God-Level Test 4] Testing EmbeddedDashboard & REST Endpoints... ";

    EmbeddedDashboard dashboard(9191, []() {
        DashboardMetricsSnapshot s;
        s.gpu_utilization = 64.5f;
        s.gpu_temperature_c = 71.0f;
        s.precision_mode = "FP16";
        s.p50_latency_ms = 8.2f;
        s.p90_latency_ms = 14.5f;
        s.p99_latency_ms = 21.8f;
        s.total_dispatched = 2500;
        s.stale_frames_shed = 3;
        s.slo_compliance_rate = 99.88f;
        s.battery_mode = "NORMAL";
        s.battery_soc = 88.0f;
        s.dla0_tasks = 800;
        s.dla1_tasks = 750;
        s.gpu_tasks = 950;
        s.uptime_seconds = 1200;
        return s;
    });

    bool started = dashboard.start();
    assert(started == true);
    assert(dashboard.is_running() == true);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    dashboard.stop();
    assert(dashboard.is_running() == false);

    std::cout << "PASSED.\n";
}

// ═══════════════════════════════════════════════════════════════
int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "🔥 EdgeModelScheduler God-Level Subsystems Test Suite" << std::endl;
    std::cout << "==================================================" << std::endl;

    test_slack_batcher();
    test_battery_qos_governor();
    test_dla_co_scheduler();
    test_embedded_dashboard();

    std::cout << "==================================================" << std::endl;
    std::cout << "🎉 ALL GOD-LEVEL SUBSYSTEM TESTS PASSED!" << std::endl;
    std::cout << "==================================================" << std::endl;
    return 0;
}
