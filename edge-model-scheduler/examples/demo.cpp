#include "Scheduler.hpp"
#include "ResourceMonitor.hpp"
#include "TensorRTRunner.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace edge_scheduler;

int main() {
    std::cout << "Starting Edge Scheduler C++ Demo..." << std::endl;

    auto monitor = std::make_shared<ResourceMonitor>();
    monitor->start(100);

    Scheduler scheduler(monitor);
    scheduler.start();

    TensorRTRunner trt;
    trt.load_engine("dummy.engine");

    auto high_priority_task = [&trt]() {
        std::cout << "Executing High Priority Task (C++)" << std::endl;
        trt.infer();
    };

    auto low_priority_task = []() {
        std::cout << "Executing Low Priority Task (C++)" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    };

    for (int i = 0; i < 5; ++i) {
        LegacyInferenceTask t_low;
        t_low.model_id = "low_model";
        t_low.priority = Priority::LOW;
        t_low.payload = low_priority_task;
        t_low.timestamp_ms = 0;
        scheduler.submit_task(t_low);

        LegacyInferenceTask t_high;
        t_high.model_id = "high_model";
        t_high.priority = Priority::HIGH;
        t_high.payload = high_priority_task;
        t_high.timestamp_ms = 0;
        scheduler.submit_task(t_high);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    scheduler.stop();
    monitor->stop();

    std::cout << "Demo complete." << std::endl;
    return 0;
}
