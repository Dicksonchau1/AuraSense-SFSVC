import { useEffect, useState } from 'react';
import { Hero } from '../components/sections/Hero';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { FeatureGrid } from '../components/sections/FeatureGrid';
import { Button } from '../components/ui/Button';
import { sfsvcContent } from '../content/sfsvc';
import type { ProductPageContent } from '../types/content';

export function ProductSFSVCPage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  const { hero, sections, specs, faqItems } = sfsvcContent as ProductPageContent;

  return (
    <main>
      <Hero content={hero} />

      {sections.map((section, index) => (
        <SectionWrapper key={section.id} id={section.id} dark={index % 2 === 0}>
          <header className="section-header">
            {section.eyebrow && (
              <span className="section-eyebrow">{section.eyebrow}</span>
            )}
            <h2>{section.title}</h2>
            <p>{section.description}</p>
          </header>
          {section.items && section.items.length > 0 && (
            <FeatureGrid items={section.items} columns={section.items.length <= 3 ? 3 : 4} />
          )}
        </SectionWrapper>
      ))}

      {specs && specs.length > 0 && (
        <SectionWrapper id="specs" dark>
          <header className="section-header">
            <h2>Technical Specifications</h2>
            <p>Measured performance characteristics under reference conditions.</p>
          </header>
          <div className="specs-table" role="table" aria-label="SFSVC Specifications">
            {specs.map((spec) => (
              <div
                key={spec.label}
                className="specs-row"
                role="row"
                style={{
                  display: 'flex',
                  justifyContent: 'space-between',
                  padding: '0.75rem 1rem',
                  borderBottom: '1px solid var(--border-subtle, #1e293b)',
                }}
              >
                <span style={{ color: '#94a3b8', fontWeight: 500 }}>{spec.label}</span>
                <span style={{ color: '#ffffff', fontFamily: 'monospace' }}>{spec.value}</span>
              </div>
            ))}
          </div>
        </SectionWrapper>
      )}

      {faqItems && faqItems.length > 0 && (
        <SectionWrapper id="faq">
          <header className="section-header">
            <h2>Frequently Asked Questions</h2>
          </header>
          <div className="faq-list">
            {faqItems.map((item) => (
              <FAQItem key={item.question} question={item.question} answer={item.answer} />
            ))}
          </div>
        </SectionWrapper>
      )}

      <SectionWrapper id="sfsvc-cta" dark>
        <header className="section-header">
          <h2>Evaluate SFSVC</h2>
          <p>
            Request SDK access, review integration documentation, or schedule a technical briefing.
          </p>
        </header>
        <div className="cta-actions">
          <Button variant="primary" href="/contact">Request SDK Access</Button>
          <Button variant="secondary" href="/products">All Products</Button>
        </div>
      </SectionWrapper>
    </main>
  );
}

function FAQItem({ question, answer }: { question: string; answer: string }) {
  const [open, setOpen] = useState(false);

  return (
    <div
      className="faq-item"
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
