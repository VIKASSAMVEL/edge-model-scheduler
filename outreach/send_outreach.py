#!/usr/bin/env python3
"""
EdgeModelScheduler Executive Outreach Dispatcher
Sends individualized, deeply technical, beautifully formatted cold emails to leadership at NVIDIA and Palo Alto Networks.
Uses MIMEMultipart('alternative') with both clean plain-text and mobile-responsive HTML.
"""

import sys
import os
import json
import time
import smtplib
import ssl
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart

# Ensure utf-8 encoding on Windows console
if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding='utf-8')
        sys.stderr.reconfigure(encoding='utf-8')
    except Exception:
        pass

# Define target recipients with company vision categories
RECIPIENTS = [
    {
        "id": "nvidia_deepu",
        "name": "Deepu Talla",
        "title": "Vice President & General Manager, Autonomous Machines",
        "company": "NVIDIA",
        "email": "dtalla@nvidia.com",
        "category": "nvidia_robotics"
    },
    {
        "id": "nvidia_danny",
        "name": "Danny Shapiro",
        "title": "Vice President, Automotive",
        "company": "NVIDIA",
        "email": "dshapiro@nvidia.com",
        "category": "nvidia_automotive"
    },
    {
        "id": "nvidia_amit",
        "name": "Amit Goel",
        "title": "Director of Product Management, Autonomous Machines",
        "company": "NVIDIA",
        "email": "agoel@nvidia.com",
        "category": "nvidia_robotics"
    },
    {
        "id": "nvidia_norm",
        "name": "Norm Marks",
        "title": "Vice President, Automotive Enterprise Solutions",
        "company": "NVIDIA",
        "email": "nmarks@nvidia.com",
        "category": "nvidia_automotive"
    },
    {
        "id": "panw_nikesh",
        "name": "Nikesh Arora",
        "title": "Chairman & Chief Executive Officer",
        "company": "Palo Alto Networks",
        "email": "narora@paloaltonetworks.com",
        "category": "panw_leadership"
    },
    {
        "id": "panw_nir",
        "name": "Nir Zuk",
        "title": "Founder & Chief Technology Officer",
        "company": "Palo Alto Networks",
        "email": "nzuk@paloaltonetworks.com",
        "category": "panw_dataplane"
    },
    {
        "id": "panw_anand",
        "name": "Anand Oswal",
        "title": "SVP & General Manager, Network Security",
        "company": "Palo Alto Networks",
        "email": "aoswal@paloaltonetworks.com",
        "category": "panw_dataplane"
    },
    {
        "id": "panw_lee",
        "name": "Lee Klarich",
        "title": "Chief Product Officer",
        "company": "Palo Alto Networks",
        "email": "lklarich@paloaltonetworks.com",
        "category": "panw_leadership"
    },
    {
        "id": "panw_mukesh",
        "name": "Mukesh Gupta",
        "title": "VP, Product Management, Hardware Security Platforms",
        "company": "Palo Alto Networks",
        "email": "mgupta@paloaltonetworks.com",
        "category": "panw_dataplane"
    }
]

def load_credentials():
    cred_file = os.path.join(os.path.dirname(__file__), ".credentials.json")
    if not os.path.exists(cred_file):
        raise FileNotFoundError(f"Credentials file not found at: {cred_file}")
    with open(cred_file, "r") as f:
        return json.load(f)

