---
# AuraSense Limited — AISS Application
**Programme:** AI Subsidy Scheme (Cyberport / Digital Policy Office)
**Funding:** Up to 70% compute cost subsidy (HK$3B pool)
**Submit to:** [aiss_application@cyberport.hk](mailto:aiss_application@cyberport.hk)
**Portal:** aisc.cyberport.hk/aiss
**Date:** March 2026

---

## Section 1 — Company Overview

AuraSense Limited is a Hong Kong-incorporated deep technology company developing the Spike-based Facade Scanning Vision Codec (SFSVC), the world's first neuromorphic artificial intelligence system purpose-built for autonomous building facade inspection. The company is currently at Technology Readiness Level 6 to 7, with a validated prototype demonstrated on synthetic and semi-structured datasets and preparing for full deployment in real-world building inspection scenarios under Hong Kong's Mandatory Building Inspection Scheme (MBIS).

The company operates as a single full-time founder and CTO, Dickson Chau, with contracted engineering and operations support. AuraSense is pre-revenue, having completed the technical architecture and enterprise platform development and now entering the pilot phase. The company has been accepted into NVIDIA Inception, has applied to Cyberport CCMF/CIP, and has applied to HKPC's technology funding programmes. SFSVC is protected by a PCT patent application filed in March 2026, covering the spike-based visual codec architecture and the STDP-based adaptive learning method specific to building inspection applications.

---

## Section 2 — AI Project Description

The SFSVC neuromorphic inference workload represents a fundamentally different computational paradigm from conventional deep learning. Unlike standard convolutional neural networks (CNNs) that process dense pixel grids at fixed frame rates, SFSVC operates on sparse temporal events — the differences between successive frames — encoded as discrete spike trains. This spike-based representation dramatically reduces redundant computation, but it creates unique GPU and NPU compute requirements for training and benchmarking that distinguish it from standard deep learning workflows.

### Spike-Based Convolution vs Standard CNN Operations

Conventional CNNs apply filter kernels uniformly across every pixel in every channel of every frame, generating dense activation maps at every layer. In contrast, spike-based convolution operates only on coordinates where a temporal change has occurred, applying convolutional kernels only to active spike locations. While this reduces inference compute dramatically on edge hardware, training the spike codec requires generating large synthetic datasets of spike trains from video, which involves frame-by-frame temporal differencing, contrast gating, refractory period enforcement, and spatial pooling — all GPU-accelerated operations that must process the full dense video at training time before sparsity benefits materialise at inference time.

### STDP Weight Updates: Compute-Intensive Training

Spike-Timing-Dependent Plasticity (STDP) is the adaptive learning rule underlying SFSVC's ability to improve autonomously with each new building it inspects. STDP training involves computing weight updates based on precise spike arrival time differences between pre-synaptic and post-synaptic neurons, requiring high-precision floating-point operations on large synaptic weight matrices. Each STDP training pass on a single building's facade dataset (approximately 10,000 tiles, 4 elevations, 30 frames per tile) requires iterating over billions of spike timing pairs. On CPU, a single STDP training epoch takes 18 to 22 hours. On GPU (A100 equivalent), the same epoch completes in 20 to 30 minutes. Iterative training to convergence requires 40 to 60 such epochs, making GPU access essential.

### 8-Lane Parallel Inference: Operations Per Second

SFSVC operates an 8-lane parallel detection pipeline, with each lane running a specialised neuromorphic inference model:
- Lane 1: Temporal spike codec (primary)
- Lane 2: Crack detection via STDP-trained defect classifier
- Lane 3: Looming detection for obstacle avoidance
- Lane 4: Vibration signature analysis
- Lane 5: Thermal anomaly detection
- Lane 6: LiDAR point cloud voxel grid processing
- Lane 7: RGB-thermal fusion inference
- Lane 8: MBIS risk band classification

During development and benchmarking, each lane must be profiled independently to validate that P95 latency remains below the 4ms real-time threshold. This requires running thousands of synthetic inference passes per lane per hardware configuration, generating performance telemetry, and iterating on architecture until targets are met. On edge hardware, this is feasible for single-lane testing, but full 8-lane parallel benchmarking at scale (simulating 50 concurrent buildings being processed) requires cloud GPU clusters to generate realistic load.

### LiDAR Point Cloud Processing

