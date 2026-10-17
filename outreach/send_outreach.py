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
        subject = "Final-year student (Vel Tech) — built low-latency C++17 runtime vs. Triton & ROS bottlenecks"
        
        plain_text = f"""Hi {first_name},

I'm reaching out directly because I follow your work on autonomous machines at NVIDIA and wanted to share a low-latency systems project I've been building.

I'm Vikas S, a final-year B.Tech CSE student at Vel Tech University in Chennai. Most of my coursework and peers focus on standard web stacks, but I've spent the past two years focused on systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and GPU runtimes.

Over the last few months, I built an open-source deterministic edge AI co-scheduler in C++17 (EdgeModelScheduler v4.0). I wanted to understand why current edge inference pipelines struggle with latency consistency on Jetson and DRIVE platforms:

1. Triton is great for datacenter servers, but on edge hardware like Jetson Orin, its ~1.9 GB idle footprint and dynamic batching queues cause unpredictable latency spikes when sensors spike.
2. ROS 2 and Isaac ROS pass sensor streams through DDS middleware, which forces multiple serialization steps and memory copies for 4K video or LiDAR pointclouds. In my benchmarks under burst jitter, unmanaged queues had a P99 latency above 73 ms and missed roughly 35% of deadlines.
3. TensorRT optimizes individual models, but doesn't coordinate multi-model workloads competing for GPU cores and DLAs when the chip hits thermal throttling.

To test alternatives, I wrote a C++17 runtime with zero heap allocations on the critical path:
- CUDA IPC zero-copy adapter: passes PointCloud2 and camera frames across processes through /dev/shm virtual memory handles, bypassing DDS serialization at 12.2 GB/s (<15 µs latency).
- Lock-free ring buffer: custom SPSC circular ring with cacheline-isolated atomics (alignas(64)), avoiding lock contention and CPU cache-line bouncing.
- Hardware co-scheduler: balances tasks between GPU streaming multiprocessors and DLAs. Under simulated 45 ms network stalls, it sheds stale frames and held a 0% deadline miss rate.
- Thermal governor: steps secondary models down from FP16 to INT8 when reaching 82°C, reserving full precision for safety-critical paths.
- Lean footprint: runs with a 4.8 MB resident memory footprint without external heavy frameworks.

I've made the code and benchmarks public:
- Technical Report (PDF): https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
- Live Bento Dashboard: https://vikassamvel.github.io/edge-model-scheduler/
- GitHub Repo: https://github.com/VIKASSAMVEL/edge-model-scheduler

I'm writing because I'm graduating soon and want to work on systems software and GPU runtimes at NVIDIA. I would love 15 minutes of your time to get your feedback and see where a junior engineer or intern with my background could fit into the Jetson, DRIVE Thor, or Isaac teams.

If someone else on the engineering or recruiting team is better to speak with, I'd appreciate you passing this along.

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
<style>
  body {{ font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; font-size: 15px; line-height: 1.6; color: #1e293b; background-color: #ffffff; margin: 0; padding: 0; }}
  .container {{ max-width: 620px; margin: 0 auto; padding: 20px 16px; }}
  p {{ margin: 0 0 14px 0; }}
  ol, ul {{ margin: 0 0 14px 0; padding-left: 20px; }}
  li {{ margin-bottom: 8px; }}
  .links-box {{ background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 6px; padding: 14px 16px; margin: 18px 0; }}
  a {{ color: #2563eb; text-decoration: underline; }}
  .footer {{ border-top: 1px solid #e2e8f0; padding-top: 14px; margin-top: 20px; color: #475569; font-size: 14px; }}
</style>
</head>
<body>
<div class="container">
  <p>Hi {first_name},</p>

  <p>I'm reaching out directly because I follow your work on autonomous machines at NVIDIA and wanted to share a low-latency systems project I've been building.</p>

  <p>I'm <strong>Vikas S</strong>, a final-year B.Tech CSE student at Vel Tech University in Chennai. Most of my coursework and peers focus on standard web stacks, but I've spent the past two years focused on systems programming: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and GPU runtimes.</p>

  <p>Over the last few months, I built an open-source deterministic edge AI co-scheduler in C++17 (<strong>EdgeModelScheduler v4.0</strong>). I wanted to understand why current edge inference pipelines struggle with latency consistency on Jetson and DRIVE platforms:</p>

  <ol>
    <li><strong>Triton</strong> is great for datacenter servers, but on edge hardware like Jetson Orin, its ~1.9 GB idle footprint and dynamic batching queues cause unpredictable latency spikes when sensors spike.</li>
    <li><strong>ROS 2 and Isaac ROS</strong> pass sensor streams through DDS middleware, which forces multiple serialization steps and memory copies for 4K video or LiDAR pointclouds. In my benchmarks under burst jitter, unmanaged queues had a P99 latency above 73 ms and missed roughly 35% of deadlines.</li>
    <li><strong>TensorRT</strong> optimizes individual models, but doesn't coordinate multi-model workloads competing for GPU cores and DLAs when the chip hits thermal throttling.</li>
  </ol>

  <p>To test alternatives, I wrote a C++17 runtime with zero heap allocations on the critical path:</p>

  <ul>
    <li><strong>CUDA IPC zero-copy adapter:</strong> passes PointCloud2 and camera frames across processes through <code>/dev/shm</code> virtual memory handles, bypassing DDS serialization at 12.2 GB/s (&lt;15 &mu;s latency).</li>
    <li><strong>Lock-free ring buffer:</strong> custom SPSC circular ring with cacheline-isolated atomics (<code>alignas(64)</code>), avoiding lock contention and CPU cache-line bouncing.</li>
    <li><strong>Hardware co-scheduler:</strong> balances tasks between GPU streaming multiprocessors and DLAs. Under simulated 45 ms network stalls, it sheds stale frames and held a 0% deadline miss rate.</li>
    <li><strong>Thermal governor:</strong> steps secondary models down from FP16 to INT8 when reaching 82&deg;C, reserving full precision for safety-critical paths.</li>
    <li><strong>Lean footprint:</strong> runs with a 4.8 MB resident memory footprint without external heavy frameworks.</li>
  </ul>

  <div class="links-box">
    <div style="font-weight: 600; margin-bottom: 6px;">Project Resources:</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Technical Report (PDF)</a> &mdash; 30-section architecture and benchmark documentation</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Bento Dashboard</a> &mdash; Interactive hardware topology and live simulator</li>
      <li><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">GitHub Repository</a> &mdash; Full C++17 source with 12 passing test suites</li>
    </ul>
  </div>

  <p>I'm writing because I'm graduating soon and want to work on systems software and GPU runtimes at NVIDIA. I would love 15 minutes of your time to get your feedback and see where a junior engineer or intern with my background could fit into the Jetson, DRIVE Thor, or Isaac teams.</p>

  <p>If someone else on the engineering or recruiting team is better to speak with, I'd appreciate you passing this along.</p>

  <p>Thanks for your time,</p>

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

    else: # panw
        subject = "Final-year student (Vel Tech) — built C++17 deterministic AI runtime vs. dataplane bottlenecks"
        
        plain_text = f"""Hi {first_name},