def generate_email_content(target):
    name = target["name"]
    first_name = name.split()[0]
    title = target["title"]
    category = target.get("category", "nvidia_robotics")

    # Common HTML style block for pristine mobile and desktop rendering
    css_styles = """
  body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; font-size: 15px; line-height: 1.62; color: #1e293b; background-color: #ffffff; margin: 0; padding: 0; }
  .container { max-width: 620px; margin: 0 auto; padding: 20px 16px; }
  p { margin: 0 0 14px 0; }
  ol, ul { margin: 0 0 14px 0; padding-left: 20px; }
  li { margin-bottom: 8px; }
  .links-box { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 6px; padding: 14px 16px; margin: 18px 0; }
  a { color: #2563eb; text-decoration: underline; font-weight: 500; }
  .footer { border-top: 1px solid #e2e8f0; padding-top: 14px; margin-top: 20px; color: #475569; font-size: 14px; }
"""

    if category == "nvidia_robotics":
        subject = "Final-year student (Vel Tech) — built low-latency C++17 runtime vs. Triton & ROS bottlenecks on Jetson"
        
        plain_text = f"""Hi {first_name},

I'm reaching out directly because I follow your leadership in Autonomous Machines and robotics at NVIDIA. With NVIDIA driving the frontier of Physical AI and the Jetson Orin ecosystem, I wanted to share a low-latency systems runtime I've been building that tackles memory and scheduling bottlenecks on edge silicon.

I'm Vikas S, a final-year B.Tech CSE student at Vel Tech University in Chennai. While most students around me focus on standard web stacks or cloud APIs, I've dedicated my college years to low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and GPU runtimes.

Over the last several months, I built an open-source deterministic edge AI co-scheduler in C++17 (EdgeModelScheduler v4.0). I engineered it specifically to solve three critical architecture bottlenecks that occur when deploying multi-model robotics pipelines on constrained Jetson SoCs:

1. Triton Inference Server Memory Footprint: Triton is the gold standard for datacenter GPU clusters, but on unified-memory edge SoCs like Jetson Orin Nano / AGX Orin, Triton carries a ~1,240 MB idle footprint. On an 8GB Orin Nano, this consumes over 15% of total system RAM before processing a single frame, starving localized obstacle voxel grids.
2. ROS 2 & Isaac ROS Middleware Overhead: Passing 12MB LiDAR pointclouds and 4K camera frames through ROS 2 DDS middleware forces four sequential serialization and memory copies between host and device memory. This burns 20–35 ms of PCIe/memory bus headroom before inference even begins. Under sensor burst jitter, standard FIFO queues suffer severe tail latency blowout (P99 > 73 ms) with a 35% deadline failure rate.
3. Cross-Model Hardware Contention: TensorRT compiles exceptional individual execution graphs, but provides no dynamic arbiter when concurrent perception models (e.g. YOLOv8 detection, depth estimation, and BEV segmentation) compete simultaneously for GPU SMs and dual Deep Learning Accelerators (DLA) under 82°C thermal downclocking.

To solve this, I built a pure C++17 runtime with zero heap allocations on the critical execution path:
- 3.125 MB Resident Memory Footprint: Operates with an idle footprint ~396x leaner than Triton (3.1 MB vs. 1,240 MB), leaving virtually all physical LPDDR5 memory available for onboard perception and mapping.
- CUDA IPC Zero-Copy TypeAdapter: Passes raw PointCloud2 LiDAR and camera frames across process boundaries via 64-byte virtual memory handles over /dev/shm, bypassing ROS 2 DDS serialization at 12.23 GB/s throughput (<15 µs mapping latency).
- Lock-Free SPSC Arbiter: Custom Single-Producer Single-Consumer circular rings with head and tail atomics isolated on separate 64-byte cache lines (alignas(64)), eliminating lock contention and CPU cache-line bouncing.
- Heterogeneous GPU/DLA Co-Scheduler: Dynamically balances execution between Ampere/Blackwell GPU SMs and dual DLAs. Under simulated 45 ms network stalls, it predictively sheds stale intermediate frames and maintained a 0.00% deadline miss rate.
- Thermal DVFS Hysteresis Governor: Automatically down-quantizes secondary perception models from FP16 to INT8 when reaching 82°C, cutting memory bus draw by 50% and cooling the die while locking critical obstacle avoidance in full precision.

I've documented the complete architecture, cache-line profiling, and reproducible benchmarks in our technical dossier:
- Technical Report (PDF): https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
- Live Bento Dashboard: https://vikassamvel.github.io/edge-model-scheduler/
- GitHub Repository: https://github.com/VIKASSAMVEL/edge-model-scheduler

I'm writing because as I prepare to graduate, my ambition is to contribute to NVIDIA's autonomous machines and robotics platform teams. I would be deeply grateful for a brief 15-minute conversation to get your perspective, or an introduction to an engineering lead or hiring manager on the Jetson System Software, Isaac ROS, or Autonomous Machines teams where an entry-level engineer or intern with my profile could contribute.

Thank you for your time and leadership in this space.

Thanks for your time,
Vikas S
Final-Year B.Tech CSE Student, Vel Tech University, Chennai
vikassamvel123@gmail.com | +91 97908 01678
LinkedIn: https://www.linkedin.com/in/vikassamvel/
GitHub: https://github.com/VIKASSAMVEL
"""

        html_body = f"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style>{css_styles}</style>
</head>
<body>
<div class="container">
  <p>Hi {first_name},</p>

  <p>I'm reaching out directly because I follow your leadership in Autonomous Machines and robotics at NVIDIA. With NVIDIA driving the frontier of Physical AI and the Jetson Orin ecosystem, I wanted to share a low-latency systems runtime I've been building that tackles memory and scheduling bottlenecks on edge silicon.</p>

  <p>I'm <strong>Vikas S</strong>, a final-year B.Tech CSE student at Vel Tech University in Chennai. While most students around me focus on standard web stacks or cloud APIs, I've dedicated my college years to low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and GPU runtimes.</p>

  <p>Over the last several months, I built an open-source deterministic edge AI co-scheduler in C++17 (<strong>EdgeModelScheduler v4.0</strong>). I engineered it specifically to solve three critical architecture bottlenecks that occur when deploying multi-model robotics pipelines on constrained Jetson SoCs:</p>

  <ol>
    <li><strong>Triton Inference Server Memory Footprint:</strong> Triton is the gold standard for datacenter GPU clusters, but on unified-memory edge SoCs like Jetson Orin Nano / AGX Orin, Triton carries a ~1,240 MB idle footprint. On an 8GB Orin Nano, this consumes over 15% of total system RAM before processing a single frame, starving localized obstacle voxel grids.</li>
    <li><strong>ROS 2 &amp; Isaac ROS Middleware Overhead:</strong> Passing 12MB LiDAR pointclouds and 4K camera frames through ROS 2 DDS middleware forces four sequential serialization and memory copies between host and device memory. This burns 20&ndash;35 ms of PCIe/memory bus headroom before inference even begins. Under sensor burst jitter, standard FIFO queues suffer severe tail latency blowout (P99 &gt; 73 ms) with a 35% deadline failure rate.</li>
    <li><strong>Cross-Model Hardware Contention:</strong> TensorRT compiles exceptional individual execution graphs, but provides no dynamic arbiter when concurrent perception models (e.g. YOLOv8 detection, depth estimation, and BEV segmentation) compete simultaneously for GPU SMs and dual Deep Learning Accelerators (DLA) under 82&deg;C thermal downclocking.</li>
  </ol>

  <p>To solve this, I built a pure C++17 runtime with zero heap allocations on the critical execution path:</p>

  <ul>
    <li><strong>3.125 MB Resident Memory Footprint:</strong> Operates with an idle footprint ~396x leaner than Triton (3.1 MB vs. 1,240 MB), leaving virtually all physical LPDDR5 memory available for onboard perception and mapping.</li>
    <li><strong>CUDA IPC Zero-Copy TypeAdapter:</strong> Passes raw PointCloud2 LiDAR and camera frames across process boundaries via 64-byte virtual memory handles over <code>/dev/shm</code>, bypassing ROS 2 DDS serialization at <strong>12.23 GB/s throughput</strong> (&lt;15 &mu;s mapping latency).</li>
    <li><strong>Lock-Free SPSC Arbiter:</strong> Custom Single-Producer Single-Consumer circular rings with head and tail atomics isolated on separate 64-byte cache lines (<code>alignas(64)</code>), eliminating lock contention and CPU cache-line bouncing.</li>
    <li><strong>Heterogeneous GPU/DLA Co-Scheduler:</strong> Dynamically balances execution between Ampere/Blackwell GPU SMs and dual DLAs. Under simulated 45 ms network stalls, it predictively sheds stale intermediate frames and maintained a <strong>0.00% deadline miss rate</strong>.</li>
    <li><strong>Thermal DVFS Hysteresis Governor:</strong> Automatically down-quantizes secondary perception models from FP16 to INT8 when reaching 82&deg;C, cutting memory bus draw by 50% and cooling the die while locking critical obstacle avoidance in full precision.</li>
  </ul>

  <div class="links-box">
    <div style="font-weight: 600; margin-bottom: 6px;">Project Resources:</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Technical Report (PDF)</a> &mdash; 30-section architecture and benchmark documentation</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Bento Dashboard</a> &mdash; Interactive hardware topology and live simulator</li>
      <li><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">GitHub Repository</a> &mdash; Full C++17 source with 12 passing test suites</li>
    </ul>
  </div>

  <p>I'm writing because as I prepare to graduate, my ambition is to contribute to NVIDIA's autonomous machines and robotics platform teams. I would be deeply grateful for a brief 15-minute conversation to get your perspective, or an introduction to an engineering lead or hiring manager on the Jetson System Software, Isaac ROS, or Autonomous Machines teams where an entry-level engineer or intern with my profile could contribute.</p>

  <p>Thank you for your time and leadership in this space.</p>

  <div class="footer">
    <p style="margin: 0 0 2px 0; font-weight: 600; color: #0f172a;">Vikas S</p>
    <p style="margin: 0 0 4px 0; color: #475569;">Final-Year B.Tech CSE Student, Vel Tech University, Chennai</p>
    <p style="margin: 0; font-size: 13px;">
      Email: <a href="mailto:vikassamvel123@gmail.com">vikassamvel123@gmail.com</a> | Phone: +91 97908 01678<br>
      LinkedIn: <a href="https://www.linkedin.com/in/vikassamvel/">linkedin.com/in/vikassamvel</a> | GitHub: <a href="https://github.com/VIKASSAMVEL">github.com/VIKASSAMVEL</a>
    </p>
  </div>
