#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include "Scheduler.hpp"
#include "ResourceMonitor.hpp"
#include "TensorRTRunner.hpp"
#include <chrono>

namespace py = pybind11;
using namespace edge_scheduler;

uint64_t current_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

PYBIND11_MODULE(edge_scheduler_py, m) {
    m.doc() = "Edge AI Model Scheduler with C++ Core for Jetson devices";

    py::enum_<Priority>(m, "Priority")
        .value("HIGH", Priority::HIGH)
        .value("MEDIUM", Priority::MEDIUM)
        .value("LOW", Priority::LOW)
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

    py::class_<Scheduler>(m, "Scheduler")
        .def(py::init<std::shared_ptr<ResourceMonitor>>())
        .def("start", &Scheduler::start)
        .def("stop", &Scheduler::stop)
        .def("submit_task", [](Scheduler& self, const std::string& model_id, Priority priority, std::function<void()> payload) {
            InferenceTask task;
            task.model_id = model_id;
            task.priority = priority;
            task.payload = payload;
            task.timestamp_ms = current_time_ms();
            
            // Release GIL before submitting in case it blocks (though our queue shouldn't block)
            py::gil_scoped_release release;
            self.submit_task(task);
        });

    py::class_<TensorRTRunner, std::shared_ptr<TensorRTRunner>>(m, "TensorRTRunner")
        .def(py::init<>())
        .def("load_engine", &TensorRTRunner::load_engine)
        .def("infer", [](TensorRTRunner& self) {
            py::gil_scoped_release release; // Release GIL during intensive C++ inference
            return self.infer();
        });
}
