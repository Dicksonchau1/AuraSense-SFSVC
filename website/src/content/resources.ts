import type { PageContent, ResourceEntry, SectionContent } from '../types/content';

export const resourceCategories: string[] = [
  'Platform Documentation',
  'Product Documentation',
  'Integration Guides',
  'Deployment Notes',
  'Licensing & Legal',
  'FAQ & Support',
  'Architecture Briefs',
  'Product Briefings',
];

export const featuredResources: ResourceEntry[] = [
  {
    title: 'NEPA Platform Overview',
    description:
      'Architecture brief covering the NEPA deterministic runtime, processing model, evidence retention layer, and governance architecture. Intended for technical leadership and enterprise evaluators assessing platform suitability for infrastructure inspection, surveillance, or robotic autonomy programs. Includes processing model diagrams, evidence chain architecture, and deployment topology examples.',
    type: 'Architecture brief',
    audience: 'Enterprise evaluators',
    cta: { label: 'Request Access', href: '/contact' },
  },
  {
    title: 'SFSVC Deployment Guide',
    description:
      'Step-by-step integration guide for deploying the SFSVC SDK on Linux x86_64 and ARM64 targets. Covers installation, license activation, model bundle loading, frame processing pipeline initialization, telemetry retrieval, and evidence export. Includes code examples in C and Python, platform-specific build instructions, and troubleshooting reference.',
    type: 'Integration guide',
    audience: 'Engineering teams',
    cta: { label: 'View Guide', href: '/resources/sfsvc-deployment-guide' },
  },
  {
    title: 'SFSVC Licensing & Activation FAQ',
    description:
      'Comprehensive FAQ covering SFSVC license types, offline Ed25519 activation workflow, hardware fingerprint binding, evaluation license terms, production deployment licensing, volume agreements, and renewal procedures. Addresses common integration questions about activation state persistence, multi-device licensing, and air-gapped deployment.',
    type: 'FAQ',
    audience: 'Engineering teams',
    cta: { label: 'Read FAQ', href: '/faq' },
  },
  {
    title: 'NERMN Navigation Stack Brief',
    description:
      'Product brief covering the NERMN navigation architecture — sensor ingestion, spike encoding, path planning, safety supervision, and platform integration. Includes pipeline diagrams, latency specifications, watchdog architecture, and deployment profiles for inspection drones, ground robots, and rail-mounted crawlers. Suitable for robotics engineers evaluating edge navigation solutions.',
    type: 'Product brief',
    audience: 'Robotics engineers',
    cta: { label: 'Request Access', href: '/contact' },
  },
  {
    title: 'NSSIM Surveillance Intelligence Overview',
    description:
      'Product brief covering NSSIM multi-sensor intelligence — temporal event correlation, alert orchestration, evidence retention, and multi-site deployment. Includes intelligence pipeline architecture, temporal reasoning model, alert severity framework, and evidence chain-of-custody workflow. Designed for security operations teams and surveillance infrastructure evaluators.',
    type: 'Product brief',
    audience: 'Security teams',
    cta: { label: 'Request Access', href: '/contact' },
  },
  {
    title: 'Edge Deployment Reference Architecture',
    description:
      'Architecture brief covering NEPA edge deployment topologies — standalone edge devices, site gateway aggregation, multi-site coordination, and enterprise uplink. Includes hardware specifications, network architecture, storage planning, model management, and operational monitoring. Intended for infrastructure engineers planning NEPA deployments across inspection, surveillance, and robotic platforms.',
    type: 'Architecture brief',
    audience: 'Infrastructure engineers',
    cta: { label: 'Request Access', href: '/contact' },
  },
  {
    title: 'Governance and Replay Verification Brief',
    description:
      'Governance documentation covering the NEPA evidence integrity model — chain-sealed audit logs, replay verification architecture, model provenance tracking, and bundle promotion pipeline. Explains how deterministic processing, sealed session logs, and cryptographic evidence chains satisfy regulatory audit, legal discovery, and operational accountability requirements.',
    type: 'Governance document',
    audience: 'Enterprise evaluators',
    cta: { label: 'Request Access', href: '/contact' },
  },
  {
    title: 'Integration API Reference',
    description:
      'Technical reference for NEPA module API surfaces — gRPC and REST endpoints for telemetry retrieval, configuration management, health monitoring, and evidence export. Covers authentication schemes (mTLS, API key), request/response formats, error codes, pagination, and versioning conventions. Available to engineering teams under evaluation or deployment agreement.',
    type: 'Technical documentation',
    audience: 'Engineering teams',
    cta: { label: 'Contact for Docs', href: '/contact' },
  },
];

