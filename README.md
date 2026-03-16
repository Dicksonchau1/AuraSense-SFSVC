# AuraSense NEPA + SFSVC

Neuromorphic Control Middleware for Infrastructure Inspection

## Overview

NEPA (Neuromorphic Event Processing Architecture) + SFSVC (Sparse Frame-delta Spike Video Codec) converts video into sparse spike representations to deliver deterministic crack-aware control under a 6 ms safety contract.

## Architecture

### Two-Lane System

**Lane 1: Real-Time Control**
- Implementation: C++
- Latency: 2.50 ms avg, 2.61 ms P95, 2.81 ms P99
- SLA Violations: 0%
- Safety Contract: 6 ms
- Control Rate: ~106 Hz

Deterministic spike-based crack detection runs within bounded time. Decision output (FORWARD / CAUTION / SLOW) is available before next frame arrives.

**Lane 2: Semantic Lane**
- Components: YOLO inference, signature enrichment
- Execution: Non-blocking, asynchronous
- Interference: Does not affect real-time control

Optional semantic analysis runs in parallel without impacting Lane 1 latency guarantees.

## SFSVC Layer

### Frame Delta Encoding
Each pixel is compared to its previous value. Only pixels that cross a threshold generate events. Static regions produce zero output.

### ON/OFF Spikes
- Brightness increases generate ON events
- Decreases generate OFF events
- Binary representation reduces data volume while preserving temporal edges

### Compression Metrics
- 9.97× compression ratio
- 89.97% bandwidth reduction
- 681 GB/day modeled savings at 1080p 30fps
- ~$116,000/year savings per 1,000 km inspected

### Implementation Note
- Public repository: Python demo (~8 ms/frame)
- Production engine: Private C++ SDK (2.50 ms avg)

## Control Output

Per-frame decision system outputs:

1. **Crack Width**: Estimated width in pixels based on spike band geometry
2. **Severity Classification**: Minor, moderate, or severe
3. **Confidence Score**: Derived from spike consistency across adjacent frames
4. **Navigation Decision**:
   - FORWARD: No cracks detected, maintain speed
   - CAUTION: Minor crack, reduce speed for higher-resolution capture
   - SLOW: Severe crack, stop and perform detailed scan

## Infrastructure Constraints

### Latency Limits
Control decisions must complete within frame intervals. At 30 Hz, each frame arrives every 33.3 ms. Cloud round-trip latency (50-200 ms) exceeds available control budget.

### Bandwidth and Storage Cost
Full video transmission at 1080p 30fps consumes 5.2 Mbps. Continuous operation generates 681 GB/day. At $8/GB cellular rates, this translates to $116,000/year per 1,000 km.

### Battery and Compute Tradeoffs
GPU-based inference draws 15-30W. CPU-only processing reduces power draw to 2-4W, directly impacting flight duration on battery-constrained platforms.

## Open Source vs Commercial

| | Public Repository | Commercial SDK |
|---|---|---|
| Language | Python | C++ |
| Latency | ~8 ms/frame | 2.50 ms avg, 2.81 ms P99 |
| Purpose | Educational, prototyping | Production deployment |
| License | Open Source | Commercial |
| Optimization | Standard NumPy | SIMD AVX2, lock-free queues |
| Support | Community | Engineering support, SLA |

## Demo

Python-based SFSVC implementation available at:
https://github.com/Dicksonchau1/AuraSense-SFSVC

Processes video at ~8 ms/frame on standard CPU hardware. Suitable for prototyping and evaluation.

## Contact

**Technical Inquiries**: support@aurasensehk.com

**Website**: www.aurasensehk.com

**Location**: Hong Kong

---

AuraSense NEPA + SFSVC
Neuromorphic Control Middleware
