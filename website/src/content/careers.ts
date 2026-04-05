import type { PageContent, RoleCategory } from '../types/content';

export const roleCategories: RoleCategory[] = [
  {
    title: 'C++ / Systems Engineers',
    description:
      'Low-latency runtime development, SIMD-accelerated processing pipelines, lock-free data structures, deterministic memory management. You write C++17 code that meets hard latency SLAs on commodity hardware. You understand cache behavior, branch prediction, and why dynamic allocation in a hot path is unacceptable. Experience with real-time scheduling, CPU pinning, and SIMD intrinsics (AVX2/NEON) is directly relevant.',
  },
  {
    title: 'Computer Vision / ML Engineers',
    description:
      'Spike-based encoding, temporal differencing, anomaly detection, prototype-based classification. You build perception systems that produce deterministic, auditable outputs — not black-box inference. Experience with neuromorphic computing, event-driven vision, or structured anomaly detection is valuable. Traditional CV experience (OpenCV, feature extraction, morphological operations) applies directly to our inspection pipeline.',
  },
  {
    title: 'Edge AI / Embedded Engineers',
    description:
      'Deployment on ARM64 embedded platforms, cross-compilation, resource-constrained optimization, offline operation. You deploy ML inference on hardware with real power, memory, and thermal constraints. Experience with Jetson, Raspberry Pi, Qualcomm RB-series, or custom embedded Linux is relevant. You understand the difference between a model that works in a notebook and a model that runs in a 10W power budget.',
  },
  {
    title: 'Robotics / Autonomy Engineers',
    description:
      'Navigation stacks, sensor fusion, safety supervision, watchdog architectures. You build systems that move physical platforms safely in unstructured environments. Experience with ROS2, MAVLink, event cameras, LiDAR processing, and real-time path planning is directly relevant. You understand degraded-safe design — what happens when the primary navigation pipeline fails.',
  },
  {
    title: 'Full-Stack Product Engineers',
    description:
      'Dashboard interfaces, telemetry visualization, configuration management, deployment tooling. You build the interfaces and tools that make edge infrastructure observable and manageable. Experience with TypeScript, React, real-time data visualization, and API design is relevant. You care about building tools that operators actually use under time pressure.',
  },
  {
    title: 'Technical Product / Deployment Operators',
    description:
      'Field deployment, customer onboarding, integration engineering, technical documentation. You bridge the gap between engineering and deployment reality. You have deployed systems in operational environments and understand the difference between a working demo and a production deployment. Experience with edge infrastructure, industrial IoT, or inspection technology is valuable.',
  },
];

const whatWeWorkOnSection = {
  id: 'what-we-work-on',
  eyebrow: 'Work',
  title: 'What We Work On',
  description:
    'AuraSense builds deterministic edge intelligence for critical infrastructure. The technical domains span neuromorphic computing, edge inference, robotic autonomy, surveillance intelligence, and governance-aware system design. Everything we build runs on real hardware under real operational constraints.',
  items: [
    {
      title: 'Neuromorphic Computing',
      description:
        'Spike-based encoding, temporal differencing, event-driven perception. We implement neuromorphic computing principles in production systems — not as academic exercises, but as deployed infrastructure that processes real sensor data on commodity CPUs with deterministic timing guarantees.',
    },
    {
      title: 'Edge Inference and Low-Latency Processing',
      description:
        'SIMD-accelerated frame processing, lock-free pipelines, pre-allocated buffer management, hard latency SLAs. Our processing lanes meet P95 < 0.5ms classification latency on x86_64 CPUs. This requires careful systems engineering — cache-aware memory layout, branch-free inner loops, and zero dynamic allocation in the hot path.',
    },
    {
      title: 'Robotic Autonomy and Navigation',
      description:
        'Event camera ingestion, spike-encoded sensor fusion, bounded-latency path planning, watchdog-supervised safety. Our navigation systems operate in GPS-denied and confined environments where reliable fallback behavior is non-negotiable. Safety is structural, not aspirational.',
    },
    {
      title: 'Surveillance Intelligence',
      description:
        'Multi-sensor temporal correlation, alert orchestration, severity tiering, evidence retention. Our surveillance systems process camera, acoustic, and environmental sensor feeds and produce actionable alerts with full chain-of-custody evidence packages.',
    },
    {
      title: 'Deterministic Systems and Governance',
      description:
        'Replay verification, chain-sealed audit logs, model provenance, versioned bundle promotion with human approval gates. Every output our systems produce is replay-verifiable and evidence-grade. Governance is not a compliance afterthought — it is an architectural requirement.',
    },
    {
      title: 'Infrastructure and Deployment',
      description:
        'Offline-capable deployment, Ed25519 license activation, air-gapped operation, multi-site coordination. Our systems deploy on inspection drones, ground robots, fixed surveillance installations, and edge gateways. Deployment engineering is a first-class discipline here.',
    },
  ],
};

const whoWeHireSection = {
  id: 'who-we-hire',
  eyebrow: 'Roles',
  title: 'Who We Hire',
  description:
    'We hire engineers who build systems that work in operational environments — not engineers who build impressive demos that break under real conditions. The roles below describe the technical profiles we look for. We do not require exact title matches. If your experience spans multiple categories, that is often more valuable than deep specialization in one.',
  items: roleCategories.map((role) => ({
    title: role.title,
    description: role.description,
  })),
};

