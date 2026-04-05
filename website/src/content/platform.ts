import type { PageContent } from '../types/content';

export const platformContent: PageContent = {
  hero: {
    headline: 'NEPA — Neuromorphic Edge Perception Architecture',
    subheadline: 'Deterministic Inspection Infrastructure',
    description:
      'NEPA is a deterministic edge runtime for infrastructure inspection, surveillance, and robotic autonomy. Every processing lane operates under bounded latency constraints. Every output is replay-verifiable. Every evidence artifact is retained under a governance-aware chain of custody with cryptographic integrity. CPU-native. Offline-capable. Enterprise-auditable.',
    primaryCta: { label: 'Request Technical Briefing', href: '/contact' },
    secondaryCta: { label: 'View Products', href: '/products' },
  },
  sections: [
    {
      id: 'what-deterministic-means',
      eyebrow: 'Foundation',
      title: 'What Deterministic Inspection Infrastructure Means',
      description:
        'Most edge inference systems operate as statistical black boxes — they ingest data, produce outputs, and offer no structural guarantee that results are reproducible, auditable, or independently verifiable. NEPA rejects this model. Deterministic inspection infrastructure means that every frame processed, every classification emitted, and every telemetry artifact produced can be traced from raw input through model state to final output. Replay verification is not a feature — it is an architectural invariant. The same input, the same model snapshot, the same runtime configuration will always produce the same output. This property enables third-party audit, regulatory evidence submission, and operational accountability at a level that probabilistic inference pipelines cannot provide.',
      items: [
        {
          title: 'Reproducible Outputs',
          description:
            'Given identical inputs and model state, NEPA produces bit-identical outputs across runs, hardware platforms, and deployment environments. No floating-point non-determinism from GPU scheduling. No order-dependent aggregation. No hidden state mutation between frames.',
        },
        {
          title: 'Structural Traceability',
          description:
            'Every processing decision is logged with sufficient context to reconstruct the full inference path. Model version, runtime configuration, input digest, and output classification are bound together in a verifiable evidence record.',
        },
        {
          title: 'Independent Verification',
          description:
            'Third parties — auditors, regulators, counterparties — can verify processing outputs without access to the live runtime. Sealed session logs and the replay verification tool are sufficient to confirm output correctness.',
        },
      ],
    },
    {
      id: 'bounded-latency',
      eyebrow: 'Runtime',
      title: 'Parallel Processing and Bounded Latency',
      description:
        'NEPA processing lanes are isolated, pre-allocated, and time-bounded. Each lane handles a single sensor stream with no shared mutable state, no dynamic memory allocation in the hot path, and no lock contention. Frame-level classification completes within hard latency bounds — P95 under 0.5ms, P99 under 0.8ms at 720p resolution on commodity x86_64 CPUs. This is achieved through pre-allocated buffer pools, SIMD-accelerated frame differencing (AVX2 on x86_64, NEON on ARM64), and lock-free inter-stage communication using single-producer single-consumer queues. There is no garbage collector. There is no GPU scheduler. There is no unbounded syscall in the frame path.',
      items: [
        {
          title: 'Pre-Allocated Processing Lanes',
          description:
            'All buffers, queues, and working memory are allocated at startup. No heap allocation occurs during frame processing. Memory layout is deterministic and cache-friendly, with aligned SIMD buffers for vectorized operations.',
        },
        {
          title: 'SIMD-Accelerated Frame Differencing',
          description:
            'Temporal frame differences are computed using 256-bit SIMD operations — AVX2 on x86_64, NEON on ARM64. The inner loop processes 32 bytes per cycle with fused multiply-add instructions, achieving 8–12× throughput over scalar implementations.',
        },
        {
          title: 'Lock-Free Communication',
          description:
            'Inter-stage data flow uses single-producer single-consumer lock-free queues. No mutex. No condition variable. No priority inversion. Stages communicate through bounded ring buffers with atomic pointer updates.',
        },
        {
          title: 'No Shared Mutable State',
          description:
            'Each processing lane owns its buffers, model snapshot, and output state. Cross-lane coordination — when required — occurs through message passing on dedicated control channels, never through shared memory mutation.',
        },
      ],
    },
    {
      id: 'replay-verification',
      eyebrow: 'Verification',
      title: 'Replay Verification',
      description:
        'Every NEPA processing session produces a sealed session log containing the input sequence, model snapshot reference, runtime configuration, and output sequence. The replay verification tool re-executes the session from the sealed log and confirms bit-identical output. This is not a test convenience — it is the primary evidence mechanism for regulatory audit, incident investigation, and operational accountability. If a classification is disputed, the sealed session log is the authoritative record. Replay it. Verify the output. No ambiguity.',
      items: [
        {
          title: 'Sealed Session Logs',
          description:
            'Session logs capture the complete processing context: input frame digests, model version hash, runtime configuration snapshot, and output classification sequence. Logs are sealed with a chain hash — each entry references the SHA-256 hash of the previous entry in canonical JSON form.',
        },
        {
          title: 'Bit-Identical Replay',
          description:
            'The replay tool ingests a sealed session log and re-executes the processing pipeline with the referenced model snapshot and configuration. Output must be bit-identical to the original session. Any divergence indicates tampering, configuration drift, or runtime defect.',
        },
        {
          title: 'Third-Party Verifiability',
          description:
            'Replay verification requires only the sealed session log, the referenced model artifact, and the replay tool binary. No access to the live runtime, production infrastructure, or internal systems is required. Auditors and regulators receive a self-contained verification package.',
        },
      ],
    },
    {
      id: 'audit-chain-of-custody',
      eyebrow: 'Governance',
      title: 'Audit and Chain-of-Custody',
      description:
        'Infrastructure inspection produces evidence that enters regulatory, legal, and operational review processes. NEPA treats evidence integrity as a structural requirement. Every classification, every telemetry artifact, every model update decision is recorded in an append-only audit chain with cryptographic integrity. The chain-of-custody model ensures that evidence provenance is traceable from field capture through processing to final archival — with no gaps, no silent mutations, and no ambiguous ownership transitions.',
      items: [
        {
          title: 'Append-Only Audit Chain',
          description:
            'Audit records are written to an append-only hash chain. Each record includes the SHA-256 hash of the previous record in canonical JSON form. Insertion, deletion, or reordering of records breaks the chain and is independently detectable.',
        },
        {
          title: 'Model Provenance',
          description:
            'Every model artifact deployed to the edge is traceable to a versioned bundle with a known build hash, promotion history, and approval chain. No model runs in production without a signed release tag and a recorded promotion decision.',
        },
        {
          title: 'Evidence Lifecycle',
          description:
            'Evidence artifacts follow a defined lifecycle: capture → seal → retain → archive → dispose. Retention policies are enforced at the storage layer. Disposal requires explicit authorization and produces its own audit record.',
        },
      ],
    },
    {
      id: 'evidence-immutability',
      eyebrow: 'Evidence',
      title: 'Evidence Immutability',
      description:
        'Structured telemetry — defect classifications, anomaly scores, sensor readings, navigation decisions — is retained as immutable evidence records. Each record carries a cryptographic digest, a chain reference, and sufficient metadata to reconstruct the processing context. Evidence immutability is not a policy preference. It is enforced at the storage layer through append-only writes, hash chain validation, and integrity checks on read.',
      items: [
        {
          title: 'Cryptographic Integrity',
          description:
            'Every evidence record includes a SHA-256 digest computed over its canonical JSON representation. Records are independently verifiable — no trust relationship with the producing runtime is required to confirm integrity.',
        },
        {
          title: 'Structured Telemetry Retention',
          description:
            'Telemetry is retained in structured, queryable form — not as opaque binary blobs. Defect type, severity grade, zone identifier, anomaly score, model version, and timestamp are first-class fields in every evidence record.',
        },
        {
          title: 'Immutable Storage Enforcement',
          description:
            'Evidence volumes are configured as append-only at the filesystem or storage service layer. Application code cannot overwrite or delete existing records. Retention and disposal are governed by infrastructure policy, not application logic.',
        },
      ],
    },
    {
      id: 'enterprise-integration',
      eyebrow: 'Integration',
      title: 'Enterprise Integration',
      description:
        'NEPA is designed for deployment within enterprise infrastructure environments — not as a standalone appliance, but as a composable edge layer that integrates with existing asset management, compliance, and operational technology systems. API surfaces, deployment models, and configuration management follow enterprise expectations for versioning, authentication, and auditability.',
      items: [
        {
          title: 'Compliance Posture',
          description:
            'NEPA supports configurable compliance profiles that map audit chain behavior, evidence retention periods, and access control policies to organizational and regulatory requirements. Compliance configuration is versioned and auditable.',
        },
        {
          title: 'API Surfaces',
          description:
            'Edge modules expose gRPC and REST API surfaces for telemetry retrieval, configuration management, health monitoring, and evidence export. API versioning follows semver conventions. Authentication uses mTLS or API key schemes depending on deployment context.',
        },
        {
          title: 'Deployment Models',
          description:
            'NEPA supports air-gapped deployment with offline activation, connected deployment with periodic uplink, and hybrid models with local processing and selective cloud synchronization. Deployment topology is configured at provisioning time and enforced by the runtime.',
        },
      ],
    },
  ],
};
