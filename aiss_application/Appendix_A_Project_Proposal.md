# Appendix A — Project Proposal
## Neuromorphic SFSVC Engine for Real-Time Drone Crack Inspection
### AuraSense Limited | AISS Application 2026

---

# Slide 1: Executive Summary

## The Problem
- Hong Kong's ageing infrastructure (4,200+ bridges, 2 international airports, 2,000+ km of roads) requires frequent crack inspection
- Current methods rely on manual visual inspection or cloud-dependent video streaming
- Cellular bandwidth costs for drone video: **$1,040/drone/month** (H.265 @ 30fps)
- Cloud dependency creates single points of failure for safety-critical inspection

## Our Solution
**SFSVC** — a neuromorphic video middleware that converts video into sparse spike events on-device, enabling:
- **94% bandwidth reduction** ($64/drone/month vs $1,040)
- **Sub-millisecond** crack detection latency
- **Zero cloud dependency** — full on-device intelligence
- **Inspection-grade accuracy** — 98.7% crack detection match

## AISS Request
- **2 GPU Cards** for **6 months** starting June 2026
- Purpose: Train neuromorphic encoders, crack detection models, and YOLO semantic modules

---

# Slide 2: Project Milestones & Timeline

## 6-Month Implementation Plan (Jun – Nov 2026)

| Month | Milestone | GPU Usage | Deliverable |
|-------|-----------|-----------|-------------|
| **M1** (Jun) | Dataset preparation & baseline training | 2 GPUs — full utilisation for encoder pre-training | Curated dataset (runway, road, bridge cracks) |
| **M2** (Jul) | Neuromorphic encoder training & optimisation | 2 GPUs — encoder + spike model training | Trained spike encoder v2 with FP16 optimisation |
| **M3** (Aug) | Crack perception model training | 2 GPUs — crack detector fine-tuning | Production-grade crack perception module |
| **M4** (Sep) | YOLO semantic module training + RT integration | 2 GPUs — YOLO training + simulation sweeps | YOLO-integrated multi-rate SFSVC engine |
| **M5** (Oct) | System integration & pilot validation | 1 GPU — inference testing + stress simulations | Pilot-ready SDK package |
| **M6** (Nov) | Benchmark, documentation & pilot deployment | 1 GPU — final benchmark runs | Full benchmark report + pilot demo |

### Key Gates
- **Gate 1 (M2):** Spike encoder achieves ≥90% sparsity with <2% accuracy loss
- **Gate 2 (M4):** End-to-end latency <6ms on Jetson with YOLO semantics
- **Gate 3 (M6):** Passed pilot validation at ≥1 Hong Kong infrastructure site

---

# Slide 3: Computing Power Requirement & Justification

## GPU Allocation Plan (2 GPU Cards × 6 Months)

```
         Jun    Jul    Aug    Sep    Oct    Nov
GPU 1:  [████] [████] [████] [████] [██░░] [██░░]
GPU 2:  [████] [████] [████] [████] [░░░░] [░░░░]

████ = Full utilisation    ██░░ = Partial    ░░░░ = Reserved/standby
```

### Training Workload Breakdown

| Workload | GPU Hours | GPU(s) | Duration |
|----------|-----------|--------|----------|
| Neuromorphic spike encoder pre-training | ~1,440 hrs | 2 | M1–M2 |
| Crack perception model fine-tuning | ~720 hrs | 2 | M3 |
| YOLOv8-nano semantic detector training | ~720 hrs | 2 | M4 |
| Large-scale simulation sweeps (latency/robustness) | ~480 hrs | 1 | M4–M5 |
| Benchmark & validation runs | ~240 hrs | 1 | M5–M6 |
| **Total** | **~3,600 GPU-hours** | | |

