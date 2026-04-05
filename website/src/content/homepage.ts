import type { PageContent, SectionContent } from '../types/content';

const whyDeterministicSection: SectionContent = {
  id: 'why-deterministic',
  eyebrow: 'Foundation',
  title: 'Why Deterministic Edge Intelligence',
  description:
    'NEPA enforces deterministic processing — every frame, decision, and artifact is traceable from ingestion to evidence retention. Structural transparency, not optional.',
  items: [
    {
      title: 'Replay Verification',
      description:
        'Sealed session logs enable bit-identical replay from raw input to final output. Third parties verify results independently without runtime access.',
    },
    {
      title: 'Bounded Latency',
      description:
        'Frame classification in P95 < 0.5ms, P99 < 0.8ms at 720p on commodity CPUs. No GC pauses, no GPU jitter, no hot-path heap allocation.',
    },
    {
      title: 'Evidence Immutability',
      description:
        'Classification outputs are chain-sealed in append-only storage. Each entry references the previous hash, forming a tamper-evident, independently verifiable sequence.',
    },
    {
      title: 'Governance Architecture',
      description:
        'Model updates propagate through versioned pipelines with human approval gates. No autonomous promotion. Every configuration traces to a signed release artifact.',
    },
  ],
};

const productModulesSection: SectionContent = {
  id: 'product-modules',
  eyebrow: 'Products',
  title: 'Product Modules',
  description:
    'Composable edge-native modules for inspection, navigation, and surveillance — sharing the same deterministic runtime, evidence retention, and governance model.',
  items: [
    {
      title: 'SFSVC — Spike-based Facade Surface Vision Codec',
      description:
        'Neuromorphic compression and perception SDK. CPU-only spike-train classification with AVX2/NEON SIMD. Structured defect telemetry for facades, bridges, and runways.',
      href: '/products/sfsvc',
      cta: { label: 'View SFSVC', href: '/products/sfsvc' },
    },
    {
      title: 'NERMN — Neuromorphic Edge Robotic Navigation Module',
      description:
        'Edge-native navigation for robotic inspection platforms. Spike-encoded sensor pipeline with bounded-latency path decisions and watchdog-supervised safety for GPS-denied environments.',
      href: '/products/nermn',
      cta: { label: 'View NERMN', href: '/products/nermn' },
    },
    {
      title: 'NSSIM — Neuromorphic Smart Surveillance Intelligence Module',
      description:
        'Multi-sensor surveillance with temporal correlation and alert orchestration. Processes camera, acoustic, and environmental feeds into auditable alerts with chain-of-custody evidence.',
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
    'Deploys across edge form factors — drones to fixed gateways. CPU-native, offline-capable, no cloud required for inference or evidence retention.',
  items: [
    {
      title: 'Drones & UAVs',
      description:
        'Airborne platforms running SFSVC or NERMN for surface scanning and navigation. Onboard evidence retention with post-flight replay verification.',
    },
    {
      title: 'Fixed Cameras',
      description:
        'Stationary NSSIM installations for continuous multi-sensor surveillance at perimeters and critical assets. Long-duration unattended operation with local evidence storage.',
    },
    {
      title: 'Robotic Platforms',
      description:
        'Ground and rail-mounted robots running NERMN for autonomous traversal of tunnels and pipelines. Spike-encoded inference for real-time obstacle avoidance.',
    },
    {
      title: 'Site Gateways',
      description:
        'Edge gateways aggregating telemetry from field devices. Local model management, evidence consolidation, and enterprise uplink on commodity x86_64 hardware.',
    },
  ],
};

const evidenceGovernanceSection: SectionContent = {
  id: 'evidence-governance',
  eyebrow: 'Governance',
  title: 'Evidence & Governance',
  description:
    'NEPA treats evidence integrity as a first-class architectural requirement — built for regulatory scrutiny, legal discovery, and operational audit.',
  items: [
    {
      title: 'Chain Sealing',
      description:
        'Every record is sealed into an append-only hash chain using SHA-256 canonical JSON. Tamper-evident and independently verifiable without runtime access.',
    },
    {
      title: 'Tamper-Evident Logs',
      description:
        'Canonical JSON logs with lexicographic ordering. Any modification breaks the hash chain and is detectable without specialized forensic tools.',
    },
    {
      title: 'Append-Only Storage',
      description:
        'Write-once storage with no in-place mutation or deletion. Retention policies enforced at the storage layer, governed by organizational policy.',
    },
    {
      title: 'Compliance Architecture',
      description:
        'Configurable compliance from internal audits to regulatory packages. Bundle versioning and promotion gates provide complete provenance from development to deployment.',
    },
  ],
};

const ctaSection: SectionContent = {
  id: 'cta',
  title: 'Ready to Evaluate?',
  description:
    'Technical briefings, architecture reviews, and evaluation access for infrastructure operators, integrators, and enterprise engineering teams.',
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
        'Schedule a technical conversation with AuraSense engineering about your deployment context and evaluation criteria.',
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
    headline: 'Perceive. Percept. Process.',
    subheadline: 'NEPA — Neuromorphic Edge Perception Architecture',
    description:
      'CPU-native. Offline-capable. Infrastructure inspection, surveillance, and robotic autonomy.',
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