</div>
</body>
</html>
"""

    elif category == "nvidia_automotive":
        subject = "Final-year student (Vel Tech) — built deterministic C++17 runtime for DRIVE Thor / Orin ADAS deadlines"
        
        plain_text = f"""Hi {first_name},

I'm reaching out directly because I follow NVIDIA's automotive vision and your leadership in driving DRIVE Thor and centralized vehicle computing. As autonomous vehicles move toward multi-camera perception and cabin intelligence on unified automotive SoCs, I wanted to share a low-latency C++17 runtime I've been building that enforces hard real-time deadlines under sensor jitter and thermal stress.

I'm Vikas S, a final-year B.Tech CSE student at Vel Tech University in Chennai. Most of my peers focus on web applications, but I've dedicated my college years to low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and GPU runtimes.

Over the last several months, I built an open-source deterministic edge AI co-scheduler in C++17 (EdgeModelScheduler v4.0). I engineered it specifically to address automotive runtime challenges on platforms like DRIVE Orin and DRIVE Thor:

1. Hard Real-Time Deadlines vs. FIFO Bufferbloat: Autonomous vehicle perception requires hard physical deadlines (sub-35ms frame-to-braking budget). Under bursty camera or LiDAR jitter, standard FIFO queues in generic runtimes queue frames indefinitely, resulting in severe tail latency spikes (P99 > 73 ms) and a 35% critical deadline failure rate.
2. Lack of ASIL-D Black Box Audit Trails: Generic inference servers like Triton do not provide on-chip deterministic flight recorders. When an emergency stop or safety watchdog trips, there is no zero-overhead mechanism to reconstruct what the neural perception stack saw in the exact preceding milliseconds.
3. Thermal DVFS Shock in In-Cabin/Under-Hood Silicon: When automotive SoCs experience thermal stress (reaching 82°C), hardware throttling drops clock frequencies, causing sudden multi-frame dropouts in safety-critical object detection.

To solve this, I built a pure C++17 runtime with zero heap allocations on the critical path:
- Slack-Aware Pre-Execution Shedding: Continuously calculates remaining deadline slack before submitting tasks to CUDA streams. If a frame has already expired due to sensor bus stalls, it sheds it immediately, saving 100% of GPU compute for current frames and maintaining a 0.00% deadline miss rate under 45ms network stalls.
- Sub-Microsecond Emergency Braking Hazard Arbitration: Prioritizes critical collision avoidance paths with 1.32 microsecond arbitration latency, ensuring that emergency braking paths are never starved by background trajectory or cabin AI tasks.
- ISO 26262 ASIL-D Telemetry Flight Recorder: Pre-allocated circular telemetry buffer (60,000 frames @ 1,000 Hz) that auto-dumps rolling SHA-256 cryptographically verified .emsbag binaries upon any safety island trip for complete forensic traceability.
- Closed-Loop Thermal Precision Governor: Seamlessly degrades secondary perception models from FP16 to INT8 at 82°C to reduce memory bus bandwidth and cool the die, while locking collision avoidance in full precision.
- Ultra-Lean 3.125 MB Resident RAM Footprint: Leaves maximum compute and memory headroom for surround-view perception models.

