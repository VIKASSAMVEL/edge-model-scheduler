#!/usr/bin/env python3
"""
EdgeModelScheduler Executive Outreach Dispatcher - Option D (Hybrid Executive Dossier)
Sends individualized, deeply technical, beautifully formatted cold emails to leadership at NVIDIA and Palo Alto Networks.
Uses MIMEMultipart('mixed') with an inner MIMEMultipart('alternative') for clean plain-text + mobile-responsive HTML,
with high-contrast competitive architecture tables, deep C++17 systems teardowns, and the verified resume PDF (S_Vikas_Resume.pdf) attached.
"""

import sys
import os
import json
import time
import smtplib
import ssl
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.mime.application import MIMEApplication

# Path to verified resume PDF
RESUME_PATH = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "S_Vikas_Resume.pdf")
if not os.path.exists(RESUME_PATH):
    RESUME_PATH = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "resume_tcs_prime_java_ai_latex.pdf")

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
  .container { max-width: 650px; margin: 0 auto; padding: 22px 16px; }
  p { margin: 0 0 14px 0; }
  ol, ul { margin: 0 0 14px 0; padding-left: 20px; }
  li { margin-bottom: 8px; }
  .section-title { font-weight: 700; color: #0f172a; margin: 22px 0 10px 0; font-size: 14.5px; text-transform: uppercase; letter-spacing: 0.5px; }
  .table-box { overflow-x: auto; margin: 18px 0; border: 1px solid #cbd5e1; border-radius: 6px; }
  table.comp-table { width: 100%; border-collapse: collapse; font-size: 13.5px; text-align: left; }
  table.comp-table th { background: #f8fafc; color: #0f172a; font-weight: 600; padding: 10px 12px; border-bottom: 2px solid #cbd5e1; }
  table.comp-table td { padding: 9px 12px; border-bottom: 1px solid #e2e8f0; vertical-align: top; color: #334155; }
  table.comp-table tr:nth-child(even) { background: #fdfdfd; }
  .metric-highlight { color: #047857; font-weight: 600; }
  .metric-standard { color: #64748b; }
  .links-box { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 6px; padding: 14px 16px; margin: 20px 0; }
  a { color: #2563eb; text-decoration: underline; font-weight: 500; }
  .footer { border-top: 1px solid #e2e8f0; padding-top: 14px; margin-top: 22px; color: #475569; font-size: 14px; }
"""

    if category == "nvidia_robotics":
        subject = "EdgeModelScheduler v4.0: Deterministic C++17 runtime bypassing Triton & ROS 2 DDS on Jetson"
        
        plain_text = f"""Hi {first_name},

I'm reaching out directly because I follow your leadership in Autonomous Machines and robotics at NVIDIA. With NVIDIA driving the frontier of Physical AI and the Jetson Orin ecosystem, I wanted to share a low-latency systems runtime I've been building that tackles memory and scheduling bottlenecks on edge silicon.

I'm S. Vikas, a final-year B.Tech CSE student at Vel Tech University in Chennai (CGPA: 8.67/10). While most students around me focus on standard web stacks, I've spent my college years going deep into low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and GPU runtimes. During my R&D internship at Ashok Leyland (guided by DGM, where I filed a patent for an autonomous agricultural robot), I worked hands-on with embedded C++ control logic on a Teensy 4.1 handling high-frequency LiDAR and encoder sensor streams.

Experiencing firsthand how middleware serialization and memory overhead choke real-time perception on edge hardware led me to build an open-source deterministic edge AI co-scheduler in C++17 (EdgeModelScheduler v4.0). I engineered it specifically to solve three critical bottlenecks that occur when deploying multi-model robotics pipelines on Jetson SoCs:

ARCHITECTURAL BENCHMARK: TRITON & ROS 2 DDS VS. EDGEMODELSCHEDULER v4.0
+--------------------------+--------------------------+---------------------------+----------------------------------------------+
| Dimension                | Standard Edge Stack      | EdgeModelScheduler v4.0   | Jetson Robotics Advantage                    |
+--------------------------+--------------------------+---------------------------+----------------------------------------------+
| Resident RAM Footprint   | ~1,240 MB (Triton Idle)  | 3.125 MB VmRSS            | 396x leaner; leaves LPDDR5 for voxel grids   |
| Sensor IPC Latency       | 20–35 ms (4x DDS copies) | <15 µs (12.23 GB/s)       | Zero-copy CUDA IPC over /dev/shm             |
| Bursty Jitter Defense    | P99 > 73 ms (35% drops)  | P99 29.02 ms (0.00% drops)| Slack-aware pre-execution shedding           |
| Thermal Stress (82°C)    | Severe throttle dropouts | Closed-loop INT8 governor | Halves bus bandwidth; cools die in real-time |
+--------------------------+--------------------------+---------------------------+----------------------------------------------+

CORE LOW-LEVEL SYSTEMS INNOVATIONS:
1. 3.125 MB Resident Memory Core: Operates ~396x leaner than Triton (3.1 MB vs. 1,240 MB), completely avoiding the memory starvation that occurs when running SLAM, local costmaps, and voxel grids concurrently on an 8GB Jetson Orin Nano.
2. CUDA IPC Zero-Copy TypeAdapter (12.23 GB/s): Transfers raw PointCloud2 LiDAR and 4K camera frames across process boundaries via 64-byte virtual memory handles over /dev/shm in <15 microseconds, bypassing ROS 2 DDS serialization entirely.
3. Sub-15ns Lock-Free SPSC Arbiter: Custom Single-Producer Single-Consumer circular rings with head and tail atomics isolated on separate 64-byte cache lines (alignas(64)), eliminating CPU mutex lock contention and cache-line bouncing across ARM Cortex-A78 cores.
4. Slack-Aware Pre-Execution Shedding: Continuously evaluates frame deadline slack [T_slack = T_deadline - (T_now + WCET)] before submitting tasks to CUDA streams. Under simulated 45 ms sensor bursts, it predictively sheds stale intermediate frames, saving 100% of GPU compute for active obstacles and locking a 0.00% deadline miss rate.
5. Dynamic Thermal DVFS Hysteresis Governor: Seamlessly down-quantizes secondary perception models from FP16 to INT8 upon reaching 82°C, cutting memory bus draw by 50% while locking critical obstacle avoidance in full precision.
6. ISO 26262 ASIL-D Telemetry Flight Recorder: Pre-allocated circular telemetry buffer (60,000 frames @ 1 kHz) that auto-dumps rolling SHA-256 cryptographically verified .emsbag binaries upon emergency stops.

I've attached my full resume (S_Vikas_Resume.pdf) and documented the complete architecture, cache-line profiling, and reproducible benchmarks in our technical dossier:
- Attached Resume: S_Vikas_Resume.pdf (Vel Tech B.Tech CSE 8.67 CGPA, 3 Patents Filed, Ashok Leyland R&D)
- Technical Report (PDF): https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
- Live Bento Dashboard: https://vikassamvel.github.io/edge-model-scheduler/
- GitHub Repository: https://github.com/VIKASSAMVEL/edge-model-scheduler
- Personal Portfolio: https://vikas-world.vercel.app

I'm writing because as I prepare to graduate, my ambition is to contribute to NVIDIA's autonomous machines and robotics platform teams. I would be deeply grateful for a brief 15-minute conversation to get your perspective, or an introduction to an engineering lead or hiring manager on the Jetson System Software, Isaac ROS, or Autonomous Machines teams where an entry-level engineer with my profile could contribute.

Thank you for your time and leadership in this space.

Thanks for your time,
S. Vikas
Systems Software & Edge AI Engineer
B.Tech Computer Science & Engineering (CGPA: 8.67 / 10), Vel Tech University, Chennai
vikassamvel123@gmail.com | +91 97908 01678
LinkedIn: https://www.linkedin.com/in/vikas-s070106
GitHub: https://github.com/VIKASSAMVEL
Portfolio: https://vikas-world.vercel.app
(Resume attached: S_Vikas_Resume.pdf)
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

  <p>I'm <strong>S. Vikas</strong>, a final-year B.Tech CSE student at Vel Tech University in Chennai (<strong>CGPA: 8.67 / 10</strong>). While most students around me focus on standard web stacks, I've spent my college years going deep into low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and GPU runtimes. During my R&amp;D internship at <strong>Ashok Leyland</strong> (guided by DGM, where I filed a patent for an autonomous agricultural robot), I worked hands-on with embedded C++ control logic on a Teensy 4.1 handling high-frequency LiDAR and encoder sensor streams.</p>

  <p>Experiencing firsthand how middleware serialization and memory overhead choke real-time perception on edge hardware led me to build an open-source deterministic edge AI co-scheduler in C++17 (<strong>EdgeModelScheduler v4.0</strong>). I engineered it specifically to solve three critical bottlenecks that occur when deploying multi-model robotics pipelines on Jetson SoCs:</p>

  <div class="section-title">Architectural Benchmark: Incumbent Stack vs. EdgeModelScheduler v4.0</div>
  <div class="table-box">
    <table class="comp-table">
      <thead>
        <tr>
          <th>Dimension</th>
          <th>Standard Stack (Triton / ROS DDS)</th>
          <th>EdgeModelScheduler v4.0</th>
          <th>Jetson Robotics Advantage</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><strong>Resident RAM</strong></td>
          <td class="metric-standard">~1,240 MB (Triton Idle)</td>
          <td class="metric-highlight">3.125 MB VmRSS</td>
          <td><strong>396x leaner;</strong> leaves physical LPDDR5 RAM for obstacle voxel grids &amp; SLAM</td>
        </tr>
        <tr>
          <td><strong>Sensor Ingestion</strong></td>
          <td class="metric-standard">20&ndash;35 ms (4x DDS serialization)</td>
          <td class="metric-highlight">&lt;15 &mu;s (12.23 GB/s)</td>
          <td>Zero-copy CUDA IPC over <code>/dev/shm</code>; bypasses ROS DDS CPU memory copies</td>
        </tr>
        <tr>
          <td><strong>Jitter Stall Defense</strong></td>
          <td class="metric-standard">P99 &gt; 73 ms (35% deadline drops)</td>
          <td class="metric-highlight">P99 29.02 ms (0.00% drops)</td>
          <td>Slack-aware pre-execution shedding drops expired frames before GPU submission</td>
        </tr>
        <tr>
          <td><strong>Thermal Stress (82&deg;C)</strong></td>
          <td class="metric-standard">Severe clock throttle dropouts</td>
          <td class="metric-highlight">Closed-Loop INT8 Governor</td>
          <td>Halves memory bus bandwidth &amp; cools die while locking obstacle detection in FP16</td>
        </tr>
      </tbody>
    </table>
  </div>

  <div class="section-title">Core Low-Level Systems Innovations:</div>
  <ul>
    <li><strong>3.125 MB Resident Memory Core:</strong> Operates ~396x leaner than Triton (3.1 MB vs. 1,240 MB), completely avoiding memory exhaustion on unified-memory Jetson Orin Nano / AGX Orin SoCs.</li>
    <li><strong>CUDA IPC Zero-Copy TypeAdapter (12.23 GB/s):</strong> Passes raw PointCloud2 LiDAR and 4K camera frames across process boundaries via 64-byte virtual memory handles over <code>/dev/shm</code> in &lt;15 microseconds.</li>
    <li><strong>Sub-15ns Lock-Free SPSC Arbiter:</strong> Custom Single-Producer Single-Consumer circular rings with head and tail atomics isolated on separate 64-byte cache lines (<code>alignas(64)</code>), eliminating CPU lock contention and cache-line bouncing.</li>
    <li><strong>Slack-Aware Pre-Execution Shedding:</strong> Continuously evaluates frame deadline slack before GPU stream submission; under simulated 45 ms network stalls, it predictively sheds stale intermediate frames and maintained a <strong>0.00% deadline miss rate</strong>.</li>
    <li><strong>Dynamic Thermal DVFS Hysteresis Governor:</strong> Seamlessly down-quantizes secondary perception models from FP16 to INT8 upon reaching 82&deg;C, cutting memory bus draw by 50% and cooling the die in real-time.</li>
    <li><strong>ISO 26262 ASIL-D Telemetry Flight Recorder:</strong> Pre-allocated circular telemetry buffer (60,000 frames @ 1 kHz) auto-dumping rolling SHA-256 cryptographically verified <code>.emsbag</code> binaries upon safety island trips.</li>
  </ul>

  <div class="links-box">
    <div style="font-weight: 600; margin-bottom: 6px;">Verified Credentials &amp; Project Resources:</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 4px;"><strong>📎 Attached Resume:</strong> <code>S_Vikas_Resume.pdf</code> (Education, 3 Patents Filed, Ashok Leyland R&amp;D)</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Technical Report (PDF)</a> &mdash; 30-section architecture and benchmark documentation</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Bento Dashboard</a> &mdash; Interactive hardware topology and live simulator</li>
      <li style="margin-bottom: 4px;"><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">GitHub Repository</a> &mdash; Full C++17 source with 12 passing test suites</li>
      <li><a href="https://vikas-world.vercel.app">Personal Portfolio</a> &mdash; vikas-world.vercel.app</li>
    </ul>
  </div>

  <p>I'm writing because as I prepare to graduate, my ambition is to contribute to NVIDIA's autonomous machines and robotics platform teams. I would be deeply grateful for a brief 15-minute conversation to get your perspective, or an introduction to an engineering lead or hiring manager on the Jetson System Software, Isaac ROS, or Autonomous Machines teams where an entry-level engineer with my profile could contribute.</p>

  <p>Thank you for your time and leadership in this space.</p>

  <div class="footer">
    <p style="margin: 0 0 2px 0; font-weight: 600; color: #0f172a;">S. Vikas</p>
    <p style="margin: 0 0 4px 0; color: #475569;">Systems Software &amp; Edge AI Engineer<br>B.Tech Computer Science &amp; Engineering (CGPA: 8.67 / 10), Vel Tech University, Chennai</p>
    <p style="margin: 0; font-size: 13px;">
      Email: <a href="mailto:vikassamvel123@gmail.com">vikassamvel123@gmail.com</a> | Phone: +91 97908 01678<br>
      LinkedIn: <a href="https://www.linkedin.com/in/vikas-s070106">linkedin.com/in/vikas-s070106</a> | GitHub: <a href="https://github.com/VIKASSAMVEL">github.com/VIKASSAMVEL</a> | Portfolio: <a href="https://vikas-world.vercel.app">vikas-world.vercel.app</a>
    </p>
  </div>
</div>
</body>
</html>
"""

    elif category == "nvidia_automotive":
        subject = "EdgeModelScheduler v4.0: Deterministic C++17 co-scheduler for sub-35ms ADAS deadlines on DRIVE Thor / Orin"
        
        plain_text = f"""Hi {first_name},

I'm reaching out directly because I follow NVIDIA's automotive vision and your leadership in driving DRIVE Thor and centralized vehicle computing. As autonomous vehicles move toward multi-camera perception and cabin intelligence on unified automotive SoCs, I wanted to share a low-latency C++17 runtime I've been building that enforces hard real-time deadlines under sensor jitter and thermal stress.

I'm S. Vikas, a final-year B.Tech CSE student at Vel Tech University in Chennai (CGPA: 8.67/10). While most students around me focus on standard web stacks, I've spent my college years going deep into low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and GPU runtimes. During my R&D internship at Ashok Leyland (guided by DGM, where I filed a patent for an autonomous agricultural robot and worked on worker safety compliance monitoring), I engineered embedded C++ control logic on a Teensy 4.1 handling real-time LiDAR and sensor telemetry.

Experiencing firsthand how physical vehicle safety depends on deterministic latency guarantees led me to build an open-source deterministic edge AI co-scheduler in C++17 (EdgeModelScheduler v4.0). I engineered it specifically to address automotive runtime challenges on platforms like DRIVE Orin and DRIVE Thor:

ARCHITECTURAL BENCHMARK: GENERIC AUTOMOTIVE FIFO VS. EDGEMODELSCHEDULER v4.0
+--------------------------+--------------------------+---------------------------+----------------------------------------------+
| Dimension                | Standard Automotive FIFO | EdgeModelScheduler v4.0   | DRIVE Thor / Orin ADAS Advantage             |
+--------------------------+--------------------------+---------------------------+----------------------------------------------+
| Real-Time Deadline Slack | P99 > 73 ms (35% fails)  | P99 29.02 ms (0.00% drops)| Sub-35ms frame-to-braking budget guaranteed  |
| Emergency Hazard Preempt | FIFO delay (>12 ms)      | 1.32 µs arbitration       | Sub-microsecond priority collision avoidance |
| Forensic Auditability    | No on-chip black box log | ISO 26262 ASIL-D Recorder | 60,000 frames @ 1 kHz rolling SHA-256 binary |
| Thermal Shock (82°C)     | Throttling frame drops   | Closed-loop INT8 Governor | Preserves die cooling without losing safety  |
| Memory Overhead          | Hundreds of MBs          | 3.125 MB VmRSS            | Maximum headroom for surround-view cameras   |
+--------------------------+--------------------------+---------------------------+----------------------------------------------+

CORE LOW-LEVEL SYSTEMS INNOVATIONS:
1. Slack-Aware Pre-Execution Shedding: Continuously calculates remaining deadline slack before submitting tasks to CUDA streams. If a frame has already expired due to sensor bus stalls, it sheds it immediately, saving 100% of GPU compute for current frames and maintaining a 0.00% deadline miss rate under 45ms network stalls.
2. Sub-Microsecond Emergency Braking Hazard Arbitration: Prioritizes critical collision avoidance paths with 1.32 microsecond arbitration latency, ensuring that emergency braking paths are never starved by background trajectory or cabin AI tasks.
3. Sub-15ns Lock-Free Dataplane Ring: Custom Single-Producer Single-Consumer (SPSC) circular rings with head and tail atomics isolated on separate 64-byte cache lines (alignas(64)), preventing CPU lock contention and cache-line bouncing across multi-core processors.
4. Direct-Memory Transport (12.23 GB/s): Maps camera and LiDAR buffers directly to AI acceleration memory in <15 microseconds via 64-byte virtual handles over /dev/shm, eliminating CPU/PCIe serialization.
5. ISO 26262 ASIL-D Telemetry Flight Recorder: Pre-allocated circular telemetry buffer (60,000 frames @ 1,000 Hz) that auto-dumps rolling SHA-256 cryptographically verified .emsbag binaries upon any safety island trip for complete forensic traceability.
6. Closed-Loop Thermal Precision Governor: Seamlessly degrades secondary perception models from FP16 to INT8 at 82°C to reduce memory bus bandwidth and cool the die, while locking collision avoidance in full precision.

I've attached my full resume (S_Vikas_Resume.pdf) and documented the complete architecture, cache-line profiling, and benchmarks in our technical dossier:
- Attached Resume: S_Vikas_Resume.pdf (Vel Tech B.Tech CSE 8.67 CGPA, 3 Patents Filed, Ashok Leyland R&D)
- Technical Report (PDF): https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
- Live Bento Dashboard: https://vikassamvel.github.io/edge-model-scheduler/
- GitHub Repository: https://github.com/VIKASSAMVEL/edge-model-scheduler
- Personal Portfolio: https://vikas-world.vercel.app

I'm writing because as I prepare to graduate, my goal is to work on low-level systems software and runtime engineering within NVIDIA Automotive. I would love 15 minutes of your time to introduce myself, or an introduction to an engineering lead on the DRIVE Thor runtime, Tegra System Software, or Autonomous Vehicle platforms team where an entry-level engineer with my systems foundation could contribute.

Thank you for your time and for driving the future of autonomous vehicles.

Thanks for your time,
S. Vikas
Systems Software & Edge AI Engineer
B.Tech Computer Science & Engineering (CGPA: 8.67 / 10), Vel Tech University, Chennai
vikassamvel123@gmail.com | +91 97908 01678
LinkedIn: https://www.linkedin.com/in/vikas-s070106
GitHub: https://github.com/VIKASSAMVEL
Portfolio: https://vikas-world.vercel.app
(Resume attached: S_Vikas_Resume.pdf)
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

  <p>I'm <strong>S. Vikas</strong>, a final-year B.Tech CSE student at Vel Tech University in Chennai (<strong>CGPA: 8.67 / 10</strong>). While most students around me focus on standard web stacks, I've spent my college years going deep into low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and GPU runtimes. During my R&amp;D internship at <strong>Ashok Leyland</strong> (guided by DGM, where I filed a patent for an autonomous agricultural robot and worked on worker safety compliance monitoring), I engineered embedded C++ control logic on a Teensy 4.1 handling real-time LiDAR and sensor telemetry.</p>

  <p>Experiencing firsthand how physical vehicle safety depends on deterministic latency guarantees led me to build an open-source deterministic edge AI co-scheduler in C++17 (<strong>EdgeModelScheduler v4.0</strong>). I engineered it specifically to address automotive runtime challenges on platforms like DRIVE Orin and DRIVE Thor:</p>

  <div class="section-title">Architectural Benchmark: Generic FIFO vs. EdgeModelScheduler v4.0</div>
  <div class="table-box">
    <table class="comp-table">
      <thead>
        <tr>
          <th>Dimension</th>
          <th>Standard Automotive FIFO</th>
          <th>EdgeModelScheduler v4.0</th>
          <th>DRIVE Thor / Orin ADAS Impact</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><strong>Hard Deadline Budget</strong></td>
          <td class="metric-standard">P99 &gt; 73 ms (35% ASIL-D fails)</td>
          <td class="metric-highlight">P99 29.02 ms (0.00% drops)</td>
          <td>Sub-35ms frame-to-braking budget strictly enforced under 45ms sensor jitter</td>
        </tr>
        <tr>
          <td><strong>Hazard Preemption</strong></td>
          <td class="metric-standard">FIFO queuing delay (&gt;12 ms)</td>
          <td class="metric-highlight">1.32 &mu;s arbitration latency</td>
          <td>Sub-microsecond priority preemption for emergency collision avoidance</td>
        </tr>
        <tr>
          <td><strong>Forensic Auditability</strong></td>
          <td class="metric-standard">No deterministic flight log</td>
          <td class="metric-highlight">ISO 26262 ASIL-D Recorder</td>
          <td>Pre-allocated buffer auto-dumping rolling SHA-256 <code>.emsbag</code> binaries on safety trips</td>
        </tr>
        <tr>
          <td><strong>Thermal Shock (82&deg;C)</strong></td>
          <td class="metric-standard">Clock throttle drops critical frames</td>
          <td class="metric-highlight">Closed-Loop INT8 Governor</td>
          <td>Gracefully down-quantizes secondary models, preserving die cooling &amp; safety</td>
        </tr>
        <tr>
          <td><strong>Memory Overhead</strong></td>
          <td class="metric-standard">Hundreds of MBs</td>
          <td class="metric-highlight">3.125 MB VmRSS</td>
          <td>Leaves maximum memory headroom for high-resolution surround-view cameras</td>
        </tr>
      </tbody>
    </table>
  </div>

  <div class="section-title">Core Low-Level Systems Innovations:</div>
  <ul>
    <li><strong>Slack-Aware Pre-Execution Shedding:</strong> Continuously calculates remaining deadline slack before submitting tasks to CUDA streams; sheds expired frames immediately, maintaining a <strong>0.00% deadline miss rate</strong> under 45ms network stalls.</li>
    <li><strong>Sub-Microsecond Emergency Braking Hazard Arbitration:</strong> Prioritizes critical collision avoidance paths with <strong>1.32 microsecond arbitration latency</strong>.</li>
    <li><strong>Sub-15ns Lock-Free Dataplane Ring:</strong> Custom Single-Producer Single-Consumer circular rings with head and tail atomics isolated on separate 64-byte cache lines (<code>alignas(64)</code>), eliminating lock contention and CPU cache-line bouncing.</li>
    <li><strong>Direct-Memory Transport (12.23 GB/s):</strong> Maps sensor buffers directly to AI acceleration memory in &lt;15 microseconds via 64-byte virtual handles over <code>/dev/shm</code>.</li>
    <li><strong>ISO 26262 ASIL-D Telemetry Flight Recorder:</strong> Pre-allocated circular telemetry buffer (60,000 frames @ 1,000 Hz) that auto-dumps rolling SHA-256 cryptographically verified <code>.emsbag</code> binaries upon safety island trips.</li>
    <li><strong>Closed-Loop Thermal Precision Governor:</strong> Seamlessly degrades secondary perception models from FP16 to INT8 at 82&deg;C to reduce memory bus bandwidth and cool the die, while locking collision avoidance in full precision.</li>
  </ul>

  <div class="links-box">
    <div style="font-weight: 600; margin-bottom: 6px;">Verified Credentials &amp; Project Resources:</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 4px;"><strong>📎 Attached Resume:</strong> <code>S_Vikas_Resume.pdf</code> (Education, 3 Patents Filed, Ashok Leyland R&amp;D)</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Technical Report (PDF)</a> &mdash; 30-section architecture and benchmark documentation</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Bento Dashboard</a> &mdash; Interactive hardware topology and live simulator</li>
      <li style="margin-bottom: 4px;"><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">GitHub Repository</a> &mdash; Full C++17 source with 12 passing test suites</li>
      <li><a href="https://vikas-world.vercel.app">Personal Portfolio</a> &mdash; vikas-world.vercel.app</li>
    </ul>
  </div>

  <p>I'm writing because as I prepare to graduate, my goal is to work on low-level systems software and runtime engineering within NVIDIA Automotive. I would love 15 minutes of your time to introduce myself, or an introduction to an engineering lead on the DRIVE Thor runtime, Tegra System Software, or Autonomous Vehicle platforms team where an entry-level engineer with my systems foundation could contribute.</p>

  <p>Thank you for your time and for driving the future of autonomous vehicles.</p>

  <div class="footer">
    <p style="margin: 0 0 2px 0; font-weight: 600; color: #0f172a;">S. Vikas</p>
    <p style="margin: 0 0 4px 0; color: #475569;">Systems Software &amp; Edge AI Engineer<br>B.Tech Computer Science &amp; Engineering (CGPA: 8.67 / 10), Vel Tech University, Chennai</p>
    <p style="margin: 0; font-size: 13px;">
      Email: <a href="mailto:vikassamvel123@gmail.com">vikassamvel123@gmail.com</a> | Phone: +91 97908 01678<br>
      LinkedIn: <a href="https://www.linkedin.com/in/vikas-s070106">linkedin.com/in/vikas-s070106</a> | GitHub: <a href="https://github.com/VIKASSAMVEL">github.com/VIKASSAMVEL</a> | Portfolio: <a href="https://vikas-world.vercel.app">vikas-world.vercel.app</a>
    </p>
  </div>
</div>
</body>
</html>
"""

    elif category == "panw_dataplane":
        subject = "EdgeModelScheduler v4.0: Sub-15ns lock-free C++17 runtime for wire-speed inline AI on Single-Pass Dataplanes"
        
        plain_text = f"""Hi {first_name},

I'm reaching out directly because I follow Palo Alto Networks' dataplane engineering and hardware platforms. Nir Zuk's Single-Pass Architecture set the global gold standard for separating control and data planes to achieve wire-speed deep packet inspection. As PAN-OS brings deep learning inline on Strata hardware appliances (e.g. PA-5400, PA-7000), I wanted to share a low-latency systems runtime I've been building that resolves the timing collision between network packet rings and neural inference.

I'm S. Vikas, a final-year B.Tech CSE student at Vel Tech University in Chennai (CGPA: 8.67/10). While most students around me focus on standard web applications, I've dedicated my college years to low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and high-throughput dataplanes. During my engineering R&D at Ashok Leyland and Vel Tech (with 3 patents filed across autonomous systems and real-time safety), I've focused on zero-heap deterministic execution.

Over the last several months, I built an open-source deterministic AI co-scheduler in C++17 (EdgeModelScheduler v4.0). I engineered it specifically to solve the architectural challenge of running deep learning inline without breaking line-rate guarantees:

ARCHITECTURAL BENCHMARK: GENERIC ML RUNTIME VS. EDGEMODELSCHEDULER v4.0
+--------------------------+--------------------------+---------------------------+----------------------------------------------+
| Dimension                | Generic ML on Hardware   | EdgeModelScheduler v4.0   | Single-Pass Dataplane Advantage              |
+--------------------------+--------------------------+---------------------------+----------------------------------------------+
| Dataplane Ingress Parity | Mutex lock contention    | Sub-15ns Lock-Free SPSC   | alignas(64) cache isolation; zero contention |
| Memory Transport Bridge  | CPU copy / socket I/O    | 12.23 GB/s Direct /dev/shm| Maps DPDK buffers directly to AI in <15 µs   |
| Traffic Burst Defense    | Tail latency spike (>70m)| P99 29.02 ms (0.00% drops)| Predictively sheds expired telemetry in ring |
| Saturation Storm Adapt   | Drops sessions / bypass  | Dynamic INT8 Governor     | Doubles throughput to sustain continuous DPI |
| Resident Core Footprint  | Hundreds of MBs          | 4.8 MB Resident Core      | Pure C++17; fits easily into on-box silicon  |
+--------------------------+--------------------------+---------------------------+----------------------------------------------+

CORE LOW-LEVEL SYSTEMS INNOVATIONS:
1. Sub-15ns Lock-Free Dataplane Parity: Custom Single-Producer Single-Consumer (SPSC) circular rings with head and tail atomics isolated on separate 64-byte cache lines (alignas(64)), eliminating lock contention and CPU cache-line bouncing across high-rate packet cores.
2. Zero-Copy Direct-Memory Transport (12.23 GB/s): Maps DPDK-style packet buffers directly to AI acceleration memory in <15 microseconds via 64-byte virtual handles over /dev/shm, eliminating CPU/PCIe serialization.
3. Predictive Jitter Shedding & Queue Defense: Under line-rate traffic bursts, the closed-loop scheduler predictively sheds stale intermediate telemetry, bounding P99 worst-case tail latency to 29.02 ms (vs. 73 ms+ queue runaway in unmanaged FIFO systems) and maintaining a 0.00% packet buffer drop rate.
4. Dynamic Precision Degradation: Closed-loop governor automatically degrades deep packet inspection models from FP16 to INT8 during saturation storms, halving memory bandwidth and doubling throughput to maintain continuous inline inspection.
5. High-Assurance Tamper-Proof Flight Recorder: Pre-allocated, zero-heap ring buffer (60,000 frames @ 1 kHz) logging inference decisions and thermals with rolling SHA-256 cryptographic signatures.
6. Ultra-Lean 4.8 MB Resident Core: Built without external framework dependencies, easily fitting into constrained on-box network processors.

I've attached my full resume (S_Vikas_Resume.pdf) and documented the complete architecture, cache-line profiling, and empirical benchmark data here:
- Attached Resume: S_Vikas_Resume.pdf (Vel Tech B.Tech CSE 8.67 CGPA, 3 Patents Filed, Ashok Leyland R&D)
- Technical Report (PDF): https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
- Live Bento Dashboard: https://vikassamvel.github.io/edge-model-scheduler/
- GitHub Repository: https://github.com/VIKASSAMVEL/edge-model-scheduler
- Personal Portfolio: https://vikas-world.vercel.app

I'm writing because as I prepare to graduate, my ambition is to work on low-latency systems and dataplane software at Palo Alto Networks. I would love 15 minutes of your time to introduce myself, or an introduction to an engineering lead on the Hardware Dataplane Engineering, Network Security Platforms, or PAN-OS core teams where an entry-level engineer with my low-level systems focus could contribute.

Thank you for your time and for building the world's most robust security dataplanes.

Thanks for your time,
S. Vikas
Systems Software & Edge AI Engineer
B.Tech Computer Science & Engineering (CGPA: 8.67 / 10), Vel Tech University, Chennai
vikassamvel123@gmail.com | +91 97908 01678
LinkedIn: https://www.linkedin.com/in/vikas-s070106
GitHub: https://github.com/VIKASSAMVEL
Portfolio: https://vikas-world.vercel.app
(Resume attached: S_Vikas_Resume.pdf)
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

  <p>I'm <strong>S. Vikas</strong>, a final-year B.Tech CSE student at Vel Tech University in Chennai (<strong>CGPA: 8.67 / 10</strong>). While most students around me focus on standard web applications, I've dedicated my college years to low-level systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and high-throughput dataplanes. During my engineering R&amp;D at <strong>Ashok Leyland</strong> and Vel Tech (with 3 patents filed across autonomous systems and real-time safety), I've focused on zero-heap deterministic execution.</p>

  <p>Over the last several months, I built an open-source deterministic AI co-scheduler in C++17 (<strong>EdgeModelScheduler v4.0</strong>). I engineered it specifically to solve the architectural challenge of running deep learning inline without breaking line-rate guarantees:</p>

  <div class="section-title">Architectural Benchmark: Incumbent vs. EdgeModelScheduler v4.0</div>
  <div class="table-box">
    <table class="comp-table">
      <thead>
        <tr>
          <th>Dimension</th>
          <th>Generic ML on Security Hardware</th>
          <th>EdgeModelScheduler v4.0</th>
          <th>Single-Pass Dataplane Impact</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><strong>Dataplane Ingress Parity</strong></td>
          <td class="metric-standard">Mutex locking &amp; cache-line bouncing</td>
          <td class="metric-highlight">Sub-15ns Lock-Free SPSC</td>
          <td><code>alignas(64)</code> cache-isolated atomics; zero contention with DPDK rings</td>
        </tr>
        <tr>
          <td><strong>Direct Memory Transport</strong></td>
          <td class="metric-standard">CPU memory copy &amp; socket I/O</td>
          <td class="metric-highlight">12.23 GB/s Direct <code>/dev/shm</code></td>
          <td>Maps packet buffers directly to AI acceleration memory in &lt;15 &mu;s</td>
        </tr>
        <tr>
          <td><strong>Line-Rate Surge Defense</strong></td>
          <td class="metric-standard">Queue tail blowout (&gt;70 ms)</td>
          <td class="metric-highlight">P99 29.02 ms (0.00% drops)</td>
          <td>Predictively sheds stale intermediate telemetry during packet bursts</td>
        </tr>
        <tr>
          <td><strong>Saturation Storm Adapt</strong></td>
          <td class="metric-standard">Drops valid sessions or bypasses DPI</td>
          <td class="metric-highlight">Dynamic INT8 Governor</td>
          <td>Automatically halves model precision to double throughput &amp; sustain DPI</td>
        </tr>
        <tr>
          <td><strong>Resident Core Footprint</strong></td>
          <td class="metric-standard">Hundreds of MBs (heavy runtimes)</td>
          <td class="metric-highlight">4.8 MB Resident Core</td>
          <td>Pure C++17; runs self-contained on on-box security processors</td>
        </tr>
      </tbody>
    </table>
  </div>

  <div class="section-title">Core Low-Level Systems Innovations:</div>
  <ul>
    <li><strong>Sub-15ns Lock-Free Dataplane Parity:</strong> Custom Single-Producer Single-Consumer circular rings with head and tail atomics isolated on separate 64-byte cache lines (<code>alignas(64)</code>), eliminating lock contention and CPU cache-line bouncing across high-rate packet cores.</li>
    <li><strong>Zero-Copy Direct-Memory Transport (12.23 GB/s):</strong> Maps DPDK-style packet buffers directly to AI acceleration memory in &lt;15 microseconds via 64-byte virtual handles over <code>/dev/shm</code>.</li>
    <li><strong>Predictive Jitter Shedding &amp; Queue Defense:</strong> Under line-rate traffic bursts, the closed-loop scheduler predictively sheds stale intermediate telemetry, bounding P99 worst-case tail latency to <strong>29.02 ms</strong> (vs. 73 ms+ queue runaway in unmanaged FIFO systems) and maintaining a <strong>0.00% packet buffer drop rate</strong>.</li>
    <li><strong>Dynamic Precision Degradation:</strong> Closed-loop governor automatically degrades deep packet inspection models from FP16 to INT8 during saturation storms, halving memory bandwidth and doubling throughput to maintain continuous inline inspection.</li>
    <li><strong>High-Assurance Tamper-Proof Flight Recorder:</strong> Pre-allocated, zero-heap ring buffer (60,000 frames @ 1 kHz) logging inference decisions and thermals with rolling SHA-256 cryptographic signatures.</li>
    <li><strong>Ultra-Lean 4.8 MB Resident Core:</strong> Built without external framework dependencies, easily fitting into constrained on-box network processors.</li>
  </ul>

  <div class="links-box">
    <div style="font-weight: 600; margin-bottom: 6px;">Verified Credentials &amp; Project Resources:</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 4px;"><strong>📎 Attached Resume:</strong> <code>S_Vikas_Resume.pdf</code> (Education, 3 Patents Filed, Ashok Leyland R&amp;D)</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Technical Report (PDF)</a> &mdash; 30-section architecture and benchmark documentation</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Bento Dashboard</a> &mdash; Interactive hardware topology and live simulator</li>
      <li style="margin-bottom: 4px;"><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">GitHub Repository</a> &mdash; Full C++17 source with 12 passing test suites</li>
      <li><a href="https://vikas-world.vercel.app">Personal Portfolio</a> &mdash; vikas-world.vercel.app</li>
    </ul>
  </div>

  <p>I'm writing because as I prepare to graduate, my ambition is to work on low-latency systems and dataplane software at Palo Alto Networks. I would love 15 minutes of your time to introduce myself, or an introduction to an engineering lead on the Hardware Dataplane Engineering, Network Security Platforms, or PAN-OS core teams where an entry-level engineer with my low-level systems focus could contribute.</p>

  <p>Thank you for your time and for building the world's most robust security dataplanes.</p>

  <div class="footer">
    <p style="margin: 0 0 2px 0; font-weight: 600; color: #0f172a;">S. Vikas</p>
    <p style="margin: 0 0 4px 0; color: #475569;">Systems Software &amp; Edge AI Engineer<br>B.Tech Computer Science &amp; Engineering (CGPA: 8.67 / 10), Vel Tech University, Chennai</p>
    <p style="margin: 0; font-size: 13px;">
      Email: <a href="mailto:vikassamvel123@gmail.com">vikassamvel123@gmail.com</a> | Phone: +91 97908 01678<br>
      LinkedIn: <a href="https://www.linkedin.com/in/vikas-s070106">linkedin.com/in/vikas-s070106</a> | GitHub: <a href="https://github.com/VIKASSAMVEL">github.com/VIKASSAMVEL</a> | Portfolio: <a href="https://vikas-world.vercel.app">vikas-world.vercel.app</a>
    </p>
  </div>
</div>
</body>
</html>
"""

    else: # panw_leadership
        subject = "EdgeModelScheduler v4.0: 4.8MB deterministic C++17 runtime for wire-speed inline AI security platforms"
        
        plain_text = f"""Hi {first_name},

I'm reaching out directly because I follow your leadership in transforming Palo Alto Networks through platformization and precision AI. Stopping zero-day threats inline before they enter an enterprise requires running complex deep learning models directly on high-speed firewall appliances. I wanted to share a low-latency systems runtime I've been building that addresses the performance and latency challenges of running neural networks inline at line rate.

I'm S. Vikas, a final-year B.Tech CSE student at Vel Tech University in Chennai (CGPA: 8.67/10). While most students around me focus on standard web applications, I've spent my college years going deep into low-level systems: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and deterministic network dataplanes. During my engineering R&D at Ashok Leyland and Vel Tech (with 3 patents filed across autonomous systems and real-time safety, and winning the national Agentic AI Hackathon 2025), I've focused on zero-heap real-time systems.

Over the last several months, I built an open-source deterministic AI co-scheduler in C++17 (EdgeModelScheduler v4.0). I engineered it specifically to solve the performance trade-offs of deploying deep learning inline on enterprise security hardware:

ARCHITECTURAL BENCHMARK: CONVENTIONAL DETECTION VS. EDGEMODELSCHEDULER v4.0
+--------------------------+--------------------------+---------------------------+----------------------------------------------+
| Dimension                | Conventional Detection   | EdgeModelScheduler v4.0   | Platformization & Precision AI Advantage     |
+--------------------------+--------------------------+---------------------------+----------------------------------------------+
| Threat Detection Window  | Multi-second cloud lag   | Wire-speed deterministic  | Stops zero-days before enterprise ingress    |
| Traffic Surge Defense    | Session drops / bypass   | Bounded P99 29.02 ms      | 0.00% packet buffer drop rate under DDoS     |
| Hardware Footprint       | Hundreds of MBs          | 4.8 MB Resident Core      | Pure C++17; runs self-contained on appliances|
| Forensic Auditability    | Post-incident disk logs  | Rolling SHA-256 binary    | 60,000 frames @ 1 kHz immutable flight trail |
+--------------------------+--------------------------+---------------------------+----------------------------------------------+

CORE LOW-LEVEL SYSTEMS INNOVATIONS:
1. Deterministic Queue Defense: Predictively sheds expired intermediate telemetry during line-rate traffic bursts, bounding P99 worst-case tail latency to 29.02 ms (vs. 73 ms+ queue runaway in unmanaged FIFO systems) with a 0.00% packet buffer drop rate.
2. Sub-15ns Lock-Free Dataplane Ring: Custom Single-Producer Single-Consumer (SPSC) circular rings with head and tail atomics isolated on separate 64-byte cache lines (alignas(64)), preventing CPU lock contention and cache-line bouncing.
3. Direct-Memory Transport (12.23 GB/s): Maps packet buffers directly to AI acceleration memory in <15 microseconds via 64-byte virtual handles over /dev/shm, eliminating memory copy overhead.
4. Dynamic Precision Governor: Automatically shifts deep packet inspection models from FP16 to INT8 during heavy traffic storms to double throughput and maintain continuous inline protection.
5. Immutable Forensic Flight Recorder: Pre-allocated circular telemetry buffer (60,000 frames @ 1 kHz) that logs every inline AI decision with rolling SHA-256 cryptographic signatures for compliance and incident response.
6. Ultra-Lean 4.8 MB Resident Footprint: Runs completely self-contained without heavy external dependencies.

I've attached my full resume (S_Vikas_Resume.pdf) and documented the complete architecture, dossier, and benchmarks here:
- Attached Resume: S_Vikas_Resume.pdf (Vel Tech B.Tech CSE 8.67 CGPA, 3 Patents Filed, Ashok Leyland R&D)
- Technical Report (PDF): https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
- Live Bento Dashboard: https://vikassamvel.github.io/edge-model-scheduler/
- GitHub Repository: https://github.com/VIKASSAMVEL/edge-model-scheduler
- Personal Portfolio: https://vikas-world.vercel.app

I'm writing because as I prepare to graduate, my ambition is to work on low-latency systems and AI infrastructure at Palo Alto Networks. I would be deeply grateful for a brief 15-minute conversation to introduce myself, or an introduction to an engineering lead on the Network Security Platforms or PAN-OS engineering teams where an entry-level engineer with my systems foundation could contribute.

Thank you for your time and leadership in shaping the future of cybersecurity.

Thanks for your time,
S. Vikas
Systems Software & Edge AI Engineer
B.Tech Computer Science & Engineering (CGPA: 8.67 / 10), Vel Tech University, Chennai
vikassamvel123@gmail.com | +91 97908 01678
LinkedIn: https://www.linkedin.com/in/vikas-s070106
GitHub: https://github.com/VIKASSAMVEL
Portfolio: https://vikas-world.vercel.app
(Resume attached: S_Vikas_Resume.pdf)
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

  <p>I'm <strong>S. Vikas</strong>, a final-year B.Tech CSE student at Vel Tech University in Chennai (<strong>CGPA: 8.67 / 10</strong>). While most students around me focus on standard web applications, I've spent my college years going deep into low-level systems: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and deterministic network dataplanes. During my engineering R&amp;D at <strong>Ashok Leyland</strong> and Vel Tech (with 3 patents filed across autonomous systems and real-time safety, and winning the national Agentic AI Hackathon 2025), I've focused on zero-heap real-time systems.</p>

  <p>Over the last several months, I built an open-source deterministic AI co-scheduler in C++17 (<strong>EdgeModelScheduler v4.0</strong>). I engineered it specifically to solve the performance trade-offs of deploying deep learning inline on enterprise security hardware:</p>

  <div class="section-title">Architectural Benchmark: Incumbent vs. EdgeModelScheduler v4.0</div>
  <div class="table-box">
    <table class="comp-table">
      <thead>
        <tr>
          <th>Dimension</th>
          <th>Conventional Detection</th>
          <th>EdgeModelScheduler v4.0</th>
          <th>Platformization &amp; Precision AI Impact</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><strong>Threat Detection Window</strong></td>
          <td class="metric-standard">Multi-second cloud / out-of-band lag</td>
          <td class="metric-highlight">Deterministic Wire-Speed Inline</td>
          <td>Stops zero-days before enterprise ingress without creating traffic latency bottlenecks</td>
        </tr>
        <tr>
          <td><strong>Traffic Surge Defense</strong></td>
          <td class="metric-standard">Session drops or inspection bypass</td>
          <td class="metric-highlight">0.00% Packet Drop Rate</td>
          <td>Predictive queue defense bounds P99 worst-case tail latency to <strong>29.02 ms</strong></td>
        </tr>
        <tr>
          <td><strong>Appliance Resource Draw</strong></td>
          <td class="metric-standard">Hundreds of MBs &amp; heavy runtime</td>
          <td class="metric-highlight">4.8 MB Resident Core</td>
          <td>Pure C++17; runs completely self-contained on enterprise hardware firewalls</td>
        </tr>
        <tr>
          <td><strong>Forensic Compliance</strong></td>
          <td class="metric-standard">Post-incident disk logs</td>
          <td class="metric-highlight">Rolling SHA-256 Flight Recorder</td>
          <td>Pre-allocated ring buffer (60,000 frames @ 1 kHz) with cryptographic immutability</td>
        </tr>
      </tbody>
    </table>
  </div>

  <div class="section-title">Core Low-Level Systems Innovations:</div>
  <ul>
    <li><strong>Deterministic Queue Defense:</strong> Predictively sheds expired intermediate telemetry during line-rate traffic bursts, bounding P99 worst-case tail latency to <strong>29.02 ms</strong> (vs. 73 ms+ queue runaway in unmanaged FIFO systems) with a <strong>0.00% packet buffer drop rate</strong>.</li>
    <li><strong>Sub-15ns Lock-Free Dataplane Ring:</strong> Custom Single-Producer Single-Consumer circular rings with head and tail atomics isolated on separate 64-byte cache lines (<code>alignas(64)</code>), preventing CPU lock contention and cache-line bouncing.</li>
    <li><strong>Direct-Memory Transport (12.23 GB/s):</strong> Maps packet buffers directly to AI acceleration memory in &lt;15 microseconds via 64-byte virtual handles over <code>/dev/shm</code>, eliminating memory copy overhead.</li>
    <li><strong>Dynamic Precision Governor:</strong> Automatically shifts deep packet inspection models from FP16 to INT8 during heavy traffic storms to double throughput and maintain continuous inline protection.</li>
    <li><strong>Immutable Forensic Flight Recorder:</strong> Pre-allocated circular telemetry buffer (60,000 frames @ 1 kHz) that logs every inline AI decision with rolling SHA-256 cryptographic signatures for compliance and incident response.</li>
    <li><strong>Ultra-Lean 4.8 MB Resident Footprint:</strong> Runs completely self-contained without heavy external dependencies.</li>
  </ul>

  <div class="links-box">
    <div style="font-weight: 600; margin-bottom: 6px;">Verified Credentials &amp; Project Resources:</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 4px;"><strong>📎 Attached Resume:</strong> <code>S_Vikas_Resume.pdf</code> (Education, 3 Patents Filed, Ashok Leyland R&amp;D)</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Technical Report (PDF)</a> &mdash; 30-section architecture and benchmark documentation</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Bento Dashboard</a> &mdash; Interactive hardware topology and live simulator</li>
      <li style="margin-bottom: 4px;"><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">GitHub Repository</a> &mdash; Full C++17 source with 12 passing test suites</li>
      <li><a href="https://vikas-world.vercel.app">Personal Portfolio</a> &mdash; vikas-world.vercel.app</li>
    </ul>
  </div>

  <p>I'm writing because as I prepare to graduate, my ambition is to work on low-latency systems and AI infrastructure at Palo Alto Networks. I would be deeply grateful for a brief 15-minute conversation to introduce myself, or an introduction to an engineering lead on the Network Security Platforms or PAN-OS engineering teams where an entry-level engineer with my systems foundation could contribute.</p>

  <p>Thank you for your time and leadership in shaping the future of cybersecurity.</p>

  <div class="footer">
    <p style="margin: 0 0 2px 0; font-weight: 600; color: #0f172a;">S. Vikas</p>
    <p style="margin: 0 0 4px 0; color: #475569;">Systems Software &amp; Edge AI Engineer<br>B.Tech Computer Science &amp; Engineering (CGPA: 8.67 / 10), Vel Tech University, Chennai</p>
    <p style="margin: 0; font-size: 13px;">
      Email: <a href="mailto:vikassamvel123@gmail.com">vikassamvel123@gmail.com</a> | Phone: +91 97908 01678<br>
      LinkedIn: <a href="https://www.linkedin.com/in/vikas-s070106">linkedin.com/in/vikas-s070106</a> | GitHub: <a href="https://github.com/VIKASSAMVEL">github.com/VIKASSAMVEL</a> | Portfolio: <a href="https://vikas-world.vercel.app">vikas-world.vercel.app</a>
    </p>
  </div>
</div>
</body>
</html>
"""

    return subject, plain_text, html_body

def send_single_mail(server, sender_email, recipient_email, subject, plain_text, html_body, attachment_path=RESUME_PATH, attachment_name="S_Vikas_Resume.pdf"):
    # Outer multipart/mixed container for alternative body and file attachments
    msg = MIMEMultipart('mixed')
    msg['From'] = f"S. Vikas <{sender_email}>"
    msg['To'] = recipient_email
    msg['Subject'] = subject

    # Inner multipart/alternative container for plain-text and HTML
    body_container = MIMEMultipart('alternative')
    part1 = MIMEText(plain_text, 'plain', 'utf-8')
    part2 = MIMEText(html_body, 'html', 'utf-8')
    body_container.attach(part1)
    body_container.attach(part2)
    msg.attach(body_container)

    # Attach verified resume PDF
    if attachment_path and os.path.exists(attachment_path):
        with open(attachment_path, "rb") as f:
            pdf_data = f.read()
        part_attach = MIMEApplication(pdf_data, _subtype="pdf")
        part_attach.add_header('Content-Disposition', 'attachment', filename=attachment_name)
        msg.attach(part_attach)
    else:
        print(f"⚠️  Warning: Resume attachment not found at {attachment_path}")

    server.sendmail(sender_email, recipient_email, msg.as_string())

def main():
    creds = load_credentials()
    sender_email = creds["email"]
    app_password = creds["app_password"].replace(" ", "")

    mode = sys.argv[1] if len(sys.argv) > 1 else "--dry-run"

    print("=================================================================")
    print("🚀 EdgeModelScheduler Executive Outreach Dispatcher (Option D)")
    print(f"   Sender     : {sender_email}")
    print(f"   Resume PDF : {RESUME_PATH} (exists: {os.path.exists(RESUME_PATH)})")
    print(f"   Attachment : S_Vikas_Resume.pdf")
    print(f"   Mode       : {mode.upper()}")
    print("=================================================================\n")

    if mode == "--dry-run":
        print(f"Total Targets: {len(RECIPIENTS)}\n")
        for idx, target in enumerate(RECIPIENTS, 1):
            subj, plain, html = generate_email_content(target)
            print(f"-----------------------------------------------------------------")
            print(f"[{idx}/{len(RECIPIENTS)}] TO: {target['name']} <{target['email']}>")
            print(f"TITLE: {target['title']} | {target['company']}")
            print(f"SUBJECT: {subj}")
            print(f"ATTACHMENT: S_Vikas_Resume.pdf (verified: {os.path.exists(RESUME_PATH)})")
            print(f"-----------------------------------------------------------------")
            print(plain[:320] + "...\n[Full body ready to send with benchmark table and resume attached]\n")
        print("Dry run completed. No emails were sent.")
        print("To send a sample verification mail, run:")
        print("  python outreach/send_outreach.py --to vtu24383@veltech.edu.in\n")
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
        print(f"✅ Test email successfully sent to {sender_email} with resume attached (S_Vikas_Resume.pdf)! Check your inbox.")

    elif mode == "--to":
        target_addr = sys.argv[2] if len(sys.argv) > 2 else "vtu24383@veltech.edu.in"
        print(f"Sending sample verification email to: {target_addr}...")
        context = ssl.create_default_context()
        with smtplib.SMTP_SSL("smtp.gmail.com", 465, context=context) as server:
            server.login(sender_email, app_password)
            subj, plain, html = generate_email_content(RECIPIENTS[0])
            sample_subj = f"[SAMPLE OPTION-D VERIFICATION] {subj}"
            send_single_mail(server, sender_email, target_addr, sample_subj, plain, html)
        print(f"✅ Sample Option-D email successfully sent to {target_addr} with resume attached (S_Vikas_Resume.pdf)!")

    elif mode == "--send":
        print("⚠️  CONFIRMED SEND MODE: Dispatching individually to each recipient...")
        context = ssl.create_default_context()
        with smtplib.SMTP_SSL("smtp.gmail.com", 465, context=context) as server:
            server.login(sender_email, app_password)
            for idx, target in enumerate(RECIPIENTS, 1):
                subj, plain, html = generate_email_content(target)
                print(f"[{idx}/{len(RECIPIENTS)}] Sending to {target['name']} <{target['email']}>...")
                send_single_mail(server, sender_email, target["email"], subj, plain, html)
                print(f"    ✅ Delivered to {target['email']} (with S_Vikas_Resume.pdf attached)")
                if idx < len(RECIPIENTS):
                    print("    Pausing 4s to adhere to SMTP rate limits...")
                    time.sleep(4)
        print("\n🎉 All 9 individualized executive emails dispatched successfully!")

    else:
        print(f"Unknown mode: {mode}. Use --dry-run, --test-me, --to <email>, or --send.")

if __name__ == "__main__":
    main()
