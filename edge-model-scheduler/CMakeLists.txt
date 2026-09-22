cmake_minimum_required(VERSION 3.14)
project(EdgeModelScheduler VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

# Optional TensorRT
find_path(TENSORRT_INCLUDE_DIR NvInfer.h)
find_library(TENSORRT_LIBRARY nvinfer)

if(TENSORRT_INCLUDE_DIR AND TENSORRT_LIBRARY)
  add_definitions(-DHAS_TENSORRT)
  include_directories(${TENSORRT_INCLUDE_DIR})
  message(STATUS "Found TensorRT: ${TENSORRT_LIBRARY}")
else()
  message(STATUS "TensorRT not found in system paths. Using native mock engine execution.")
endif()

include_directories(include)

# Core Sources
set(CORE_SOURCES
    src/ResourceMonitor.cpp
    src/Scheduler.cpp
    src/AdvancedScheduler.cpp
    src/TensorRTRunner.cpp
)

# Core Library
add_library(scheduler_core SHARED ${CORE_SOURCES})
if(TENSORRT_LIBRARY)
  target_link_libraries(scheduler_core PUBLIC ${TENSORRT_LIBRARY} pthread)
else()
  target_link_libraries(scheduler_core PUBLIC pthread)
endif()

# Standalone C++ Demo
add_executable(scheduler_demo examples/demo.cpp)
target_link_libraries(scheduler_demo PRIVATE scheduler_core)

# C++ Unit Tests (CTest)
enable_testing()
add_executable(test_scheduler_core tests/test_scheduler_core.cpp)
target_link_libraries(test_scheduler_core PRIVATE scheduler_core)
add_test(NAME CoreSchedulerUnitTests COMMAND test_scheduler_core)

# Python Bindings (pybind11)
find_package(pybind11 QUIET)
if(pybind11_FOUND)
  pybind11_add_module(edge_scheduler_py python/bindings.cpp)
  target_link_libraries(edge_scheduler_py PRIVATE scheduler_core)
  message(STATUS "pybind11 found. Building Python module: edge_scheduler_py")
else()
  message(STATUS "pybind11 not found via CMake. Python module skipped.")
endif()