I'm reaching out directly because I follow Palo Alto Networks' dataplane engineering and wanted to share a low-latency systems project I've been building.

I'm Vikas S, a final-year B.Tech CSE student at Vel Tech University in Chennai. Most of my peers focus on web applications, but I've spent the past two years focused on low-level systems: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and high-throughput dataplanes.

Over the last few months, I built an open-source deterministic AI co-scheduler in C++17 (EdgeModelScheduler v4.0). I wanted to study how hardware firewalls can run inline deep learning without breaking line-rate guarantees:

1. Palo Alto Networks built the Single-Pass Architecture to inspect traffic at line rate without re-assembly delays. But as PAN-OS runs deep learning inline (for DNS tunneling detection, URL classification, or encrypted traffic), a bottleneck appears: network dataplanes work on microsecond deadlines, while ML inference engines like TensorRT or ONNX Runtime are asynchronous and queue-dependent.
2. During traffic spikes or DDoS events, ML queues back up, pushing P99 tail latencies past 70 ms. This risks filling packet ring buffers or forcing appliances to bypass deep inspection.
3. Standard ML frameworks have large memory footprints (hundreds of megabytes) and use coarse mutexes that cause cache bouncing across multicore packet processors.

To test a solution, I engineered a C++17 runtime with zero heap allocations on the critical path:
- Lock-free SPSC circular rings with head and tail atomics isolated on separate 64-byte cache lines (alignas(64)), avoiding cache bouncing across high-speed packet cores.
- Direct shared-memory transport over /dev/shm, moving packet buffers to AI memory at 12.2 GB/s in under 15 microseconds.
- Predictive queue defense: during burst traffic, it drops stale intermediate telemetry, keeping P99 latency at 29 ms (vs. 73 ms in standard FIFO queues) with zero packet buffer drops.
- Dynamic precision governor: temporarily shifts deep inspection models from FP16 to INT8 during heavy load to prevent buffer exhaustion.
- Tamper-proof telemetry recorder: a circular buffer (60,000 frames @ 1 kHz) logging decisions and temperatures with rolling SHA-256 signatures for audit compliance.
- Lean footprint: runs in 4.8 MB resident memory with no heavy runtime dependencies.

I've published the full implementation and benchmarks here:
- Technical Report (PDF): https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf
- Live Bento Dashboard: https://vikassamvel.github.io/edge-model-scheduler/
- GitHub Repo: https://github.com/VIKASSAMVEL/edge-model-scheduler

I'm writing because I'm graduating soon and want to work on low-latency systems and dataplane software at Palo Alto Networks. I would love 15 minutes of your time to introduce myself and see where an entry-level engineer or intern with my profile could contribute on the Dataplane Engineering, Network Security, or PAN-OS core teams.

