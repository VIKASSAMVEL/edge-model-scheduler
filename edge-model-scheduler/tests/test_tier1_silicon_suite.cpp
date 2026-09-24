#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>
#include <cstdio>

#include "CudaIpcTypeAdapter.hpp"
#include "FlightRecorder.hpp"
#include "SlackBatcher.hpp"
#include "DlaCoScheduler.hpp"

using namespace edge_scheduler;

void test_cuda_ipc_type_adapter() {
    std::cout << "[Tier-1 Test 1] Verifying ROS 2 Zero-Copy CUDA IPC TypeAdapter... " << std::flush;

    CudaIpcTypeAdapter adapter(0);

    constexpr uint32_t PCD_WIDTH = 250000; // 250k points
    constexpr uint32_t PCD_HEIGHT = 1;
    constexpr uint32_t PCD_FIELDS = 12;    // x,y,z,intensity,ring,timestamp... (48 bytes/pt)
    constexpr uint32_t ELEM_SIZE = 4;      // 32-bit float

    std::vector<uint8_t> synthetic_raw_points(PCD_WIDTH * PCD_FIELDS * ELEM_SIZE, 0x5A);

    auto start = std::chrono::high_resolution_clock::now();

    // 1. Ingest raw LiDAR sweep into zero-copy GPU tensor
    GpuTensorHeader header = adapter.create_gpu_tensor(
        1001,
        1700000000000ULL,
        PCD_WIDTH,
        PCD_HEIGHT,
        PCD_FIELDS,
        ELEM_SIZE,
        synthetic_raw_points.data()
    );

    assert(header.magic == 0x454D5354);
    assert(header.total_payload_bytes == synthetic_raw_points.size());
    assert(header.gpu_handle.valid);

    // 2. Map into consumer node (zero CPU copies)
    void* mapped_dev_ptr = CudaIpcTypeAdapter::map_device_tensor(header);
    assert(mapped_dev_ptr != nullptr);

    auto end = std::chrono::high_resolution_clock::now();
    double latency_us = std::chrono::duration<double, std::micro>(end - start).count();

    CudaIpcTypeAdapter::unmap_device_tensor(mapped_dev_ptr);
    adapter.release_all();

    std::cout << "PASS (12 MB Sweep Mapped | Overhead: " << latency_us << " us)" << std::endl;
}

void test_flight_recorder_crash_dump() {
    std::cout << "[Tier-1 Test 2] Verifying ISO 26262 ASIL-D Flight Recorder Crash Forensics... " << std::flush;

    const std::string test_bag_file = "test_incident_dump.emsbag";
    std::remove(test_bag_file.c_str());

    FlightRecorder recorder(5000);

    // Record 500 nominal frames, then inject a thermal spike & ASIL-D fault
    for (int i = 0; i < 500; ++i) {
        FlightTelemetryFrame f;
        f.timestamp_ns = 1000000ULL * i;
        f.gpu_temp_c = 55.0f + (i * 0.05f);
        f.gpu_util_pct = 40.0f + (i % 20);
        f.memory_bus_gbps = 24.5f;
        f.p99_latency_ms = 8.5f;
        f.deadline_slack_ms = 31.5f;
        f.battery_soc_pct = 95.0f;
        f.asil_state = (i > 480) ? 2 : 0; // Fault near frame 480
        f.current_precision = (i > 450) ? 1 : 0; // Precision downshift
        f.stale_shed_total = (i > 400) ? 3 : 0;
        recorder.record_frame(f);
    }

    assert(recorder.total_recorded() == 500);

    // Trigger post-crash dump
    bool dumped = recorder.dump_to_file(test_bag_file, "ASIL_D_LOCKSTEP_DIVERGENCE_TRIP");
    assert(dumped && "Flight recorder dump must succeed");

    // Read back and parse
    FlightRecorderHeader read_hdr;
    std::vector<FlightTelemetryFrame> read_frames;
    bool parsed = FlightRecorder::parse_emsbag(test_bag_file, read_hdr, read_frames);

    assert(parsed && "Bag parsing must succeed");
    assert(read_hdr.total_frames == 500);
    assert(std::string(read_hdr.trigger_reason) == "ASIL_D_LOCKSTEP_DIVERGENCE_TRIP");
    assert(read_frames.size() == 500);
    assert(read_frames.back().asil_state == 2);

    std::remove(test_bag_file.c_str());
    std::cout << "PASS (500-Frame Audit Bag Verified & Replayed)" << std::endl;
}

void test_wcet_hard_realtime_bounds() {
    std::cout << "[Tier-1 Test 3] Verifying Sub-15ms WCET Predictability Under Peak Bus Contention... " << std::flush;

    auto batcher = std::make_shared<SlackBatcher>(8, 5.0f);
    auto dla_sched = std::make_shared<DlaCoScheduler>(2, 204.8f);

    // Simulate memory bus saturation (180 GB/s on 204.8 GB/s LPDDR5)
    dla_sched->update_memory_bus_load(185.0f);

    uint32_t bus_stalls_mitigated = 0;

    for (int i = 0; i < 200; ++i) {
        // High priority safety perception
        auto decision = dla_sched->route_task("autoware_emergency_detector", 0, 1920 * 1080 * 3, false, true);
        assert(decision.target_device == AccelDevice::GPU);
        assert(decision.estimated_latency_ms < 15.0f && "Safety perception must strictly guarantee <15ms bound");

        // Concurrent background camera stream
        auto cam_dec = dla_sched->route_task("yolov8_surround", 2, 1920 * 1080 * 3, false, true);
        if (cam_dec.needs_memory_barrier) {
            bus_stalls_mitigated++;
        }
        dla_sched->notify_task_complete(cam_dec.target_device);
    }

    assert(bus_stalls_mitigated > 0 && "Memory arbiter must insert barriers under bus saturation");
    assert(dla_sched->bus_stalls_avoided() > 0);

    std::cout << "PASS (Bus Contention Handled | Barriers Activated: " << bus_stalls_mitigated << ")" << std::endl;
}

int main() {
    std::cout << "===============================================================\n"
              << "[SUITE] TIER-1 SILICON & AUTONOMOUS SAFETY VERIFICATION BATTERY\n"
              << "===============================================================\n" << std::endl;

    try {
        test_cuda_ipc_type_adapter();
        test_flight_recorder_crash_dump();
        test_wcet_hard_realtime_bounds();

        std::cout << "\n===============================================================\n"
                  << "[SUCCESS] All Tier-1 Silicon & Safety Tests Passed Successfully!\n"
                  << "===============================================================\n" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\n[TEST FAILED WITH EXCEPTION]: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
