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

namespace py = pybind11;
using namespace edge_scheduler;

PYBIND11_MODULE(edge_scheduler_py, m) {
    m.doc() = "Enterprise Edge AI Model Scheduler with C++17 Core for Jetson & Embedded SoCs";

    // 1. Priority Enum
    py::enum_<Priority>(m, "Priority")
        .value("HIGH", Priority::HIGH)
        .value("MEDIUM", Priority::MEDIUM)
        .value("LOW", Priority::LOW)
        .export_values();

    // 2. PrecisionMode Enum
    py::enum_<PrecisionMode>(m, "PrecisionMode")
        .value("FP16", PrecisionMode::FP16)
        .value("INT8", PrecisionMode::INT8)
        .export_values();

    // 3. System Metrics
    py::class_<SystemMetrics>(m, "SystemMetrics")
        .def_readonly("gpu_utilization", &SystemMetrics::gpu_utilization)
        .def_readonly("thermal_temperature", &SystemMetrics::thermal_temperature)
        .def_readonly("is_throttling", &SystemMetrics::is_throttling);

    // 4. Resource Monitor
    py::class_<ResourceMonitor, std::shared_ptr<ResourceMonitor>>(m, "ResourceMonitor")
        .def(py::init<>())
        .def("start", &ResourceMonitor::start, py::arg("polling_interval_ms") = 100)
        .def("stop", &ResourceMonitor::stop)
        .def("get_current_metrics", &ResourceMonitor::get_current_metrics);

    // 5. Zero-Copy Pinned Memory Pool
    py::class_<PinnedMemoryPool, std::shared_ptr<PinnedMemoryPool>>(m, "PinnedMemoryPool")
        .def(py::init<size_t, size_t>(), py::arg("buffer_size_bytes") = 1024 * 1024, py::arg("pool_capacity") = 4)
        .def("buffer_size", &PinnedMemoryPool::buffer_size);

    // 6. Quantization Governor
    py::class_<QuantizationGovernor, std::shared_ptr<QuantizationGovernor>>(m, "QuantizationGovernor")
        .def(py::init<>())
        .def("evaluate_telemetry", &QuantizationGovernor::evaluate_telemetry)
        .def("get_current_mode", &QuantizationGovernor::get_current_mode);

    // 7. EWMA Execution Profiler
    py::class_<EWMAExecutionProfiler, std::shared_ptr<EWMAExecutionProfiler>>(m, "EWMAExecutionProfiler")
        .def(py::init<float>(), py::arg("alpha") = 0.2f)
        .def("record_execution_time", &EWMAExecutionProfiler::record_execution_time)
        .def("get_expected_latency_ms", &EWMAExecutionProfiler::get_expected_latency_ms)
        .def("will_exceed_deadline", &EWMAExecutionProfiler::will_exceed_deadline);

    // 8. Model Config & Registry
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

    // 8.5 Legacy Scheduler
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

    // 9. AdvancedScheduler Core
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

    // 10. TensorRT Runner
    py::class_<TensorRTRunner, std::shared_ptr<TensorRTRunner>>(m, "TensorRTRunner")
        .def(py::init<>())
        .def("load_engine", &TensorRTRunner::load_engine)
        .def("infer", [](TensorRTRunner& self) {
            py::gil_scoped_release release;
            return self.infer();
        });
}