I've made the implementation, architecture dossier, and benchmarks public:
- Technical Report (PDF): https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
- Live Bento Dashboard: https://vikassamvel.github.io/edge-model-scheduler/
- GitHub Repository: https://github.com/VIKASSAMVEL/edge-model-scheduler

I'm writing because as I prepare to graduate, my goal is to work on low-level systems software and runtime engineering within NVIDIA Automotive. I would love 15 minutes of your time to introduce myself, or an introduction to an engineering lead on the DRIVE Thor runtime, Tegra System Software, or Autonomous Vehicle platforms team where an entry-level engineer with my systems foundation could contribute.

Thank you for your time and for driving the future of autonomous vehicles.

Thanks for your time,
Vikas S
Final-Year B.Tech CSE Student, Vel Tech University, Chennai
vikassamvel123@gmail.com | +91 97908 01678
LinkedIn: https://www.linkedin.com/in/vikassamvel/
GitHub: https://github.com/VIKASSAMVEL
"""

        html_body = f"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style>{css_styles}</style>
</head>
<body>
<div class="container">
  <p>Hi {first_name},</p>

  <p>I'm reaching out directly because I follow NVIDIA's automotive vision and your leadership in driving DRIVE Thor and centralized vehicle computing. As autonomous vehicles move toward multi-camera perception and cabin intelligence on unified automotive SoCs, I wanted to share a low-latency C++17 runtime I've been building that enforces hard real-time deadlines under sensor jitter and thermal stress.</p>

  <p>I'm <strong>Vikas S</strong>, a final-year B.Tech CSE student at Vel Tech University in Chennai. Most of my peers focus on web applications, but I've dedicated my college years to low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and GPU runtimes.</p>

  <p>Over the last several months, I built an open-source deterministic edge AI co-scheduler in C++17 (<strong>EdgeModelScheduler v4.0</strong>). I engineered it specifically to address automotive runtime challenges on platforms like DRIVE Orin and DRIVE Thor:</p>

  <ol>
    <li><strong>Hard Real-Time Deadlines vs. FIFO Bufferbloat:</strong> Autonomous vehicle perception requires hard physical deadlines (sub-35ms frame-to-braking budget). Under bursty camera or LiDAR jitter, standard FIFO queues in generic runtimes queue frames indefinitely, resulting in severe tail latency spikes (P99 &gt; 73 ms) and a 35% critical deadline failure rate.</li>
    <li><strong>Lack of ASIL-D Black Box Audit Trails:</strong> Generic inference servers like Triton do not provide on-chip deterministic flight recorders. When an emergency stop or safety watchdog trips, there is no zero-overhead mechanism to reconstruct what the neural perception stack saw in the exact preceding milliseconds.</li>
    <li><strong>Thermal DVFS Shock in Automotive Silicon:</strong> When automotive SoCs experience thermal stress (reaching 82&deg;C), hardware throttling drops clock frequencies, causing sudden multi-frame dropouts in safety-critical object detection.</li>
  </ol>

  <p>To solve this, I built a pure C++17 runtime with zero heap allocations on the critical path:</p>

  <ul>
    <li><strong>Slack-Aware Pre-Execution Shedding:</strong> Continuously calculates remaining deadline slack before submitting tasks to CUDA streams. If a frame has already expired due to sensor bus stalls, it sheds it immediately, saving 100% of GPU compute for current frames and maintaining a <strong>0.00% deadline miss rate</strong> under 45ms network stalls.</li>
    <li><strong>Sub-Microsecond Emergency Braking Hazard Arbitration:</strong> Prioritizes critical collision avoidance paths with <strong>1.32 microsecond arbitration latency</strong>, ensuring that emergency braking paths are never starved by background trajectory or cabin AI tasks.</li>
    <li><strong>ISO 26262 ASIL-D Telemetry Flight Recorder:</strong> Pre-allocated circular telemetry buffer (60,000 frames @ 1,000 Hz) that auto-dumps rolling SHA-256 cryptographically verified <code>.emsbag</code> binaries upon any safety island trip for complete forensic traceability.</li>
    <li><strong>Closed-Loop Thermal Precision Governor:</strong> Seamlessly degrades secondary perception models from FP16 to INT8 at 82&deg;C to reduce memory bus bandwidth and cool the die, while locking collision avoidance in full precision.</li>
    <li><strong>Ultra-Lean 3.125 MB Resident RAM Footprint:</strong> Leaves maximum compute and memory headroom for surround-view perception models.</li>
  </ul>

  <div class="links-box">
    <div style="font-weight: 600; margin-bottom: 6px;">Project Resources:</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Technical Report (PDF)</a> &mdash; 30-section architecture and benchmark documentation</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Bento Dashboard</a> &mdash; Interactive hardware topology and live simulator</li>
      <li><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">GitHub Repository</a> &mdash; Full C++17 source with 12 passing test suites</li>
    </ul>
  </div>

  <p>I'm writing because as I prepare to graduate, my goal is to work on low-level systems software and runtime engineering within NVIDIA Automotive. I would love 15 minutes of your time to introduce myself, or an introduction to an engineering lead on the DRIVE Thor runtime, Tegra System Software, or Autonomous Vehicle platforms team where an entry-level engineer with my systems foundation could contribute.</p>

  <p>Thank you for your time and for driving the future of autonomous vehicles.</p>

  <div class="footer">
    <p style="margin: 0 0 2px 0; font-weight: 600; color: #0f172a;">Vikas S</p>
    <p style="margin: 0 0 4px 0; color: #475569;">Final-Year B.Tech CSE Student, Vel Tech University, Chennai</p>
    <p style="margin: 0; font-size: 13px;">
      Email: <a href="mailto:vikassamvel123@gmail.com">vikassamvel123@gmail.com</a> | Phone: +91 97908 01678<br>
      LinkedIn: <a href="https://www.linkedin.com/in/vikassamvel/">linkedin.com/in/vikassamvel</a> | GitHub: <a href="https://github.com/VIKASSAMVEL">github.com/VIKASSAMVEL</a>
    </p>
  </div>
</div>
</body>
</html>
"""

    elif category == "panw_dataplane":
        subject = "Final-year student (Vel Tech) — built C++17 deterministic runtime for inline AI on Single-Pass dataplanes"
        
        plain_text = f"""Hi {first_name},

I'm reaching out directly because I follow Palo Alto Networks' dataplane engineering and hardware platforms. Nir Zuk's Single-Pass Architecture set the global gold standard for separating control and data planes to achieve wire-speed deep packet inspection. As PAN-OS brings deep learning inline on Strata hardware appliances (e.g. PA-5400, PA-7000), I wanted to share a low-latency systems runtime I've been building that resolves the timing collision between network packet rings and neural inference.

I'm Vikas S, a final-year B.Tech CSE student at Vel Tech University in Chennai. Most of my peers focus on web applications, but I've dedicated my college years to low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and high-throughput dataplanes.

Over the last several months, I built an open-source deterministic AI co-scheduler in C++17 (EdgeModelScheduler v4.0). I engineered it specifically to solve the architectural challenge of running deep learning inline without breaking line-rate guarantees:

1. Microsecond Dataplane Deadlines vs. Millisecond ML Queues: Network dataplanes (DPDK / hardware NIC rings) operate with microsecond-level deterministic deadlines (<100µs budgets). In contrast, deep learning inference engines (TensorRT, ONNX Runtime) are asynchronous, batch-oriented, and queue-dependent. Under traffic bursts or DDoS conditions, ML queues back up, creating multi-millisecond tail latency blowouts (>70 ms) that either exhaust packet buffer rings (causing dropped sessions) or force appliances to bypass deep inspection.
2. Cache Contention & Mutex Bouncing Across Packet Cores: Standard ML runtimes carry heavy resident footprints (hundreds of MBs) and use coarse mutex locking that creates severe cache-line bouncing across multi-core dataplane processors.
3. Lack of Tamper-Proof Audit Logging: Threat classification decisions made inline need immutable forensic verification for regulatory compliance without incurring disk I/O latency penalties on the packet path.

To solve this, I engineered a C++17 runtime with zero heap allocations on the critical execution path:
- Sub-15ns Lock-Free Dataplane Parity: Custom Single-Producer Single-Consumer (SPSC) circular rings with head and tail atomics isolated on separate 64-byte cache lines (alignas(64)), eliminating lock contention and CPU cache-line bouncing across high-rate packet cores.
- Zero-Copy Direct-Memory Transport (12.23 GB/s): Maps DPDK-style packet buffers directly to AI acceleration memory in <15 microseconds via 64-byte virtual handles over /dev/shm, eliminating CPU/PCIe serialization.
- Predictive Jitter Shedding & Queue Defense: Under line-rate traffic bursts, the closed-loop scheduler predictively sheds stale intermediate telemetry, bounding P99 worst-case tail latency to 29.02 ms (vs. 73 ms+ queue runaway in unmanaged FIFO systems) and maintaining a 0.00% packet buffer drop rate.
- Dynamic Precision Degradation: Closed-loop governor automatically degrades deep packet inspection models from FP16 to INT8 during saturation storms, halving memory bandwidth and doubling throughput to maintain continuous inline inspection.
- High-Assurance Tamper-Proof Flight Recorder: Pre-allocated, zero-heap ring buffer (60,000 frames @ 1 kHz) logging inference decisions and thermals with rolling SHA-256 cryptographic signatures.
- Ultra-Lean 4.8 MB Resident Core: Built without external framework dependencies, easily fitting into constrained on-box network processors.

I've documented the complete architecture, cache-line profiling, and empirical benchmark data here:
- Technical Report (PDF): https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
- Live Bento Dashboard: https://vikassamvel.github.io/edge-model-scheduler/
- GitHub Repository: https://github.com/VIKASSAMVEL/edge-model-scheduler

I'm writing because as I prepare to graduate, my ambition is to work on low-latency systems and dataplane software at Palo Alto Networks. I would love 15 minutes of your time to introduce myself, or an introduction to an engineering lead on the Hardware Dataplane Engineering, Network Security Platforms, or PAN-OS core teams where an entry-level engineer with my low-level systems focus could contribute.

Thank you for your time and for building the world's most robust security dataplanes.

Thanks for your time,
Vikas S
Final-Year B.Tech CSE Student, Vel Tech University, Chennai
vikassamvel123@gmail.com | +91 97908 01678
LinkedIn: https://www.linkedin.com/in/vikassamvel/
GitHub: https://github.com/VIKASSAMVEL
"""

        html_body = f"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style>{css_styles}</style>
