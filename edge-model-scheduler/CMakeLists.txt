cmake_minimum_required(VERSION 3.10)
project(EdgeModelScheduler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find TensorRT
find_path(TENSORRT_INCLUDE_DIR NvInfer.h)
find_library(TENSORRT_LIBRARY nvinfer)

# Find pybind11
find_package(pybind11 REQUIRED)

include_directories(include)
include_directories(${TENSORRT_INCLUDE_DIR})

# Add source files
set(SOURCES
    src/ResourceMonitor.cpp
    src/Scheduler.cpp
    src/TensorRTRunner.cpp
)

# Core library
add_library(scheduler_core SHARED ${SOURCES})
target_link_libraries(scheduler_core ${TENSORRT_LIBRARY} pthread)

# Example executable
add_executable(scheduler_demo examples/demo.cpp)
target_link_libraries(scheduler_demo scheduler_core)

# Python Bindings
pybind11_add_module(edge_scheduler_py python/bindings.cpp)
target_link_libraries(edge_scheduler_py PRIVATE scheduler_core)