SFSVC fuses LiDAR depth data with RGB and thermal imagery to improve crack depth estimation and facade surface curvature mapping. LiDAR point clouds from each scan contain 500,000 to 1,200,000 points, which must be voxelised into 3D grids, downsampled, and registered to the 2D image plane. Voxel grid operations (nearest-neighbour search, octree traversal, surface normal estimation) are highly parallelisable and benefit from GPU acceleration. On CPU, point cloud processing adds 80 to 120ms per frame; on GPU, it adds 4 to 6ms, keeping it within the real-time budget.

### Thermal + RGB Fusion Model Inference

The fusion model is a lightweight residual neural network trained to predict facade defect probability from concatenated thermal-RGB input. While inference is edge-deployable, training this fusion model requires paired thermal-RGB datasets with annotated defect labels. Generating augmented training data (rotation, scaling, lighting variation, synthetic defect injection) for 50 building types × 4 elevation angles × 1000 tiles per building requires batch processing on GPU clusters. Each training iteration of the fusion model involves processing 200,000 image pairs, taking approximately 6 GPU-hours per iteration on A100-class hardware.

### Scale: Per-Building Inference Volume

Each building facade scan involves approximately 10,000 tile inferences (4 elevations × 2,500 tiles per elevation). For a single building, SFSVC processes 10,000 tiles × 8 lanes = 80,000 lane-inference operations. At current edge hardware performance, this takes 8 to 12 minutes per building. To validate that the system scales to 50 concurrent buildings (the target RI firm portfolio load), we must benchmark the cloud backend's ability to handle 50 × 80,000 = 4,000,000 inference operations in parallel, which requires dedicated GPU quota that is not available on shared cloud free tiers.

### Why GPU/NPU Compute Subsidy is Essential

STDP training, fusion model training, and full-scale benchmarking represent the three binding constraints on SFSVC's progress from TRL 7 to TRL 8. STDP training requires GPU clusters not available on edge devices; each model iteration consumes 40 to 80 GPU-hours. Development and benchmarking require repeated experimentation across hyperparameter sweeps, hardware configurations, and building datasets, and each experiment run consumes GPU quota. Without subsidised GPU access, AuraSense must either severely limit training iterations (compromising model accuracy) or delay pilots (compromising time-to-market). The AISS subsidy removes this constraint, enabling AuraSense to train STDP models to convergence, validate performance at scale, and deliver production-grade accuracy for Hong Kong's RI firms.

---

## Section 3 — Compute Requirement Justification

### Monthly Compute Budget

AuraSense's AI development workload requires the following monthly GPU allocation:

**STDP Training Runs:** 120 GPU-hours per month (A100 equivalent or V100)
- 3 STDP model iterations per month, 40 GPU-hours per iteration
- Includes hyperparameter tuning and cross-validation on HK building dataset

**Inference Benchmarking:** 40 GPU-hours per month
- Full 8-lane pipeline stress testing at 10 ×, 25 ×, 50 × building concurrency
- P95 latency profiling and bottleneck identification

**LiDAR Fusion Training:** 60 GPU-hours per month
- Thermal-RGB-LiDAR fusion model training and augmentation pipeline
- Voxel grid preprocessing and registration alignment

**Total Monthly Requirement:** ~220 GPU-hours per month

**Estimated Cost at AWS p3.2xlarge (V100 16GB):** ~HK$14,000 per month

**With 70% AISS Subsidy:** HK$9,800 per month saved

**Annual Saving:** HK$117,600

All compute will be allocated across AWS, Google Cloud Platform, and Microsoft Azure, all of which are AISS-approved cloud service providers. AuraSense will maintain detailed usage logs, timestamped experiment records, and cost attribution per training run to ensure full compliance with AISS reporting requirements. The compute budget is realistic and conservative, based on measured runtime of existing STDP training jobs and scaled to the expected monthly iteration cadence required to reach TRL 8 within 12 months.

---

## Section 4 — Technical Team Summary

**Founder & CTO — Dickson Chau:**
Dickson is a neuromorphic engineering specialist with deep expertise in spike-based neural networks, real-time embedded systems optimisation, C++ performance engineering (AVX2, SIMD intrinsics, multithreading), and autonomous drone integration. He has architected and implemented the complete SFSVC engine, including the 8-lane parallel processing pipeline, the STDP adaptive learning layer, the LiDAR-thermal-RGB fusion module, and the MBIS-compliant enterprise reporting platform. He is the sole inventor on the PCT patent application filed March 2026. Based full-time in Hong Kong.