const howWeWorkSection = {
  id: 'how-we-work',
  eyebrow: 'Culture',
  title: 'How We Work',
  description:
    'AuraSense is a small engineering organization building production infrastructure for critical applications. The culture reflects the work — careful, accountable, and technically rigorous. We are not building a consumer product. We are not iterating on growth metrics. We are building systems that people rely on to make consequential decisions about physical infrastructure.',
  items: [
    {
      title: 'Ownership',
      description:
        'Engineers own their systems from design through deployment. There is no separate team that deploys your code, debugs your production issues, or writes your documentation. If you build it, you support it. This is a feature, not a burden — it produces engineers who build systems that are actually operable.',
    },
    {
      title: 'Technical Depth',
      description:
        'We value engineers who understand the systems they build at the level of detail that matters for production reliability. If you write a SIMD inner loop, you should know its throughput on the target microarchitecture. If you design an evidence chain, you should know exactly how tampering is detected.',
    },
    {
      title: 'Deployment Realism',
      description:
        'Every design decision is evaluated against deployment reality — power constraints, compute budgets, connectivity limitations, operational procedures. A technically elegant solution that cannot be deployed under field conditions is not a solution. We optimize for systems that work in the environments where they actually run.',
    },
    {
      title: 'Careful Engineering',
      description:
        'We ship carefully, not cautiously. There is a difference. Cautious engineering avoids risk. Careful engineering understands risk, bounds it, and ships with confidence. We prefer thorough testing, bounded rollout, and explicit safety margins over both reckless speed and paralytic risk aversion.',
    },
    {
      title: 'Low-Ego, High-Accountability',
      description:
        'Technical disagreements are resolved with evidence and reasoning, not seniority or volume. Mistakes are expected and addressed without blame — but repeated failure to learn from mistakes is not tolerated. We hold each other accountable because the systems we build are accountable to their operators.',
    },
  ],
};

const whatCandidatesShouldExpectSection = {
  id: 'interview-process',
  eyebrow: 'Process',
  title: 'What Candidates Should Expect',
  description:
    'Our hiring process is designed to assess whether you can do the work we do — not to test your ability to solve algorithmic puzzles under artificial time pressure. We look for technical depth, practical judgment, and clear communication.',
  items: [
    {
      title: 'Technical Conversation',
      description:
        'A discussion of your technical background, the systems you have built, and the decisions you made in building them. We are interested in your engineering judgment — why you chose an approach, what tradeoffs you considered, and what you would do differently. This is a conversation, not an interrogation.',
    },
    {
      title: 'Project Discussion',
      description:
        'A deeper dive into a specific project from your background — architecture, implementation challenges, operational experience, and lessons learned. We want to understand how you think about system design in practice, not in theory. Bring a project you are genuinely proud of or one that taught you something valuable.',
    },
    {
      title: 'Practical Relevance',
      description:
        'Depending on the role, we may ask you to discuss or work through a problem relevant to our technical domains — edge processing, sensor fusion, real-time systems, evidence integrity, or deployment engineering. Problems are drawn from our actual work, not from algorithmic puzzle collections.',
    },
    {
      title: 'Writing Clarity',
      description:
        'We value engineers who communicate clearly in writing. Technical documentation, design proposals, and incident reports are part of the job. We may ask you to explain a technical concept in writing as part of the process. Clarity and precision matter more than volume.',
    },
  ],
};

const openRolesSection = {
  id: 'open-roles',
  eyebrow: 'Roles',
  title: 'Open Roles',
  description:
    'We are selectively hiring across systems engineering, robotics, edge AI, computer vision, and product engineering. AuraSense does not maintain a large open requisition board. We hire when we identify engineers whose background aligns with our work and who demonstrate the technical depth and practical judgment we value.',
  items: [
    {
      title: 'How to Apply',
      description:
        'If your background aligns with the work described on this page, send a short note and relevant portfolio, GitHub profile, or project references to careers@aurasensehk.com. Tell us what you have built, what you are interested in working on, and why AuraSense is a fit. We read every message and respond to candidates whose experience is relevant to our current work.',
    },
    {
      title: 'What to Include',
      description:
        'A brief description of your technical background and the types of systems you have built. Links to relevant work — GitHub repositories, published papers, technical writing, project portfolios. A note about which of our technical domains interests you most. We do not require a formal resume, but we need enough information to assess relevance.',
    },
    {
      title: 'Location and Structure',
      description:
        'AuraSense engineering is based in Hong Kong with distributed team members. We support remote work for roles where it is effective, but some roles — particularly those involving hardware, deployment, or field operations — may require physical presence. Compensation is competitive and reflects the technical depth we expect.',
    },
  ],
};

export const careersPageContent: PageContent = {
  hero: {
    headline: 'Build Systems That Survive Contact with Reality',
    subheadline: 'Engineering at AuraSense',
    description:
      'AuraSense works on real edge infrastructure deployed under operational constraints — not demos, not proofs-of-concept, not research prototypes. The systems we build run on inspection drones, robotic platforms, and surveillance installations where reliability is not a feature request. If your engineering instinct is to ship things that work under pressure, and you care about the difference between a working system and a convincing slide deck, we should talk.',
    primaryCta: { label: 'View Open Roles', href: '#open-roles' },
    secondaryCta: { label: 'Contact Careers', href: 'mailto:careers@aurasensehk.com' },
  },
  sections: [
    whatWeWorkOnSection,
    whoWeHireSection,
    howWeWorkSection,
    whatCandidatesShouldExpectSection,
    openRolesSection,
  ],
};
