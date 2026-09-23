# Research Gap & Theoretical Formulation
**EdgeModelScheduler: Deterministic Mission-Utility Arbitration for Contended Edge AI**  
*Systems Research Whitepaper & Mathematical Problem Formulation*

---

## 1. Existing Runtime Systems & Their Optimization Targets

Autonomous mobile robotics and edge AI systems rely on a layered software stack. Each layer is engineered around distinct optimization objectives:

```
┌──────────────────────────────────────────────────────────────┐
│  Robotics Application Layer (Autoware / Navigation2 / ROS 2)  │
├──────────────────────────────────────────────────────────────┤
│  Middleware Transport Layer (ROS 2 DDS / Intra-Process Comms) │  <-- Optimizes: Message Delivery & Topic QoS
├──────────────────────────────────────────────────────────────┤
│  Serving & Batching Layer (Triton / Model Server)             │  <-- Optimizes: Server Throughput & GPU Utilization
├──────────────────────────────────────────────────────────────┤
│  Inference Engine Layer (TensorRT / ONNX Runtime)            │  <-- Optimizes: Isolated Kernel Latency & Graph Ops
├──────────────────────────────────────────────────────────────┤
│  Hardware Driver Layer (CUDA Driver / Streams / MPS)         │  <-- Optimizes: Asynchronous Hardware Execution
├──────────────────────────────────────────────────────────────┤
│  Edge System-on-Chip (Jetson Orin: CPU + GPU + Dual DLA)     │  <-- Constrained: Shared Unified Memory & Thermal TDP
└──────────────────────────────────────────────────────────────┘
```

### 1.1 NVIDIA Triton Inference Server
* **Primary Optimization Target**: Server-class throughput, multi-model concurrency, and high GPU utilization.
* **Mechanism**: Dynamic batching queues requests across a batch window (e.g., $10\text{--}50\text{ ms}$) to maximize matrix-multiplication saturation.
* **Failure Mode on Edge**: Dynamic batching introduces artificial queuing latency. Under compute saturation, request queues grow unbounded, resulting in high latency tails and bufferbloat. Triton lacks semantic awareness of autonomous vehicle trajectory deadlines, camera frame staleness, and thermal throttling states.

### 1.2 NVIDIA TensorRT
* **Primary Optimization Target**: Execution latency of an *individual* model graph.
* **Mechanism**: Layer fusion, kernel auto-tuning, precision quantization (FP16/INT8), and memory footprint minimization.
* **Failure Mode on Edge**: TensorRT is an inference engine, *not an arbiter*. It executes whatever work is dispatched to its `IExecutionContext`. When three TensorRT engines (e.g. LiDAR segmentation, camera perception, and trajectory prediction) attempt to execute concurrently on an edge GPU, they collide on CUDA stream resources and shared memory bandwidth with no holistic cross-model governor.

### 1.3 CUDA Streams, Events, and MPS (Multi-Process Service)
* **Primary Optimization Target**: Asynchronous concurrent kernel execution on hardware streaming multiprocessors (SMs).
* **Mechanism**: Work items submitted to separate CUDA streams can overlap if hardware resources permit.
* **Failure Mode on Edge**: Hardware priority streams offer coarse static tiers (e.g., high vs low), but cannot dynamically inspect deadlines, shed stale frames before kernel launch, or re-route workloads to heterogeneous accelerators (e.g. NVDLA) based on thermal telemetry. MPS is unsupported or restricted on low-power Tegra architectures.

### 1.4 ROS 2 (Robot Operating System) Executors & QoS
* **Primary Optimization Target**: Distributed publish/subscribe communication and callback dispatching.
* **Mechanism**: ROS 2 executors (Single-Threaded, Multi-Threaded, Events-Executor) schedule callbacks based on message arrival or timer triggers. QoS profiles provide sensor history (`KEEP_LAST`, `RELIABLE` vs `BEST_EFFORT`).
* **Failure Mode on Edge**: ROS 2 executors have no visibility into GPU/DLA accelerator pipeline state, memory bus contention, or Tensor Core utilization. Once a callback begins, it dispatches inference to the GPU blindly, irrespective of whether the GPU is thermally throttled or running behind schedule.

---

## 2. The Unmet Edge Robotics Scheduling Problem

In autonomous edge systems (autonomous vehicles, drones, surgical robots), the operating environment is characterized by:
1. **Heterogeneous Workloads**: Critical real-time safety tasks (collision detection, emergency braking) co-exist with latency-tolerant background tasks (semantic segmentation, 3D mapping, driver monitoring).
2. **Strict Deadlines**: Missing a collision detection deadline by $20 \text{ ms}$ can cause fatal physical collisions; missing a semantic segmentation deadline merely results in a temporary visualization lag.
3. **Temporal Sensor Staleness**: A camera frame is only valid for its capture epoch. Processing an old camera frame when a newer frame has already arrived in the buffer wastes compute on useless historical data.
4. **Physical Constraints**: Edge SoCs are thermally and power bounded. Ambient heat or sustained inference causes dynamic frequency scaling (DVFS), dropping clock frequencies by up to $40\%$ and causing sudden runtime execution spikes.

When simultaneous sensor streams peak:

$$\sum_{i=1}^n \text{Resource Demand}(w_i) > \text{Available Hardware Capacity}(R)$$

Under this **impossible condition**, conventional FIFO, round-robin, and server-class dynamic batching architectures experience catastrophic deadline collapse. Every model degrades simultaneously.

