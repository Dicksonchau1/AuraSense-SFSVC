import type { PageContent, SectionContent } from '../types/content';

const whyDeterministicSection: SectionContent = {
  id: 'why-deterministic',
  eyebrow: 'Foundation',
  title: 'Why Deterministic Edge Intelligence',
  description:
    'Conventional inference pipelines produce outputs that cannot be independently verified, replayed, or audited under operational constraints. NEPA enforces deterministic processing boundaries — every frame, every decision, every telemetry artifact is traceable from ingestion through classification to evidence retention. This is not optional transparency. It is structural.',
  items: [
    {
      title: 'Replay Verification',
      description:
        'Every processing session can be replayed from raw input to final output using sealed session logs. Replay produces bit-identical results, enabling third-party verification without access to the runtime itself. Audit teams, regulators, and counterparties can independently confirm that outputs match inputs under the same model state.',
    },
    {
      title: 'Bounded Latency',
      description:
        'Processing lanes operate under hard time budgets with no shared mutable state. Frame-level classification completes within deterministic latency bounds — P95 under 0.5ms, P99 under 0.8ms at 720p on commodity x86_64 CPUs. No garbage collection pauses, no GPU scheduling jitter, no unbounded memory allocation in the hot path.',
    },
    {
      title: 'Evidence Immutability',
      description:
        'Structured telemetry and classification outputs are written to append-only storage with cryptographic integrity. Every evidence record is chain-sealed — each entry references the hash of the previous entry, forming a tamper-evident sequence. Extraction or modification of individual records is structurally detectable.',
    },
    {
      title: 'Governance Architecture',
      description:
        'Model updates, threshold changes, and deployment configurations propagate through a versioned promotion pipeline with human approval gates. No autonomous model promotion. No silent parameter drift. Every runtime configuration is traceable to a signed release artifact with a known provenance chain.',
    },
  ],
};

const productModulesSection: SectionContent = {
  id: 'product-modules',
  eyebrow: 'Products',
  title: 'Product Modules',
  description:
    'NEPA ships as composable edge-native modules. Each module addresses a distinct operational domain — inspection, navigation, or surveillance — while sharing the same deterministic runtime, evidence retention layer, and governance model.',
  items: [
    {
      title: 'SFSVC — Spike-based Facade Surface Vision Codec',
      description:
        'Neuromorphic compression and perception SDK for edge inspection. Encodes temporal frame differences as spike trains, runs classification entirely on CPU with AVX2/NEON SIMD acceleration, and produces structured defect telemetry. No GPU required. Offline activation with Ed25519 license verification. Targets infrastructure facades, bridges, runways, and structural surfaces.',
      href: '/products/sfsvc',
      cta: { label: 'View SFSVC', href: '/products/sfsvc' },
    },
    {
      title: 'NERMN — Neuromorphic Edge Robotic Navigation Module',
      description:
        'Edge-native navigation stack for robotic platforms and autonomous inspection vehicles. Ingests event camera and LiDAR data through a spike-encoded sensor pipeline, produces bounded-latency path decisions, and enforces watchdog-supervised safety constraints. Designed for GPS-denied and degraded environments where reliable fallback behavior is non-negotiable.',
      href: '/products/nermn',
      cta: { label: 'View NERMN', href: '/products/nermn' },
    },
    {
      title: 'NSSIM — Neuromorphic Smart Surveillance Intelligence Module',
      description:
        'Multi-sensor surveillance intelligence with temporal event correlation, alert orchestration, and evidence retention. Processes camera, acoustic, and environmental sensor feeds through a spike-based temporal reasoning engine. Produces actionable alerts with full audit trails and chain-of-custody evidence packages for regulatory and operational review.',
      href: '/products/nssim',
      cta: { label: 'View NSSIM', href: '/products/nssim' },
    },
  ],
};

