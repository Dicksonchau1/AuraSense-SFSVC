# Artificial Intelligence Subsidy Scheme ("AISS") — Application Form

**Applicant:** AuraSense Limited  
**Date of Submission:** ____/____/2026

---

## Notes for Applicant

> Please refer to the "Guide and Conditions for Applicants of the AISS" at https://aisc.cyberport.hk/aiss before completing this form.

---

## Part A — Applicant Information

| Field | Detail |
|-------|--------|
| **Category** | ☒ AI start-ups |
| **Entity Name (Eng)** | AuraSense Limited |
| **Entity Name (Chi)** | 歐雅生物科技有限公司 |
| **Address (Eng)** | *(To be filled — company registered address)* |
| **Address (Chi)** | *(To be filled — company registered address in Chinese)* |

### Entity Description

**(English — ~250 words)**

AuraSense Limited is a Hong Kong deep-tech AI start-up focusing on neuromorphic computing and edge intelligence for autonomous drone inspection and infrastructure safety. Our core technology, the Spike-based Frequency-Spatial Video Codec (SFSVC), is a neuromorphic video middleware that emulates the spiking and temporal coding behaviour of the biological visual system. Instead of transmitting dense video frames, SFSVC converts spatio-temporal changes into sparse spike events and performs on-device crack perception and control decisions in real time. This removes dependency on cloud compute while reducing communication bandwidth demand by up to 94%.

SFSVC adopts a bio-inspired multi-rate four-lane architecture — from a hard real-time control lane to higher-level semantic integration lanes — closely mirroring the hierarchical timing structure observed in recent neuroscience research on visual and language processing. In runway crack inspection scenarios, our latest C++-accelerated engine achieves approximately 0.40 ms P50 and 0.56 ms P95 end-to-end control latency at 1280×720 resolution, with 93.8% sparsity and approximately 94% bandwidth saving compared to raw H.265 video. The underlying spike encoding method is protected by a provisional patent, and the system is being positioned as an inspection-grade, vendor-agnostic neuromorphic middleware for drone and robotics platforms.

AuraSense is actively exploring pilot projects with airports, logistics operators and infrastructure owners in Hong Kong, the Greater Bay Area and overseas, aiming to establish Hong Kong as a regional innovation hub for neuromorphic edge AI and autonomous inspection systems.

**(Chinese — ~400 characters)**

歐雅生物科技有限公司是一家香港本地人工智能深科技初創企業，專注於神經形態計算及邊緣人工智能技術，應用於無人機自主巡視及基礎設施智能檢測系統。本司核心技術「脈衝頻率空間視頻編解碼器」（SFSVC）是一種具有自主知識產權的神經形態視頻中間件，其工作原理模擬生物視覺神經系統的時序脈衝編碼機制，將視頻幀間的時空變化以稀疏神經脈衝事件（Spike Events）進行表徵與編碼，於邊緣設備端直接完成實時裂縫感知與控制決策，徹底擺脫對雲端算力的依賴，同時將帶寬需求降低高達94%。SFSVC採用仿生多速率四通道神經形態架構，各通道分別對應不同時間尺度的神經處理層級——由硬實時控制脈衝通道至高層語義整合通道——與近期神經科學研究所揭示的大腦語言及視覺信息處理的層級時序結構高度吻合。本技術已在跑道裂縫檢測場景中完成驗證，達至0.40毫秒P50及0.56毫秒P95控制延遲、93.8%脈衝稀疏度及約94%帶寬節省。本司持有SFSVC脈衝編碼方法的臨時專利，現積極拓展香港、粵港澳大灣區及全球機場、公用設施及物流走廊的無人機巡檢試點合作，致力推動香港成為神經形態邊緣人工智能創新中心。

### Workforce

| Field | Detail |
|-------|--------|
| **Total Number of Employees in Hong Kong** | 1 |
| **Number of R&D Employees in Hong Kong** | 1 |

### Key Contact Person 1

| Field | Detail |
|-------|--------|
| **Name (Eng)** | Chau Kai Cho |
| **Name (Chi)** | 周繼祖 |
| **Title (Eng)** | CEO |
| **Title (Chi)** | 行政總裁 |
| **Department (Eng)** | Executive Office |
| **Department (Chi)** | 行政辦公室 |
| **Contact No.** | 92918674 |
| **Contact Email** | Dicksonchau@aurasensehk.com |

### Key Contact Person 2

*(Not applicable)*

---

## Part B — Project Details

### Project 1

| Field | Detail |
|-------|--------|
| **Project Title (Eng)** | Neuromorphic SFSVC Engine for Real-Time Drone Crack Inspection |
| **Project Title (Chi)** | 用於無人機實時裂縫檢測的神經形態SFSVC引擎 |

