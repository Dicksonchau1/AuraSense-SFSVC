# AuraSense SFSVC – Neuromorphic Codec for Drone Inspection

AuraSense SFSVC is a **neuromorphic video codec layer** for drone inspections that sits between the camera, your existing models, and the uplink – running on standard CPU/GPU hardware – to deliver **real‑time crack‑aware control** with a fraction of the bandwidth.

Instead of streaming raw video to the cloud, SFSVC converts frame changes into **sparse spike representations**, applies a **real‑time control loop**, and only transmits what matters for safety, analytics, and audit.

This repository contains a **simple Python neuromorphic codec demo** and reference benchmark structure that demonstrate the core concept on real inspection video.


---

## Why this matters for drone inspection

Modern inspection drones hit three hard limits:

1. **Latency** – Cloud or heavy on‑board models can't always make decisions fast enough for safety‑critical, low‑altitude flight near concrete and infrastructure.
2. **Bandwidth & storage** – Raw or lightly compressed video is expensive to stream and store at scale, especially for long inspection routes.
3. **Battery & compute** – Throwing more GPU power at the problem doesn't help if every extra watt steals flight time.

SFSVC tackles all three by using a **neuromorphic codec as middleware**, not as a replacement for your stack:

- Runs on **CPU or GPU‑grade edge hardware** you already deploy
- Plugs into existing **camera → edge box → cloud** pipelines
- Reduces the load and cost of your current models and infrastructure instead of competing with them

---

## Neuromorphic codecs: where we are today

Neuromorphic codecs are where **LLMs were a few years ago**: exciting papers, early hardware prototypes, but very few **practical, plug‑and‑play layers** you can drop into a real product without building everything from scratch.

Existing tools are either:

- Tied to **specific neuromorphic hardware** (event cameras, Loihi chips)
- **Research‑grade frameworks** requiring significant engineering to integrate
- Sensor‑centric SDKs, not general middleware codecs

What's missing is a **hardware‑agnostic neuromorphic codec layer** that takes in normal video frames, outputs compressed spike representations with control‑relevant features, and runs on commodity hardware.

This repo is meant to be that **first accessible step** – a simple, understandable neuromorphic codec you can experiment with, plus real numbers from a production‑grade engine.

---

## Key benchmark results (from production engine)

Using AuraSense's **C++‑accelerated SFSVC engine** on a 720p runway inspection video, we measured:

### Real‑time control (Lane 1, C++)

- **2.50 ms** average control latency
- **2.61 ms P95**, **2.81 ms P99**
- **0% SLA violations** against a **6 ms** safety contract
- Effective control rate ≈ **106 Hz**

### Neuromorphic compression (codec layer)

- **9.97× compression**
- **89.97% bandwidth reduction**
- ~**681 GB/day** storage savings (modeled on typical inspection routes)
- ≈ **US$116,000/year** saved per **1,000 km** of infrastructure inspected

### Perception & behavior

- Every frame scored for **crack severity** (width, severity class, confidence)
- System chooses between **FORWARD / CAUTION / SLOW** actions, not just bounding boxes
- Signature and YOLO lanes run in the background, enriching semantics **without touching the hard real‑time loop**

> **Note:** These numbers come from AuraSense's **production C++ engine**, which is part of a private SDK used in pilot programs. This public repo provides a **simple Python neuromorphic codec** and benchmark structure for concept exploration; it is not the full commercial engine.

---

## What's in this repository

This public repo is a **reference implementation** and learning resource, not the full production SDK.

### `examples/simple_neurocodec_python.py`

A minimal Python implementation showing:

- Frame‑to‑frame difference computation
- Simple ON/OFF spike generation with fixed thresholds
- Per‑frame latency measurement
- Typical performance: **~8 ms/frame** on 720p video with pure Python

**Purpose:** Demonstrate the core neuromorphic codec concept in an accessible way, showing that even a basic implementation can achieve single‑digit millisecond latency.

**Usage:**

```bash
python examples/simple_neurocodec_python.py --input your_video.mp4