### Justification
- **Encoder training** is the most GPU-intensive: event-based temporal convolutions over video sequences of 1,000+ frames require large batch processing and mixed-precision (FP16/FP32) training
- **Simulation sweeps** test the engine under 100+ combinations of lighting, weather, motion speed, and network conditions — impossible on CPU alone
- **Two GPUs** enable parallel experimentation (e.g. training encoder on GPU 1 while running crack detector ablation on GPU 2)

### Other ICT Infrastructure Requirements
| Resource | Specification | Justification |
|----------|--------------|---------------|
| Storage | 2 TB NVMe SSD | Crack datasets (100+ GB) + model checkpoints + simulation logs |
| RAM | 64 GB | Large batch training + data pipeline buffering |
| OS | Ubuntu 22.04 LTS | PREEMPT_RT kernel support + CUDA compatibility |
| Framework | PyTorch 2.x + CUDA 12 | Neuromorphic model training; TensorRT for deployment |
| Networking | 10 Gbps internal | High-speed data loading from storage to GPU |

---

# Slide 4: Market Analysis

## Target Market: Autonomous Infrastructure Inspection

### Hong Kong & GBA Addressable Market

| Segment | HK Market | GBA Market | Key Need |
|---------|-----------|------------|----------|
| Airport runway inspection | HK$50M/yr | HK$200M/yr | Real-time, safety-critical |
| Bridge & tunnel inspection | HK$80M/yr | HK$500M/yr | Hard-to-access, high-risk |
| Road surface monitoring | HK$30M/yr | HK$300M/yr | Large-area, cost-sensitive |
| Building façade inspection | HK$40M/yr | HK$150M/yr | Urban density, regulatory |
| **Total TAM** | **HK$200M/yr** | **HK$1.15B/yr** | |

### Competitive Landscape

| Solution | Latency | Bandwidth | Edge-Ready | Neuromorphic |
|----------|---------|-----------|------------|--------------|
| **AuraSense SFSVC** | **<1 ms** | **-94%** | **✅** | **✅** |
| Cloud YOLO (AWS/Azure) | 50–200 ms | 0% saving | ❌ | ❌ |
| Edge YOLO (Jetson) | 5–15 ms | 0% saving | ✅ | ❌ |
| Event cameras + SNN | 1–5 ms | -80% | ✅ | Partial |
| H.265 + cloud analytics | 100+ ms | -50% (codec) | ❌ | ❌ |

### SFSVC Unique Advantage
Our approach is the **only solution** combining neuromorphic spike encoding, sub-millisecond control latency, and 94% bandwidth reduction in a single middleware — no special hardware (event cameras) needed.

---

# Slide 5: R&D Methodology — Neuromorphic Engine Architecture

## SFSVC Multi-Rate Four-Lane Architecture

```
Camera Feed (H.264/H.265) ─────────────────────────────────────────
    │
    ├─ Lane 1: Hard Real-Time Control (≤0.6ms P95)
    │   └─ Spike encoding → Delta detection → Crack score → Control signal
    │
    ├─ Lane 2: Signature Memory (~33ms)
    │   └─ Temporal spike accumulation → Pattern matching → Anomaly flag
    │
    ├─ Lane 3: YOLO Semantics (~100ms) ← GPU TRAINING FOCUS
    │   └─ Frame buffer → YOLOv8-nano → Bounding boxes → Severity labels
    │
    └─ Lane 4: Uplink & Reporting (~1s)
        └─ Sparse event compression → Cellular upload → Dashboard
```

### Training Pipeline (GPU-Accelerated)

1. **Spike Encoder Training**
   - Input: Video pairs (frame_t, frame_t+1) from crack datasets
   - Output: Optimised threshold parameters, temporal kernels
   - Method: Self-supervised contrastive learning on spike representations
   - GPU need: Mixed-precision training, batch size 32–64

2. **Crack Perception Training**
   - Input: Labelled crack image datasets (CrackForest, DeepCrack, custom runway data)
   - Output: Lightweight crack classifier operating on spike features
   - Method: Transfer learning from pre-trained backbone → spike-domain fine-tuning

