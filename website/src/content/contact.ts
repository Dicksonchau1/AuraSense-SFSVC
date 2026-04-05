import type { PageContent } from '../types/content';

export const contactPageContent: PageContent = {
  hero: {
    headline: 'Talk to Engineering',
    subheadline: 'Technical Evaluation, Partnership, and Support',
    description:
      'AuraSense engineering handles technical inquiries directly. Whether you are evaluating the NEPA platform for an infrastructure inspection program, exploring integration for a robotic autonomy deployment, or seeking support for an existing installation — reach the team that builds the systems.',
    primaryCta: { label: 'Email Engineering', href: 'mailto:contact@aurasensehk.com' },
  },
  sections: [
    {
      id: 'technical-evaluation',
      eyebrow: 'Evaluation',
      title: 'Technical Evaluation',
      description:
        'For organizations evaluating the NEPA platform, SFSVC SDK, NERMN navigation module, or NSSIM surveillance intelligence for infrastructure inspection, robotic autonomy, or surveillance applications. Technical briefings are conducted by engineering — not sales. We discuss your deployment context, integration requirements, and evaluation criteria, and provide architecture-level detail sufficient for technical assessment.',
      items: [
        {
          title: 'Request a Technical Briefing',
          description:
            'Contact engineering to schedule a technical briefing covering platform architecture, product capabilities, deployment requirements, and evaluation access. Briefings are scoped to your specific use case and technical context. Email contact@aurasensehk.com with a brief description of your organization, deployment context, and evaluation objectives.',
        },
        {
          title: 'Evaluation Access',
          description:
            'Qualified organizations can receive time-limited evaluation access to NEPA modules including SDK, documentation, sample model bundles, and engineering support during the evaluation period. Evaluation scope is defined collaboratively based on your deployment requirements and integration context.',
        },
      ],
    },
    {
      id: 'enterprise-partnership',
      eyebrow: 'Partnership',
      title: 'Enterprise and Partner Inquiry',
      description:
        'For system integrators, technology partners, and enterprise organizations exploring NEPA integration into existing infrastructure inspection, surveillance, or robotic autonomy programs. We work with partners who deploy real systems in operational environments and who value deterministic, auditable, governance-aware edge intelligence.',
      items: [
        {
          title: 'Integration Partnership',
          description:
            'System integrators deploying NEPA modules within larger inspection, surveillance, or robotic solutions. We provide integration architecture guidance, co-deployment support, and joint solution validation. Contact contact@aurasensehk.com to discuss partnership scope.',
        },
        {
          title: 'Enterprise Deployment',
          description:
            'Enterprise organizations deploying NEPA at scale across multiple sites, platforms, or operational domains. We provide deployment architecture review, volume licensing, enterprise support agreements, and dedicated engineering liaison. Contact contact@aurasensehk.com for enterprise inquiries.',
        },
      ],
    },
    {
      id: 'support',
      eyebrow: 'Support',
      title: 'Support',
      description:
        'For organizations with existing NEPA deployments or active evaluation agreements. Support covers runtime issues, model bundle management, evidence integrity questions, deployment configuration, and integration troubleshooting.',
      items: [
        {
          title: 'Production Support',
          description:
            'Organizations with production deployments covered by enterprise support agreements should contact support@aurasensehk.com for runtime issues, configuration assistance, and escalation. Support SLAs and escalation paths are defined in your support agreement.',
        },
        {
          title: 'Evaluation Support',
          description:
            'Organizations in active evaluation receive engineering support during the evaluation period. For technical questions, integration issues, or deployment configuration assistance during evaluation, contact support@aurasensehk.com with your evaluation reference.',
        },
        {
          title: 'Documentation and FAQ',
          description:
            'Public documentation, product overviews, and frequently asked questions are available in the resources section. Many common questions about platform capabilities, deployment requirements, and licensing are addressed there.',
          cta: { label: 'View Resources', href: '/resources' },
        },
      ],
    },
    {
      id: 'careers-inquiry',
      eyebrow: 'Careers',
      title: 'Careers Inquiry',
      description:
        'For engineers interested in working on deterministic edge intelligence, neuromorphic computing, robotic autonomy, or surveillance intelligence at AuraSense.',
      items: [
        {
          title: 'Engineering Roles',
          description:
            'We are selectively hiring across systems engineering, computer vision, edge AI, robotics, and product engineering. If your background aligns with our work, send a short note and relevant portfolio or GitHub profile to careers@aurasensehk.com. Review the careers page for detailed role descriptions and what we look for.',
          cta: { label: 'View Careers', href: '/careers' },
        },
      ],
    },
  ],
};

export const contactEmails = {
  general: 'contact@aurasensehk.com',
  support: 'support@aurasensehk.com',
  careers: 'careers@aurasensehk.com',
};