If there is an engineering lead or hiring manager on your team who would be the right person to speak with, I'd appreciate you passing this along.

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
<style>
  body {{ font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; font-size: 15px; line-height: 1.6; color: #1e293b; background-color: #ffffff; margin: 0; padding: 0; }}
  .container {{ max-width: 620px; margin: 0 auto; padding: 20px 16px; }}
  p {{ margin: 0 0 14px 0; }}
  ol, ul {{ margin: 0 0 14px 0; padding-left: 20px; }}
  li {{ margin-bottom: 8px; }}
  .links-box {{ background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 6px; padding: 14px 16px; margin: 18px 0; }}
  a {{ color: #2563eb; text-decoration: underline; }}
  .footer {{ border-top: 1px solid #e2e8f0; padding-top: 14px; margin-top: 20px; color: #475569; font-size: 14px; }}
</style>
</head>
<body>
<div class="container">
  <p>Hi {first_name},</p>

  <p>I'm reaching out directly because I follow Palo Alto Networks' dataplane engineering and wanted to share a low-latency systems project I've been building.</p>

  <p>I'm <strong>Vikas S</strong>, a final-year B.Tech CSE student at Vel Tech University in Chennai. Most of my peers focus on web applications, but I've spent the past two years focused on low-level systems: bare-metal C++17, Linux kernel memory management, lock-free concurrency, and high-throughput dataplanes.</p>

  <p>Over the last few months, I built an open-source deterministic AI co-scheduler in C++17 (<strong>EdgeModelScheduler v4.0</strong>). I wanted to study how hardware firewalls can run inline deep learning without breaking line-rate guarantees:</p>

  <ol>
    <li><strong>Palo Alto Networks</strong> built the Single-Pass Architecture to inspect traffic at line rate without re-assembly delays. But as PAN-OS runs deep learning inline (for DNS tunneling detection, URL classification, or encrypted traffic), a bottleneck appears: network dataplanes work on microsecond deadlines, while ML inference engines like TensorRT or ONNX Runtime are asynchronous and queue-dependent.</li>
    <li><strong>During traffic spikes or DDoS events</strong>, ML queues back up, pushing P99 tail latencies past 70 ms. This risks filling packet ring buffers or forcing appliances to bypass deep inspection.</li>
    <li><strong>Standard ML frameworks</strong> have large memory footprints (hundreds of megabytes) and use coarse mutexes that cause cache bouncing across multicore packet processors.</li>
  </ol>

  <p>To test a solution, I engineered a C++17 runtime with zero heap allocations on the critical path:</p>

  <ul>
    <li><strong>Lock-free ring buffers:</strong> custom SPSC circular rings with head and tail atomics isolated on separate 64-byte cache lines (<code>alignas(64)</code>), avoiding cache bouncing across high-speed packet cores.</li>
    <li><strong>Direct shared-memory transport:</strong> moves packet buffers to AI memory over <code>/dev/shm</code> at 12.2 GB/s in under 15 microseconds.</li>
    <li><strong>Predictive queue defense:</strong> during burst traffic, it drops stale intermediate telemetry, keeping P99 latency at 29 ms (vs. 73 ms in standard FIFO queues) with zero packet buffer drops.</li>
    <li><strong>Dynamic precision governor:</strong> temporarily shifts deep inspection models from FP16 to INT8 during heavy load to prevent buffer exhaustion.</li>
    <li><strong>Tamper-proof telemetry recorder:</strong> a circular buffer (60,000 frames @ 1 kHz) logging decisions and temperatures with rolling SHA-256 signatures for audit compliance.</li>
    <li><strong>Lean footprint:</strong> runs in 4.8 MB resident memory with no heavy runtime dependencies.</li>
  </ul>

  <div class="links-box">
    <div style="font-weight: 600; margin-bottom: 6px;">Project Resources:</div>
    <ul style="margin: 0; padding-left: 18px;">
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf">Technical Report (PDF)</a> &mdash; 30-section architecture and benchmark documentation</li>
      <li style="margin-bottom: 4px;"><a href="https://vikassamvel.github.io/edge-model-scheduler/">Live Bento Dashboard</a> &mdash; Interactive hardware topology and live simulator</li>
      <li><a href="https://github.com/VIKASSAMVEL/edge-model-scheduler">GitHub Repository</a> &mdash; Full C++17 source with 12 passing test suites</li>
    </ul>
  </div>

  <p>I'm writing because I'm graduating soon and want to work on low-latency systems and dataplane software at Palo Alto Networks. I would love 15 minutes of your time to introduce myself and see where an entry-level engineer or intern with my profile could contribute on the Dataplane Engineering, Network Security, or PAN-OS core teams.</p>

  <p>If there is an engineering lead or hiring manager on your team who would be the right person to speak with, I'd appreciate you passing this along.</p>

  <p>Thanks for your time,</p>

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