3. **YOLO Semantic Training**
   - Input: Annotated inspection images with crack bounding boxes + severity labels
   - Output: YOLOv8-nano model optimised for TensorRT deployment on Jetson
   - Method: Progressive training (pre-train on COCO → fine-tune on crack dataset)

4. **System Simulation**
   - Monte Carlo sweeps over 100+ parameter combinations
   - Validate latency, accuracy, and bandwidth under degraded conditions

---

# Slide 6: R&D Methodology — Detailed Technical Approach

## Current Proven Performance (Internal Benchmarks)

| Metric | Result | Test Conditions |
|--------|--------|-----------------|
| P50 Latency | **0.40 ms** | 1280×720, 30fps, 1,127 frames |
| P95 Latency | **0.56 ms** | Same |
| Throughput | **125 fps** | 6 processing lanes |
| Sparsity | **93.8%** | 6.84M spikes across 1.127M pixels/frame |
| Bandwidth Saving | **94%** | vs H.265 @ 5.2 Mbps |
| Crack Accuracy | **98.7%** | vs manual labelling |
| False Positive Rate | **<0.3%** | |

## GPU-Enabled Improvements Targeted

| Current State | AISS-Enabled Target | How |
|--------------|---------------------|-----|
| Hand-tuned spike thresholds | Learned adaptive thresholds | GPU-trained encoder |
| Rule-based crack scoring | Neural crack perception | CNN on spike features |
| No semantic labels | YOLO severity classification | YOLOv8,nano training |
| Fixed parameters | Adaptive to scene conditions | Simulation sweeps on GPU |
| Single resolution tested | Multi-resolution validated | Parallel GPU experiments |

## Key Innovation: Bio-Inspired Temporal Coding

SFSVC's spike encoding mimics biological retinal ganglion cells:
- **ON spikes:** Brightness increase > threshold (delta > 8 gray levels)
- **OFF spikes:** Brightness decrease > threshold
- **Encoding:** AVX2-accelerated, 32 pixels per SIMD register
- **Result:** 93.8% of pixels produce zero spikes (sparse) → massive bandwidth reduction

This aligns with the **14th Five-Year Plan** priority on **Brain Science and Brain-Like Technology Research**.

---

# Slide 7: Risk Assessment

## Technical Risks

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Trained encoder underperforms hand-tuned | Medium | Medium | Keep hand-tuned as fallback; progressive training with ablation |
| YOLO semantic lane exceeds latency budget | Medium | Low | Lane 3 operates asynchronously; does not block control lane |
| Insufficient crack dataset diversity | Low | High | Synthetic augmentation (rotation, lighting, blur) + partner data |
| Edge hardware (Jetson) memory constraints | Medium | Medium | INT8 quantisation + TensorRT optimisation |

## Operational Risks

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Key person dependency (1-person team) | High | High | Modular codebase + comprehensive documentation; recruitment planned upon funding |
| Pilot partner delays | Medium | Medium | Multiple parallel discussions; can demo on synthetic data first |
| IP disclosure during pilot | Low | High | Provisional patent filed; NDA with all partners |

## Financial Risks

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| GPU costs exceed subsidy duration | Low | Medium | Efficient training schedule; early stopping; model distillation |
| No revenue during R&D phase | High | Medium | Low burn rate (1-person); AISS covers compute; seeking angel round |

---

# Slide 8: Market Entry & Business Model

## Go-to-Market Strategy

### Phase 1: Pilot Validation (M1–M6, AISS Period)
- Validate with 1–2 Hong Kong infrastructure partners (airport/bridge)
- Free pilot deployments to build reference cases
- **Output:** Benchmark data, testimonials, case studies

