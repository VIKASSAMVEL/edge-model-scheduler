# ₹50 LPA+ Career & Project Strategy — S. Vikas

*Prepared September 2026. Compensation figures are sourced from Levels.fyi (verified offer submissions), AmbitionBox/Indeed (self-reported HR aggregators), and Blind (unverified employee posts) as noted. Where a figure comes from an unverified aggregator or forum, it is flagged. Nothing here is fabricated — where current data wasn't findable, that's stated explicitly rather than guessed.*

---

## 1. Executive Summary

₹50 LPA+ total compensation in India is a **real, achievable band**, but it is overwhelmingly an **experienced-hire band, not a fresher band** — with three narrow exceptions (elite HFT/quant firms, and rare top-of-band offers at NVIDIA/Google/high-growth startups for exceptional candidates). Verified data shows:

- **Fresher/new-grad software engineers** at product companies (Google, Amazon, Microsoft, NVIDIA) in India typically land in the ₹18–35 LPA band. ₹50 LPA+ *as a fresher* is rare and mostly reserved for people with unusually strong signal — competitive-programming pedigree, a research publication, or a genuinely deep systems project that survives interrogation.
- **₹50 LPA+ becomes the norm**, not the exception, once an engineer hits **SDE2/L5-equivalent with ~3–6 years of experience** at these companies. Amazon India SDE2 (L5) reported base+TC has ranged roughly ₹54–79L; SDE3/L6 roughly ₹1.1–1.7 Cr TC, per Blind-reported internal bands and Levels.fyi data.
- **HFT/quant prop-trading firms** (Optiver, IMC, Jane Street, Citadel Securities, DE Shaw, Graviton) are the one segment where ₹50L+ *is* realistically reachable straight out of college for exceptionally strong candidates — but the bar is almost entirely CS-fundamentals/competitive-programming depth, not systems/robotics project depth. This is a materially different preparation path than the rest of this report addresses.
- Your resume (Java/Spring backend, YOLO-based edge CV, embedded Jetson-class/Rock5/Raspberry Pi deployments, three patents, one Ashok Leyland R&D internship) is **strongest for Tracks F (Robotics/Autonomous Systems) and H (Embedded/Edge AI)**, with real crossover into Track E (Developer Infrastructure) if you build the right project. It is comparatively weak for Track L (Quant) and thin for Track E as currently written (no infra/tooling artifact yet).

**Bottom line of this report:** the single highest-leverage 6–12 month investment is **not** another robot demo. It's a systems-depth artifact — an **adaptive multi-model inference scheduler/resource arbiter for constrained edge-AI devices (Jetson-class)** — because it sits at the exact intersection of what you've already built three times (YOLO + embedded compute + real sensors) and a documented, currently-unsolved pain point that NVIDIA, robotics companies, and edge-AI teams all have. Section 9 explains why, with evidence.

---

## 2. Target Company Landscape

