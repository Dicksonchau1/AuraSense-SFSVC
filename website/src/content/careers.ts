import type { PageContent } from '../types/content';

const whatWeWorkOnSection = {
  id: 'what-we-work-on',
  eyebrow: 'Work',
  title: 'What We Work On',
  description:
    'Deterministic edge intelligence for critical infrastructure — neuromorphic computing, edge inference, robotic autonomy, surveillance, and governance. Everything runs on real hardware under real constraints.',
  items: [
    {
      title: 'Neuromorphic Computing',
      description:
        'Spike-based encoding and event-driven perception deployed on commodity CPUs with deterministic timing. Production systems, not academic exercises.',
    },
    {
      title: 'Edge Inference and Low-Latency Processing',
      description:
        'SIMD-accelerated pipelines, lock-free queues, pre-allocated buffers. P95 < 0.5ms classification on x86_64. Cache-aware, branch-free, zero hot-path allocation.',
    },
    {
      title: 'Robotic Autonomy and Navigation',
      description:
        'Event camera ingestion, spike-encoded sensor fusion, bounded-latency planning, watchdog safety. GPS-denied and confined environments. Safety is structural.',
    },
    {
      title: 'Surveillance Intelligence',
      description:
        'Multi-sensor temporal correlation, alert orchestration, severity tiering, evidence retention. Actionable alerts with full chain-of-custody evidence packages.',
    },
    {
      title: 'Deterministic Systems and Governance',
      description:
        'Replay verification, chain-sealed audit logs, model provenance, versioned bundle promotion. Every output is replay-verifiable and evidence-grade.',
    },
    {
      title: 'Infrastructure and Deployment',
      description:
        'Offline deployment, Ed25519 activation, air-gapped operation, multi-site coordination. Deploy on drones, robots, surveillance, and gateways. Deployment engineering is first-class.',
    },
  ],
};

const openRolesSection = {
  id: 'open-roles',
  eyebrow: 'Roles',
  title: 'Open Roles',
  description:
    'Selectively hiring in systems, robotics, edge AI, CV, and product engineering. We hire when backgrounds align with our work and technical depth.',
  items: [
    {
      title: 'How to Apply',
      description:
        'Send a short note with portfolio or GitHub to careers@aurasensehk.com. Tell us what you built and why AuraSense fits.',
    },
    {
      title: 'What to Include',
      description:
        'Technical background, links to work (GitHub, papers, portfolios), and which domain interests you most. No formal resume required.',
    },
    {
      title: 'Location and Structure',
      description:
        'Hong Kong based with distributed team. Remote supported where effective. Some roles require physical presence. Competitive compensation.',
    },
  ],
};

export const careersPageContent: PageContent = {
  hero: {
    headline: 'Build Systems That Survive Contact with Reality',
    subheadline: 'Engineering at AuraSense',
    description:
      'We build real edge infrastructure for inspection, surveillance, and autonomy. If you ship reliable systems under operational constraints, not demos, we should talk.',
    primaryCta: { label: 'View Open Roles', href: '#open-roles' },
    secondaryCta: { label: 'Contact Careers', href: 'mailto:careers@aurasensehk.com' },
  },
  sections: [
    whatWeWorkOnSection,
    openRolesSection,
  ],
};
