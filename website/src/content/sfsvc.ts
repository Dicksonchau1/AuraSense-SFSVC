import type { ProductPageContent } from '../types/content';

export const sfsvcContent: ProductPageContent = {
  hero: {
    headline: 'SFSVC — Neuromorphic Compression and Perception SDK',
    subheadline: 'Spike-based Facade Surface Vision Codec',
    description:
      'SFSVC is a neuromorphic compression and perception SDK for edge infrastructure inspection. It encodes temporal frame differences as spike trains, runs classification entirely on CPU with SIMD acceleration, and produces structured defect telemetry with full evidence retention and replay verification. No GPU required. No cloud dependency. Offline activation with Ed25519 license verification.',
    primaryCta: { label: 'Request SFSVC Evaluation', href: '/contact' },
    secondaryCta: { label: 'View All Products', href: '/products' },
  },
  sections: [
    {
      id: 'why-sfsvc',
      eyebrow: 'Overview',
      title: 'Why SFSVC',
      description:
        'Infrastructure inspection at scale requires perception that is deterministic, auditable, and deployable on constrained edge hardware without GPU dependencies or cloud connectivity. SFSVC achieves this through spike-based temporal encoding — a neuromorphic compression approach that represents scene changes as sparse spike events rather than dense frame reconstructions. This produces structured defect telemetry at sub-millisecond classification latency on commodity CPUs, with full evidence retention for regulatory and engineering review.',
      items: [
        {
          title: 'CPU-First Architecture',
          description:
            'SFSVC runs entirely on CPU. No GPU allocation, no CUDA dependency, no driver compatibility matrix. The processing pipeline uses AVX2 (x86_64) or NEON (ARM64) SIMD instructions for vectorized frame differencing and spike encoding. This eliminates GPU scheduling jitter and enables deployment on compute-constrained edge platforms — inspection drones, embedded gateways, and ruggedized field units.',
        },
        {
          title: 'Spike-Based Temporal Encoding',
          description:
            'Rather than processing full frames, SFSVC computes temporal differences between consecutive frames and encodes significant changes as spike events. This produces a sparse, event-driven representation that captures structural changes — cracks, spalling, delamination, surface anomalies — while discarding static background. Compression ratios of 10–50× over raw frame storage are typical depending on scene dynamics.',
        },
        {
          title: 'Deterministic Classification',
          description:
            'The classification pipeline produces identical outputs for identical inputs regardless of execution timing, thread scheduling, or hardware platform. No floating-point non-determinism from GPU parallel reduction. No order-dependent aggregation. Every classification is replay-verifiable from the sealed session log.',
        },
        {
          title: 'Offline Operation',
          description:
            'SFSVC operates without network connectivity. License activation uses Ed25519 offline verification — no license server callback required after initial activation. Model bundles, configuration, and runtime binaries are self-contained. Evidence retention and telemetry storage are local. Post-mission data retrieval occurs via physical media or selective uplink.',
        },
      ],
    },
    {
      id: 'compression-pipeline',
      eyebrow: 'Architecture',
      title: 'Compression Pipeline',
      description:
        'The SFSVC processing pipeline transforms raw camera frames into structured defect telemetry through a sequence of deterministic stages: temporal differencing, spike encoding, feature extraction, anomaly scoring, and classification. Each stage operates within bounded latency and produces intermediate artifacts that are retained for replay verification.',
      items: [
        {
          title: 'Temporal Differencing',
          description:
            'Consecutive frames are subtracted using SIMD-accelerated pixel-wise operations. The resulting difference map highlights regions of structural change — new cracks, crack propagation, surface deformation, environmental deposits. Static background is suppressed at the input stage, reducing downstream processing load by 80–95% in typical inspection scenarios.',
        },
        {
          title: 'Spike Encoding',
          description:
            'Significant temporal differences are encoded as spike events — discrete, timestamped activation signals associated with spatial coordinates. The spike encoding threshold is configurable per deployment context. Spike trains form a sparse, event-driven representation of scene dynamics that is naturally compressed and temporally precise.',
        },
        {
          title: 'Feature Extraction and Anomaly Scoring',
          description:
            'Spike patterns are matched against learned prototypes using cosine similarity with L2-normalized embeddings (D=256). Anomaly scores are computed as 1 minus the cosine similarity to the best-matching prototype. Scores above the advisory threshold (0.35) trigger telemetry emission. Scores above the high-severity threshold (0.55) trigger elevated alerts.',
        },
        {
          title: 'Structured Telemetry Output',
          description:
            'Classification results are emitted as structured telemetry records containing defect type, severity grade, zone identifier, anomaly score, model version, frame reference, and timestamp. Records are chain-sealed into the evidence log and retained for post-mission engineering review, regulatory submission, and replay verification.',
        },
      ],
    },
    {
      id: 'deployment-profile',
      eyebrow: 'Deployment',
      title: 'Deployment Profile',
      description:
        'SFSVC is packaged as a native shared library with C headers and language bindings for Python and C++. It targets Linux on x86_64 and ARM64 architectures with SIMD acceleration. Deployment is offline-capable with Ed25519 license verification and self-contained model bundles.',
      items: [
        {
          title: 'Platform Targets',
          description:
            'Linux x86_64 with AVX2 (primary target). Linux ARM64 with NEON (embedded and drone platforms). Minimum 2GB RAM, 1GB storage for runtime plus model bundles. No GPU, no specialized accelerator, no kernel module dependencies beyond standard Linux.',
        },
        {
          title: 'SIMD Acceleration',
          description:
            'Frame differencing and spike encoding inner loops use 256-bit SIMD operations — _mm256_fmadd_ps on AVX2, equivalent intrinsics on NEON. Minimum 4-way unrolling with 32-byte aligned buffers. Compile-time target selection with scalar fallback for non-SIMD targets.',
        },
        {
          title: 'Offline Activation',
          description:
            'License activation uses Ed25519 public-key verification. The activation token is generated offline and verified locally — no license server callback, no network dependency, no periodic re-validation. Activation state persists across reboots and power cycles.',
        },
        {
          title: 'Model Bundle Management',
          description:
            'Model prototypes and adaptation deltas are packaged as versioned bundles with SHA-256 integrity hashes. Bundle promotion follows the governance pipeline — development → staging → production — with human approval gates at each transition. No autonomous bundle promotion.',
        },
      ],
    },
    {
      id: 'supported-environments',
      eyebrow: 'Compatibility',
      title: 'Supported Environments',
      description:
        'SFSVC is validated against a defined set of target environments. Deployment outside these targets may work but is not covered by support or performance guarantees.',
      items: [
        {
          title: 'Linux x86_64 (Primary)',
          description:
            'Ubuntu 20.04+, Debian 11+, RHEL 8+. GCC 9+ or Clang 11+. AVX2-capable CPU (Intel Haswell/2013+ or AMD Excavator/2015+). Recommended: 4+ cores, 4GB+ RAM for production workloads.',
        },
        {
          title: 'Linux ARM64 (Embedded)',
          description:
            'Ubuntu 20.04+ (aarch64), Yocto/Buildroot with glibc 2.31+. NEON-capable SoC (Cortex-A53+). Validated on NVIDIA Jetson (CPU-only mode), Raspberry Pi 4/5, and Qualcomm RB5 platforms.',
        },
        {
          title: 'Containerized Deployment',
          description:
            'Docker multi-stage build with minimal runtime image. Compatible with Kubernetes, Docker Compose, and standalone container runtimes. Resource limits and CPU pinning recommended for latency-sensitive deployments.',
        },
        {
          title: 'Cross-Compilation',
          description:
            'CMake-based build system supports cross-compilation for ARM64 targets from x86_64 build hosts. Toolchain files provided for common embedded Linux configurations.',
        },
      ],
    },
    {
      id: 'licensing',
      eyebrow: 'Licensing',
      title: 'Licensing',
      description:
        'SFSVC is distributed under a commercial license with offline activation. Evaluation access is available for qualified infrastructure operators, system integrators, and enterprise engineering teams.',
      items: [
        {
          title: 'Activation Model',
          description:
            'Licenses are activated offline using Ed25519-signed activation tokens. Tokens are bound to a hardware fingerprint and do not require network connectivity for verification or renewal. License terms, entitlements, and expiration are encoded in the activation token.',
        },
        {
          title: 'Evaluation Access',
          description:
            'Time-limited evaluation licenses are available for qualified organizations. Evaluation includes the full SDK, sample model bundles, documentation, and engineering support during the evaluation period. Contact engineering to discuss evaluation scope and requirements.',
        },
        {
          title: 'Deployment Licensing',
          description:
            'Production deployment licenses are scoped by device count, deployment site, and contract term. Volume licensing and enterprise agreements are available for multi-site deployments. Refer to the licensing FAQ for detailed terms.',
          cta: { label: 'Read Licensing FAQ', href: '/faq' },
        },
      ],
    },
    {
      id: 'integration-path',
      eyebrow: 'Integration',
      title: 'Integration Path',
      description:
        'SFSVC integrates into existing inspection and monitoring systems through public C headers, Python bindings, and a documented API surface. Integration engineers receive documentation, example code, and onboarding support.',
      items: [
        {
          title: 'Public Headers',
          description:
            'The SDK exposes a stable C API through versioned public headers. Types, function signatures, and error codes are defined in a single header file. ABI stability is maintained across minor versions. Breaking changes are reserved for major version increments.',
        },
        {
          title: 'Python Bindings',
          description:
            'Python bindings provide access to the SpikeCodec, CrackDetector, and VideoProcessor classes. Bindings use the native shared library through ctypes/cffi. NumPy array interop is supported for frame data. Suitable for prototyping, evaluation, and integration testing.',
        },
        {
          title: 'Example Code and Documentation',
          description:
            'The SDK ships with documented examples covering initialization, frame processing, telemetry retrieval, and evidence export. A quickstart guide walks through first-frame classification in under 10 minutes. API reference documentation is generated from annotated headers.',
        },
        {
          title: 'Developer Onboarding',
          description:
            'Engineering teams receive guided onboarding during evaluation — architecture walkthrough, integration planning, deployment configuration review, and access to engineering support for technical questions during the integration period.',
        },
      ],
    },
  ],
  specs: [
    { label: 'Classification Latency (P95)', value: '< 0.5ms at 720p' },
    { label: 'Classification Latency (P99)', value: '< 0.8ms at 720p' },
    { label: 'Throughput', value: '125+ FPS on commodity x86_64' },
    { label: 'Compression Ratio', value: '10–50× over raw frames' },
    { label: 'Embedding Dimension', value: '256 (float32)' },
    { label: 'Similarity Metric', value: 'Cosine (L2-normalized dot product)' },
    { label: 'SIMD Requirement', value: 'AVX2 (x86_64) / NEON (ARM64)' },
    { label: 'Memory Allocation', value: 'Pre-allocated, no hot-path heap' },
    { label: 'License Verification', value: 'Ed25519 offline activation' },
    { label: 'Platform', value: 'Linux x86_64, Linux ARM64' },
  ],
  faqItems: [
    {
      question: 'Does SFSVC require a GPU?',
      answer:
        'No. SFSVC runs entirely on CPU using AVX2 (x86_64) or NEON (ARM64) SIMD acceleration. There is no GPU dependency, no CUDA requirement, and no driver compatibility concern.',
    },
    {
      question: 'Can SFSVC operate offline?',
      answer:
        'Yes. SFSVC is designed for offline operation. License activation uses Ed25519 offline verification. Model bundles, runtime, and evidence storage are self-contained. No network connectivity is required for inference or evidence retention.',
    },
    {
      question: 'What defect types does SFSVC classify?',
      answer:
        'SFSVC classifies structural surface defects including cracks, spalling, delamination, corrosion, and surface anomalies. Defect types and severity grades are configurable per deployment context through the model bundle.',
    },
    {
      question: 'How is evidence integrity maintained?',
      answer:
        'Every telemetry record is chain-sealed — each entry includes the SHA-256 hash of the previous entry in canonical JSON form. The evidence chain is append-only and tamper-evident. Replay verification confirms output correctness from sealed session logs.',
    },
  ],
};