export const audienceSegments: { title: string; description: string }[] = [
  {
    title: 'Engineering Teams',
    description:
      'Integration guides, API references, deployment notes, and SDK documentation for engineers building on or integrating with NEPA modules. Covers C/C++ headers, Python bindings, build system configuration, and platform-specific deployment instructions.',
  },
  {
    title: 'Enterprise Evaluators',
    description:
      'Architecture briefs, platform overviews, governance documentation, and compliance materials for technical leadership assessing NEPA suitability. Covers processing model, evidence integrity, deployment topology, and regulatory compliance posture.',
  },
  {
    title: 'Security & Operations',
    description:
      'Product briefs, deployment architecture, and operational documentation for security operations teams and infrastructure operators. Covers surveillance intelligence, alert orchestration, evidence handling, and multi-site coordination.',
  },
  {
    title: 'Partners & Integrators',
    description:
      'Integration architecture, co-deployment guides, and partnership materials for system integrators and technology partners. Covers API surfaces, deployment models, and joint solution architectures for inspection, surveillance, and robotic autonomy programs.',
  },
];

const categoriesSection: SectionContent = {
  id: 'categories',
  eyebrow: 'Library',
  title: 'Resource Categories',
  description:
    'Resources are organized by type and function. Platform documentation covers the NEPA runtime and architecture. Product documentation covers individual modules. Integration guides provide step-by-step deployment instructions. Architecture briefs provide high-level technical overviews for evaluation and planning.',
  items: [
    {
      title: 'Platform Documentation',
      description:
        'NEPA architecture, processing model, evidence retention, governance, and replay verification. Foundation documentation that applies across all product modules.',
    },
    {
      title: 'Product Documentation',
      description:
        'Module-specific documentation for SFSVC, NERMN, and NSSIM. Covers architecture, API reference, configuration, deployment, and operational guidance for each module.',
    },
    {
      title: 'Integration Guides',
      description:
        'Step-by-step guides for deploying NEPA modules on target hardware, integrating with existing systems, and configuring operational parameters. Includes code examples and platform-specific instructions.',
    },
    {
      title: 'Architecture Briefs',
      description:
        'High-level technical overviews intended for evaluation and planning. Covers processing model, deployment topology, evidence architecture, and governance model without implementation-level detail.',
    },
    {
      title: 'Licensing & Legal',
      description:
        'License activation guides, terms of service, privacy policy, and compliance documentation. Covers offline activation workflow, hardware fingerprint binding, and deployment licensing.',
    },
    {
      title: 'FAQ & Support',
      description:
        'Frequently asked questions, troubleshooting guides, and support contact information. Organized by product module and deployment context.',
    },
  ],
};

const audienceSection: SectionContent = {
  id: 'audience',
  eyebrow: 'Audience',
  title: 'Resources by Audience',
  description:
    'Not all resources are relevant to all readers. We organize materials by audience so that engineers find integration details, evaluators find architecture overviews, and operators find deployment guidance — without wading through documentation intended for a different role.',
  items: audienceSegments.map((segment) => ({
    title: segment.title,
    description: segment.description,
  })),
};

const supportModelSection: SectionContent = {
  id: 'support',
  eyebrow: 'Support',
  title: 'Support Model',
  description:
    'AuraSense provides tiered support depending on engagement stage and deployment context. Public documentation is available on the website. Guided onboarding is provided during evaluation periods. Enterprise support agreements cover production deployments.',
  items: [
    {
      title: 'Public Documentation',
      description:
        'Product overviews, architecture briefs, and FAQ materials are available on the AuraSense website without registration. These resources provide sufficient information to assess platform suitability and begin evaluation planning.',
    },
    {
      title: 'Guided Onboarding',
      description:
        'Organizations in active evaluation receive guided onboarding — architecture walkthrough, integration planning, deployment configuration review, and direct access to engineering for technical questions. Onboarding is scoped to the evaluation agreement.',
    },
    {
      title: 'Enterprise Briefings',
      description:
        'Technical briefings for enterprise evaluation teams cover platform architecture, evidence integrity model, governance pipeline, and deployment topology in depth. Briefings are scheduled through the contact page and conducted by AuraSense engineering.',
      cta: { label: 'Schedule Briefing', href: '/contact' },
    },
    {
      title: 'Production Support',
      description:
        'Production deployments are covered by enterprise support agreements with defined SLAs, escalation paths, and engineering access. Support scope includes runtime issues, model bundle management, evidence integrity questions, and deployment configuration assistance. Contact support@aurasensehk.com for production support inquiries.',
    },
  ],
};

export const resourcesPageContent: PageContent = {
  hero: {
    headline: 'Technical Resources and Evaluation Materials',
    subheadline: 'Documentation, Guides, and Architecture Briefs',
    description:
      'AuraSense publishes platform documentation, deployment guides, product briefs, and governance materials for evaluators, engineers, and partners. Resources are organized by audience and function — from high-level architecture overviews for enterprise evaluators to integration references for engineering teams deploying NEPA modules in production.',
    primaryCta: { label: 'Request Access', href: '/contact' },
    secondaryCta: { label: 'Contact Engineering', href: '/contact' },
  },
  sections: [categoriesSection, audienceSection, supportModelSection],
};