const deploymentInfrastructureSection: SectionContent = {
  id: 'deployment-infrastructure',
  eyebrow: 'Deployment',
  title: 'Deployment Infrastructure',
  description:
    'NEPA modules deploy across a range of edge form factors — from airborne inspection platforms to fixed-site gateways. The runtime is CPU-native and offline-capable, requiring no cloud connectivity for real-time inference or evidence retention.',
  items: [
    {
      title: 'Drones & UAVs',
      description:
        'Airborne inspection platforms running SFSVC or NERMN for structural surface scanning and autonomous navigation. Operates within payload, power, and compute constraints of commercial inspection drones. Offline-capable with onboard evidence retention and post-flight replay verification.',
    },
    {
      title: 'Fixed Cameras',
      description:
        'Stationary surveillance and monitoring installations running NSSIM for continuous multi-sensor intelligence. Deployed at infrastructure perimeters, facility access points, and critical asset monitoring stations. Supports long-duration unattended operation with local evidence storage and periodic uplink.',
    },
    {
      title: 'Robotic Platforms',
      description:
        'Ground-based and rail-mounted inspection robots running NERMN for autonomous traversal of tunnels, pipelines, and confined structural environments. Integrates event camera and LiDAR sensor payloads with spike-encoded inference for real-time obstacle avoidance and path planning.',
    },
    {
      title: 'Site Gateways',
      description:
        'Edge compute gateways aggregating telemetry from multiple field devices. Provides local model management, evidence consolidation, bundle promotion staging, and uplink to enterprise systems. Runs on commodity x86_64 hardware with no specialized accelerator requirements.',
    },
  ],
};

const evidenceGovernanceSection: SectionContent = {
  id: 'evidence-governance',
  eyebrow: 'Governance',
  title: 'Evidence & Governance',
  description:
    'Infrastructure inspection, surveillance, and autonomous operations produce evidence that must withstand regulatory scrutiny, legal discovery, and operational audit. NEPA treats evidence integrity as a first-class architectural requirement — not an afterthought bolted onto an inference pipeline.',
  items: [
    {
      title: 'Chain Sealing',
      description:
        'Every telemetry record, classification output, and session artifact is sealed into an append-only hash chain. Each entry contains the SHA-256 hash of the previous entry in canonical JSON form, producing a tamper-evident sequence that can be independently verified without access to the runtime or its keys.',
    },
    {
      title: 'Tamper-Evident Logs',
      description:
        'Session logs and audit trails are structured as canonical JSON with lexicographic key ordering and no extraneous whitespace. Any modification — insertion, deletion, or reordering — breaks the hash chain and is detectable by the replay verification tool without specialized forensic infrastructure.',
    },
    {
      title: 'Append-Only Storage',
      description:
        'Evidence artifacts are written to append-only storage volumes. No in-place mutation. No record deletion. Retention policies are enforced at the storage layer, not the application layer, ensuring that evidence lifecycle is governed by organizational policy rather than runtime behavior.',
    },
    {
      title: 'Compliance Architecture',
      description:
        'The governance model supports configurable compliance postures — from internal audit trails to regulatory evidence packages. Bundle versioning, model promotion gates, and deployment manifests provide a complete provenance chain from development through field deployment to evidence submission.',
    },
  ],
};

const ctaSection: SectionContent = {
  id: 'cta',
  title: 'Ready to Evaluate?',
  description:
    'AuraSense provides technical briefings, deployment architecture reviews, and evaluation access for qualified infrastructure operators, system integrators, and enterprise engineering teams.',
  items: [
    {
      title: 'Explore Products',
      description:
        'Review the SFSVC, NERMN, and NSSIM product modules — architecture, deployment profiles, and integration paths.',
      cta: { label: 'View Products', href: '/products' },
    },
    {
      title: 'Request a Briefing',
      description:
        'Schedule a technical conversation with AuraSense engineering to discuss your deployment context, integration requirements, and evaluation criteria.',
      cta: { label: 'Contact Engineering', href: '/contact' },
    },
    {
      title: 'Access Resources',
      description:
        'Platform documentation, deployment guides, architecture briefs, and governance materials for evaluators and integration engineers.',
      cta: { label: 'View Resources', href: '/resources' },
    },
  ],
};

export const homepageContent: PageContent = {
  hero: {
    headline: 'Deterministic Edge Intelligence for Critical Infrastructure',
    subheadline: 'Neuromorphic Edge Perception Architecture',
    description:
      'AuraSense builds NEPA — a neuromorphic edge platform engineered for infrastructure inspection, surveillance, and robotic autonomy. Every decision is bounded, every output is replay-verified, and every evidence artifact is retained under a governance-aware chain of custody. CPU-native. Offline-capable. Audit-ready.',
    primaryCta: { label: 'Request Technical Briefing', href: '/contact' },
    secondaryCta: { label: 'View Platform', href: '/platform' },
  },
  sections: [
    whyDeterministicSection,
    productModulesSection,
    deploymentInfrastructureSection,
    evidenceGovernanceSection,
    ctaSection,
  ],
};