### Project Objectives and Summary

**(English — ~250 words)**

This project aims to build and validate a production-grade neuromorphic video engine based on AuraSense's Spike-based Frequency-Spatial Video Codec (SFSVC) for real-time crack inspection and control on autonomous drones. The objective is to deliver a C++-accelerated, PREEMPT_RT-ready SFSVC engine that can run on edge devices (e.g. NVIDIA Jetson, x86 with GPU) with P95 end-to-end control latency below 6 ms and communication bandwidth savings of at least 80–90% compared to conventional H.264 video streaming. The requested GPU computing power will be used to train and fine-tune neuromorphic encoders, crack detection models and YOLO-based semantic modules, and to perform large-scale simulations over inspection datasets from airport runways, roads and other linear infrastructure.

Technically, the project will: (1) refine and scale the multi-rate four-lane SFSVC architecture (hard real-time control, signature memory, YOLO semantics, and uplink) with Linux PREEMPT_RT integration; (2) train and optimize neuromorphic spike encoders and crack perception modules using GPU clusters, focusing on low-bitwidth, sparse and event-driven representations; (3) build a reusable inspection middleware SDK for drone and robotics partners; and (4) validate the system in pilot scenarios in Hong Kong and the Greater Bay Area. The project targets inspection-grade robustness (stability under varying lighting, weather, and motion) and aims to establish Hong Kong as a reference site for neuromorphic edge AI infrastructure for smart airports, utilities and logistics corridors.

**(Chinese — ~400 characters)**

本項目旨在構建及驗證一套基於AuraSense「脈衝頻率空間視頻編解碼器」（SFSVC）的生產級神經形態視頻引擎，用於支援無人機在邊緣端實時進行裂縫檢測及控制決策。項目目標是交付一套以C++加速、可在Linux PREEMPT_RT實時系統上運行的SFSVC引擎，於邊緣設備（如NVIDIA Jetson或配備GPU的x86平台）實現低於6毫秒P95端到端控制延遲，同時較傳統H.264視頻串流節省至少80–90%的通訊帶寬。申請的GPU算力將用於訓練及微調神經形態編碼器、裂縫感知模型及基於YOLO的語義模塊，並對機場跑道、道路及其他線性基建的巡檢數據集進行大規模模擬與實驗。

技術上，本項目將：（1）完善及擴展多速率四通道SFSVC架構（硬實時控制通道、簽名記憶通道、YOLO語義通道及上行通訊通道），並與Linux PREEMPT_RT實時內核深度整合；（2）在GPU叢集上訓練及優化神經形態脈衝編碼器與裂縫感知模塊，重點針對低比特寬度、稀疏及事件驅動表徵；（3）構建可供無人機及機械人合作夥伴集成的巡檢中間件SDK；（4）在香港及大灣區的實際場景中進行試點驗證。項目以檢測級穩定性為目標，致力將香港打造為神經形態邊緣人工智能巡檢基礎設施的示範基地。

### Project Parameters

| Field | Detail |
|-------|--------|
| **Preferred Start Date** | 01/06/2026 |
| **Computing Power Required** | 2 GPU Card(s) |
| **Consumption Period** | 6 Month(s) |

### Technology Area

- ☒ Computer Science
- ☒ Information and Communication Technologies
- ☒ Robotics

### Industry Sector

- ☒ Construction
- ☒ Information Technology
- ☒ Transportation

### Relevancy to the National 14th Five-Year Plan

- ☒ Artificial Intelligence
- ☒ Brain Science and Brain-Like Technology Research

### R&D Employees in This Project

| Type | Count |
|------|-------|
| Local R&D employee(s) | 1 |
| Non-local R&D employee(s) | 0 |

### Project Deliverables

- Production-grade SFSVC C++/Python SDK for drone and robotics integration
- PREEMPT_RT-ready multi-rate neuromorphic engine for real-time crack inspection
- Benchmark reports (latency, compression, bandwidth, energy) on representative datasets
- Pilot-ready demo package for airports / infrastructure owners

### Expected Outcomes

| Type | Count | Details |
|------|-------|---------|
| Patent(s) | 1 | Provisional patent → full patent filing for spike-based neuromorphic video encoding and multi-rate control architecture |
| Publication(s) | 1 | 1–2 conference or journal papers on neuromorphic video compression and RT control for drone inspection (subject to IP strategy) |
| Award(s) | 0 | — |

### R&D Details

