export interface NavLink {
  label: string;
  href: string;
  children?: NavLink[];
}

export interface HeroContent {
  headline: string;
  subheadline: string;
  description: string;
  primaryCta: { label: string; href: string };
  secondaryCta?: { label: string; href: string };
}

export interface SectionContent {
  id?: string;
  eyebrow?: string;
  title: string;
  description: string;
  items?: ContentItem[];
}

export interface ContentItem {
  title: string;
  description: string;
  icon?: string;
  href?: string;
  cta?: { label: string; href: string };
}

export interface ProductSummary {
  id: string;
  name: string;
  tagline: string;
  description: string;
  href: string;
  features: string[];
}

export interface ResourceEntry {
  title: string;
  description: string;
  type: string;
  audience: string;
  cta: { label: string; href: string };
}

export interface RoleCategory {
  title: string;
  description: string;
}

export interface FooterColumn {
  title: string;
  links: { label: string; href: string }[];
}

export interface PageContent {
  hero: HeroContent;
  sections: SectionContent[];
}

export interface ProductPageContent extends PageContent {
  specs?: { label: string; value: string }[];
  faqItems?: { question: string; answer: string }[];
}

export interface SFSVCPageContent {
  hero: {
    eyebrow: string;
    title: string;
    description: string;
    primaryCta: { label: string; href: string };
    secondaryCta: { label: string; href: string };
    supportLine: string;
  };
  proofStrip: string[];
  overview: { eyebrow: string; title: string; description: string };
  visualBand: {
    eyebrow: string;
    title: string;
    description: string;
    mediaLabels: string[];
  };
  architecture: {
    eyebrow: string;
    title: string;
    description: string;
    flow: string[];
    cards: { title: string; description: string }[];
  };
  capabilityOne: { eyebrow: string; title: string; description: string; items: string[] };
  capabilityTwo: { eyebrow: string; title: string; description: string; items: string[] };
  capabilityThree: { eyebrow: string; title: string; description: string; items: string[] };
  deployment: {
    eyebrow: string;
    title: string;
    description: string;
    bands: string[];
  };
  governance: { eyebrow: string; title: string; description: string; items: string[] };
  integration: { eyebrow: string; title: string; description: string; items: string[] };
  technicalProfile: {
    eyebrow: string;
    title: string;
    description: string;
    chips: string[];
  };
  faq: {
    eyebrow: string;
    title: string;
    items: { question: string; answer: string }[];
  };
  finalCta: {
    eyebrow: string;
    title: string;
    description: string;
    primaryCta: { label: string; href: string };
    secondaryCta: { label: string; href: string };
  };
}