</head>
<body>
<div class="container">
  <p>Hi {first_name},</p>

  <p>I'm reaching out directly because I follow Palo Alto Networks' dataplane engineering and hardware platforms. Nir Zuk's Single-Pass Architecture set the global gold standard for separating control and data planes to achieve wire-speed deep packet inspection. As PAN-OS brings deep learning inline on Strata hardware appliances (e.g. PA-5400, PA-7000), I wanted to share a low-latency systems runtime I've been building that resolves the timing collision between network packet rings and neural inference.</p>

  <p>I'm <strong>Vikas S</strong>, a final-year B.Tech CSE student at Vel Tech University in Chennai. Most of my peers focus on web applications, but I've dedicated my college years to low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and high-throughput dataplanes.</p>

  <p>Over the last several months, I built an open-source deterministic AI co-scheduler in C++17 (<strong>EdgeModelScheduler v4.0</strong>). I engineered it specifically to solve the architectural challenge of running deep learning inline without breaking line-rate guarantees:</p>

  <ol>
    <li><strong>Microsecond Dataplane Deadlines vs. Millisecond ML Queues:</strong> Network dataplanes (DPDK / hardware NIC rings) operate with microsecond-level deterministic deadlines (&lt;100&mu;s budgets). In contrast, deep learning inference engines (TensorRT, ONNX Runtime) are asynchronous, batch-oriented, and queue-dependent. Under traffic bursts or DDoS conditions, ML queues back up, creating multi-millisecond tail latency blowouts (&gt;70 ms) that either exhaust packet buffer rings (causing dropped sessions) or force appliances to bypass deep inspection.</li>
    <li><strong>Cache Contention &amp; Mutex Bouncing Across Packet Cores:</strong> Standard ML runtimes carry heavy resident footprints (hundreds of MBs) and use coarse mutex locking that creates severe cache-line bouncing across multi-core dataplane processors.</li>
    <li><strong>Lack of Tamper-Proof Audit Logging:</strong> Threat classification decisions made inline need immutable forensic verification for regulatory compliance without incurring disk I/O latency penalties on the packet path.</li>
  </ol>

  <p>To solve this, I engineered a C++17 runtime with zero heap allocations on the critical execution path:</p>

  <ul>
    <li><strong>Sub-15ns Lock-Free Dataplane Parity:</strong> Custom Single-Producer Single-Consumer (SPSC) circular rings with head and tail atomics isolated on separate 64-byte cache lines (<code>alignas(64)</code>), eliminating lock contention and CPU cache-line bouncing across high-rate packet cores.</li>
    <li><strong>Zero-Copy Direct-Memory Transport (12.23 GB/s):</strong> Maps DPDK-style packet buffers directly to AI acceleration memory in &lt;15 microseconds via 64-byte virtual handles over <code>/dev/shm</code>, eliminating CPU/PCIe serialization.</li>
    <li><strong>Predictive Jitter Shedding &amp; Queue Defense:</strong> Under line-rate traffic bursts, the closed-loop scheduler predictively sheds stale intermediate telemetry, bounding P99 worst-case tail latency to <strong>29.02 ms</strong> (vs. 73 ms+ queue runaway in unmanaged FIFO systems) and maintaining a <strong>0.00% packet buffer drop rate</strong>.</li>
    <li><strong>Dynamic Precision Degradation:</strong> Closed-loop governor automatically degrades deep packet inspection models from FP16 to INT8 during saturation storms, halving memory bandwidth and doubling throughput to maintain continuous inline inspection.</li>
    <li><strong>High-Assurance Tamper-Proof Flight Recorder:</strong> Pre-allocated, zero-heap ring buffer (60,000 frames @ 1 kHz) logging inference decisions and thermals with rolling SHA-256 cryptographic signatures.</li>
    <li><strong>Ultra-Lean 4.8 MB Resident Core:</strong> Built without external framework dependencies, easily fitting into constrained on-box network processors.</li>
  </ul>

  <div class="links-box">
    <div style="font-weight: 600; margin-bottom: 6px;">Project Resources:</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Technical Report (PDF)</a> &mdash; 30-section architecture and benchmark documentation</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Bento Dashboard</a> &mdash; Interactive hardware topology and live simulator</li>
      <li><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">GitHub Repository</a> &mdash; Full C++17 source with 12 passing test suites</li>
    </ul>
  </div>

  <p>I'm writing because as I prepare to graduate, my ambition is to work on low-latency systems and dataplane software at Palo Alto Networks. I would love 15 minutes of your time to introduce myself, or an introduction to an engineering lead on the Hardware Dataplane Engineering, Network Security Platforms, or PAN-OS core teams where an entry-level engineer with my low-level systems focus could contribute.</p>

  <p>Thank you for your time and for building the world's most robust security dataplanes.</p>

  <div class="footer">
    <p style="margin: 0 0 2px 0; font-weight: 600; color: #0f172a;">Vikas S</p>
    <p style="margin: 0 0 4px 0; color: #475569;">Final-Year B.Tech CSE Student, Vel Tech University, Chennai</p>
    <p style="margin: 0; font-size: 13px;">
      Email: <a href="mailto:vikassamvel123@gmail.com">vikassamvel123@gmail.com</a> | Phone: +91 97908 01678<br>
      LinkedIn: <a href="https://www.linkedin.com/in/vikassamvel/">linkedin.com/in/vikassamvel</a> | GitHub: <a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">github.com/VIKASSAMVEL</a>
    </p>
  </div>
