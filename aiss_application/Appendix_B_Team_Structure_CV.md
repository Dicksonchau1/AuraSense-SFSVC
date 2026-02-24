# Appendix B — Team Structure and Curricula Vitae
## AuraSense Limited | AISS Application 2026

---

## 1. Team Structure

### Current Team (Hong Kong)

```
┌────────────────────────────────────────────────────────┐
│              AuraSense Limited — Team Structure         │
├────────────────────────────────────────────────────────┤
│                                                        │
│            Chau Kai Cho (周繼祖)                        │
│            CEO & Lead R&D Engineer                     │
│            ─────────────────────                       │
│            • Architecture & System Design              │
│            • C++/Python Engine Development              │
│            • ML Model Design & Training                │
│            • Pilot Integration & Business Dev          │
│                                                        │
├────────────────────────────────────────────────────────┤
│  Planned Hiring (Post-AISS Approval)                   │
│                                                        │
│  ┌──────────────────┐  ┌──────────────────────────┐    │
│  │ ML Engineer (FTE) │  │ Embedded Engineer (Cont.) │   │
│  │ • Spike encoder   │  │ • Jetson/ARM porting      │   │
│  │   training        │  │ • TensorRT optimisation   │   │
│  │ • YOLO fine-tuning│  │ • PREEMPT_RT integration  │   │
│  │ • Dataset curation│  │ • Hardware validation     │   │
│  └──────────────────┘  └──────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Roles & Responsibilities for This Project

| Team Member | Role | Responsibilities | % Allocation |
|-------------|------|-----------------|--------------|
| Chau Kai Cho | CEO & Lead R&D Engineer | Overall project leadership; architecture design; C++ engine development; ML model design; GPU training supervision; pilot coordination | 100% |

---

## 2. Curriculum Vitae

### Chau Kai Cho (周繼祖)
**CEO & Lead R&D Engineer, AuraSense Limited**

| Field | Detail |
|-------|--------|
| **Name** | Chau Kai Cho (周繼祖) |
| **Title** | CEO & Lead R&D Engineer |
| **Organisation** | AuraSense Limited |
| **Location** | Hong Kong |
| **Contact** | Dicksonchau@aurasensehk.com / +852 9291 8674 |

### Education

| Qualification | Institution | Notes |
|---------------|-------------|-------|
| **Postgraduate Degree — Nursing** | The Hong Kong Polytechnic University (PolyU) | Formal training in systematic assessment, critical analysis, and evidence-based methodology — directly applicable to safety-critical inspection system design and clinical-grade quality assurance |

### Professional Summary

Founder of AuraSense Limited with a unique cross-disciplinary background combining healthcare science training (PolyU Nursing postgraduate) with deep expertise in real-time systems engineering, neuromorphic computing, and edge AI deployment. The clinical training in systematic observation, evidence-based analysis, and safety-critical decision-making provides a distinctive foundation for designing inspection-grade visual perception systems. Single-handedly designed, implemented, and validated the complete SFSVC neuromorphic video engine — from low-level AVX2 SIMD optimisation to high-level multi-rate architecture and Python SDK integration. Proven ability to take a concept from neuroscience research to a working, benchmarked production prototype.

### Technical Capabilities

**Systems Engineering & Real-Time**
- C++ real-time engine development with PREEMPT_RT Linux
- AVX2 SIMD optimisation for pixel-level processing (32 pixels per register)
- Lock-free queue and multi-threaded pipeline architecture
- Latency profiling and P50/P95/P99 benchmark methodology

**Machine Learning & AI**
- Neuromorphic spike encoding design (bio-inspired temporal coding)
- YOLO object detection training and deployment
- PyTorch model training with mixed precision (FP16/FP32)
- pybind11 C++/Python integration for production ML pipelines

**Edge Computing & Deployment**
- NVIDIA Jetson platform experience
- Docker containerisation for edge deployment
- Hardware profiling and adaptive performance tuning
- Degraded mode policy and failsafe subsystem design

### Key Achievement: SFSVC Engine (2025–2026)

Designed and built the complete AuraSense SFSVC system from scratch:

| Component | Description |
|-----------|-------------|
| **rt_core.cpp** | AVX2-accelerated spike encoding and crack detection core |
| **engine.cpp** | Multi-lane real-time processing engine with lock-free queues |
| **detection_controller** | Adaptive YOLO integration with gating engine |
| **failsafe.cpp** | Safety-critical failsafe and degraded mode policy |
| **rt_core_pybind.cpp** | Python bindings for ML integration |
| **crack_statistics** | Statistical analysis and band-based crack scoring |
| **hardware_profiler** | Runtime hardware capability detection |

**Benchmarked Results:**
- P50: 0.40 ms / P95: 0.56 ms / P99: 0.67 ms end-to-end latency
- 125 fps throughput on 1280×720 @ 30fps input
- 93.8% spike sparsity (94% bandwidth reduction)
- 98.7% crack detection accuracy vs manual labelling
- Zero frame drops in 1,127-frame continuous processing

### Intellectual Property

- **Provisional Patent Filed:** Spike-based neuromorphic video encoding and multi-rate control architecture for autonomous inspection systems

---

*Note: Additional team members will be recruited upon AISS approval. CVs will be provided to HKCMCL as they join the project.*
