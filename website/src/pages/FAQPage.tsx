import { useEffect, useState } from 'react';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { Button } from '../components/ui/Button';
import type { HeroContent } from '../types/content';

const faqHero: HeroContent = {
  headline: 'Frequently Asked Questions',
  subheadline: 'Common questions about AuraSense, NEPA, and the product stack.',
  description:
    'Answers for engineering teams, evaluators, and procurement stakeholders.',
  primaryCta: { label: 'Contact Us', href: '/contact' },
};

interface FAQEntry {
  question: string;
  answer: string;
}

const faqEntries: FAQEntry[] = [
  {
    question: 'What is AuraSense?',
    answer:
      'AuraSense is the company behind NEPA — Neuromorphic Edge Perception Architecture. We build deterministic, CPU-native inspection runtimes for critical infrastructure: facades, bridges, runways, and other structural assets.',
  },
  {
    question: 'What is NEPA?',
    answer:
      'NEPA (Neuromorphic Edge Perception Architecture) is the platform runtime that underpins all AuraSense products. It provides bounded-latency, offline-capable perception and classification with full audit chain support.',
  },
  {
    question: 'What is SFSVC?',
    answer:
      'SFSVC (Spike-based Facade Surface Vision Codec) is the core vision module for surface defect detection and classification. It operates at sub-millisecond latency on CPU, with no GPU required.',
  },
  {
    question: 'Does NEPA require a GPU?',
    answer:
      'No. All NEPA modules — SFSVC, NERMN, and NSSIM — are designed to run on standard x86-64 CPUs with AVX2 support. GPU acceleration is neither required nor used in the production runtime.',
  },
  {
    question: 'How is the platform licensed?',
    answer:
      'NEPA is distributed as an SDK under commercial license. Evaluation access is available for qualified engineering teams. Contact us for licensing terms, pricing, and evaluation packages.',
  },
  {
    question: 'Can the system operate offline?',
    answer:
      'Yes. All perception, classification, and evidence generation modules operate fully offline. Network connectivity is only required for telemetry upload and license validation, both of which can be deferred.',
  },
  {
    question: 'What deployment environments are supported?',
    answer:
      'NEPA supports Linux-based edge devices, ruggedized compute platforms, and standard server environments. Docker and bare-metal deployment paths are documented. Minimum: x86-64, AVX2, 4GB RAM.',
  },
  {
    question: 'How do I request technical support?',
    answer:
      'Evaluation and licensed partners have access to direct engineering support. For general inquiries, contact support@aurasensehk.com. For evaluation access, use the contact form.',
  },
];

export function FAQPage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  return (
    <main>
      {/* Hero */}
      <SectionWrapper id="faq-hero" dark>
        <div style={{ maxWidth: '720px', paddingTop: '3rem', paddingBottom: '2rem' }}>
          <h1 style={{ color: '#ffffff', fontSize: '2.5rem', fontWeight: 700, marginBottom: '1rem' }}>
            {faqHero.headline}
          </h1>
          <p style={{ color: '#94a3b8', fontSize: '1.125rem', lineHeight: 1.6 }}>
            {faqHero.subheadline}
          </p>
        </div>
      </SectionWrapper>

      {/* FAQ Items */}
      <SectionWrapper id="faq-list">
        <div style={{ maxWidth: '720px', margin: '0 auto' }}>
          {faqEntries.map((entry) => (
            <FAQItem key={entry.question} question={entry.question} answer={entry.answer} />
          ))}
        </div>
      </SectionWrapper>

      {/* CTA */}
      <SectionWrapper id="faq-cta" dark>
        <header className="section-header">
          <h2>Still have questions?</h2>
          <p>Reach out to our team for technical or commercial inquiries.</p>
        </header>
        <div className="cta-actions">
          <Button variant="primary" href="/contact">Contact Us</Button>
          <Button variant="secondary" href="/resources">View Resources</Button>
        </div>
      </SectionWrapper>
    </main>
  );
}

function FAQItem({ question, answer }: FAQEntry) {
  const [open, setOpen] = useState(false);

  return (
    <div
      style={{
        borderBottom: '1px solid var(--border-subtle, #1e293b)',
        padding: '1.25rem 0',
      }}
    >
      <button
        onClick={() => setOpen(!open)}
        aria-expanded={open}
        style={{
          all: 'unset',
          cursor: 'pointer',
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
          width: '100%',
          color: '#ffffff',
          fontWeight: 600,
          fontSize: '1rem',
        }}
      >
        <span>{question}</span>
        <span style={{ color: '#14b8a6', fontSize: '1.25rem', marginLeft: '1rem' }}>
          {open ? '−' : '+'}
        </span>
      </button>
      {open && (
        <p style={{ color: '#94a3b8', marginTop: '0.75rem', lineHeight: 1.6 }}>
          {answer}
        </p>
      )}
    </div>
  );
}