### Phase 2: Commercial Launch (M7–M12)
- Subscription pricing: **$500–$1,000/drone/month** (SDK + support)
- Target: 10–30 drone fleet operators in HK & GBA
- **Revenue target:** HK$300K–$900K/month by end of Year 1

### Phase 3: Scale (Year 2+)
- Platform licensing to system integrators
- Expand to ground robots, fixed CCTV, autonomous vehicles
- GBA and international expansion

### Business Model

| Revenue Stream | Pricing | Target Customers |
|----------------|---------|-----------------|
| SFSVC SDK License | $500–$1,000/drone/month | Drone fleet operators |
| Pilot & Integration | $50K one-time | Infrastructure owners |
| Enterprise Platform | Custom | System integrators |

### Unit Economics (30-drone fleet)
- **Customer pays:** $15,000–$30,000/month
- **Customer saves:** $29,280/month on bandwidth alone
- **ROI for customer:** Positive from Month 1

---

# Slide 9: Team Structure & Qualifications

## Core Team

### Chau Kai Cho (周繼祖) — CEO & Lead Engineer

- **Role:** Full-stack R&D — architecture, C++ engine, ML training, pilot integration
- **Background:**
  - Deep expertise in real-time systems, neuromorphic computing, and edge AI
  - Designed and implemented the complete SFSVC engine in C++ with AVX2 SIMD optimisation
  - Built the four-lane multi-rate architecture from first principles
  - Filed provisional patent for spike-based neuromorphic video encoding
- **Technical Achievements:**
  - 0.40ms P50 latency (125 fps) on 1280×720 video — fully functional prototype
  - Complete C++/Python SDK with pybind11 bindings
  - Lock-free queue architecture for real-time multi-lane processing
  - Hardware profiler, degraded mode policy, and failsafe subsystems

### Planned Hiring (Post-Funding)
- **ML Engineer** (1 FTE) — focus on spike encoder and YOLO model training
- **Embedded Engineer** (1 FTE, contract) — Jetson/ARM deployment and optimisation

## Organisational Chart

```
CEO / Lead Engineer (Chau Kai Cho)
    ├── R&D: Neuromorphic Engine & Architecture
    ├── R&D: ML Model Training (GPU) — to hire
    └── Engineering: Edge Deployment — to hire (contract)
```

---

# Slide 10: Summary & Expected Impact

## Project Summary

| Item | Detail |
|------|--------|
| **Applicant** | AuraSense Limited (HK AI start-up) |
| **Project** | Neuromorphic SFSVC Engine for Drone Crack Inspection |
| **GPU Request** | 2 GPU Cards × 6 Months |
| **Start Date** | 1 June 2026 |
| **Key Deliverable** | Production-grade SFSVC SDK + Pilot Deployment |

## Expected Impact

### For Hong Kong I&T Ecosystem
- First neuromorphic edge AI middleware originating from Hong Kong
- Reference implementation for brain-inspired computing research
- Bridges academic neuroscience concepts into commercial deployment

### For Hong Kong Economy & Society
- Safer infrastructure inspection (reduces manual high-risk work)
- New market opportunities in smart airports, GBA logistics corridors
- Positions HK as a testbed for neuromorphic AI + autonomous systems

### Measurable Outcomes
- **1 patent** filed (provisional → full)
- **1–2 publications** in neuromorphic computing / real-time AI
- **≥1 pilot** validated with HK infrastructure partner
- **94% bandwidth reduction** demonstrated in production conditions
- **Sub-6ms latency** with GPU-trained models on edge hardware

## Alignment with National Priorities

| 14th Five-Year Plan Area | This Project's Contribution |
|--------------------------|-----------------------------|
| Artificial Intelligence | Production neuromorphic AI for autonomous inspection |
| Brain Science & Brain-Like Technology | Bio-inspired spike encoding architecture |

---

*AuraSense Limited — Building the neural system for autonomous infrastructure inspection*
*Contact: Dicksonchau@aurasensehk.com | +852 9291 8674*
