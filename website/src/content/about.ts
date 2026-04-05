import type { PageContent, SectionContent } from '../types/content';

const storySection: SectionContent = {
  id: 'our-story',
  eyebrow: 'Our Story',
  title: 'Why We Built NEPA',
  description:
    'AuraSense was founded in Hong Kong to solve a problem that kept appearing in drone inspection deployments: the moment a drone flies beyond line of sight, into a tunnel, or under a bridge, cloud-dependent CV pipelines fail. Latency spikes. Connectivity drops. The mission stops.',
  items: [
    {
      title: 'The Question',
      description:
        'What if the vision system worked the way a biological retina works — processing only change, not every pixel, at the edge, in real time?',
    },
    {
      title: 'The Answer',
      description:
        'SFSVC is a spike-based frequency spatial video codec that encodes video as sparse neural events, processes them through independent parallel lanes, and outputs structural defect detection in under 2 milliseconds — on a mini PC drawing 4.2 watts.',
    },
    {
      title: 'The Result',
      description:
        'No cloud. No GPU. No compromise. A deterministic, CPU-native edge runtime that works fully offline for infrastructure inspection, surveillance, and robotic autonomy.',
    },
  ],
};

const principlesSection: SectionContent = {
  id: 'principles',
  eyebrow: 'Principles',
  title: 'How We Think',
  description:
    'Four engineering principles guide every decision at AuraSense — from architecture to deployment.',
  items: [
    {
      title: '01 — Edge or Nothing',
      description:
        'If it needs the cloud, it is not done yet. Every feature we ship must work fully offline. BVLOS mission capability is a hard requirement, not a future roadmap item.',
    },
    {
      title: '02 — Biology First',
      description:
        'The retina solved this 500 million years ago. Neuromorphic computation is not a research curiosity. It is the correct architecture for low-power, high-speed visual processing. SFSVC is proof.',
    },
    {
      title: '03 — One Problem',
      description:
        'We do structural defect detection. That is it. Deep focus on one problem is how you get to 99.2% accuracy at sub-2ms latency.',
    },
    {
      title: '04 — Honest Benchmarks',
      description:
        'Real hardware. Real conditions. All performance figures are measured on production hardware — Intel N100, Raspberry Pi 5, Jetson Orin — under real inspection conditions. Not lab synthetic benchmarks.',
    },
  ],
};

const teamSection: SectionContent = {
  id: 'team',
  eyebrow: 'The Team',
  title: 'A Small Team. A Focused Mission.',
  description:
    'Engineers and researchers who believe the most important AI runs at the edge, not in a data centre. Every member has shipped hardware products and understands real physical constraints.',
  items: [
    {
      title: 'Headquartered in Hong Kong',
      description:
        'Building for Asia-Pacific infrastructure inspection from Hong Kong SAR, China.',
    },
    {
      title: 'Hiring',
      description:
        'Selectively hiring in neuromorphic computing, real-time systems, drone autonomy, and edge AI. Send portfolio or GitHub to careers@aurasensehk.com.',
      cta: { label: 'See open roles', href: '/careers' },
    },
    {
      title: 'Contact',
      description:
        'General inquiries: support@aurasensehk.com. Engineering: contact@aurasensehk.com. Careers: careers@aurasensehk.com.',
      cta: { label: 'Contact us', href: '/contact' },
    },
  ],
};

export const aboutPageContent: PageContent = {
  hero: {
    headline: 'We Built the Retina',
    subheadline: 'AuraSense — Hong Kong',
    description:
      'Because drone inspection deserved better than a GPU in the cloud.',
    primaryCta: { label: 'View Platform', href: '/platform' },
    secondaryCta: { label: 'Contact Us', href: '/contact' },
  },
  sections: [storySection, principlesSection, teamSection],
};