</div>
</body>
</html>
"""

    else: # panw_leadership
        subject = "Final-year student (Vel Tech) — built deterministic C++17 runtime for inline AI network security"
        
        plain_text = f"""Hi {first_name},

I'm reaching out directly because I follow your leadership in transforming Palo Alto Networks through platformization and precision AI. Stopping zero-day threats inline before they enter an enterprise requires running complex deep learning models directly on high-speed firewall appliances. I wanted to share a low-latency systems runtime I've been building that addresses the performance and latency challenges of running neural networks inline at line rate.

I'm Vikas S, a final-year B.Tech CSE student at Vel Tech University in Chennai. While most students around me focus on standard web applications, I've spent my college years going deep into low-level systems: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and deterministic network dataplanes.

Over the last several months, I built an open-source deterministic AI co-scheduler in C++17 (EdgeModelScheduler v4.0). I engineered it specifically to solve the performance trade-offs of deploying deep learning inline on enterprise security hardware:

1. Stopping Threats Inline vs. Line-Rate Latency: Running AI out-of-band or in the cloud leaves a multi-second detection window that modern automated attacks exploit. However, embedding deep learning directly into hardware firewalls risks creating latency bottlenecks whenever traffic bursts occur, since standard ML frameworks were designed for datacenter servers rather than wire-speed network appliances.
2. Managing Traffic Surges Without Dropping Traffic: Under DDoS conditions or packet bursts, standard inference queues back up, creating multi-millisecond tail latency spikes (>70 ms). This forces firewalls to either drop valid sessions or bypass inspection.
3. Memory and Hardware Resource Constraints: Standard AI runtimes require hundreds of megabytes of resident memory and rely on heavy frameworks that are unsuitable for on-box security processors.

