#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "Scheduler.hpp"
#include "AdvancedScheduler.hpp"
#include "ResourceMonitor.hpp"
#include "PinnedMemoryPool.hpp"
#include "QuantizationGovernor.hpp"
#include "EWMAProfiler.hpp"
#include "ModelRegistry.hpp"
#include "TensorRTRunner.hpp"
#include "CudaGraphExecutor.hpp"
#include "HardwareProfiler.hpp"
#include "WatchdogDaemon.hpp"
#include "MetricsExporter.hpp"
#include "IpcTransport.hpp"
#include "ShmRingBuffer.hpp"
#include "SlackBatcher.hpp"
#include "BatteryQosGovernor.hpp"
#include "DlaCoScheduler.hpp"
#include "EmbeddedDashboard.hpp"

namespace py = pybind11;
using namespace edge_scheduler;

PYBIND11_MODULE(edge_scheduler_py, m) {
    m.doc() = "EdgeModelScheduler v3.0 — Production Edge AI Platform with IPC, CUDA Graphs & Fleet Observability";

    // ═════════════════════════════════════════════════════════
    // v2.x Core Types (preserved)
    // ═════════════════════════════════════════════════════════

    py::enum_<Priority>(m, "Priority")
        .value("HIGH", Priority::HIGH)
        .value("MEDIUM", Priority::MEDIUM)
        .value("LOW", Priority::LOW)
        .export_values();

    py::enum_<PrecisionMode>(m, "PrecisionMode")
        .value("FP16", PrecisionMode::FP16)
        .value("INT8", PrecisionMode::INT8)
        .export_values();

    py::class_<SystemMetrics>(m, "SystemMetrics")
        .def_readonly("gpu_utilization", &SystemMetrics::gpu_utilization)
        .def_readonly("thermal_temperature", &SystemMetrics::thermal_temperature)
        .def_readonly("is_throttling", &SystemMetrics::is_throttling);

    py::class_<ResourceMonitor, std::shared_ptr<ResourceMonitor>>(m, "ResourceMonitor")
        .def(py::init<>())
        .def("start", &ResourceMonitor::start, py::arg("polling_interval_ms") = 100)
        .def("stop", &ResourceMonitor::stop)
        .def("get_current_metrics", &ResourceMonitor::get_current_metrics);

    py::class_<PinnedMemoryPool, std::shared_ptr<PinnedMemoryPool>>(m, "PinnedMemoryPool")
        .def(py::init<size_t, size_t>(), py::arg("buffer_size_bytes") = 1024 * 1024, py::arg("pool_capacity") = 4)
        .def("buffer_size", &PinnedMemoryPool::buffer_size);

    py::class_<QuantizationGovernor, std::shared_ptr<QuantizationGovernor>>(m, "QuantizationGovernor")
        .def(py::init<>())
        .def("evaluate_telemetry", &QuantizationGovernor::evaluate_telemetry)
        .def("get_current_mode", &QuantizationGovernor::get_current_mode);

    py::class_<EWMAExecutionProfiler, std::shared_ptr<EWMAExecutionProfiler>>(m, "EWMAExecutionProfiler")
        .def(py::init<float>(), py::arg("alpha") = 0.2f)
        .def("record_execution_time", &EWMAExecutionProfiler::record_execution_time)
        .def("get_expected_latency_ms", &EWMAExecutionProfiler::get_expected_latency_ms)
        .def("will_exceed_deadline", &EWMAExecutionProfiler::will_exceed_deadline);

    py::class_<ModelConfig>(m, "ModelConfig")
        .def(py::init<>())
        .def_readwrite("model_id", &ModelConfig::model_id)
        .def_readwrite("engine_path", &ModelConfig::engine_path)
        .def_readwrite("priority_level", &ModelConfig::priority_level)
        .def_readwrite("latency_slo_ms", &ModelConfig::latency_slo_ms)
        .def_readwrite("input_buffer_bytes", &ModelConfig::input_buffer_bytes)
        .def_readwrite("allow_quantization_fallback", &ModelConfig::allow_quantization_fallback);

    py::class_<ModelRegistry, std::shared_ptr<ModelRegistry>>(m, "ModelRegistry")
        .def(py::init<>())
        .def("register_model", &ModelRegistry::register_model)
        .def("has_model", &ModelRegistry::has_model)
        .def("count", &ModelRegistry::count)
        .def("get_all_model_ids", &ModelRegistry::get_all_model_ids)
        .def("load_from_csv", &ModelRegistry::load_from_csv);

    py::class_<Scheduler, std::shared_ptr<Scheduler>>(m, "Scheduler")
        .def(py::init<std::shared_ptr<ResourceMonitor>>())
        .def("start", &Scheduler::start)
        .def("stop", &Scheduler::stop)
        .def("submit_task", [](Scheduler& self, const std::string& model_id, Priority priority, std::function<void()> payload) {
            LegacyInferenceTask task;
            task.model_id = model_id;
            task.priority = priority;
            task.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            task.payload = [payload]() {
                if (payload) {
                    py::gil_scoped_acquire acquire;
                    payload();
                }
            };
            py::gil_scoped_release release;
            self.submit_task(task);
        });

    py::class_<AdvancedScheduler, std::shared_ptr<AdvancedScheduler>>(m, "AdvancedScheduler")
        .def(py::init<std::shared_ptr<ResourceMonitor>, std::shared_ptr<PinnedMemoryPool>>())
        .def("start", &AdvancedScheduler::start)
        .def("stop", &AdvancedScheduler::stop)
        .def("set_starvation_threshold_ms", &AdvancedScheduler::set_starvation_threshold_ms)
        .def("submit_task", [](AdvancedScheduler& self, const std::string& model_id, Priority priority, std::function<void()> payload) {
            py::gil_scoped_release release;
            self.submit_task(model_id, priority, [payload](void* /*buf*/) {
                if (payload) {
                    py::gil_scoped_acquire acquire;
                    payload();
                }
            });
        });

    py::class_<TensorRTRunner, std::shared_ptr<TensorRTRunner>>(m, "TensorRTRunner")
        .def(py::init<>())
        .def("load_engine", &TensorRTRunner::load_engine)
        .def("infer", [](TensorRTRunner& self) {
            py::gil_scoped_release release;
            return self.infer();
        });

    // ═════════════════════════════════════════════════════════
    // v3.0 Production Subsystems
    // ═════════════════════════════════════════════════════════

    // CUDA Graph Executor
    py::enum_<GraphState>(m, "GraphState")
        .value("IDLE", GraphState::IDLE)
        .value("CAPTURING", GraphState::CAPTURING)
        .value("READY", GraphState::READY)
        .value("ERROR", GraphState::ERROR)
        .export_values();

    py::class_<CudaGraphExecutor>(m, "CudaGraphExecutor")
        .def(py::init<>())
        .def("begin_capture", &CudaGraphExecutor::begin_capture, py::arg("cuda_stream") = nullptr)
        .def("end_capture", &CudaGraphExecutor::end_capture)
        .def("replay", &CudaGraphExecutor::replay, py::arg("cuda_stream") = nullptr)
        .def("destroy", &CudaGraphExecutor::destroy)
        .def("state", &CudaGraphExecutor::state)
        .def("replay_count", &CudaGraphExecutor::replay_count)
        .def("captured_node_count", &CudaGraphExecutor::captured_node_count)
        .def("is_ready", &CudaGraphExecutor::is_ready);

    // Hardware Profiler
    py::class_<GpuInfo>(m, "GpuInfo")
        .def(py::init<>())
        .def_readwrite("device_name", &GpuInfo::device_name)
        .def_readwrite("sm_count", &GpuInfo::sm_count)
        .def_readwrite("cuda_cores", &GpuInfo::cuda_cores)
        .def_readwrite("total_memory_bytes", &GpuInfo::total_memory_bytes)
        .def_readwrite("compute_major", &GpuInfo::compute_major)
        .def_readwrite("compute_minor", &GpuInfo::compute_minor);

    py::class_<ThermalProfile>(m, "ThermalProfile")
        .def(py::init<>())
        .def_readwrite("idle_temp_c", &ThermalProfile::idle_temp_c)
        .def_readwrite("sustained_temp_c", &ThermalProfile::sustained_temp_c)
        .def_readwrite("thermal_rise_rate_c_per_sec", &ThermalProfile::thermal_rise_rate_c_per_sec)
        .def_readwrite("thermal_dissipation_rate_c_per_sec", &ThermalProfile::thermal_dissipation_rate_c_per_sec)
        .def_readwrite("recommended_throttle_threshold_c", &ThermalProfile::recommended_throttle_threshold_c)
        .def_readwrite("recommended_recovery_threshold_c", &ThermalProfile::recommended_recovery_threshold_c);

    py::class_<CalibrationResult>(m, "CalibrationResult")
        .def(py::init<>())
        .def_readwrite("gpu", &CalibrationResult::gpu)
        .def_readwrite("thermal", &CalibrationResult::thermal)
        .def_readwrite("ewma_alpha", &CalibrationResult::ewma_alpha)
        .def_readwrite("starvation_threshold_ms", &CalibrationResult::starvation_threshold_ms)
        .def_readwrite("hysteresis_degrade_temp", &CalibrationResult::hysteresis_degrade_temp)
        .def_readwrite("hysteresis_recover_temp", &CalibrationResult::hysteresis_recover_temp)
        .def_readwrite("platform_class", &CalibrationResult::platform_class)
        .def_readwrite("calibration_duration_sec", &CalibrationResult::calibration_duration_sec);

    py::class_<HardwareProfiler>(m, "HardwareProfiler")
        .def(py::init<>())
        .def("calibrate", &HardwareProfiler::calibrate, py::arg("verbose") = true)
        .def_static("export_yaml", &HardwareProfiler::export_yaml)
        .def_static("load_yaml", &HardwareProfiler::load_yaml);

    // Watchdog Daemon
    py::enum_<DaemonState>(m, "DaemonState")
        .value("STARTING", DaemonState::STARTING)
        .value("RUNNING", DaemonState::RUNNING)
        .value("DEGRADED", DaemonState::DEGRADED)
        .value("STOPPING", DaemonState::STOPPING)
        .value("STOPPED", DaemonState::STOPPED)
        .export_values();

    py::class_<WatchdogDaemon>(m, "WatchdogDaemon")
        .def(py::init<uint32_t>(), py::arg("heartbeat_interval_ms") = 2000)
        .def("add_health_check", [](WatchdogDaemon& self, const std::string& name, std::function<bool()> check) {
            self.add_health_check(name, [check]() {
                py::gil_scoped_acquire acquire;
                return check();
            });
        })
        .def("start", &WatchdogDaemon::start)
        .def("stop", &WatchdogDaemon::stop)
        .def("state", &WatchdogDaemon::state)
        .def("heartbeat_count", &WatchdogDaemon::heartbeat_count)
        .def("is_healthy", &WatchdogDaemon::is_healthy);

    // Metrics Exporter
    py::class_<SchedulerMetrics>(m, "SchedulerMetrics")
        .def(py::init<>())
        .def_readwrite("gpu_utilization", &SchedulerMetrics::gpu_utilization)
        .def_readwrite("gpu_temperature_c", &SchedulerMetrics::gpu_temperature_c)
        .def_readwrite("is_throttling", &SchedulerMetrics::is_throttling)
        .def_readwrite("tasks_dispatched", &SchedulerMetrics::tasks_dispatched)
        .def_readwrite("tasks_completed", &SchedulerMetrics::tasks_completed)
        .def_readwrite("tasks_dropped", &SchedulerMetrics::tasks_dropped)
        .def_readwrite("queue_depth", &SchedulerMetrics::queue_depth)
        .def_readwrite("avg_latency_ms", &SchedulerMetrics::avg_latency_ms)
        .def_readwrite("p99_latency_ms", &SchedulerMetrics::p99_latency_ms)
        .def_readwrite("precision_mode", &SchedulerMetrics::precision_mode)
        .def_readwrite("precision_swaps", &SchedulerMetrics::precision_swaps)
        .def_readwrite("uptime_seconds", &SchedulerMetrics::uptime_seconds);

    py::class_<MetricsExporter>(m, "MetricsExporter")
        .def(py::init<uint32_t>(), py::arg("export_interval_ms") = 5000)
        .def("to_prometheus", &MetricsExporter::to_prometheus)
        .def("to_jsonl", &MetricsExporter::to_jsonl)
        .def("export_to_file", &MetricsExporter::export_to_file)
        .def("start", &MetricsExporter::start)
        .def("stop", &MetricsExporter::stop)
        .def("export_count", &MetricsExporter::export_count);

    // ═════════════════════════════════════════════════════════
    // God-Level Subsystems: SlackBatcher, BatteryQoS, DLA, Dashboard
    // ═════════════════════════════════════════════════════════

    // SlackBatcher
    py::class_<SlackBatcher, std::shared_ptr<SlackBatcher>>(m, "SlackBatcher")
        .def(py::init<uint32_t, float>(), py::arg("max_batch_size") = 4, py::arg("max_coalesce_window_ms") = 5.0f)
        .def("pending_count", &SlackBatcher::pending_count)
        .def("total_submitted", &SlackBatcher::total_submitted)
        .def("total_dispatched", &SlackBatcher::total_dispatched)
        .def("stale_frames_shed", &SlackBatcher::stale_frames_shed)
        .def("batches_created", &SlackBatcher::batches_created)
        .def("slo_compliance_rate", &SlackBatcher::slo_compliance_rate);

    // Battery QoS Governor
    py::enum_<BatteryQosMode>(m, "BatteryQosMode")
        .value("EXTERNAL_POWER", BatteryQosMode::EXTERNAL_POWER)
        .value("NORMAL", BatteryQosMode::NORMAL)
        .value("ECO", BatteryQosMode::ECO)
        .value("CRITICAL_RTH", BatteryQosMode::CRITICAL_RTH)
        .export_values();

    py::class_<BatteryTelemetry>(m, "BatteryTelemetry")
        .def(py::init<>())
        .def_readwrite("soc_percent", &BatteryTelemetry::soc_percent)
        .def_readwrite("voltage_v", &BatteryTelemetry::voltage_v)
        .def_readwrite("current_a", &BatteryTelemetry::current_a)
        .def_readwrite("power_w", &BatteryTelemetry::power_w)
        .def_readwrite("is_charging", &BatteryTelemetry::is_charging);

    py::class_<BatteryQosGovernor, std::shared_ptr<BatteryQosGovernor>>(m, "BatteryQosGovernor")
        .def(py::init<float, float>(), py::arg("eco_threshold") = 40.0f, py::arg("critical_threshold") = 20.0f)
        .def("update_telemetry", &BatteryQosGovernor::update_telemetry, py::arg("soc"), py::arg("is_charging") = false, py::arg("voltage") = 16.8f, py::arg("current") = 2.5f)
        .def("current_mode", &BatteryQosGovernor::current_mode)
        .def("mode_to_string", &BatteryQosGovernor::mode_to_string)
        .def("telemetry", &BatteryQosGovernor::telemetry)
        .def("should_admit_task", &BatteryQosGovernor::should_admit_task)
        .def("should_force_int8", &BatteryQosGovernor::should_force_int8);

    // DLA Co-Scheduler
    py::enum_<AccelDevice>(m, "AccelDevice")
        .value("GPU", AccelDevice::GPU)
        .value("DLA0", AccelDevice::DLA0)
        .value("DLA1", AccelDevice::DLA1)
        .value("HYBRID", AccelDevice::HYBRID)
        .export_values();

    py::class_<RoutingDecision>(m, "RoutingDecision")
        .def(py::init<>())
        .def_readwrite("target_device", &RoutingDecision::target_device)
        .def_readwrite("reason", &RoutingDecision::reason)
        .def_readwrite("estimated_latency_ms", &RoutingDecision::estimated_latency_ms)
        .def_readwrite("needs_memory_barrier", &RoutingDecision::needs_memory_barrier);

    py::class_<DlaCoScheduler, std::shared_ptr<DlaCoScheduler>>(m, "DlaCoScheduler")
        .def(py::init<uint32_t, float>(), py::arg("available_dla_cores") = 2, py::arg("max_lpddr5_bandwidth_gbps") = 204.8f)
        .def("route_task", &DlaCoScheduler::route_task, py::arg("model_name"), py::arg("priority"), py::arg("input_bytes"), py::arg("has_custom_ops") = false, py::arg("is_pure_cnn") = true)
        .def("notify_task_complete", &DlaCoScheduler::notify_task_complete)
        .def("update_memory_bus_load", &DlaCoScheduler::update_memory_bus_load)
        .def("available_dla_cores", &DlaCoScheduler::available_dla_cores)
        .def("gpu_tasks", &DlaCoScheduler::gpu_tasks)
        .def("dla0_tasks", &DlaCoScheduler::dla0_tasks)
        .def("dla1_tasks", &DlaCoScheduler::dla1_tasks)
        .def("bus_stalls_avoided", &DlaCoScheduler::bus_stalls_avoided)
        .def("device_to_string", &DlaCoScheduler::device_to_string);

    // Embedded Dashboard
    py::class_<EmbeddedDashboard>(m, "EmbeddedDashboard")
        .def(py::init<uint16_t>(), py::arg("port") = 9090)
        .def("start", &EmbeddedDashboard::start)
        .def("stop", &EmbeddedDashboard::stop)
        .def("is_running", &EmbeddedDashboard::is_running)
        .def("port", &EmbeddedDashboard::port);
}
