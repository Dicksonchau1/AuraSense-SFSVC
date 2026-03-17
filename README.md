# AuraSense SFSVC — Neuromorphic Edge Perception SDK

**Website**: [www.aurasensehk.com](https://www.aurasensehk.com)

## Overview

This repository contains:

1. **SFSVC Commercial Website** — Marketing site for the production neuromorphic perception SDK
2. **NEPA Research Demo** — Public Python reference implementation (educational/prototyping only)

## IMPORTANT: SFSVC vs NEPA

### SFSVC (Commercial SDK)
- **Language**: C++
- **Latency**: 2.50 ms avg, 2.61 ms P95, 2.81 ms P99
- **Purpose**: Production deployment for edge autonomy
- **Optimization**: SIMD AVX2, lock-free queues, deterministic scheduling
- **Support**: Engineering support, SLA available
- **License**: Commercial licensing (Ed25519 cryptographic validation)

### NEPA (Research Demo)
- **Language**: Python
- **Latency**: ~8 ms/frame
- **Purpose**: Educational, prototyping, concept demonstration
- **Optimization**: Standard NumPy
- **Support**: Community support
- **License**: Open Source

**The performance benchmarks on [www.aurasensehk.com](https://www.aurasensehk.com) refer to the SFSVC SDK, NOT the NEPA demo.**

## Architecture

### SFSVC Core Principles

**Spike-Based Encoding**
- Temporal contrast encoding reduces redundant pixel processing
- ON/OFF events based on brightness changes
- Binary representation preserves temporal edges

**Deterministic CPU Execution**
- SIMD acceleration (AVX2 / NEON)
- Bounded hot paths, predictable scheduling
- No GPU dependency, offline-first architecture

**Single-Digit Millisecond Latency**
- Full 720p pipeline: P50 1.52ms, P95 7.87ms, P99 13.04ms
- Artifact-backed benchmark data
- Platform: Linux x86_64 reference hardware

### NEPA Demo (Python Reference)

NEPA demonstrates neuromorphic codec concepts:
- Frame-delta spike encoding
- Temporal contrast detection
- Data compression via sparse representation
- Basic crack detection pipeline

**Public Repository**: [github.com/Dicksonchau1/AuraSense-NEPA_v1](https://github.com/Dicksonchau1/AuraSense-NEPA_v1)

**Important**: NEPA is a research demo only. For production deployments, use the SFSVC SDK.

## Vertical Solutions

### Façade Risk Engine
Built on SFSVC SDK for Hong Kong MBIS compliance:
- AI + LiDAR façade inspection
- Structured, regulator-aligned reporting
- Offline-first deployment
- Edge processing on mini PC or appliance

**Learn more**: [www.aurasensehk.com/solutions/facade.html](https://www.aurasensehk.com/solutions/facade.html)

## Website Structure

```
/
├── index.html              # SFSVC platform homepage
├── technology.html         # Core SDK architecture
├── solutions.html          # Vertical deployments overview
├── solutions/
│   └── facade.html        # Façade Risk Engine details
├── research.html          # NEPA research demo page
├── benchmark.html         # Artifact-backed performance data
├── pricing.html           # SDK licensing
├── security.html          # Security & data architecture
└── about.html             # Company information
```

## Navigation (Consistent Across Site)

1. Home
2. Technology
3. Solutions
4. Benchmarks
5. Pricing
6. Security
7. Research
8. About

## Performance Claims

### SFSVC SDK (Commercial)
- **Core spike lane**: Sub-2ms processing latency
- **Full 720p pipeline**: P95 7.87ms (artifact v2)
- **Compression**: 9.97× ratio, 89.97% bandwidth reduction
- **Power**: Runs on Intel N100-class systems at ~4W

### NEPA Demo (Research)
- **Processing**: ~8 ms/frame on standard CPU
- **Purpose**: Educational demonstration of concepts only

## Deployment

### SFSVC Production SDK
- Official Docker image
- REST API
- C++ SDK
- Ed25519 cryptographic licensing
- Offline operation with periodic HTTPS validation

### NEPA Research Demo
- Python-based
- Suitable for prototyping and evaluation
- Not intended for production use

## Contact

**Technical Inquiries**: support@aurasensehk.com

**Website**: www.aurasensehk.com

**Location**: Hong Kong

---

© 2026 AuraSense Limited. All rights reserved.

**SFSVC SDK v2.0** — Neuromorphic perception infrastructure for edge autonomy.
