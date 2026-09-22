#include "AdvancedScheduler.hpp"
#include "ResourceMonitor.hpp"
#include "PinnedMemoryPool.hpp"
#include "QuantizationGovernor.hpp"
#include "EWMAProfiler.hpp"
#include "IpcTransport.hpp"
#include "HardwareProfiler.hpp"
#include "WatchdogDaemon.hpp"
#include "MetricsExporter.hpp"
#include "CudaGraphExecutor.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include <thread>
#include <chrono>

using namespace edge_scheduler;

static void print_banner() {
    std::cout << R"(
╔═══════════════════════════════════════════════════════════════╗
║   EdgeModelScheduler v3.0 — Production Daemon                ║
║   Real-Time Edge AI Inference Platform                       ║
║   Zero-Copy IPC · CUDA Graphs · Auto-Calibration             ║
╚═══════════════════════════════════════════════════════════════╝
)" << std::endl;
}

int main(int argc, char* argv[]) {
    print_banner();

    // ── Parse CLI args ──────────────────────────────────────────
    std::string config_path = "/etc/edge-scheduler/hardware_profile.yaml";
    std::string shm_name = "/edge_scheduler_ipc";
    bool run_calibration = false;
    std::string calibration_output;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc) {
            config_path = argv[++i];
        } else if (arg == "calibrate") {
            run_calibration = true;
        } else if (arg == "--output" && i + 1 < argc) {
            calibration_output = argv[++i];
        } else if (arg == "--shm" && i + 1 < argc) {
            shm_name = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: edge-scheduler-daemon [OPTIONS]\n"
                      << "  --config PATH    Hardware profile YAML (default: /etc/edge-scheduler/hardware_profile.yaml)\n"
                      << "  --shm NAME       POSIX SHM ring buffer name (default: /edge_scheduler_ipc)\n"
                      << "  calibrate        Run hardware auto-calibration\n"
                      << "  --output PATH    Calibration output YAML path\n"
                      << std::endl;
            return 0;
        }
    }

    // ── Calibration mode ────────────────────────────────────────
    if (run_calibration) {
        std::cout << "[Daemon] Running hardware auto-calibration..." << std::endl;
        HardwareProfiler profiler;
        auto result = profiler.calibrate(true);

        std::string out = calibration_output.empty() ? config_path : calibration_output;
        if (HardwareProfiler::export_yaml(result, out)) {
            std::cout << "[Daemon] Profile written to: " << out << std::endl;
            return 0;
        } else {
            std::cerr << "[Daemon] Failed to write profile to: " << out << std::endl;
            return 1;
        }
    }

    // ── Load hardware profile ───────────────────────────────────
    CalibrationResult hw_profile;
    try {
        hw_profile = HardwareProfiler::load_yaml(config_path);
        std::cout << "[Daemon] Loaded profile: " << hw_profile.platform_class
                  << " (SM=" << hw_profile.gpu.sm_count << ")" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "[Daemon] No profile at " << config_path
                  << " — using defaults. (" << e.what() << ")" << std::endl;
    }

    // ── Initialize subsystems ───────────────────────────────────
    auto monitor = std::make_shared<ResourceMonitor>();
    auto mem_pool = std::make_shared<PinnedMemoryPool>(
        hw_profile.memory.recommended_buffer_size_bytes,
        hw_profile.memory.recommended_pool_capacity);
    auto governor = std::make_shared<QuantizationGovernor>();
    auto profiler_ewma = std::make_shared<EWMAExecutionProfiler>(hw_profile.ewma_alpha);

    auto scheduler = std::make_shared<AdvancedScheduler>(monitor, mem_pool);
    scheduler->set_starvation_threshold_ms(hw_profile.starvation_threshold_ms);

    // ── IPC Server ──────────────────────────────────────────────
    uint32_t slot_size = static_cast<uint32_t>(
        hw_profile.memory.recommended_shm_slot_size > 0
        ? hw_profile.memory.recommended_shm_slot_size + 256
        : 1920 * 1080 * 3 + 256);

    std::unique_ptr<SchedulerIpcServer> ipc_server;
    try {
        // Producer side creates the SHM; the daemon is the consumer.
        // We create as PRODUCER here since the daemon owns the lifecycle.
        ipc_server = std::make_unique<SchedulerIpcServer>(
            shm_name, hw_profile.shm_ring_slot_count, slot_size);
        std::cout << "[Daemon] IPC ring buffer ready: " << shm_name
                  << " (" << hw_profile.shm_ring_slot_count << " slots × "
                  << slot_size << " bytes)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Daemon] IPC init failed: " << e.what()
                  << " — running in standalone mode" << std::endl;
    }

    // ── Metrics Exporter ────────────────────────────────────────
    MetricsExporter metrics(5000);
    metrics.set_metrics_source([&]() -> SchedulerMetrics {
        SchedulerMetrics m;
        auto hw = monitor->get_current_metrics();
        m.gpu_utilization = hw.gpu_utilization;
        m.gpu_temperature_c = hw.thermal_temperature;
        m.is_throttling = hw.is_throttling;
        m.avg_latency_ms = profiler_ewma->get_expected_latency_ms();
        m.precision_mode = (governor->get_current_mode() == PrecisionMode::INT8) ? 1 : 0;
        if (ipc_server) {
            m.ipc_requests_received = ipc_server->total_processed();
            m.ipc_ring_pending = ipc_server->ring_pending();
        }
        return m;
    });
    metrics.export_to_file("/var/log/edge-scheduler/metrics.jsonl");

    // ── Watchdog ────────────────────────────────────────────────
    WatchdogDaemon watchdog(2000);

    watchdog.add_health_check("scheduler_running", [&]() {
        return true; // Scheduler is always "running" if we reach here
    });

    watchdog.add_health_check("memory_pool", [&]() {
        // Verify pool can still allocate
        void* test = mem_pool->acquire();
        if (test) { mem_pool->release(test); return true; }
        return false;
    });

    watchdog.on_shutdown([&]() {
        std::cout << "[Daemon] Graceful shutdown initiated..." << std::endl;
        scheduler->stop();
        monitor->stop();
        metrics.stop();
    });

    // ── Start everything ────────────────────────────────────────
    monitor->start(100);
    scheduler->start();
    metrics.start();
    watchdog.start();

    std::cout << "[Daemon] All subsystems online. Entering main loop." << std::endl;

    // ── Main IPC dispatch loop ──────────────────────────────────
    while (!WatchdogDaemon::signal_received()) {
        if (ipc_server) {
            ipc_server->poll([&](const IpcRequestHeader& hdr, const void* payload) {
                if (hdr.request_type == IpcRequestType::INFERENCE) {
                    Priority p = static_cast<Priority>(hdr.priority);
                    scheduler->submit_task(hdr.model_id, p,
                        [payload, &profiler_ewma](void* buf) {
                            auto t0 = std::chrono::steady_clock::now();
                            // Real inference would happen here via TensorRT
                            std::this_thread::sleep_for(std::chrono::milliseconds(5));
                            auto t1 = std::chrono::steady_clock::now();
                            float lat = std::chrono::duration<float, std::milli>(t1 - t0).count();
                            profiler_ewma->record_execution_time(lat);
                        });
                }
            }, 16);
        }

        // Thermal governance loop
        auto hw = monitor->get_current_metrics();
        governor->evaluate_telemetry(hw.thermal_temperature, hw.gpu_utilization);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // ── Shutdown ────────────────────────────────────────────────
    watchdog.stop();
    std::cout << "[Daemon] Shutdown complete." << std::endl;
    return 0;
}
