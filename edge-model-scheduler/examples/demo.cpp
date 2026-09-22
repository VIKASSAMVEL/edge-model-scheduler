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
        InferenceTask t_low{"low_model", Priority::LOW, low_priority_task, 0};
        scheduler.submit_task(t_low);

        InferenceTask t_high{"high_model", Priority::HIGH, high_priority_task, 0};
        scheduler.submit_task(t_high);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    scheduler.stop();
    monitor->stop();

    std::cout << "Demo complete." << std::endl;
    return 0;
}
