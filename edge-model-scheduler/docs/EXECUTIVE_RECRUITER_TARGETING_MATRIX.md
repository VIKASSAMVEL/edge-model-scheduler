# Executive Recruiter & Engineering Leadership Targeting Matrix
### Strategic Outreach Blueprints for Staff / Principal Systems Engineering Roles (50+ LPA / Tier-1 Global)

This dossier provides targeted outreach messages, specific engineering pain points, and technical proof points mapped directly to hiring managers and technical directors at top-tier autonomous vehicle and robotics companies.

---

## 1. Tesla — Autopilot & Optimus Robotics (Inference Platform)
- **Target Roles:** Staff Software Engineer – AI Inference Platform, Senior Systems Software Engineer – Autopilot OS, Low-Level Runtime Engineer – Optimus Compute
- **Key Pain Point:** Maximizing multi-model perception throughput on proprietary FSD silicon / Jetson evaluation rigs while ensuring zero latency spikes for safety-critical collision avoidance.
- **Outreach Blueprint (Direct to Director of Autopilot Software / Optimus Compute Lead):**

> **Subject:** Eliminating UMA memory contention and FIFO queue latency spikes in edge perception (EdgeModelScheduler)
>
> Hi [Name],
>
> Noticed your team's ongoing focus on reducing inference tail latency across concurrent perception and planning networks on edge silicon.
>
> Standard datacenter runtimes like Triton introduce severe memory bloat (1.2GB idle) and process frames in FIFO order, leading to thermal throttling and wasted compute on expired frames.
>
> I built **EdgeModelScheduler**—an open-source, pure C++17 hardware arbiter that achieves:
> - **3.125 MB VmRSS memory footprint** (396x leaner than Triton).
> - **9.23 nanosecond lock-free memory allocation** with zero OS heap fragmentation.
> - **Slack-Aware Pre-Execution Shedding**: drops expired frames before hardware dispatch, saving 100% of GPU compute during thermal or sensor bursts.
> - **Sub-15ms emergency braking hazard arbitration** (measured at 1.32 microseconds).
>
> Code and reproducible benchmarks: `https://github.com/VIKASSAMVEL/edge-model-scheduler`
>
> Would love to connect and discuss how these micro-architectural scheduling primitives map to high-density autonomy and humanoid robot compute platforms.
>
> Best,  
> Vikas Samvel

---

## 2. Waymo / Zoox — Autonomous Vehicle Compute Platform
- **Target Roles:** Staff Software Engineer – Onboard Compute & Infrastructure, Senior Systems Engineer – Real-Time Perception Runtime
- **Key Pain Point:** Deterministic P99 tail latency guarantees across 10+ concurrent vision, LiDAR, and prediction networks without safety SLO violations.
- **Outreach Blueprint (Direct to Head of Onboard Platform / Perception Systems):**

> **Subject:** Zero-copy ROS 2 / Autoware perception pipeline with 84.5% P99 tail latency cut
>
> Hi [Name],
>
> In multi-camera autonomous driving pipelines, memory bus saturation and non-preemptive GPU scheduling frequently cause P99 tail latency spikes during sensor bursts.
>
> To solve this, I designed **EdgeModelScheduler**, an Autoware-compatible C++17 inference arbiter:
> - Delivers **10,061 MB/s zero-copy IPC** over cache-line aligned POSIX `/dev/shm` ring buffers.
> - Employs **Predictive EWMA Execution Profiling** coupled with dynamic slack admission to eliminate safety deadline violations (>35ms) under 85°C thermal shock.
> - Incorporates **Asymmetric DLA + GPU Co-Scheduling** to arbitrate memory controller contention across perception backbones.
>
> In our 1,000-frame surround camera stress test, the safety-critical front camera achieved a **99.8% SLO compliance rate** with sub-millisecond emergency stop arbitration.
>
> Technical whitepaper and repository: `https://github.com/VIKASSAMVEL/edge-model-scheduler`
>
> Open to a brief conversation on onboard perception scheduling and deterministic runtime architecture.
>
> Best,  
> Vikas Samvel

---

## 3. Skydio — Autonomous Flight & Edge AI
- **Target Roles:** Staff Embedded AI Engineer – Jetson Platform, Senior Robotics Systems Engineer – Autonomy Firmware
- **Key Pain Point:** Operating under tight 15W–30W battery budgets on Jetson Orin Nano/NX where thermal throttling causes drone flight instability.
- **Outreach Blueprint (Direct to VP of Engineering / Autonomy Lead):**