**Planned Hires Post-Funding:**
- **Neuromorphic Engineer 1:** STDP algorithm development, FPGA acceleration, spike codec optimisation.
- **Neuromorphic Engineer 2:** Multi-spectral sensor fusion, real-time pipeline tuning, edge deployment.

**Combined Team Expertise:**
The planned engineering team brings together expertise in spike neural network training, GPU-accelerated compute, C++ systems programming, computer vision algorithm implementation, and real-time embedded systems deployment. This combination is essential to successfully train STDP models on GPU, optimise them for edge inference, and deploy them in production building inspection workflows.

---

## Section 5 — Business Plan Summary

### Market

Hong Kong's MBIS regime mandates facade inspection of 40,000+ buildings every 10 years, creating an annual addressable market of HK$2 billion to HK$4 billion. Current inspection costs range from HK$80,000 to HK$200,000 per building with 3 to 6 week turnaround. SFSVC delivers HK$8,000 to HK$15,000 per scan with 48-hour turnaround, representing a 60% cost reduction and 10× faster delivery. The Greater Bay Area presents a comparable adjacent market, and global regulatory analogs (Singapore, UK, Australia, EU) represent a USD 12 billion total addressable market.

### Traction

SFSVC is at TRL 6 to 7 with validated prototype. NVIDIA Inception accepted. Cyberport CCMF/CIP, HKPC, and HKSTP applications submitted. Pilot partnerships with RI firms in negotiation. Full enterprise platform implemented, including RI dashboard, report generation, and MBIS-compliant output templates.

### Revenue Projections

- **Year 1 (2026):** HK$150,000 — 3 pilot buildings, 1 SaaS subscription
- **Year 2 (2027):** HK$2,500,000 — 50 buildings scanned, 5 RI firm SaaS subscribers, first enterprise contract
- **Year 3 (2028):** HK$8,000,000+ — 200 buildings scanned, 15 RI firms, GBA entry, international licensing discussions

---

## Section 6 — HK AI Blueprint Alignment

SFSVC advances Hong Kong's AI & Digital Economy Blueprint across five strategic dimensions:

**AI in Smart City Infrastructure (Built Environment):**
SFSVC digitises MBIS facade inspection, a mandatory government compliance process that has remained manual and paper-based for decades. By generating structured, machine-readable building condition data at scale, SFSVC creates the foundation for a citywide facade health database that informs urban planning, maintenance budgeting, and disaster risk assessment.

**Edge AI Sovereignty (Data Stays in HK, No Cloud Dependency):**
Unlike cloud-dependent CNN platforms that transmit building imagery to overseas servers, SFSVC processes all data on-site using edge hardware. Inspection data never leaves Hong Kong, satisfying stringent client data sovereignty requirements and demonstrating Hong Kong's capability to deploy AI infrastructure without reliance on foreign cloud providers.

**Local AI Talent Development and Retention:**
AuraSense will create a minimum of 3 skilled AI engineering jobs in Hong Kong within 12 months, specifically in neuromorphic computing and spike-based neural networks — disciplines that are globally scarce and strategically important. By building a Hong Kong-based neuromorphic engineering team, AuraSense contributes to the city's AI talent pipeline and demonstrates that cutting-edge AI research and commercialisation can be conducted locally.

**Demonstration of Neuromorphic AI Commercial Viability:**
SFSVC is one of the first neuromorphic AI systems globally to move from research prototype to commercial deployment. By demonstrating that neuromorphic computing can deliver measurable cost and performance advantages over conventional deep learning in a high-stakes regulatory compliance domain, AuraSense positions Hong Kong as a leader in next-generation AI architectures.

**MBIS Digitisation Supporting Smart Government Agenda:**
The Buildings Department and Housing Authority manage thousands of MBIS inspections annually, all conducted manually. SFSVC's deployment provides a pathway for government to modernise this process, improve inspection consistency, reduce costs, and create a longitudinal dataset of building facade condition that has never existed before. This directly supports the Smart Government pillar of Hong Kong's Digital Economy strategy.