To address these challenges, I built a pure C++17 runtime with zero heap allocations on the critical path:
- Deterministic Queue Defense: Predictively sheds expired intermediate telemetry during line-rate traffic bursts, bounding P99 worst-case tail latency to 29.02 ms (vs. 73 ms+ queue runaway in unmanaged FIFO systems) with a 0.00% packet buffer drop rate.
- Sub-15ns Lock-Free Dataplane Ring: Custom Single-Producer Single-Consumer (SPSC) circular rings with head and tail atomics isolated on separate 64-byte cache lines (alignas(64)), preventing CPU lock contention and cache-line bouncing.
- Direct-Memory Transport (12.23 GB/s): Maps packet buffers directly to AI acceleration memory in <15 microseconds via 64-byte virtual handles over /dev/shm, eliminating memory copy overhead.
- Dynamic Precision Governor: Automatically shifts deep packet inspection models from FP16 to INT8 during heavy traffic storms to double throughput and maintain continuous inline protection.
- Immutable Forensic Flight Recorder: Pre-allocated circular telemetry buffer (60,000 frames @ 1 kHz) that logs every inline AI decision with rolling SHA-256 cryptographic signatures for compliance and incident response.
- Ultra-Lean 4.8 MB Resident Footprint: Runs completely self-contained without heavy external dependencies.

I've made the implementation, architecture dossier, and benchmarks public:
- Technical Report (PDF): https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
- Live Bento Dashboard: https://vikassamvel.github.io/edge-model-scheduler/
- GitHub Repository: https://github.com/VIKASSAMVEL/edge-model-scheduler

I'm writing because as I prepare to graduate, my ambition is to work on low-latency systems and AI infrastructure at Palo Alto Networks. I would be deeply grateful for a brief 15-minute conversation to introduce myself, or an introduction to an engineering lead on the Network Security Platforms or PAN-OS engineering teams where an entry-level engineer with my systems foundation could contribute.

Thank you for your time and leadership in shaping the future of cybersecurity.

Thanks for your time,
Vikas S
Final-Year B.Tech CSE Student, Vel Tech University, Chennai
vikassamvel123@gmail.com | +91 97908 01678
LinkedIn: https://www.linkedin.com/in/vikassamvel/
GitHub: https://github.com/VIKASSAMVEL
"""

        html_body = f"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style>{css_styles}</style>
</head>
<body>
<div class="container">
  <p>Hi {first_name},</p>

  <p>I'm reaching out directly because I follow your leadership in transforming Palo Alto Networks through platformization and precision AI. Stopping zero-day threats inline before they enter an enterprise requires running complex deep learning models directly on high-speed firewall appliances. I wanted to share a low-latency systems runtime I've been building that addresses the performance and latency challenges of running neural networks inline at line rate.</p>

  <p>I'm <strong>Vikas S</strong>, a final-year B.Tech CSE student at Vel Tech University in Chennai. While most students around me focus on standard web applications, I've spent my college years going deep into low-level systems: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and deterministic network dataplanes.</p>

  <p>Over the last several months, I built an open-source deterministic AI co-scheduler in C++17 (<strong>EdgeModelScheduler v4.0</strong>). I engineered it specifically to solve the performance trade-offs of deploying deep learning inline on enterprise security hardware:</p>

  <ol>
    <li><strong>Stopping Threats Inline vs. Line-Rate Latency:</strong> Running AI out-of-band or in the cloud leaves a multi-second detection window that modern automated attacks exploit. However, embedding deep learning directly into hardware firewalls risks creating latency bottlenecks whenever traffic bursts occur, since standard ML frameworks were designed for datacenter servers rather than wire-speed network appliances.</li>
    <li><strong>Managing Traffic Surges Without Dropping Traffic:</strong> Under DDoS conditions or packet bursts, standard inference queues back up, creating multi-millisecond tail latency spikes (&gt;70 ms). This forces firewalls to either drop valid sessions or bypass inspection.</li>
    <li><strong>Memory and Hardware Resource Constraints:</strong> Standard AI runtimes require hundreds of megabytes of resident memory and rely on heavy frameworks that are unsuitable for on-box security processors.</li>
  </ol>

  <p>To address these challenges, I built a pure C++17 runtime with zero heap allocations on the critical path:</p>

  <ul>
    <li><strong>Deterministic Queue Defense:</strong> Predictively sheds expired intermediate telemetry during line-rate traffic bursts, bounding P99 worst-case tail latency to <strong>29.02 ms</strong> (vs. 73 ms+ queue runaway in unmanaged FIFO systems) with a <strong>0.00% packet buffer drop rate</strong>.</li>
    <li><strong>Sub-15ns Lock-Free Dataplane Ring:</strong> Custom Single-Producer Single-Consumer (SPSC) circular rings with head and tail atomics isolated on separate 64-byte cache lines (<code>alignas(64)</code>), preventing CPU lock contention and cache-line bouncing.</li>
    <li><strong>Direct-Memory Transport (12.23 GB/s):</strong> Maps packet buffers directly to AI acceleration memory in &lt;15 microseconds via 64-byte virtual handles over <code>/dev/shm</code>, eliminating memory copy overhead.</li>
    <li><strong>Dynamic Precision Governor:</strong> Automatically shifts deep packet inspection models from FP16 to INT8 during heavy traffic storms to double throughput and maintain continuous inline protection.</li>
    <li><strong>Immutable Forensic Flight Recorder:</strong> Pre-allocated circular telemetry buffer (60,000 frames @ 1 kHz) that logs every inline AI decision with rolling SHA-256 cryptographic signatures for compliance and incident response.</li>
    <li><strong>Ultra-Lean 4.8 MB Resident Footprint:</strong> Runs completely self-contained without heavy external dependencies.</li>
  </ul>

  <div class="links-box">
    <div style="font-weight: 600; margin-bottom: 6px;">Project Resources:</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Technical Report (PDF)</a> &mdash; 30-section architecture and benchmark documentation</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Bento Dashboard</a> &mdash; Interactive hardware topology and live simulator</li>
      <li><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">GitHub Repository</a> &mdash; Full C++17 source with 12 passing test suites</li>
    </ul>
  </div>

  <p>I'm writing because as I prepare to graduate, my ambition is to work on low-latency systems and AI infrastructure at Palo Alto Networks. I would be deeply grateful for a brief 15-minute conversation to introduce myself, or an introduction to an engineering lead on the Network Security Platforms or PAN-OS engineering teams where an entry-level engineer with my systems foundation could contribute.</p>

  <p>Thank you for your time and leadership in shaping the future of cybersecurity.</p>

  <div class="footer">
    <p style="margin: 0 0 2px 0; font-weight: 600; color: #0f172a;">Vikas S</p>
    <p style="margin: 0 0 4px 0; color: #475569;">Final-Year B.Tech CSE Student, Vel Tech University, Chennai</p>
    <p style="margin: 0; font-size: 13px;">
      Email: <a href="mailto:vikassamvel123@gmail.com">vikassamvel123@gmail.com</a> | Phone: +91 97908 01678<br>
      LinkedIn: <a href="https://www.linkedin.com/in/vikassamvel/">linkedin.com/in/vikassamvel</a> | GitHub: <a href="https://github.com/VIKASSAMVEL">github.com/VIKASSAMVEL</a>
    </p>
  </div>
</div>
</body>
</html>
"""

    return subject, plain_text, html_body