> **Subject:** Solving Jetson Orin DVFS thermal collapse and battery-aware QoS (EdgeModelScheduler)
>
> Hi [Name],
>
> When autonomous drones operate in direct sunlight, Jetson die temperatures reach 80°C–85°C, triggering hardware DVFS throttling that cuts GPU clocks by 45% and causes flight perception frame drops.
>
> I developed an open-source hardware arbiter, **EdgeModelScheduler**, tailored specifically for Jetson Orin:
> - **Thermal Hysteresis Quantization Governor**: Dynamically hot-swaps active execution to INT8 at 80°C, cutting memory bandwidth by 50% and cooling the die without clock throttling.
> - **4-Tier Battery QoS Governor**: Seamlessly adapts compute duty cycling as battery state-of-charge transitions from Normal (100%) to Critical Return-To-Home (<20%).
> - **3.1 MB physical RAM usage**, preserving drone memory for local obstacle voxel maps.
>
> Live physical benchmark numbers and code: `https://github.com/VIKASSAMVEL/edge-model-scheduler`
>
> Would enjoy exchanging insights on edge robotics power envelopes and low-level Jetson optimization.
>
> Best,  
> Vikas Samvel

---

## 4. Figure AI / Physical Intelligence — Embodied AI & Humanoid Robotics
- **Target Roles:** Staff Systems Software Engineer – Robot Platform, Senior AI Runtime Engineer – Embodied Compute
- **Key Pain Point:** Running real-time vision-language models (VLMs) and policy networks concurrently with high-frequency motor control and balance loops.
- **Outreach Blueprint (Direct to Head of Software / Platform Lead):**

> **Subject:** Multimodal VLM cooperative token preemption on unified memory architectures
>
> Hi [Name],
>
> As embodied humanoid robotics stacks integrate Vision-Language Models (VLMs) alongside real-time 60 FPS safety cameras, VLM KV-cache memory traffic saturates the unified memory bus, threatening real-time motor control deadlines.
>
> I recently implemented **VlmCoScheduler** in **EdgeModelScheduler**:
> - **Cooperative Inter-Token Preemption**: Instruments the autoregressive VLM generation loop to yield within <1.5ms when safety perception frames arrive.
> - **EMC Memory Controller Pacing**: Caps VLM memory bus draw at 65% of peak bandwidth, guaranteeing zero starvation for real-time safety models.
> - Pure C++17 runtime with an initial **3.1 MB RAM footprint** and **9.2 ns lock-free allocation**.
>
> Repository & C4 Architecture: `https://github.com/VIKASSAMVEL/edge-model-scheduler`
>
> Would love to discuss how cooperative multimodal arbitration can support next-generation embodied robot compute platforms.
>
> Best,  
> Vikas Samvel

---

## 5. NVIDIA — Isaac Robotics & Tegra System Software
- **Target Roles:** Principal System Software Engineer – Jetson / Tegra, Senior Solutions Architect – Isaac Autonomous Machines
- **Key Pain Point:** Customer complaints regarding Triton's memory bloat and missing MPS support on Jetson Orin.
- **Outreach Blueprint (Direct to Engineering Director, Autonomous Machines):**

> **Subject:** Empirical benchmark: Beating Triton on Jetson Orin with 3MB modern C++17 runtime
>
> Hi [Name],
>
> While Triton Inference Server excels in datacenter clusters, edge developers on Jetson Orin frequently struggle with its 1.2GB idle memory footprint and lack of hardware MPS support.
>
> To demonstrate what is possible on Tegra silicon, I built **EdgeModelScheduler**:
> - **3.125 MB VmRSS** (396x leaner than Triton).
> - **9.23 ns allocation latency** using lock-free CAS on unified pinned memory.
> - **10,061 MB/s zero-copy IPC** via POSIX `/dev/shm`.
> - **DLA + GPU Co-Scheduling** with dynamic memory controller pacing.
>
> We published the complete whitepaper, academic preprint, and 100% reproducible benchmark suite here: `https://github.com/VIKASSAMVEL/edge-model-scheduler`
>
> I'd welcome the opportunity to discuss my findings with your Jetson system software and Isaac robotics runtime teams.
>
> Best,  
> Vikas Samvel
