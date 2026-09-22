# Interview Defense Guide: Edge Model Scheduler

> **Purpose**: This document contains direct, technical, no-fluff answers to the exact questions senior engineers and hiring managers at **NVIDIA, Qualcomm, Amazon Robotics, and Google** will ask you about this project. Memorize and understand these concepts.

---

## 1. The Core Pitch (30-Second Elevator Pitch)
> *"Most edge AI deployments run into unpredictable latency spikes when multiple models (e.g. obstacle detection, tracking, pose estimation) contend for shared GPU compute and unified memory on embedded SoCs like the Jetson Orin. Instead of relying on manual profiling or brittle hardcoded sleeps, I designed and built an asynchronous, priority-aware C++ inference scheduler with pybind11 bindings. It monitors real-time sysfs hardware telemetry (thermal zones and GPU utilization) to dynamically arbitrate execution queues and drop non-critical frames during thermal throttling or high contention. In benchmarks, this reduced high-priority SLO violations from 77% down to 0%, bringing p99 latency down from 86ms to 36ms under heavy synthetic contention."*

---

## 2. Hard Technical Questions & How to Answer Them

### Q1: "Why not just use NVIDIA Triton Inference Server?"
* **Expected Answer**: 
  *"Triton is exceptional for datacenter and server-class deployments where compute and memory headroom are elastic. However, on constrained edge SoCs (e.g., Jetson Orin Nano/NX), Triton's dynamic batching and concurrent model execution models assume uniform resource availability. They do not dynamically throttle or shed non-critical inference requests based on live SoC thermal throttling or memory bus saturation. My scheduler is an edge-native resource arbiter specifically designed for safety-critical real-time domains (like robotics) where meeting a 45ms obstacle detection deadline is strictly prioritized over telemetry or analytics."*

### Q2: "Why write the core in C++ instead of Python?"
* **Expected Answer**:
  *"Two words: the Global Interpreter Lock (GIL) and deterministic latency. If multiple models are dispatching inference tasks simultaneously, Python's GIL causes thread contention and latency jitter in the dispatch loop—often introducing 5 to 15ms of non-deterministic delay before a CUDA stream is even launched. By building the thread pool, priority queue, and hardware polling in modern C++ (C++17) with `py::gil_scoped_release`, the scheduler operates in microsecond territory, reserving Python purely as the high-level configuration and API wrapper."*

### Q3: "How do you prevent Priority Inversion or Starvation of low-priority tasks?"
* **Expected Answer**:
  *"In a pure priority queue, low-priority tasks (like background scene analytics) could starve indefinitely if high-priority frames keep arriving. To mitigate this, the scheduler supports an aging mechanism (timestamp-aware comparator). If a low-priority task sits in the queue longer than a configured threshold and system metrics (GPU load < 85%, temperature < 75°C) indicate sufficient headroom, its effective priority is temporarily elevated to flush the buffer before reverting."*

### Q4: "What happens under Thermal Throttling on Tegra/Jetson?"
* **Expected Answer**:
  *"When a Jetson board crosses its thermal threshold (~85°C depending on the thermal zone), the Tegra hardware governor automatically scales down the GPU clock frequency (DVFS). This dramatically inflates inference execution time. The ResourceMonitor polls `/sys/devices/virtual/thermal/thermal_zone*/temp` and detects this state change immediately. The scheduler reacts by entering degraded mode: it immediately sheds LOW priority payloads before allocating CUDA memory buffers, keeping the GPU pipeline clear exclusively for safety-critical streams."*

### Q5: "How does the memory hierarchy impact multi-model inference on Jetson?"
* **Expected Answer**:
  *"Jetson uses a Unified Memory Architecture (UMA) where CPU and GPU share the same physical LPDDR5 RAM over a unified 128-bit or 256-bit memory bus. Contention on edge devices isn't just about GPU compute cores (ALUs)—it is often a memory bandwidth bottleneck caused by concurrent weights fetching and image tensor copies. Serializing high-priority inference through dedicated CUDA streams and avoiding unnecessary Host-to-Device copies via zero-copy pinned memory buffers is crucial."*

---

## 3. Resume Bullet Points (Ready to Copy)

- **Edge Systems / C++ Engineer**:
  > *"Architected a high-throughput, low-latency C++ inference scheduler for Jetson-class edge SoCs, integrating native TensorRT C++ APIs and `pybind11` for sub-millisecond dispatch overhead."*
  
- **Performance & Reliability Engineering**:
  > *"Implemented live sysfs hardware telemetry monitoring (thermal zones, GPU load) to dynamically shed non-critical inference tasks during thermal throttling, reducing p99 latency by 57% and eliminating safety-critical SLO violations under high contention."*

- **Benchmarking & Validation**:
  > *"Engineered an automated synthetic contention test suite benchmarking multi-model edge pipelines, verifying deterministic 36ms p99 latency against a 77% failure rate in naive concurrent baselines."*
