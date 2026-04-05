import type { PageContent, ProductSummary, SectionContent } from '../types/content';

export const productSummaries: ProductSummary[] = [
  {
    id: 'sfsvc',
    name: 'SFSVC',
    tagline: 'Spike-based Facade Surface Vision Codec',
    description:
      'Neuromorphic compression and perception SDK for edge infrastructure inspection. SFSVC encodes temporal frame differences as spike trains, runs classification on CPU with SIMD acceleration, and produces structured defect telemetry with full evidence retention. Designed for facade inspection, bridge assessment, runway monitoring, and structural surface analysis. No GPU required. Offline activation with Ed25519 license verification.',
    href: '/products/sfsvc',
    features: [
      'Spike-based temporal compression',
      'CPU-native with AVX2/NEON SIMD',
      'Structured defect telemetry output',
      'Offline Ed25519 license activation',
      'Replay-verified evidence chain',
      'Sub-millisecond frame classification',
    ],
  },
  {
    id: 'nermn',
    name: 'NERMN',
    tagline: 'Neuromorphic Edge Robotic Navigation Module',
    description:
      'Edge-native navigation stack for robotic inspection platforms and autonomous vehicles operating in GPS-denied, degraded, or confined environments. NERMN ingests event camera and LiDAR data through a spike-encoded sensor pipeline, produces bounded-latency path decisions, and enforces watchdog-supervised safety constraints. Designed for tunnel crawlers, pipeline robots, structural inspection drones, and confined-space autonomy.',
    href: '/products/nermn',
    features: [
      'Event camera and LiDAR sensor fusion',
      'Spike-encoded sensor-to-decision pipeline',
      'Watchdog-supervised safety constraints',
      'Degraded-safe fallback policy',
      'GPS-denied navigation capability',
      'Bounded-latency path planning',
    ],
  },
  {
    id: 'nssim',
    name: 'NSSIM',
    tagline: 'Neuromorphic Smart Surveillance Intelligence Module',
    description:
      'Multi-sensor surveillance intelligence with temporal event correlation, alert orchestration, and evidence retention. NSSIM processes camera, acoustic, and environmental sensor feeds through a spike-based temporal reasoning engine. Produces actionable alerts with full audit trails and chain-of-custody evidence packages for regulatory and operational review. Designed for perimeter security, critical asset monitoring, and multi-site surveillance operations.',
    href: '/products/nssim',
    features: [
      'Multi-sensor temporal event correlation',
      'Spike-based temporal reasoning engine',
      'Alert orchestration with severity tiering',
      'Chain-of-custody evidence packages',
      'Multi-site deployment coordination',
      'Continuous unattended operation',
    ],
  },
];

const comparisonSection: SectionContent = {
  id: 'comparison',
  eyebrow: 'Comparison',
  title: 'Module Comparison',
  description:
    'Each NEPA module addresses a distinct operational domain with purpose-built sensor ingestion, inference, and evidence pipelines. All modules share the deterministic runtime core, governance model, and replay verification architecture.',
  items: [
    {
      title: 'Primary Domain',
      description:
        'SFSVC targets structural surface inspection and defect classification. NERMN targets autonomous navigation and path planning for robotic platforms. NSSIM targets multi-sensor surveillance intelligence and alert orchestration.',
    },
    {
      title: 'Sensor Inputs',
      description:
        'SFSVC ingests conventional camera frames and computes temporal differences. NERMN ingests event cameras and LiDAR with spike-encoded fusion. NSSIM ingests camera, acoustic, and environmental sensor feeds with temporal correlation.',
    },
    {
      title: 'Output Type',
      description:
        'SFSVC produces structured defect telemetry — type, severity, zone, anomaly score. NERMN produces navigation commands — heading, velocity, obstacle avoidance decisions. NSSIM produces alert packages — event classification, severity tier, evidence bundle.',
    },
    {
      title: 'Deployment Profile',
      description:
        'SFSVC deploys on inspection drones, fixed cameras, and edge gateways. NERMN deploys on robotic platforms with real-time control requirements. NSSIM deploys on fixed surveillance installations and multi-site gateway networks.',
    },
    {
      title: 'Safety Model',
      description:
        'SFSVC operates in advisory mode — classification outputs inform human operators. NERMN enforces watchdog-supervised safety with degraded-safe fallback. NSSIM operates in alert-and-retain mode with human-in-the-loop escalation.',
    },
  ],
};

const deploymentContextSection: SectionContent = {
  id: 'deployment-context',
  eyebrow: 'Deployment',
  title: 'Deployment Context',
  description:
    'NEPA modules are designed for deployment in environments where reliability, auditability, and offline capability are operational requirements — not optional features. Typical deployment contexts include infrastructure inspection programs, autonomous robotic operations, and critical facility surveillance.',
  items: [
    {
      title: 'Infrastructure Inspection',
      description:
        'Bridge decks, building facades, runway surfaces, tunnel linings, pipeline interiors. SFSVC and NERMN work together to scan, classify, and navigate structural environments with full evidence retention for engineering review and regulatory submission.',
    },
    {
      title: 'Autonomous Robotic Operations',
      description:
        'Confined-space robots, inspection drones, rail-mounted crawlers. NERMN provides bounded-latency navigation with watchdog supervision. SFSVC provides onboard classification. Both produce replay-verified evidence chains for post-mission audit.',
    },
    {
      title: 'Critical Facility Surveillance',
      description:
        'Perimeter security, access control monitoring, critical asset observation. NSSIM provides continuous multi-sensor intelligence with temporal event correlation, severity-tiered alerts, and evidence packages suitable for regulatory and legal review.',
    },
    {
      title: 'Multi-Site Operations',
      description:
        'Distributed infrastructure programs with multiple inspection and surveillance sites. Site gateways aggregate telemetry, manage model bundles, and coordinate evidence consolidation across field devices with selective uplink to enterprise systems.',
    },
  ],
};

export const productsPageContent: PageContent = {
  hero: {
    headline: 'Edge-Native Intelligence Modules',
    subheadline: 'Composable Perception for Critical Infrastructure',
    description:
      'AuraSense ships three edge-native intelligence modules — each addressing a distinct operational domain while sharing the NEPA deterministic runtime, evidence retention layer, and governance model. Modules deploy independently or together across inspection, navigation, and surveillance use cases. CPU-native. Offline-capable. Replay-verified.',
    primaryCta: { label: 'Request Product Briefing', href: '/contact' },
    secondaryCta: { label: 'View Platform', href: '/platform' },
  },
  sections: [comparisonSection, deploymentContextSection],
};
