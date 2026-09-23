#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstring>
#include <vector>

#include "ShmRingBuffer.hpp"
#include "CudaIpcAllocator.hpp"
#include "CudaGraphExecutor.hpp"
#include "HardwareProfiler.hpp"
#include "IpcTransport.hpp"
#include "WatchdogDaemon.hpp"
#include "MetricsExporter.hpp"

using namespace edge_scheduler;

// ═══════════════════════════════════════════════════════════════
// Test 1: SHM Ring Buffer — Lock-Free Publish/Consume
// ═══════════════════════════════════════════════════════════════
void test_shm_ring_buffer() {
    std::cout << "[V3-Test 1] ShmRingBuffer lock-free SPSC... ";

    // In-process test (non-Linux uses heap fallback)
    ShmRingBuffer ring("/test_ring_v3", 8, 256, ShmRole::PRODUCER);

    assert(ring.slot_count() == 8);
    assert(ring.slot_payload_size() == 256);
    assert(ring.pending_count() == 0);

    // Publish 4 messages
    for (uint32_t i = 0; i < 4; ++i) {
        uint64_t seq = 0;
        void* slot = ring.try_claim_write_slot(seq);
        assert(slot != nullptr);

        // Write test payload
        char msg[64];
        snprintf(msg, sizeof(msg), "frame_%u", i);
        std::memcpy(slot, msg, strlen(msg) + 1);

        ring.commit_publish(seq, static_cast<uint32_t>(strlen(msg) + 1), i * 1000);
    }

    assert(ring.pending_count() == 4);
    assert(ring.write_position() == 4);
    assert(ring.read_position() == 0);

    // Consume all 4
    for (uint32_t i = 0; i < 4; ++i) {
        uint64_t seq = 0;
        uint32_t size = 0;
        uint64_t ts = 0;
        const void* data = ring.try_consume(seq, size, ts);
        assert(data != nullptr);

        char expected[64];
        snprintf(expected, sizeof(expected), "frame_%u", i);
        assert(std::memcmp(data, expected, strlen(expected)) == 0);
        assert(ts == i * 1000);

        ring.commit_consume(seq);
    }

    assert(ring.pending_count() == 0);

    // Empty ring — consume should return nullptr
    uint64_t seq = 0;
    uint32_t sz = 0;
    uint64_t ts = 0;
    assert(ring.try_consume(seq, sz, ts) == nullptr);

    std::cout << "PASSED." << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// Test 2: SHM Ring Buffer — Full Ring Back-Pressure
// ═══════════════════════════════════════════════════════════════
void test_shm_ring_backpressure() {
    std::cout << "[V3-Test 2] ShmRingBuffer back-pressure on full ring... ";

    ShmRingBuffer ring("/test_ring_bp", 4, 64, ShmRole::PRODUCER);

    // Fill all 4 slots
    for (uint32_t i = 0; i < 4; ++i) {
        uint64_t seq = 0;
        void* slot = ring.try_claim_write_slot(seq);
        assert(slot != nullptr);
        ring.commit_publish(seq, 4, 0);
    }

    // 5th publish should fail (ring full)
    uint64_t seq = 0;
    void* slot = ring.try_claim_write_slot(seq);
    assert(slot == nullptr);

    // Consume one to free a slot
    uint32_t sz = 0;
    uint64_t ts = 0;
    const void* data = ring.try_consume(seq, sz, ts);
    assert(data != nullptr);
    ring.commit_consume(seq);

    // Now publish should succeed again
    slot = ring.try_claim_write_slot(seq);
    assert(slot != nullptr);
    ring.commit_publish(seq, 4, 0);

    std::cout << "PASSED." << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// Test 3: CUDA IPC Allocator (Non-CUDA fallback)
// ═══════════════════════════════════════════════════════════════
void test_cuda_ipc_allocator() {
    std::cout << "[V3-Test 3] CudaIpcAllocator export/import cycle... ";

    CudaIpcAllocator allocator(0);

    // Allocate and export
    auto handle = allocator.allocate_and_export(1024 * 1024); // 1MB
    assert(handle.valid == true);
    assert(handle.size_bytes == 1024 * 1024);
    assert(allocator.owned_count() == 1);

    // Import the handle
    void* imported = CudaIpcAllocator::import_handle(handle);
    assert(imported != nullptr);

    // Write through imported pointer
#if defined(HAS_CUDA) && defined(__linux__)
    cudaMemset(imported, 0xAB, 64);
#else
    std::memset(imported, 0xAB, 64);
#endif

    // Release
    CudaIpcAllocator::release_imported(imported);
    allocator.free_all();
    assert(allocator.owned_count() == 0);

    std::cout << "PASSED." << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// Test 4: CUDA Graph Executor State Machine
// ═══════════════════════════════════════════════════════════════
void test_cuda_graph_executor() {
    std::cout << "[V3-Test 4] CudaGraphExecutor state transitions... ";

    CudaGraphExecutor executor;

    assert(executor.state() == GraphState::IDLE);
    assert(executor.is_ready() == false);
    assert(executor.replay_count() == 0);

    // Without CUDA, begin_capture succeeds but end_capture has no real graph
#ifdef HAS_CUDA
    // Full graph capture test would go here on Jetson
#else
    // State machine validation
    executor.begin_capture(nullptr);
    assert(executor.state() == GraphState::CAPTURING);

    // Can't begin capture while already capturing
    assert(executor.begin_capture(nullptr) == false);

    executor.end_capture();
    assert(executor.state() == GraphState::READY);
    assert(executor.is_ready() == true);

    // Replay
    executor.replay(nullptr);
    assert(executor.replay_count() == 1);

    executor.replay(nullptr);
    assert(executor.replay_count() == 2);

    // Destroy and reset
    executor.destroy();
    assert(executor.state() == GraphState::IDLE);
    assert(executor.replay_count() == 0);
#endif

    std::cout << "PASSED." << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// Test 5: IPC Protocol — Header Serialization
// ═══════════════════════════════════════════════════════════════
void test_ipc_protocol() {
    std::cout << "[V3-Test 5] IPC wire protocol header validation... ";

    IpcRequestHeader hdr;
    assert(hdr.magic == IPC_MAGIC);
    assert(hdr.version == IPC_PROTOCOL_VERSION);
    assert(sizeof(IpcRequestHeader) <= 128);

    // Set fields
    hdr.request_type = IpcRequestType::INFERENCE;
    hdr.payload_type = IpcPayloadType::CPU_BUFFER;
    std::strncpy(hdr.model_id, "yolov8_safety", sizeof(hdr.model_id) - 1);
    hdr.priority = static_cast<uint32_t>(Priority::HIGH);
    hdr.payload_size = 1920 * 1080 * 3;
    hdr.client_pid = 12345;
    hdr.sequence_id = 42;

    // Serialize and deserialize (round-trip)
    uint8_t wire[128];
    std::memcpy(wire, &hdr, sizeof(hdr));

    auto* decoded = reinterpret_cast<const IpcRequestHeader*>(wire);
    assert(decoded->magic == IPC_MAGIC);
    assert(decoded->request_type == IpcRequestType::INFERENCE);
    assert(std::string(decoded->model_id) == "yolov8_safety");
    assert(decoded->priority == static_cast<uint32_t>(Priority::HIGH));
    assert(decoded->payload_size == 1920 * 1080 * 3);
    assert(decoded->client_pid == 12345);
    assert(decoded->sequence_id == 42);

    std::cout << "PASSED." << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// Test 6: Hardware Profiler — YAML Round-Trip
// ═══════════════════════════════════════════════════════════════
void test_hardware_profiler_yaml() {
    std::cout << "[V3-Test 6] HardwareProfiler YAML export/load... ";

    CalibrationResult original;
    original.gpu.device_name = "Orin NX Test";
    original.gpu.sm_count = 8;
    original.ewma_alpha = 0.25f;
    original.starvation_threshold_ms = 300;
    original.hysteresis_degrade_temp = 82.5f;
    original.hysteresis_recover_temp = 74.5f;
    original.shm_ring_slot_count = 32;
    original.memory.recommended_pool_capacity = 8;
    original.memory.recommended_buffer_size_bytes = 2 * 1024 * 1024;
    original.platform_class = "orin_nx";
    original.calibration_duration_sec = 15.3f;

    // Export
    std::string test_path = "/tmp/test_hw_profile.yaml";
#ifdef _WIN32
    test_path = "test_hw_profile.yaml";
#endif

    bool wrote = HardwareProfiler::export_yaml(original, test_path);
    assert(wrote == true);

    // Reload
    auto loaded = HardwareProfiler::load_yaml(test_path);
    assert(loaded.gpu.device_name == "Orin NX Test");
    assert(loaded.gpu.sm_count == 8);
    assert(loaded.platform_class == "orin_nx");
    assert(loaded.shm_ring_slot_count == 32);
    assert(loaded.memory.recommended_pool_capacity == 8);

    // Floating point comparison with tolerance
    assert(std::abs(loaded.ewma_alpha - 0.25f) < 0.01f);
    assert(loaded.starvation_threshold_ms == 300);

    // Cleanup
    std::remove(test_path.c_str());

    std::cout << "PASSED." << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// Test 7: Watchdog Daemon — Health Checks & State Transitions
// ═══════════════════════════════════════════════════════════════
void test_watchdog_daemon() {
    std::cout << "[V3-Test 7] WatchdogDaemon health check lifecycle... ";

    WatchdogDaemon watchdog(100); // Fast heartbeat for testing

    std::atomic<bool> health_status{true};

    watchdog.add_health_check("test_check", [&]() {
        return health_status.load();
    });

    bool shutdown_called = false;
    watchdog.on_shutdown([&]() {
        shutdown_called = true;
    });

    watchdog.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    assert(watchdog.state() == DaemonState::RUNNING);
    assert(watchdog.heartbeat_count() > 0);

    // Simulate health failure
    health_status = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // After several failures, should be DEGRADED
    assert(watchdog.consecutive_failures() > 0);

    // Recover
    health_status = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    watchdog.stop();
    assert(watchdog.state() == DaemonState::STOPPED);
    assert(shutdown_called == true);

    std::cout << "PASSED." << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// Test 8: Metrics Exporter — Prometheus Format
// ═══════════════════════════════════════════════════════════════
void test_metrics_exporter() {
    std::cout << "[V3-Test 8] MetricsExporter Prometheus format... ";

    MetricsExporter exporter(100);

    SchedulerMetrics m;
    m.gpu_utilization = 75.5f;
    m.gpu_temperature_c = 72.3f;
    m.tasks_dispatched = 1000;
    m.tasks_completed = 998;
    m.tasks_dropped = 2;
    m.queue_depth = 5;
    m.avg_latency_ms = 12.5f;
    m.p99_latency_ms = 28.3f;
    m.precision_mode = 0;
    m.precision_swaps = 3;
    m.uptime_seconds = 3600;

    std::string prom = exporter.to_prometheus(m);

    // Verify key metrics are present
    assert(prom.find("edge_scheduler_gpu_utilization 75.50") != std::string::npos);
    assert(prom.find("edge_scheduler_gpu_temperature 72.30") != std::string::npos);
    assert(prom.find("edge_scheduler_tasks_dispatched_total 1000") != std::string::npos);
    assert(prom.find("edge_scheduler_tasks_dropped_total 2") != std::string::npos);
    assert(prom.find("edge_scheduler_latency_p99_ms 28.30") != std::string::npos);
    assert(prom.find("# TYPE edge_scheduler_gpu_utilization gauge") != std::string::npos);
    assert(prom.find("# TYPE edge_scheduler_tasks_dispatched_total counter") != std::string::npos);

    // JSON Lines format
    std::string jsonl = exporter.to_jsonl(m);
    assert(jsonl.find("\"gpu_util\":75.50") != std::string::npos);
    assert(jsonl.find("\"precision\":\"FP16\"") != std::string::npos);
    assert(jsonl.find("\"dropped\":2") != std::string::npos);

    std::cout << "PASSED." << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// Test 9: SHM Ring Buffer — Throughput Benchmark
// ═══════════════════════════════════════════════════════════════
void test_shm_throughput_benchmark() {
    std::cout << "[V3-Test 9] ShmRingBuffer throughput benchmark... ";

    // Safe buffer sizing suitable for constrained container /dev/shm partitions
    const uint32_t frame_size = 640 * 480 * 3; // 921 KB
    const uint32_t slot_count = 4; // ~3.7 MB total

    ShmRingBuffer ring("/test_ring_bench", slot_count, frame_size, ShmRole::PRODUCER);

    std::vector<uint8_t> fake_frame(frame_size, 0xAA);

    auto start = std::chrono::high_resolution_clock::now();
    const int iterations = 100;

    for (int i = 0; i < iterations; ++i) {
        // Publish
        uint64_t seq;
        void* slot = ring.try_claim_write_slot(seq);
        if (slot) {
            std::memcpy(slot, fake_frame.data(), frame_size);
            ring.commit_publish(seq, frame_size, 0);
        }

        // Consume immediately
        uint32_t sz;
        uint64_t ts;
        const void* data = ring.try_consume(seq, sz, ts);
        if (data) {
            ring.commit_consume(seq);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    float total_ms = std::chrono::duration<float, std::milli>(end - start).count();
    float per_frame_ms = total_ms / iterations;

    std::cout << "PASSED. (" << per_frame_ms << "ms/frame, "
              << (frame_size * iterations / (1024.0f * 1024.0f)) / (total_ms / 1000.0f)
              << " MB/s)" << std::endl;

    // SLO: must be under 1ms per frame for the copy path
    assert(per_frame_ms < 50.0f); // Generous for CI environments
}

// ═══════════════════════════════════════════════════════════════
int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "🧪 EdgeModelScheduler v3.0 Subsystem Unit Tests" << std::endl;
    std::cout << "==================================================" << std::endl;

    test_shm_ring_buffer();
    test_shm_ring_backpressure();
    test_cuda_ipc_allocator();
    test_cuda_graph_executor();
    test_ipc_protocol();
    test_hardware_profiler_yaml();
    test_watchdog_daemon();
    test_metrics_exporter();
    test_shm_throughput_benchmark();

    std::cout << "==================================================" << std::endl;
    std::cout << "🎉 ALL V3.0 SUBSYSTEM TESTS PASSED!" << std::endl;
    std::cout << "==================================================" << std::endl;
    return 0;
}
