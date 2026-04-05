import type { PageContent } from '../types/content';

export const platformContent: PageContent = {
  hero: {
    subheadline: 'Deterministic Edge Middleware for Critical Infrastructure',
    headline: 'Perceive. Percept. Process.',
    description:
      'NEPA is the middleware layer between raw sensor input and structured operational output. Bounded latency. Replay-verified evidence. Offline-capable. Audit-ready.',
    primaryCta: { label: 'Request Technical Briefing', href: '/contact' },
    secondaryCta: { label: 'View Products', href: '/products' },
  },
  sections: [
    {
      id: 'what-deterministic-means',
      eyebrow: 'Foundation',
      title: 'What Deterministic Inspection Infrastructure Means',
      description:
        'Every frame, classification, and artifact is traceable from input through model state to output. Replay verification is an architectural invariant enabling third-party audit and regulatory submission.',
      items: [
        {
          title: 'Reproducible Outputs',
          description:
            'Bit-identical outputs across runs and platforms given the same inputs and model state. No GPU non-determinism, no hidden state mutation.',
        },
        {
          title: 'Structural Traceability',
          description:
            'Every processing decision is logged with model version, configuration, input digest, and output — bound together in a verifiable evidence record.',
        },
        {
          title: 'Independent Verification',
          description:
            'Auditors and regulators verify outputs without live runtime access. Sealed session logs and the replay tool confirm correctness independently.',
        },
      ],
    },
    {
      id: 'enterprise-integration',
      eyebrow: 'Integration',
      title: 'Enterprise Integration',
      description:
        'Composable edge layer integrating with existing asset management, compliance, and OT systems. Enterprise-grade API versioning, authentication, and auditability.',
      items: [
        {
          title: 'Compliance Posture',
          description:
            'Configurable compliance profiles mapping audit behavior, retention, and access control to regulatory requirements. Versioned and auditable configuration.',
        },
        {
          title: 'API Surfaces',
          description:
            'gRPC and REST APIs for telemetry, configuration, health, and evidence export. Semver versioning with mTLS or API key authentication.',
        },
        {
          title: 'Deployment Models',
          description:
            'Air-gapped, connected, or hybrid deployment models. Offline activation, periodic uplink, or selective cloud sync — configured at provisioning time.',
        },
      ],
    },
  ],
};
