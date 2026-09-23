cmake_minimum_required(VERSION 3.14)
project(EdgeModelScheduler VERSION 3.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(
    -Wall -Wextra
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-result
    -O3
    -funroll-loops
    -fomit-frame-pointer
    -fno-plt
    -ffast-math
  )
endif()

# ── Optional: CUDA ──────────────────────────────────────────
find_package(CUDA QUIET)
if(CUDA_FOUND)
  add_definitions(-DHAS_CUDA)
  include_directories(${CUDA_INCLUDE_DIRS})
  message(STATUS "CUDA found: ${CUDA_VERSION}")
endif()

# ── Optional: TensorRT ──────────────────────────────────────
find_path(TENSORRT_INCLUDE_DIR NvInfer.h)
find_library(TENSORRT_LIBRARY nvinfer)

if(TENSORRT_INCLUDE_DIR AND TENSORRT_LIBRARY)
  add_definitions(-DHAS_TENSORRT)
  include_directories(${TENSORRT_INCLUDE_DIR})
  message(STATUS "Found TensorRT: ${TENSORRT_LIBRARY}")
else()
  message(STATUS "TensorRT not found. Using mock engine execution.")
endif()

# ── Optional: systemd (for sd_notify watchdog) ──────────────
find_library(SYSTEMD_LIBRARY systemd)
if(SYSTEMD_LIBRARY)
  add_definitions(-DHAS_SYSTEMD)
  message(STATUS "Found systemd: ${SYSTEMD_LIBRARY}")
endif()

include_directories(include)

# ── Core Library Sources ────────────────────────────────────
set(CORE_SOURCES
    src/ResourceMonitor.cpp
    src/Scheduler.cpp
    src/AdvancedScheduler.cpp
    src/TensorRTRunner.cpp
)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
if(WIN32)
  add_library(scheduler_core STATIC ${CORE_SOURCES})
else()
  add_library(scheduler_core SHARED ${CORE_SOURCES})
endif()

set(CORE_LINK_LIBS "")
if(UNIX OR MINGW)
  list(APPEND CORE_LINK_LIBS pthread)
endif()
if(TENSORRT_LIBRARY)
  list(APPEND CORE_LINK_LIBS ${TENSORRT_LIBRARY})
endif()
if(CUDA_FOUND)
  list(APPEND CORE_LINK_LIBS ${CUDA_LIBRARIES})
endif()
if(SYSTEMD_LIBRARY)
  list(APPEND CORE_LINK_LIBS ${SYSTEMD_LIBRARY})
endif()

# POSIX shared memory & real-time
if(UNIX)
  list(APPEND CORE_LINK_LIBS rt)
endif()

target_link_libraries(scheduler_core PUBLIC ${CORE_LINK_LIBS})

# ── Production Daemon Executable ────────────────────────────
add_executable(edge-scheduler-daemon src/daemon_main.cpp)
target_link_libraries(edge-scheduler-daemon PRIVATE scheduler_core)

# ── Standalone Demo ─────────────────────────────────────────
add_executable(scheduler_demo examples/demo.cpp)
target_link_libraries(scheduler_demo PRIVATE scheduler_core)

# ── Unit Tests ──────────────────────────────────────────────
enable_testing()

add_executable(test_scheduler_core tests/test_scheduler_core.cpp)
target_link_libraries(test_scheduler_core PRIVATE scheduler_core)
add_test(NAME CoreSchedulerUnitTests COMMAND test_scheduler_core)

add_executable(test_v3_subsystems tests/test_v3_subsystems.cpp)
target_link_libraries(test_v3_subsystems PRIVATE scheduler_core)
add_test(NAME V3SubsystemTests COMMAND test_v3_subsystems)

add_executable(test_god_level tests/test_god_level.cpp)
target_link_libraries(test_god_level PRIVATE scheduler_core)
add_test(NAME GodLevelSubsystemTests COMMAND test_god_level)

add_executable(test_model_compiler tests/test_model_compiler.cpp)
target_link_libraries(test_model_compiler PRIVATE scheduler_core)
add_test(NAME ModelCompilerUnitTests COMMAND test_model_compiler)

# ── Chaos Engine Test Runner ────────────────────────────────
add_executable(chaos_runner tests/chaos_runner.cpp)
target_link_libraries(chaos_runner PRIVATE scheduler_core)

# ── Live Hardware Measurements Benchmark ───────────────────
add_executable(benchmark_live_measurements benchmarks/benchmark_live_measurements.cpp)
target_link_libraries(benchmark_live_measurements PRIVATE scheduler_core)

# ── Scientific Impossible Workload Benchmark (7 Baselines) ──
add_executable(impossible_workload_benchmark benchmarks/impossible_workload_benchmark.cpp)
target_link_libraries(impossible_workload_benchmark PRIVATE scheduler_core)
add_test(NAME ImpossibleWorkloadBenchmark COMMAND impossible_workload_benchmark)

# ── Autoware / ROS 2 Perception Stack Test ──────────────────
add_executable(test_ros2_autoware_pipeline tests/test_ros2_autoware_pipeline.cpp)
target_link_libraries(test_ros2_autoware_pipeline PRIVATE scheduler_core)
add_test(NAME AutowarePerceptionPipelineTests COMMAND test_ros2_autoware_pipeline)

# ── Autoware Universe Zero-Copy PointCloud Pipeline Test ────
add_executable(test_autoware_zero_copy_pipeline tests/test_autoware_zero_copy_pipeline.cpp)
target_link_libraries(test_autoware_zero_copy_pipeline PRIVATE scheduler_core)
add_test(NAME AutowareZeroCopyPipelineTests COMMAND test_autoware_zero_copy_pipeline)

# ── Multimodal VLM Co-Scheduler Test ────────────────────────
add_executable(test_vlm_coscheduler tests/test_vlm_coscheduler.cpp)
target_link_libraries(test_vlm_coscheduler PRIVATE scheduler_core)
add_test(NAME VlmCoSchedulerTests COMMAND test_vlm_coscheduler)

# ── NVIDIA Thor 4-Tier Quantization Test ────────────────────
add_executable(test_thor_quantization tests/test_thor_quantization.cpp)
target_link_libraries(test_thor_quantization PRIVATE scheduler_core)
add_test(NAME ThorQuantizationTests COMMAND test_thor_quantization)

# ── ISO 26262 ASIL-D Functional Safety Test ─────────────────
add_executable(test_safety_island tests/test_safety_island.cpp)
target_link_libraries(test_safety_island PRIVATE scheduler_core)
add_test(NAME SafetyIslandTests COMMAND test_safety_island)

# ── Device-Side Conditional CUDA Graph Test ─────────────────
add_executable(test_device_graph_arbiter tests/test_device_graph_arbiter.cpp)
target_link_libraries(test_device_graph_arbiter PRIVATE scheduler_core)
add_test(NAME DeviceGraphArbiterTests COMMAND test_device_graph_arbiter)

# ── Python Bindings (pybind11) ──────────────────────────────
find_package(pybind11 QUIET)
if(pybind11_FOUND)
  pybind11_add_module(edge_scheduler_py python/bindings.cpp)
  target_link_libraries(edge_scheduler_py PRIVATE scheduler_core)
  message(STATUS "pybind11 found. Building Python module: edge_scheduler_py")
else()
  message(STATUS "pybind11 not found via CMake. Python module skipped.")
endif()

# ── Install targets ─────────────────────────────────────────
install(TARGETS edge-scheduler-daemon DESTINATION bin)
install(TARGETS scheduler_core DESTINATION lib)
install(FILES systemd/edge-scheduler.service
        DESTINATION lib/systemd/system)
install(DIRECTORY include/ DESTINATION include/edge-scheduler
        FILES_MATCHING PATTERN "*.hpp")

# ── CPack for .deb packaging ────────────────────────────────
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "edge-scheduler")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Vikas Samvel")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Ultra-Low-Latency Deterministic Co-Scheduler for Edge AI & Autonomous Robotics")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/VIKASSAMVEL/edge-model-scheduler")
set(CPACK_PACKAGE_CONTACT "engineering@vikassamvel.dev")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.31)")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
include(CPack)
