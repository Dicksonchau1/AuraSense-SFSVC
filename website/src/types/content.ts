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