def send_single_mail(server, sender_email, recipient_email, subject, plain_text, html_body):
    msg = MIMEMultipart('alternative')
    msg['From'] = f"Vikas S <{sender_email}>"
    msg['To'] = recipient_email
    msg['Subject'] = subject

    part1 = MIMEText(plain_text, 'plain', 'utf-8')
    part2 = MIMEText(html_body, 'html', 'utf-8')
    msg.attach(part1)
    msg.attach(part2)

    server.sendmail(sender_email, recipient_email, msg.as_string())

def main():
    creds = load_credentials()
    sender_email = creds["email"]
    app_password = creds["app_password"].replace(" ", "")

    mode = sys.argv[1] if len(sys.argv) > 1 else "--dry-run"

    print("=================================================================")
    print("🚀 EdgeModelScheduler Executive Outreach Dispatcher")
    print(f"   Sender : {sender_email}")
    print(f"   Mode   : {mode.upper()}")
    print("=================================================================\n")

    if mode == "--dry-run":
        print(f"Total Targets: {len(RECIPIENTS)}\n")
        for idx, target in enumerate(RECIPIENTS, 1):
            subj, plain, html = generate_email_content(target)
            print(f"-----------------------------------------------------------------")
            print(f"[{idx}/{len(RECIPIENTS)}] TO: {target['name']} <{target['email']}>")
            print(f"TITLE: {target['title']} | {target['company']}")
            print(f"SUBJECT: {subj}")
            print(f"-----------------------------------------------------------------")
            print(plain[:260] + "...\n[Full body ready to send]\n")
        print("Dry run completed. No emails were sent.")
        print("To send a test verification mail to yourself, run:")
        print("  python outreach/send_outreach.py --test-me\n")
        print("To send individually to all 9 executives, run:")
        print("  python outreach/send_outreach.py --send\n")

    elif mode == "--test-me":
        print(f"Sending test verification email to: {sender_email}...")
        context = ssl.create_default_context()
        with smtplib.SMTP_SSL("smtp.gmail.com", 465, context=context) as server:
            server.login(sender_email, app_password)
            subj, plain, html = generate_email_content(RECIPIENTS[0])
            test_subj = f"[TEST VERIFICATION] {subj}"
            send_single_mail(server, sender_email, sender_email, test_subj, plain, html)
        print(f"✅ Test email successfully sent to {sender_email}! Check your inbox.")

    elif mode == "--send":
        print("⚠️  CONFIRMED SEND MODE: Dispatching individually to each recipient...")
        context = ssl.create_default_context()
        with smtplib.SMTP_SSL("smtp.gmail.com", 465, context=context) as server:
            server.login(sender_email, app_password)
            for idx, target in enumerate(RECIPIENTS, 1):
                subj, plain, html = generate_email_content(target)
                print(f"[{idx}/{len(RECIPIENTS)}] Sending to {target['name']} <{target['email']}>...")
                send_single_mail(server, sender_email, target["email"], subj, plain, html)
                print(f"    ✅ Delivered to {target['email']}")
                if idx < len(RECIPIENTS):
                    print("    Pausing 4s to adhere to SMTP rate limits...")
                    time.sleep(4)
        print("\n🎉 All 9 individualized executive emails dispatched successfully!")

    else:
        print(f"Unknown mode: {mode}. Use --dry-run, --test-me, or --send.")

if __name__ == "__main__":
    main()