---

## 3. Mathematical Formulation of Mission-Utility Arbitration

### 3.1 Workload Model
Let the system execute a set of $n$ heterogeneous neural network workloads:

$$\mathcal{W} = \{w_1, w_2, \dots, w_n\}$$

For each workload $w_i \in \mathcal{W}$, we define the parameter tuple:

$$w_i = \langle d_i, e_i, c_i, \tau_i, r_i, \mathcal{Q}_i \rangle$$

Where:
* $d_i \in \mathbb{R}^+$: **Relative Deadline** ($ms$). The maximum permissible time from sensor exposure to actuation output.
* $e_i \in \mathbb{R}^+$: **Expected Execution Time** ($ms$). Profiled runtime on the target accelerator under nominal conditions.
* $c_i \in [0, 1]$: **Criticality / Mission Importance Weight**. A normalized factor where $c_i = 1.0$ for emergency safety systems and $c_i = 0.1$ for low-priority comfort/mapping tasks.
* $\tau_i \in \mathbb{R}^+$: **Temporal Freshness Window** ($ms$). Maximum age beyond which a frame is stale and useless:
  $$\text{Freshness}(t) = \max\left(0, 1 - \frac{t - t_{\text{capture}}}{\tau_i}\right)$$
* $r_i = \langle \text{GPU}_{\%}, \text{DLA}_{\%}, \text{BW}_{\text{MB/s}}, \text{Power}_{\text{W}} \rangle$: **Vector of Resource Demands**.
* $\mathcal{Q}_i = \{q_i^{\text{FP32}}, q_i^{\text{FP16}}, q_i^{\text{INT8}}, q_i^{\text{NVFP4}}\}$: **Set of Operating Precision Modes**, where lower precision trades inference fidelity for lower execution time and power.

### 3.2 Resource Model
The edge System-on-Chip provides a bounded set of physical execution resources:

$$\mathcal{R}(t) = \langle \text{Cap}_{\text{GPU}}(t), \text{Cap}_{\text{DLA0}}(t), \text{Cap}_{\text{DLA1}}(t), \text{BW}_{\text{max}}(t), T_{\text{thermal}}(t), P_{\text{budget}}(t) \rangle$$

Under thermal rise ($T_{\text{thermal}} > T_{\text{threshold}}$) or battery depletion ($P_{\text{battery}} < P_{\text{critical}}$), the available computational capacity degrades dynamically:

$$\text{Cap}_{\text{GPU}}(t) = f(\text{Clock}_{\text{DVFS}}(t)) \cdot \text{Cap}_{\text{nominal}}$$

### 3.3 The Arbitration Decision Policy
At each scheduling epoch $t$, the arbiter must compute an assignment policy:

$$\mathcal{A}(t) : \mathcal{W} \longrightarrow \{\text{Execute}(\text{Device}, \text{Precision}), \text{Defer}, \text{Shed}\}$$

### 3.4 Optimization Objective: Mission Utility Maximization

The goal of EdgeModelScheduler is **NOT** to maximize total raw throughput (inferences/sec). Its objective is to **maximize Mission Utility** $\mathcal{U}(\mathcal{A})$:

$$\max_{\mathcal{A}(t)} \mathcal{U}(\mathcal{A}) = \sum_{w_i \in \mathcal{W}_{\text{completed}}} c_i \cdot \text{Quality}(q_i) \cdot \text{Freshness}(t_i) - \sum_{w_i \in \mathcal{W}_{\text{missed}}} \lambda \cdot c_i$$

Subject to:
1. **Hard Safety Constraints**:
   $$\forall w_i \text{ with } c_i \ge c_{\text{critical}} \implies t_{\text{completion}}(w_i) \le d_i \quad (\text{Zero Critical Deadline Misses})$$
2. **Thermal & Power Budgets**:
   $$\sum_{w_i \in \mathcal{A}(t)} \text{Power}(w_i, q_i) \le P_{\text{budget}}(t)$$
3. **Accelerator Non-Interference**:
   $$\sum \text{GPU Demand} \le \text{Cap}_{\text{GPU}}(t), \quad \sum \text{DLA Demand} \le \text{Cap}_{\text{DLA}}(t)$$
4. **Pre-Execution Stale Shedding**:
   $$\text{If } t_{\text{now}} + e_i > d_i \lor t_{\text{now}} - t_{\text{capture}} > \tau_i \implies \mathcal{A}(w_i) = \text{Shed}$$

---

## 4. Why This Problem Cannot Be Solved at Other Layers

| Layer | Why It Fails to Solve the Problem |
| :--- | :--- |
| **Model Compiler (TensorRT)** | Compiles static graphs offline; cannot observe multi-model contention or dynamic sensor jitter. |
| **Hardware Driver (CUDA)** | Manages stream execution queues; cannot inspect task semantics or drop already-stale frames. |
| **Serving Framework (Triton)** | Batches for throughput; exacerbates latency tails under overload; lacks automotive criticality decomposition. |
| **Robotics Middleware (ROS 2)** | Publishes and routes messages; blind to accelerator micro-architecture, thermal throttling, and memory bus pressure. |

**EdgeModelScheduler occupies the exact architectural missing link**: A real-time, deterministic resource arbiter positioned between robotics middleware and hardware accelerator drivers that enforces mission utility and prevents deadline collapse under physical resource exhaustion.