- **Model type:** Neuromorphic spike encoder + crack perception model + YOLO-based semantic detector
- **Training:** Event-based and frame-based convolutional models (tens of millions of parameters), batch sizes 16–64, mixed precision (FP16/FP32) on 1–2 GPUs
- **Datasets:** Runway and pavement crack datasets (tens to hundreds of GB) plus synthetic augmentations, including different lighting and motion patterns
- **Inference:** Highly optimized C++ core with pybind11 bindings, running at ≥125 Hz control rate with P95 end-to-end latency ~0.56 ms in internal CodeSpaces tests (projected ~0.30 ms on bare metal)
- **GPU usage:** Mainly for training/tuning neuromorphic encoders and YOLO models, plus large-scale simulation sweeps to stress-test latency and robustness under different traffic and network conditions

### R&D Advantages

- Unique neuromorphic codec (SFSVC) that directly outputs spikes and control signals, rather than compressing video for the cloud
- Demonstrated sub-0.6 ms P95 control latency at 1280×720 resolution with 93.8% sparsity and ~94% bandwidth reduction in internal C++ benchmarks
- Bio-inspired multi-rate architecture aligned with recent brain-like computation research, providing a defensible technical narrative and patent space
- Strong fit for Hong Kong's airport, logistics and infrastructure inspection needs, where low latency, low bandwidth and on-device robustness are critical

### Contribution to I&T and AI Development

- Introduces a neuromorphic edge AI middleware layer that can be reused across drones, ground robots and fixed cameras
- Advances Hong Kong's capabilities in real-time, safety-critical AI (autonomous inspection and control) beyond cloud-centric deep learning
- Bridges academic neuromorphic computing concepts with practical deployment, creating reference implementations and benchmarks for the local ecosystem

### Contribution to Technological, Social or Economic Growth of Hong Kong I&T and AI Sectors

- Supports safer and more efficient inspection of runways, bridges and public infrastructure, reducing manual, high-risk work
- Opens new market opportunities for Hong Kong AI and robotics companies in smart airport operations and infrastructure management in the GBA and globally
- Positions Hong Kong as a testbed for neuromorphic edge AI infrastructure, attracting collaborations with drone operators, system integrators and research labs

### Partnership

- In discussion / exploratory talks with drone delivery and inspection companies (e.g. regional delivery operators, AI robotics start-ups) for pilot integrations
- Open to collaboration with local universities and R&D centres on neuromorphic algorithms and real-time systems

### Re-submission

☒ No — This is a new application.

---

## Part C — Attachment List

| # | Attachment | File Name | Status |
|---|-----------|-----------|--------|
| 1 | Project proposal (max 10 slides, PDF) | Appendix_A.pdf | ✅ Prepared |
| 2 | Team structure and CVs | Appendix_B.pdf | ✅ Prepared |
| 3 | Track records of computing power usage | Appendix_C.pdf | ⬜ If applicable |
| 4 | Financial proof | Appendix_D.pdf | ⬜ **You must provide** (bank statements past 3 months or latest audit) |
| 5 | Certificate of Incorporation & BR | Appendix_E.pdf | ⬜ **You must provide** (CI + BR copies) |
| 6 | Start-up document proof | Appendix_F.pdf | ⬜ If applicable (Cyberport/HKSTP incubatee, TSSSU, RAISe+) |
| 7 | Shareholding structure | Appendix_G.pdf | ⬜ If applicable |
| 8 | Other supporting documents | Appendix_H.pdf | ⬜ If applicable |

---

## Part D — Declaration

I, on behalf of **AuraSense Limited**, declare that —

(a) the applicant has carefully read and fully understands the Guide and Conditions for Applicants of the Artificial Intelligence Subsidy Scheme (the Guide) and all explanatory notes as set out in this form;

(b) all the information provided in this form as well as the accompanying information is true, complete and accurate and reflect the status of affairs as at the date of submission;

(c) the applicant has obtained consent from all relevant persons/entities for the disclosure, use and further disclosure by the Government of their information/personal data;

(d) the applicant has not applied for or accepted and will not apply for or accept any additional subsidy, incentive, payment, reimbursement or indemnity for the same scope of the project or its related subject matter from other Government subsidy schemes;

(e) the applicant will not disclose the Confidential Information to any third party except as permitted under the Guide;

(f) the applicant shall conform in all respects with all legislation, regulations and by-laws of the Hong Kong Special Administrative Region in carrying out the project.

| Field | Detail |
|-------|--------|
| **Signature** | *(To be signed)* |
| **Company Chop** | *(To be stamped)* |
| **Name of Signatory** | Chau Kai Cho (周繼祖) |
| **Post Title** | CEO, AuraSense Limited |
| **Date** | ____/____/2026 |
