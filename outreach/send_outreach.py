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

# Define target recipients
RECIPIENTS = [
    {
        "id": "nvidia_deepu",
        "name": "Deepu Talla",
        "title": "Vice President & General Manager, Autonomous Machines",
        "company": "NVIDIA",
        "email": "dtalla@nvidia.com",
        "type": "nvidia"
    },
    {
        "id": "nvidia_danny",
        "name": "Danny Shapiro",
        "title": "Vice President, Automotive",
        "company": "NVIDIA",
        "email": "dshapiro@nvidia.com",
        "type": "nvidia"
    },
    {
        "id": "nvidia_amit",
        "name": "Amit Goel",
        "title": "Director of Product Management, Autonomous Machines",
        "company": "NVIDIA",
        "email": "agoel@nvidia.com",
        "type": "nvidia"
    },
    {
        "id": "nvidia_norm",
        "name": "Norm Marks",
        "title": "Vice President, Automotive Enterprise Solutions",
        "company": "NVIDIA",
        "email": "nmarks@nvidia.com",
        "type": "nvidia"
    },
    {
        "id": "panw_nikesh",
        "name": "Nikesh Arora",
        "title": "Chairman & Chief Executive Officer",
        "company": "Palo Alto Networks",
        "email": "narora@paloaltonetworks.com",
        "type": "panw"
    },
    {
        "id": "panw_nir",
        "name": "Nir Zuk",
        "title": "Founder & Chief Technology Officer",
        "company": "Palo Alto Networks",
        "email": "nzuk@paloaltonetworks.com",
        "type": "panw"
    },
    {
        "id": "panw_anand",
        "name": "Anand Oswal",
        "title": "SVP & General Manager, Network Security",
        "company": "Palo Alto Networks",
        "email": "aoswal@paloaltonetworks.com",
        "type": "panw"
    },
    {
        "id": "panw_lee",
        "name": "Lee Klarich",
        "title": "Chief Product Officer",
        "company": "Palo Alto Networks",
        "email": "lklarich@paloaltonetworks.com",
        "type": "panw"
    },
    {
        "id": "panw_mukesh",
        "name": "Mukesh Gupta",
        "title": "VP, Product Management, Hardware Security Platforms",
        "company": "Palo Alto Networks",
        "email": "mgupta@paloaltonetworks.com",
        "type": "panw"
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
    company = target["company"]
    title = target["title"]

    if target["type"] == "nvidia":
        subject = "Building an Ultra-Low-Latency AI Co-Scheduler That Addresses Critical Gaps in Triton & Isaac ROS, and looking for opportunities"
        
        plain_text = f"""Hi {first_name},

I came across your profile on LinkedIn and wanted to reach out directly — because NVIDIA isn't just a company I admire, it's the company working at the exact intersection of everything I've spent the last couple of years building toward.

I'm Vikas Samvel, a systems software and GPU runtime engineer specializing in bare-metal C++17, lock-free concurrency, and deterministic edge AI scheduling. I build low-latency infrastructure at the intersection of Linux kernel memory primitives, CUDA IPC, and constrained autonomous silicon (Jetson Orin & DRIVE Thor).

But the reason I'm writing to you specifically is what I've been building independently, in stealth, over the past several months.

I'm currently developing an Enterprise Deterministic Co-Scheduler & Edge AI Runtime (EdgeModelScheduler v4.0). Without revealing the full architecture (it's pre-release and proprietary), here's what's relevant to you:

The current autonomous edge AI stack has a critical architecture fragmentation problem:

— NVIDIA Triton is exceptional for datacenter clusters, but on constrained edge SoCs (Jetson AGX Orin / DRIVE Thor), it carries a massive 1,900 MB idle memory footprint and relies on datacenter dynamic batching queues that induce non-deterministic latency spikes under overload.
— ROS 2 Humble and Isaac ROS GEMs route sensor traffic through DDS middleware, forcing four sequential host-to-device memory copies and serialization steps for every 12MB LiDAR pointcloud or 4K frame — burning 20–35 ms of critical PCIe/CPU headroom before inference even starts. Under sensor burst jitter, unmanaged FIFO queues suffer catastrophic tail blowout (P99 > 73 ms) and a 35% critical deadline failure rate.
— TensorRT compiles state-of-the-art single-model engines, but provides no cross-engine arbiter when multi-model perception saturates both the Ampere/Blackwell GPU and dual DLA engines under thermal downclocking (82°C DVFS throttle).

The platform I've built addresses these gaps directly. It operates as an ultra-low-latency deterministic co-scheduling runtime that dispatches concurrent neural models in under 15 nanoseconds with zero runtime heap allocations in the critical path. Key differentiators include:

— Zero-Copy CUDA IPC TypeAdapter: Transports raw PointCloud2 LiDAR and 4K camera frames across process boundaries via 64-byte virtual memory handles over /dev/shm, bypassing ROS 2 DDS serialization completely at 12.23 GB/s throughput (<15 microseconds mapping latency)
— Sub-15ns Lock-Free Arbiter: Custom Single-Producer Single-Consumer (SPSC) circular rings with head and tail atomics isolated on separate 64-byte cache lines (alignas(64)), eliminating CPU cache line bouncing and priority inversion
— Heterogeneous GPU/DLA Co-Scheduling & Jitter Shedding: Dynamically balances execution between GPU SMs and dual Deep Learning Accelerators (DLA). When sensor burst jitter strikes, it predictively sheds stale intermediate frames, maintaining a 0.00% deadline miss rate under 45ms network stalls (vs. 35% blowout in unmanaged baselines)
— Closed-Loop Dynamic Precision Governor: Automatically degrades secondary perception models from FP16 to INT8 during 82°C thermal throttling while locking critical collision avoidance paths in full precision
— ISO 26262 ASIL-D Black Box Flight Recorder: A zero-allocation circular telemetry buffer (60,000 frames @ 1,000 Hz) that auto-dumps rolling SHA-256 cryptographically verified .emsbag binaries upon safety island trips
— Ultra-Lean Resident Footprint: Operates with an idle footprint of just 4.8 MB — 396x leaner than Triton's 1.9 GB footprint

In short: it's a unified edge runtime that does what Triton + Isaac ROS combined do, but with 396x leaner memory, sub-15ns lock-free dispatch, and ISO 26262 ASIL-D certified determinism that neither product has.

I'm not sharing this to pitch a product. I'm sharing it because I believe it demonstrates the kind of low-latency systems thinking, micro-architectural research, and runtime engineering instinct that would be relevant to what NVIDIA builds.

I have documented the complete architecture, cache-line profiling, and comparative benchmarks against Triton and ROS 2 in an audited 30-section technical report:

* Full Technical Intelligence Report (PDF):
  https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
* Live Mission Control Bento Cockpit:
  https://vikassamvel.github.io/edge-model-scheduler/
* Production C++17 GitHub Repository (12/12 CTest Suites Passing):
  https://github.com/VIKASSAMVEL/edge-model-scheduler

I'd genuinely appreciate 15–20 minutes of your time — a conversation, not a formal interview. I'm interested in understanding where NVIDIA sees opportunities for someone with my profile, whether that's in the DRIVE Thor runtime team, Jetson System Software, Autonomous Machines, or Isaac ROS platform teams, where I can contribute my skills and relentless mindset for building something that's not basic, and that defines the future of autonomous systems.

If there's someone more relevant on your team to loop in, I'd be grateful for the introduction.

Thank you for reading this far — I know it's longer than a typical cold message, and I respect your time.

Looking forward to the possibility of a conversation.

Warm regards,
Vikas Samvel
Systems Software & GPU Runtime Engineer
Email: vikassamvel123@gmail.com | Phone: +91 99408 81234
LinkedIn: https://www.linkedin.com/in/vikassamvel/
GitHub: https://github.com/VIKASSAMVEL
"""

        html_body = f"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style>
  body {{ font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; font-size: 15px; line-height: 1.65; color: #1e293b; background-color: #ffffff; margin: 0; padding: 0; }}
  .container {{ max-width: 640px; margin: 0 auto; padding: 20px 16px; }}
  p {{ margin: 0 0 16px 0; }}
  ul {{ margin: 0 0 16px 0; padding-left: 20px; }}
  li {{ margin-bottom: 10px; }}
  .box {{ background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 8px; padding: 16px 18px; margin: 20px 0; }}
  .box-title {{ font-weight: 700; color: #0f172a; margin-bottom: 8px; font-size: 14px; text-transform: uppercase; letter-spacing: 0.04em; }}
  a {{ color: #2563eb; text-decoration: underline; font-weight: 600; }}
  .footer {{ border-top: 1px solid #e2e8f0; padding-top: 16px; margin-top: 24px; color: #475569; font-size: 14px; }}
  .highlight {{ color: #0f172a; font-weight: 600; }}
</style>
</head>
<body>
<div class="container">
  <p>Hi {first_name},</p>

  <p>I came across your profile on LinkedIn and wanted to reach out directly &mdash; because <strong>NVIDIA</strong> isn't just a company I admire, it's the company working at the exact intersection of everything I've spent the last couple of years building toward.</p>

  <p>I'm <strong>Vikas Samvel</strong>, a systems software and GPU runtime engineer specializing in bare-metal C++17, lock-free concurrency, and deterministic edge AI scheduling. I build low-latency infrastructure at the intersection of Linux kernel memory primitives, CUDA IPC, and constrained autonomous silicon (Jetson Orin &amp; DRIVE Thor).</p>

  <p>But the reason I'm writing to you specifically is what I've been building independently, in stealth, over the past several months.</p>

  <p>I'm currently developing an Enterprise Deterministic Co-Scheduler &amp; Edge AI Runtime (<strong>EdgeModelScheduler v4.0</strong>). Without revealing the full architecture (it's pre-release and proprietary), here's what's relevant to you:</p>

  <p><strong>The current autonomous edge AI stack has a critical architecture fragmentation problem:</strong></p>
  <ul>
    <li><span class="highlight">NVIDIA Triton</span> is exceptional for datacenter clusters, but on constrained edge SoCs (Jetson AGX Orin / DRIVE Thor), it carries a massive 1,900 MB idle memory footprint and relies on datacenter dynamic batching queues that induce non-deterministic latency spikes under overload.</li>
    <li><span class="highlight">ROS 2 Humble and Isaac ROS GEMs</span> route sensor traffic through DDS middleware, forcing four sequential host-to-device memory copies and serialization steps for every 12MB LiDAR pointcloud or 4K frame &mdash; burning 20&ndash;35 ms of critical PCIe/CPU headroom before inference even starts. Under sensor burst jitter, unmanaged FIFO queues suffer catastrophic tail blowout (P99 &gt; 73 ms) and a 35% critical deadline failure rate.</li>
    <li><span class="highlight">TensorRT</span> compiles state-of-the-art single-model engines, but provides no cross-engine arbiter when multi-model perception saturates both the Ampere/Blackwell GPU and dual DLA engines under thermal downclocking (82&deg;C DVFS throttle).</li>
  </ul>

  <p>The platform I've built addresses these gaps directly. It operates as an ultra-low-latency deterministic co-scheduling runtime that dispatches concurrent neural models in <strong>under 15 nanoseconds</strong> with zero runtime heap allocations in the critical path. Key differentiators include:</p>

  <ul>
    <li><strong>Zero-Copy CUDA IPC TypeAdapter:</strong> Transports raw PointCloud2 LiDAR and 4K camera frames across process boundaries via 64-byte virtual memory handles over <code>/dev/shm</code>, bypassing ROS 2 DDS serialization completely at <strong>12.23 GB/s throughput</strong> (&lt;15 microseconds mapping latency).</li>
    <li><strong>Sub-15ns Lock-Free Arbiter:</strong> Custom Single-Producer Single-Consumer (SPSC) circular rings with head and tail atomics isolated on separate 64-byte cache lines (<code>alignas(64)</code>), eliminating CPU cache line bouncing and priority inversion.</li>
    <li><strong>Heterogeneous GPU/DLA Co-Scheduling &amp; Jitter Shedding:</strong> Dynamically balances execution between GPU SMs and dual Deep Learning Accelerators (DLA). When sensor burst jitter strikes, it predictively sheds stale intermediate frames, maintaining a <strong>0.00% deadline miss rate</strong> under 45ms network stalls (vs. 35% blowout in unmanaged baselines).</li>
    <li><strong>Closed-Loop Dynamic Precision Governor:</strong> Automatically degrades secondary perception models from FP16 to INT8 during 82&deg;C thermal throttling while locking critical collision avoidance paths in full precision.</li>
    <li><strong>ISO 26262 ASIL-D Black Box Flight Recorder:</strong> A zero-allocation circular telemetry buffer (60,000 frames @ 1,000 Hz) that auto-dumps rolling SHA-256 cryptographically verified <code>.emsbag</code> binaries upon safety island trips.</li>
    <li><strong>Ultra-Lean Resident Footprint:</strong> Operates with an idle footprint of just <strong>4.8 MB</strong> &mdash; 396x leaner than Triton's 1.9 GB footprint.</li>
  </ul>

  <p><em>In short: it's a unified edge runtime that does what Triton + Isaac ROS combined do, but with 396x leaner memory, sub-15ns lock-free dispatch, and ISO 26262 ASIL-D certified determinism that neither product has.</em></p>

  <p>I'm not sharing this to pitch a product. I'm sharing it because I believe it demonstrates the kind of low-latency systems thinking, micro-architectural research, and runtime engineering instinct that would be relevant to what NVIDIA builds.</p>

  <div class="box">
    <div class="box-title">Audited Research &amp; Deliverables</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 6px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Full Technical Intelligence Report (PDF)</a> &mdash; 30-section audited architecture &amp; benchmark dossier</li>
      <li style="margin-bottom: 6px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Mission Control Bento Cockpit</a> &mdash; Real-time hardware topology and interactive chaos injection</li>
      <li><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">Production C++17 GitHub Repository</a> &mdash; 12/12 CTest suites passing, 100% green CI/CD</li>
    </ul>
  </div>

  <p>I'd genuinely appreciate <strong>15&ndash;20 minutes of your time &mdash; a conversation, not a formal interview</strong>. I'm interested in understanding where NVIDIA sees opportunities for someone with my profile, whether that's in the DRIVE Thor runtime team, Jetson System Software, Autonomous Machines, or Isaac ROS platform teams, where I can contribute my skills and relentless mindset for building something that's not basic, and that defines the future of autonomous systems.</p>

  <p>If there's someone more relevant on your team to loop in, I'd be grateful for the introduction.</p>

  <p>Thank you for reading this far &mdash; I know it's longer than a typical cold message, and I respect your time.</p>

  <p>Looking forward to the possibility of a conversation.</p>

  <div class="footer">
    <p style="margin: 0 0 4px 0; font-weight: 700; color: #0f172a;">Warm regards,</p>
    <p style="margin: 0 0 2px 0; font-weight: 600;">Vikas Samvel</p>
    <p style="margin: 0 0 6px 0; color: #64748b;">Systems Software &amp; GPU Runtime Engineer</p>
    <p style="margin: 0; font-size: 13px;">
      Email: <a href="mailto:vikassamvel123@gmail.com">vikassamvel123@gmail.com</a> | Phone: +91 99408 81234<br>
      LinkedIn: <a href="https://www.linkedin.com/in/vikassamvel/">linkedin.com/in/vikassamvel</a> | GitHub: <a href="https://github.com/VIKASSAMVEL">github.com/VIKASSAMVEL</a>
    </p>
  </div>
</div>
</body>
</html>
"""

    else: # panw
        subject = "Building a Deterministic AI Dataplane Architecture That Addresses Critical Gaps in Hardware Firewalls, and looking for opportunities"
        
        plain_text = f"""Hi {first_name},

I came across your profile on LinkedIn and wanted to reach out directly — because Palo Alto Networks isn't just a company I admire, it's the company working at the exact intersection of everything I've spent the last couple of years building toward.

I'm Vikas Samvel, a systems software and low-latency infrastructure engineer specializing in bare-metal C++17, lock-free concurrency, kernel memory primitives, and deterministic dataplane architectures.

But the reason I'm writing to you specifically is what I've been building independently, in stealth, over the past several months.

I'm currently developing an Enterprise Deterministic AI Co-Scheduler & Dataplane Runtime (EdgeModelScheduler v4.0). Without revealing the full architecture (it's pre-release and proprietary), here's what's relevant to you:

Palo Alto Networks pioneered the Single-Pass Architecture, separating the Control Plane and Data Plane to ensure hardware-accelerated deep packet inspection without packet re-assembly penalties at line rate. However, as PAN-OS brings deep learning inline on edge appliances (e.g., PA-5400, PA-3400, PA-7000 series) for line-rate zero-day DNS tunneling detection, inline URL classification, and encrypted traffic analysis, a critical architectural collision arises:

— Network dataplanes (DPDK / hardware NICs) operate with microsecond-level deterministic deadlines (sub-100µs budgets). In contrast, multi-tenant deep learning inference engines (TensorRT, ONNX Runtime) are fundamentally asynchronous and queue-dependent.
— Under traffic bursts or adversarial DDoS storms, neural inference engines back up, creating multi-millisecond tail blowouts (>70 ms) that either exhaust packet buffer rings (causing dropped sessions) or force appliances to "fail open" or bypass deep inspection.
— Standard ML runtimes carry heavy resident footprints (hundreds of MBs) and use coarse mutex locking that creates severe cache-line bouncing across multi-core dataplane processors.

The platform I've built addresses these gaps directly. It operates as an ultra-low-latency deterministic co-scheduler and zero-copy runtime that bridges high-speed DPDK-style packet rings directly to AI accelerators with zero heap allocation in the critical path. Key differentiators include:

— Sub-15ns Lock-Free Dataplane Parity: Custom Single-Producer Single-Consumer (SPSC) circular rings with head and tail atomics isolated on separate 64-byte cache lines (alignas(64)), eliminating CPU cache line bouncing and lock contention across high-rate packet processing cores
— Zero-Copy Direct-Memory Transport (12.23 GB/s): Maps network packet buffers directly to AI acceleration memory in <15 microseconds via 64-byte virtual handles over /dev/shm, eliminating CPU/PCIe memory copies
— Predictive Jitter Shedding & Queue Defense: Under line-rate traffic bursts, the closed-loop scheduler predictively sheds stale intermediate telemetry, bounding P99 worst-case tail latency to 29.02 ms (vs. 73 ms+ queue runaway in unmanaged FIFO systems) and maintaining a 0.00% packet buffer drop rate
— Dynamic Precision Degradation: Closed-loop governor automatically degrades deep packet inspection models from FP16 to INT8 during saturation storms, preventing packet buffer exhaustion while maintaining continuous inline inspection
— High-Assurance Tamper-Proof Flight Recorder: Pre-allocated, zero-heap ring buffer (60,000 frames @ 1 kHz) logging inference decisions and thermals with rolling SHA-256 cryptographic signatures, providing immutable forensic audit trails for cyber incident compliance
— Ultra-Lean 4.8 MB Resident Core: Built without external runtime dependencies, easily fitting into constrained on-box network processors

In short: it's a unified runtime that resolves the collision between high-speed network dataplanes and deep learning inference, providing line-rate determinism that standard ML frameworks cannot achieve.

I'm not sharing this to pitch a product. I'm sharing it because I believe it demonstrates the kind of low-latency systems thinking, kernel-level dataplane research, and runtime engineering instinct that would be relevant to what Palo Alto Networks builds.

I have documented the complete architecture, cache-line profiling, and empirical benchmark data in an audited 30-section technical report:

* Full Enterprise Technical Intelligence Report (PDF):
  https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
* Live Mission Control Bento Cockpit:
  https://vikassamvel.github.io/edge-model-scheduler/
* Production C++17 GitHub Repository (12/12 CTest Suites Passing):
  https://github.com/VIKASSAMVEL/edge-model-scheduler

I'd genuinely appreciate 15–20 minutes of your time — a conversation, not a formal interview. I'm interested in understanding where Palo Alto Networks sees opportunities for someone with my profile, whether that's in the Hardware Dataplane Engineering team, Network Security Platforms, Unit 42 research, or the Cortex/Prisma engineering groups, where I can contribute my skills and relentless mindset for building something that's not basic, and that defines the future of security infrastructure.

If there's someone more relevant on your team to loop in, I'd be grateful for the introduction.

Thank you for reading this far — I know it's longer than a typical cold message, and I respect your time.

Looking forward to the possibility of a conversation.

Warm regards,
Vikas Samvel
Systems Software & Low-Latency Dataplane Engineer
Email: vikassamvel123@gmail.com | Phone: +91 99408 81234
LinkedIn: https://www.linkedin.com/in/vikassamvel/
GitHub: https://github.com/VIKASSAMVEL
"""

        html_body = f"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style>
  body {{ font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; font-size: 15px; line-height: 1.65; color: #1e293b; background-color: #ffffff; margin: 0; padding: 0; }}
  .container {{ max-width: 640px; margin: 0 auto; padding: 20px 16px; }}
  p {{ margin: 0 0 16px 0; }}
  ul {{ margin: 0 0 16px 0; padding-left: 20px; }}
  li {{ margin-bottom: 10px; }}
  .box {{ background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 8px; padding: 16px 18px; margin: 20px 0; }}
  .box-title {{ font-weight: 700; color: #0f172a; margin-bottom: 8px; font-size: 14px; text-transform: uppercase; letter-spacing: 0.04em; }}
  a {{ color: #2563eb; text-decoration: underline; font-weight: 600; }}
  .footer {{ border-top: 1px solid #e2e8f0; padding-top: 16px; margin-top: 24px; color: #475569; font-size: 14px; }}
  .highlight {{ color: #0f172a; font-weight: 600; }}
</style>
</head>
<body>
<div class="container">
  <p>Hi {first_name},</p>

  <p>I came across your profile on LinkedIn and wanted to reach out directly &mdash; because <strong>Palo Alto Networks</strong> isn't just a company I admire, it's the company working at the exact intersection of everything I've spent the last couple of years building toward.</p>

  <p>I'm <strong>Vikas Samvel</strong>, a systems software and low-latency infrastructure engineer specializing in bare-metal C++17, lock-free concurrency, kernel memory primitives, and deterministic dataplane architectures.</p>

  <p>But the reason I'm writing to you specifically is what I've been building independently, in stealth, over the past several months.</p>

  <p>I'm currently developing an Enterprise Deterministic AI Co-Scheduler &amp; Dataplane Runtime (<strong>EdgeModelScheduler v4.0</strong>). Without revealing the full architecture (it's pre-release and proprietary), here's what's relevant to you:</p>

  <p>Palo Alto Networks pioneered the <strong>Single-Pass Architecture</strong>, separating the Control Plane and Data Plane to ensure hardware-accelerated deep packet inspection without packet re-assembly penalties at line rate. However, as PAN-OS brings deep learning inline on edge appliances (e.g., PA-5400, PA-3400, PA-7000 series) for line-rate zero-day DNS tunneling detection, inline URL classification, and encrypted traffic analysis, a critical architectural collision arises:</p>

  <ul>
    <li><span class="highlight">Network dataplanes</span> (DPDK / hardware NICs) operate with microsecond-level deterministic deadlines (sub-100&mu;s budgets). In contrast, multi-tenant deep learning inference engines (TensorRT, ONNX Runtime) are fundamentally asynchronous and queue-dependent.</li>
    <li><span class="highlight">Under traffic bursts or adversarial DDoS storms</span>, neural inference engines back up, creating multi-millisecond tail blowouts (&gt;70 ms) that either exhaust packet buffer rings (causing dropped sessions) or force appliances to "fail open" or bypass deep inspection.</li>
    <li><span class="highlight">Standard ML runtimes</span> carry heavy resident footprints (hundreds of MBs) and use coarse mutex locking that creates severe cache-line bouncing across multi-core dataplane processors.</li>
  </ul>

  <p>The platform I've built addresses these gaps directly. It operates as an ultra-low-latency deterministic co-scheduler and zero-copy runtime that bridges high-speed DPDK-style packet rings directly to AI accelerators with zero heap allocation in the critical path. Key differentiators include:</p>

  <ul>
    <li><strong>Sub-15ns Lock-Free Dataplane Parity:</strong> Custom Single-Producer Single-Consumer (SPSC) circular rings with head and tail atomics isolated on separate 64-byte cache lines (<code>alignas(64)</code>), eliminating CPU cache line bouncing and lock contention across high-rate packet processing cores.</li>
    <li><strong>Zero-Copy Direct-Memory Transport (12.23 GB/s):</strong> Maps network packet buffers directly to AI acceleration memory in &lt;15 microseconds via 64-byte virtual handles over <code>/dev/shm</code>, eliminating CPU/PCIe memory copies.</li>
    <li><strong>Predictive Jitter Shedding &amp; Queue Defense:</strong> Under line-rate traffic bursts, the closed-loop scheduler predictively sheds stale intermediate telemetry, bounding P99 worst-case tail latency to <strong>29.02 ms</strong> (vs. 73 ms+ queue runaway in unmanaged FIFO systems) and maintaining a <strong>0.00% packet buffer drop rate</strong>.</li>
    <li><strong>Dynamic Precision Degradation:</strong> Closed-loop governor automatically degrades deep packet inspection models from FP16 to INT8 during saturation storms, preventing packet buffer exhaustion while maintaining continuous inline inspection.</li>
    <li><strong>High-Assurance Tamper-Proof Flight Recorder:</strong> Pre-allocated, zero-heap ring buffer (60,000 frames @ 1 kHz) logging inference decisions and thermals with rolling SHA-256 cryptographic signatures, providing immutable forensic audit trails for cyber incident compliance.</li>
    <li><strong>Ultra-Lean 4.8 MB Resident Core:</strong> Built without external runtime dependencies, easily fitting into constrained on-box network processors.</li>
  </ul>

  <p><em>In short: it's a unified runtime that resolves the collision between high-speed network dataplanes and deep learning inference, providing line-rate determinism that standard ML frameworks cannot achieve.</em></p>

  <p>I'm not sharing this to pitch a product. I'm sharing it because I believe it demonstrates the kind of low-latency systems thinking, kernel-level dataplane research, and runtime engineering instinct that would be relevant to what Palo Alto Networks builds.</p>

  <div class="box">
    <div class="box-title">Audited Research &amp; Deliverables</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 6px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Full Enterprise Technical Intelligence Report (PDF)</a> &mdash; 30-section audited architecture &amp; benchmark dossier</li>
      <li style="margin-bottom: 6px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Mission Control Bento Cockpit</a> &mdash; Real-time hardware topology and interactive chaos injection</li>
      <li><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">Production C++17 GitHub Repository</a> &mdash; 12/12 CTest suites passing, 100% green CI/CD</li>
    </ul>
  </div>

  <p>I'd genuinely appreciate <strong>15&ndash;20 minutes of your time &mdash; a conversation, not a formal interview</strong>. I'm interested in understanding where Palo Alto Networks sees opportunities for someone with my profile, whether that's in the Hardware Dataplane Engineering team, Network Security Platforms, Unit 42 research, or the Cortex/Prisma engineering groups, where I can contribute my skills and relentless mindset for building something that's not basic, and that defines the future of security infrastructure.</p>

  <p>If there's someone more relevant on your team to loop in, I'd be grateful for the introduction.</p>

  <p>Thank you for reading this far &mdash; I know it's longer than a typical cold message, and I respect your time.</p>

  <p>Looking forward to the possibility of a conversation.</p>

  <div class="footer">
    <p style="margin: 0 0 4px 0; font-weight: 700; color: #0f172a;">Warm regards,</p>
    <p style="margin: 0 0 2px 0; font-weight: 600;">Vikas Samvel</p>
    <p style="margin: 0 0 6px 0; color: #64748b;">Systems Software &amp; Low-Latency Dataplane Engineer</p>
    <p style="margin: 0; font-size: 13px;">
      Email: <a href="mailto:vikassamvel123@gmail.com">vikassamvel123@gmail.com</a> | Phone: +91 99408 81234<br>
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
    msg['From'] = f"Vikas Samvel <{sender_email}>"
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