| Tier | Companies researched | ₹50L+ reality for you (4th-year → 0–2 YOE) |
|---|---|---|
| A — Big tech | NVIDIA, Google, Microsoft, Amazon, Apple, Meta, Qualcomm, AMD, Intel, Cisco, Oracle, Adobe, Salesforce, ServiceNow, Uber, Atlassian, Airbnb, LinkedIn | ₹50L+ is the **L5/SDE2+ band**, not the fresher band. Fresher offers cluster ₹18–35L; exceptional freshers (IIT-tier + strong project/research signal) occasionally clear ₹40–45L. NVIDIA India fresher SWE compensation clusters lower than its senior bands (IC1/IC2 medians ~₹26–33L per Levels.fyi) — the ₹50L+ NVIDIA number people quote is usually IC3+ (~₹62L median) or IC4 (~₹92L median). |
| B — AI/data/infra | OpenAI, Anthropic, Databricks, Snowflake, Scale AI, Hugging Face, Cohere, Cloudflare, MongoDB, Confluent, Elastic, Palantir, Rubrik | Thin India-specific hiring presence for most of these at entry level; several (OpenAI, Anthropic, Cohere) have little to no India engineering headcount as of 2026. Databricks, Confluent, Rubrik have Bengaluru/Hyderabad engineering hubs and pay competitively (Blind users cite Rubrik/Nutanix "upwards of ₹60L TC" for experienced hires, unverified). Treat this tier as a 2–4 year horizon, not an immediate target.
| C — HFT/quant | Jane Street, Goldman Sachs, Morgan Stanley, Optiver, Tower Research, DE Shaw, Citadel Securities, Quadeye, Graviton, Alphagrep | This is the one place ₹50L+ is realistic **immediately** for exceptional candidates — but 2026 reporting shows firms like Optiver and IMC paying **₹50–60 lakh for two-month internships alone**, reflecting how CP/quant-skewed and competitive this pipeline is (Citadel's intern acceptance rate is reported near 0.27%). This track rewards a different skill stack than your resume currently shows (see Track L in Section 4). |
| D — Indian high-growth tech | Razorpay, PhonePe, Flipkart, Meesho, Swiggy, CRED, Zepto, Dream11, Groww, Zerodha, BrowserStack, Postman, Freshworks, Zoho | ₹50L+ mostly starts at Staff/Lead level (~4–7 YOE). Razorpay Levels.fyi data: Software Engineer (entry) median ~₹28L, Senior SWE ~₹46L, Lead SWE ~₹59L, Staff SWE ~₹75L. Flipkart median SWE reported total comp ~₹30L (broad mix of levels). These companies are a *plausible mid-career waypoint*, not a fresher ₹50L+ path. |

**Read on the "₹50 LPA target" as stated:** given you're a 2027-degree 4th-year currently at TCS Prime, the realistic sequencing is: **land a strong first product-company role in the next 12–18 months (₹18–40L realistic band), then reach ₹50L+ within 3–5 years at SDE2/Senior level** — *unless* you specifically target the HFT/quant path (different prep) or land an exceptional NVIDIA/robotics-specific offer on the strength of a genuinely deep systems project (possible but not the median outcome).

---

## 3. Current Job Requirements (Recurring, Evidence-Based)

Pulled from live NVIDIA India robotics postings (July–Sept 2026) and cross-checked against Qualcomm/Amazon postings:

**NVIDIA — Senior SWE, Applied Research in Robotics (Bengaluru, posted Jul 2026):**
- 5+ years robotics/embedded/accelerated computing/CV/perception
- Strong C/C++, low-level debugging, performance profiling, production code
- Mapping algorithms to heterogeneous hardware (CPU/GPU/DSP/SIMD/VLIW/domain accelerators)
- Embedded systems depth: SoC architecture, memory hierarchy, cache, DMA, real-time scheduling
- Embedded Linux and/or QNX
- ROS 2, real robot bring-up, sensor integration, actuator/control paths
- "Stand out" list: Jetson, DRIVE, IGX, Isaac, VPI, NvMedia, PVA; CUDA for perception/planning/real-time sensor processing; ISO 26262 / IEC 61508 safety process familiarity

**NVIDIA — Senior Developer Relations Manager, Robotics (Bengaluru, current):**
- Hands-on with Isaac Sim, Isaac ROS, Isaac Lab, Omniverse Replicator, Jetson, TensorRT, **Triton Inference Server**, CUDA, Nsight Systems/Compute, NGC containers
- Profiling robotics workloads for GPU suitability: compute intensity, memory bandwidth, sensor IO, latency, throughput, batching, power/thermal

**Qualcomm (India postings, ongoing):** heavy weighting toward embedded/firmware, AI Software team roles, systems test/verification for CPU/GPU, automotive edge compute.

| Company | Role type | Location | Experience | Required skills (recurring) | Preferred/stand-out | Comp evidence |
|---|---|---|---|---|---|---|
| NVIDIA | SWE, Applied Research Robotics | Bengaluru | 5+ yrs | C/C++, embedded Linux/QNX, ROS 2, heterogeneous hardware mapping, real-time scheduling | Jetson/Isaac/TensorRT/CUDA, ISO 26262 | IC3 median ₹62.2L, IC4 median ₹91.8L (Levels.fyi) |
| NVIDIA | Senior DevRel, Robotics | Bengaluru | Senior | Isaac Sim/ROS/Lab, Jetson, TensorRT, Triton, CUDA, profiling | — | Posting lists ₹25–50L band (aijobs.net aggregator) |
| Qualcomm | AI Software / Systems Engineer | Bengaluru/Hyderabad | 0–5 yrs | Embedded software, algorithm implementation, debugging | AI/ML, systems test | AmbitionBox: SWE ₹17.1–30L (0–5 yrs); Senior SWE ₹23.9–40L |
| Amazon | SDE2/SDE3 | Bengaluru/Hyderabad | 3–8 yrs | Distributed systems, ownership, Java/Python/C++ | AWS breadth | Internal band screenshots (Blind, unverified): L5 SDE2 ₹54–79L TC; L6 SDE3 ₹1.13–1.69 Cr TC |
| Google | L4 (mid) / L5 (senior) SWE | Bengaluru/Hyderabad | 2–6+ yrs | DS&A, distributed systems, ML infra (team-dependent) | ML infra, systems | Median TC ~$259K globally (Levels.fyi); India L5 reported ~₹1.6 Cr top-end (Blind, unverified) |
| Razorpay | SWE → Staff SWE | Bengaluru | 0–7+ yrs | Backend, Java/Go, payments-scale systems | Distributed systems, reliability | SWE median ₹28L → Staff SWE ₹75L (Levels.fyi) |

---

## 4. Cross-Company Skill Intersection (Track Comparison)

| Track | Entry difficulty | Salary ceiling (India) | # of opportunities | Fit to your resume | Skill gap | Interview difficulty |
|---|---|---|---|---|---|---|
| A. Software Engineering (generalist) | Low–Medium | High (with seniority) | Very high | Strong (Java/Spring, DBMS, DSA) | DSA depth for FAANG-style interviews | Medium |
| B. Backend/Distributed Systems | Medium | High | High | Medium (you have Spring Boot/MongoDB/WebSocket, not distributed-systems-at-scale) | Consensus, sharding, queuing at scale | Medium–High |
| C. AI/ML Engineering | Medium | Medium–High | High but saturated | Medium (you have applied ML, less ML theory/training-at-scale) | Deep learning theory, large-scale training | Medium |
| D. ML Systems / AI Infrastructure | High | Very high | Growing fast | **Weak today, but your edge-deployment work is adjacent** | Serving frameworks (Triton, vLLM), scheduling, systems programming | High |
| E. GPU/CUDA/Systems | High | Very high | Narrow but elite-paying | Weak (no CUDA yet) | CUDA programming, kernel-level perf work | Very high |
| F. Robotics/Autonomous Systems | Medium | High (esp. NVIDIA) | Narrow but growing (NVIDIA actively hiring in India) | **Strongest fit — patents, ROS-adjacent, real hardware, sensor fusion** | Formal ROS 2 (not just embedded control), real-time scheduling theory | Medium–High |
| G. Computer Vision | Low–Medium | Medium–High | High but saturated (YOLO demos are the "reject it" example in Step 9's own anti-slop rule) | Strong practically, oversaturated as a *portfolio differentiator* | Depth beyond detection: tracking, 3D, calibration | Medium |
| H. Embedded/Edge AI | Medium | High | Growing (physical AI is NVIDIA's stated strategic bet) | **Very strong — RKNN quantization, Teensy, Raspberry Pi GPIO, ONNX export** | Formal real-time OS/scheduling theory, memory-constrained systems | Medium–High |
| I. Performance Engineering | High | Very high | Narrow | Weak (no profiling-focused project yet) | Nsight, perf tooling, benchmarking discipline | High |
| J. Developer Infrastructure | Medium–High | High | Growing | Weak today | CI/CD at scale, build systems, tooling design | Medium–High |
| K. Security Engineering | High | High | Medium | Weak (only auth/JWT-level security) | Systems security, threat modeling | High |
| L. Quantitative SWE | Very high | Highest (₹50L+ from campus, exceptional cases) | Narrow, hyper-competitive | Weak (no CP track record shown) | Competitive programming, probability/stats, low-latency C++ | Extremely high |

**Where you actually sit:** F and H are your genuine strengths, not aspirational ones — you've shipped three separate embedded/CV systems (construction safety on Raspberry Pi + GPIO relay + EdgeTAM tracking; agricultural robot on Rock 5 ITX + Teensy 4.1 with RKNN-quantized YOLOv8; the Ashok Leyland R&D robot lineage). D (ML Systems/AI Infra) is the track that **multiplies your existing work** rather than starting from zero, because the underlying skill — getting models to run reliably and fast on constrained hardware — is identical; the only new layer is *scheduling multiple models under contention*, which is a systems problem, not a new domain.

---

## 5. Product/Technology Gap Research

Evidence gathered directly from primary/near-primary sources:

1. **Multi-model contention on edge GPUs is an acknowledged, unsolved operational problem.** A March 2026 r/[robotics-adjacent subreddit] thread (archived via a public snapshot service) describes an engineer running 4–5 concurrent models (object detection, SLAM, path planner, gesture model) on a single Jetson Orin, hitting latency-target misses under load shifts, and reports their current mitigation is "basically manual profiling and hardcoded priorities" that breaks every time a model is added or swapped. This is a first-person practitioner account of exactly the gap.
2. **NVIDIA's own developer blog names the problem class** ("Simplifying AI Model Deployment at the Edge with NVIDIA Triton Inference Server") — listing multi-framework backends, mixed query types (real-time + streaming + multi-model pipelines), and continuously-updated models as the core edge-deployment pain points that motivated Triton in the first place.
3. **Triton solves framework heterogeneity, not adaptive priority-aware scheduling under resource contention on power/thermal-constrained edge boards.** Triton is designed primarily for server/datacenter deployment patterns; its dynamic batching and model-instance concurrency features assume comparatively elastic GPU/memory headroom. On a single Jetson-class SoC with a shared GPU, shared memory bandwidth, and a power/thermal envelope, the open problem is *dynamically re-prioritizing which model gets compute when a scene gets busy* — which is closer to an OS scheduling problem than an inference-serving problem.
4. **Current academic work confirms this is unresolved, not solved.** A 2024 IEEE JSYST paper and a 2026 IEEE INFOCOM paper both propose multi-agent reinforcement-learning-based schedulers for multi-model edge inference, explicitly framing "simultaneous scheduling and execution of multiple model inference tasks" as "an emerging scenario" facing "many challenges." These are research prototypes, not shipped, usable, open-source tools — there is no equivalent of "sklearn for edge model scheduling."
5. **The robotics-specific open-source tooling gap:** a live GitHub feature request against Hugging Face's LeRobot project (robot-learning framework) asks for a general-purpose ONNX/TensorRT export pipeline for edge deployment on Jetson Orin-class hardware, noting the only current option is a hardware-vendor-specific tool and that "PyTorch inference is too slow or memory-heavy" for resource-constrained robots. This is independent evidence that the edge-deployment tooling layer for robotics is thin and manual, one layer below the scheduling problem above.

**Underlying engineering skill this points to:** dynamic, latency-aware, resource-arbitrated model scheduling on constrained heterogeneous compute — which is directly relevant to NVIDIA (Jetson/Isaac/Triton ecosystem, PVA workload-mapping roles), robotics/autonomous-systems companies broadly, and any company doing on-device AI (Qualcomm edge AI, automotive perception stacks, warehouse robotics).

---

## 6. Existing-Solution Rejection Log

| Idea | Why rejected |
|---|---|
| "Build a YOLO-based [X] detector" | Explicitly the example of a saturated student project in your own prompt, and you already have three of these on your resume (leaf disease, PPE compliance, weed detection). A fourth adds no new signal. |
| "Build a RAG chatbot over robotics docs" | Wrapper around existing LLM APIs; no systems contribution; doesn't survive "what was the bottleneck / how does it scale" interrogation. |
| "Full NVIDIA Triton clone" | NVIDIA already ships and actively maintains Triton; re-implementing it is not a gap, it's redundant engineering. |
| "General-purpose LLM inference server (vLLM-style)" | vLLM, TensorRT-LLM, and Triton already dominate this space; a student reimplementation adds no measurable contribution and is effectively unbenchmarkable against production systems with any credibility. |
| "Dashboard for monitoring Jetson device fleets" | This is a CRUD/observability wrapper — real DevOps tools (Prometheus/Grafana stacks) already do this; no novel engineering claim. |
| "ROS 2 Nav2-based warehouse robot demo" | Nav2 already solves general-purpose navigation; a demo on top of it is an application, not a systems contribution — good for a resume line, not a differentiator project. |

---

## 7. Project Candidates (10, With Evidence)

For each: problem, why existing solutions are insufficient, target companies, buildability on your existing hardware (RTX 3050 laptop / Jetson-class board / Raspberry Pi / free-tier cloud).

**1. Adaptive Multi-Model Inference Scheduler for Constrained Edge GPUs** *(the recommended direction — see Section 9)*
Problem: no open, general-purpose tool dynamically re-prioritizes concurrent model inference on a single edge GPU under changing scene load. Evidence: Section 5, items 1–4. Target: NVIDIA (Jetson/Isaac/PVA/Triton-adjacent teams), robotics/AV perception teams, Qualcomm edge AI. Buildable on a Jetson-class board (Orin Nano/NX) or even simulated contention on your RTX 3050 with multiple concurrently-loaded ONNX/TensorRT models.

**2. Real-Time Sensor-Fusion Failure Injector & Recovery Harness for ROS 2 Robots**
Problem: ROS 2/Nav2 stacks degrade unpredictably when a sensor (LIDAR, IMU, camera) drops out or degrades mid-operation; most student and even production stacks are tested only on the happy path. Existing solutions: no standard, general-purpose fault-injection harness for ROS 2 sensor pipelines exists (ros2_control and Nav2 have limited built-in fault handling, not systematic injection/testing). Target: robotics reliability teams, autonomous-vehicle perception teams. Buildable: yes, on Raspberry Pi/your existing rover-class hardware with simulated sensor dropout.

**3. On-Device Quantization-Aware Retraining Loop for Edge CV Models**
Problem: INT8/RKNN quantization (which you've already used) causes accuracy loss that's hard to recover without full retraining infrastructure; most pipelines quantize once and accept the drop. Existing: TensorRT/RKNN quantization tools exist but a *closed feedback loop* (detect drift on-device → flag → retrain → redeploy, on constrained hardware) is not a standard shipped tool. Target: edge-AI/embedded-CV teams broadly. Buildable: yes, on your Rock5/Jetson-class boards.

**4. Deterministic Latency Profiler for Heterogeneous Robotics Pipelines (CPU+GPU+DSP)**
Problem: NVIDIA's own PVA job posting explicitly lists "mapping algorithms to heterogeneous hardware" and "real-time scheduling" as core requirements; there's no lightweight, open profiling tool that shows *why* a robotics pipeline missed its latency budget across CPU/GPU/DSP boundaries on Jetson-class hardware (Nsight is powerful but heavyweight/NVIDIA-proprietary and not built for cross-boundary root-cause). Target: NVIDIA robotics/PVA team directly, general embedded perception teams. Buildable: partially — full DSP/PVA access requires NVIDIA dev hardware you likely don't have; GPU+CPU boundary is buildable on Jetson/RTX.

**5. Priority-Aware ROS 2 Executor for Multi-Robot/Multi-Sensor Nodes**
Problem: ROS 2's default executor doesn't prioritize safety-critical callbacks (e.g., obstacle stop) over lower-priority ones (e.g., logging) under CPU contention. Existing: some academic real-time executors exist (e.g., research prototypes), but nothing production-grade and widely adopted. Target: robotics safety-critical teams, NVIDIA/Qualcomm embedded. Buildable: yes, directly on your existing ROS-adjacent stack.

**6. Zero-Trust Encrypted Telemetry Layer for Fleet Robotics** (extends your E2EE Chat System expertise)
Problem: most robot fleet telemetry (ROS 2 DDS, MQTT bridges) is not end-to-end encrypted by default; you've already built a zero-knowledge-server chat system with envelope encryption. Existing: DDS security plugins exist but are complex/underused; no simple drop-in E2EE telemetry layer for small robotics fleets. Target: robotics security, fleet-ops teams. Novelty risk: narrower target-company list; more of a security/robotics crossover than a headline systems project.

**7. Benchmark Suite Comparing ONNX Runtime vs TensorRT vs RKNN on Real Robotics Workloads**
Problem: cross-runtime comparisons exist but are usually academic/synthetic; a benchmark grounded in real sensor-fusion + detection + planning pipelines (like your rover/safety-monitoring stacks) with reproducible methodology would be genuinely citable. Risk: "just a benchmark" — flagged as a reject-by-default category unless paired with a contribution (e.g., feeding results into Candidate 1's scheduler).

**8. Formal Verification / Static Analysis Tool for ROS 2 Launch/Parameter Configs**
Problem: misconfigured ROS 2 launch files are a common source of field failures; no widely used static-analysis tool exists for this (unlike, say, linters for general code). Target: developer-tooling angle into robotics companies. Buildable: yes, purely software, no special hardware needed. Risk: narrower engineering depth than Candidate 1; more "useful tool" than "hard systems contribution."

**9. LLM-Free Voice/Gesture Command Interpreter for Field Robots Under Compute Budget** — rejected outright per your own anti-slop rule; too close to "LLM interface" wrapper category unless justified purely as an edge-inference-under-budget problem, and even then it's crowded.

**10. Distributed SLAM Map-Merging Across Low-Bandwidth Robot Swarms**
Problem: real, hard (multi-robot SLAM map fusion under bandwidth constraints), but PhD-adjacent in difficulty and requires multiple physical robots you don't currently have — flagged for elimination in Section 8.

*(Candidates 2–8 are viable fallbacks/extensions; Section 8 explains why Candidate 1 is the lead recommendation, and Candidate 2 and 5 are the strongest complements if you want a two-part portfolio rather than one deep project.)*

---

## 8. Elimination Round

- **Candidate 9 (voice/gesture LLM interpreter):** eliminated — falls into the "LLM interface" reject category from your own anti-slop rule; not clearly distinguishable from a wrapper.
- **Candidate 10 (distributed SLAM swarm):** eliminated for this cycle — requires multiple physical robot units, multi-month distributed-systems-plus-SLAM depth that risks becoming a multi-year effort, and is hard to validate experimentally without hardware you don't currently have. Revisit as a *second* project after Candidate 1, if you want to go deeper into F later.
- **Candidate 7 (pure benchmark suite):** eliminated as a standalone project — matches the "just a benchmark" rejection category explicitly named in your prompt. Retained only as a *component* feeding into Candidate 1's evaluation section, not a project on its own.
- **Candidate 6 (E2EE telemetry):** not eliminated, but deprioritized — real and buildable, but narrows your target-company relevance (security teams specifically) rather than maximizing cross-company optionality per your Step 4 requirement.
- **Candidate 4 (deterministic latency profiler for PVA/DSP boundary):** partially eliminated — the DSP/PVA-specific piece requires NVIDIA proprietary dev hardware you don't have access to; the CPU/GPU-boundary subset survives and could be folded into Candidate 1 as a diagnostic module rather than a standalone project.
- **Candidates 1, 2, 3, 5, 8 survive** to the shortlist; 1, 2, 5 are the strongest three (Section 9's comparison table covers 1, 2, and 5, since 3 and 8 are reasonable but shallower in both technical depth and evidence strength).

---

## 9. Final 3 Candidates — Neutral Comparison

| Dimension | 1. Adaptive Multi-Model Scheduler | 2. Sensor-Fusion Failure Injector/Recovery Harness | 5. Priority-Aware ROS 2 Executor |
|---|---|---|---|
| Existing solution maturity | Low — Triton solves adjacent but not this problem; academic RL schedulers are unshipped research | Low — no standard fault-injection harness for ROS 2 sensor pipelines | Medium — some academic real-time executors exist, none dominant |
| Technical depth | High — OS-scheduling-adjacent, real-time constraints, resource arbitration | Medium-High — requires deep understanding of sensor fusion failure modes + ROS 2 internals | High — requires modifying/extending ROS 2 executor internals (rclcpp/rclpy core) |
| Feasibility (6–12 months, solo) | Medium — ambitious but scopeable (start with 2–3 models, expand) | High — very buildable incrementally | Medium — touching core ROS 2 internals has a steeper on-ramp |
| Hardware requirement | Jetson Orin Nano/NX (or simulate contention on RTX 3050) | Existing rover/Pi-class hardware + sensors you already have | Any ROS 2-capable board you already have |
| Research potential | High — directly extends the 2026 INFOCOM problem framing with an open-source, hardware-validated system | Medium — publishable as a systems/reliability paper | Medium-High — publishable as a real-time systems paper |
| NVIDIA relevance | Very high — matches PVA/robotics job posting language almost line-for-line (heterogeneous workload mapping, real-time scheduling, latency/power/throughput) | Medium — relevant to robotics reliability, less to NVIDIA's stated priorities | High — ROS 2 + real-time scheduling both named in NVIDIA's posting |
| Google/Microsoft/Amazon relevance | Medium — scheduling/resource-arbitration is a generically valuable systems skill (relevant to any ML infra team) | Low-Medium — niche to robotics | Low-Medium — niche to robotics |
| AI infrastructure relevance | High — this *is* an AI infra problem, just at the edge instead of the datacenter | Low | Low |
| Interview depth | Very high — bottlenecks, baselines, failure modes, scaling, trade-offs all have real answers because the problem is genuinely hard | High — but conversation stays robotics-scoped | High — but conversation stays ROS-internals-scoped |

**Trade-offs:** Candidate 1 has the widest company relevance (NVIDIA *and* generic AI-infra teams) and the strongest evidence trail, but is the most ambitious to scope well — you'll need to deliberately bound the MVP (Section 14) rather than trying to solve general-purpose scheduling on day one. Candidate 2 is the safest, most reliably finishable option and strengthens your robotics-reliability story, but has a narrower audience. Candidate 5 sits between the two: it's the most "NVIDIA job-posting-literal" match (ROS 2 + real-time scheduling explicitly named) but requires the deepest ROS 2 internals knowledge to even start, which is a real on-ramp cost.

---

## 9b. Recommended Direction *(not "the best" — a fit given your constraints)*

**Candidate 1 (Adaptive Multi-Model Scheduler)**, scoped as an MVP that runs 2–3 concurrent models on a Jetson Orin Nano-class board with a resource monitor and a simple priority-aware scheduling policy, with Candidate 2's fault-injection concept folded in later as a v2 stress-test module (rather than a separate project). This direction:

- Reuses skills you already have (ONNX/TensorRT export, RKNN quantization, real-time embedded control) instead of starting a new domain from zero.
- Has the strongest, most specific evidence trail of any candidate (a first-person practitioner account of the exact pain point, a vendor blog naming the problem class, and two 2024–2026 academic papers confirming it's still open research).
- Produces a project whose bottlenecks, baselines, and scaling behavior are *inherently* interview-worthy, because the problem doesn't have a known-good textbook answer — which is precisely what "why did you design it this way" questions need.
- Trade-off you should go in accepting: this is harder to finish cleanly than a fault-injection harness. You will need real discipline about MVP scope (Section 14) or it can sprawl into an unfinishable general-purpose scheduler.

---

## 10. Detailed Product Concept

**Problem:** On a single shared-GPU edge SoC (Jetson Orin-class), running multiple concurrent perception/planning models (e.g., object detection + a secondary classifier + a lightweight planner), compute/memory contention causes unpredictable latency spikes when scene complexity changes — and today's mitigation, per direct practitioner evidence, is manual profiling and hardcoded priorities that breaks whenever the model set changes.

**Users:** robotics/edge-AI engineers deploying 2+ concurrent models on Jetson-class hardware — construction-safety monitoring (your own past project is a real example), agricultural robots, warehouse robots, small AV platforms.

**Workflow:** engineer registers each model with a declared latency budget and priority class → scheduler monitors real-time GPU/memory/queue-depth telemetry → on contention, scheduler dynamically adjusts batch size, execution order, or (as a fallback) drops/downsamples the lowest-priority model's input rate → logs decisions for post-hoc analysis.

**Architecture (MVP):**
- Model registry (declares latency SLO + priority per model)
- Resource monitor (GPU utilization, memory, per-model queue depth — via `tegrastats`/`jtop` on Jetson)
- Scheduler core (policy: static priority → SLO-aware priority → adaptive, in that increasing order of ambition)
- Execution layer wrapping TensorRT/ONNX Runtime engines
- Decision/telemetry logger for evaluation

**Core technology:** Python orchestration layer + TensorRT/ONNX Runtime execution backends + Jetson system telemetry APIs; C++ for any hot-path scheduling logic if profiling shows Python overhead matters.

**MVP → V2 → V3:**
- MVP: 2–3 fixed models, static + SLO-aware priority scheduling, on-device evaluation on real workload (reuse your own YOLO models from prior projects as the test models).
- V2: adaptive policy (adjust priority based on observed scene complexity, not just fixed rules), fault-injection stress testing (folding in Candidate 2), open-source release with benchmarks.
- V3 (stretch, research-paper territory): compare against a learned (lightweight RL or heuristic-learned) policy vs. your rule-based scheduler, quantify the gap.

**Benchmarks/evaluation:** p50/p95/p99 latency per model under contention; missed-SLO rate under synthetic load spikes; comparison against "no scheduler / naive round-robin" baseline; comparison against manual hardcoded priorities (replicating the Reddit practitioner's baseline).

**Failure modes to test and document:** what happens when a new model is added at runtime; what happens under thermal throttling; what happens when all models simultaneously exceed budget (no good answer exists — document your trade-off, that's a legitimate finding).

**Deployment target:** Jetson Orin Nano/NX (or documented as portable to any CUDA-capable edge SoC).

---

## 11. Technical Learning Roadmap (Week-by-Week, first 12 weeks)

- **Weeks 1–2:** Jetson platform deep-dive — `tegrastats`, `jtop`, CUDA MPS, power/thermal modes, memory architecture (unified memory on Jetson matters a lot here).
- **Weeks 3–4:** TensorRT engine building and profiling (Nsight Systems basics) using your existing YOLO models as test subjects; establish single-model latency baselines.
- **Weeks 5–6:** Build the 2-model contention scenario; measure and document what breaks under naive concurrent execution (this *is* your baseline for the whole project).
- **Weeks 7–8:** Implement static + SLO-aware priority scheduling; instrument telemetry logging.
- **Weeks 9–10:** Stress-test harness (synthetic load spikes, scene-complexity triggers); start documenting failure modes.
- **Weeks 11–12:** Write up MVP results; decide V2 scope (adaptive policy vs. broaden model count) based on what the MVP actually revealed.

## 12. 90-Day Build Plan (Milestones)

- **Day 30:** Single-model TensorRT pipelines running and benchmarked on Jetson; naive 2-model concurrent baseline measured and documented (this alone is a concrete, demonstrable artifact if nothing else ships).
- **Day 60:** Static + SLO-aware scheduler working; telemetry/logging pipeline complete; first comparison chart (naive vs. scheduled) produced.
- **Day 90:** MVP feature-complete, README + architecture doc + benchmark results written, repository public, decision made on V2 direction based on real findings.

## 13. GitHub Strategy

```text
edge-model-scheduler/
├── README.md              — problem statement, evidence links, quickstart
├── ARCHITECTURE.md         — component diagram, design rationale
├── docs/
│   ├── installation.md
│   ├── benchmarks.md       — reproducible methodology + results
│   └── failure-modes.md    — documented, not hidden
├── examples/               — your own reused YOLO models as demo workloads
├── scheduler/               — core scheduling logic
├── telemetry/                — Jetson resource monitoring wrappers
├── tests/
└── research/                — any paper draft, related-work notes
```

The **benchmarks + failure-modes docs** are the component most likely to attract external interest/contributors, because they're the part no existing open tool documents honestly for edge hardware.

## 14. Resume Positioning (No Exaggeration)

Draft line: *"Designed and built a latency-aware, priority-based scheduler for running multiple concurrent TensorRT/ONNX models on a resource-constrained Jetson Orin edge device; measured up to [X]% reduction in SLO violations versus naive concurrent execution under synthetic contention, and documented failure modes under thermal throttling and runtime model addition."*

Only fill in `[X]%` with your actual measured number — don't pre-write a claim you haven't earned yet.

## 15. Interview Strategy — Questions This Project Enables You to Answer

- Why priority-based scheduling over pure round-robin? (Trade-off discussion: fairness vs. safety-criticality)
- What was your baseline, and how did you validate the naive-execution numbers were representative?
- What broke first as you added models — compute, memory bandwidth, or thermal?
- How does your approach degrade under 2x expected load? Under thermal throttling specifically?
- What did you try that failed, and why? (Have a real answer — document it as you go, don't reconstruct it later.)
- How would this port to a different SoC (e.g., Qualcomm edge NPU) — what's Jetson-specific vs. general?
- What's the theoretical connection to OS scheduling (priority inversion, starvation) — did you hit either?

## 16. Target Job Application Strategy

- **Now → 6 months:** stay at TCS Prime as the safety net; apply selectively to product-company SDE/Robotics-adjacent roles once the MVP (Day 60–90 milestone) exists as a demonstrable artifact, not before — an unfinished project is weaker signal than a finished narrow one.
- **6–12 months:** target NVIDIA robotics/embedded postings directly (the Applied Research in Robotics req is a plausible long-term target even if not immediately, given the 5+ YOE bar — but internship/junior robotics roles and India-based perception/edge-AI startups are realistic nearer-term targets), Qualcomm edge AI, and Indian high-growth companies with real edge/IoT/robotics work (fewer of Tier D fit this — most are pure backend/consumer).
- **12+ months:** re-evaluate ₹50L+ SDE2-level roles at Tier A/D once you have 1–2 years of real product-company experience plus this project as differentiation.

## 17. Risk Analysis

- **Scope creep risk (highest):** "adaptive scheduling" can balloon into an unbounded research problem. Mitigation: the MVP is explicitly static/SLO-aware first — resist building the RL-based adaptive version until the simple version is solid and measured.
- **Hardware access risk:** if you don't already own a Jetson Orin Nano/NX, this is a real cost (~₹15–25K) — factor it in before committing; simulating contention on your RTX 3050 is a fallback but weakens the "edge device" framing.
- **Evidence-of-impact risk:** without real comparative numbers (naive vs. scheduled), this is just another cool demo. The benchmark discipline in Section 10 is not optional.
- **Market risk:** ₹50 LPA+ is not guaranteed by any project — Anthropic's own product-gap framing in your prompt is right to warn against promising an outcome. This project realistically improves resume signal, interview depth, and recruiter differentiation; it does not by itself guarantee an offer.

## 18. Evidence & Sources

- NVIDIA India robotics job postings (Applied Research in Robotics; Senior DevRel Robotics): builtinbengaluru.in, inferencejobs.com, foundit.in, jobs.anitab.org (all pulling the same live NVIDIA req, Jul–Sep 2026)
- NVIDIA Triton edge-deployment challenges: developer.nvidia.com/blog "Simplifying AI Model Deployment at the Edge with NVIDIA Triton Inference Server"
- Practitioner evidence of multi-model Jetson contention: Reddit thread (archived snapshot, March 2026) — "Those of you running multiple AI models on a single edge GPU (Jetson, etc.) - how do you handle resource allocation?"
- Open research confirming the problem is unsolved: IEEE JSYST 2024 (multi-agent RL scheduling for edge inference), IEEE INFOCOM 2026 "Online Scheduling of Edge Multiple-Model Inference with DAG Structure and Retraining" (NSF-funded, par.nsf.gov)
- Robotics edge-export tooling gap: GitHub huggingface/lerobot issue #3146
- Compensation data: Levels.fyi (NVIDIA, Amazon, Razorpay pages, accessed Sep 2026), AmbitionBox/Indeed (Qualcomm), Blind (Amazon/Google internal band discussions — flagged unverified throughout), Business Today (Aug 26, 2026, HFT intern pay reporting)

---

## Most Important Question — Answered

> *"If I have 6–12 months, a TCS Prime ₹9 LPA safety net, my existing robotics/AI/software background, and a goal of eventually reaching ₹50 LPA+, what single technical direction should I invest my portfolio effort into?"*

**Direction: edge-AI systems engineering — specifically, resource-constrained multi-model scheduling — built on top of the robotics/embedded work you already have.**

The evidence supports this over the alternatives for three concrete reasons, not general career advice:

1. It's the only candidate direction with a **documented, currently-unsolved gap** backed by a practitioner account, a vendor's own problem-framing, and unshipped academic research — not an inferred gap.
2. It **reuses rather than replaces** your existing skill investment (ONNX/TensorRT export, RKNN quantization, embedded real-time control) — you're going deeper on a stack you already know, which is buildable in 6–12 months solo, unlike CUDA-kernel-level work (Track E) or quant-CP depth (Track L), both of which would require starting closer to zero.
3. It has the **widest genuine cross-company relevance** of any candidate examined — it maps almost line-for-line onto live NVIDIA robotics job requirements, while the underlying skill (resource-aware scheduling for ML workloads) also reads as legitimate AI-infrastructure experience to Google, Amazon, Databricks-tier companies, which none of your current robotics projects individually claim.

This does not guarantee ₹50 LPA+. What it realistically buys you, per your own Step 12 framing, is resume signal, interview depth that survives real interrogation, and differentiation from the sea of YOLO-demo portfolios — which is the actual, achievable target of a 6–12 month solo project.
