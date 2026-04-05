import { useEffect } from 'react';
import { Hero } from '../components/sections/Hero';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { FeatureGrid } from '../components/sections/FeatureGrid';
import { Button } from '../components/ui/Button';
import { nermnContent } from '../content/nermn';
import type { ProductPageContent } from '../types/content';

export function ProductNERMNPage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  const { hero, sections, specs } = nermnContent as ProductPageContent;

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
            <p>Reference performance characteristics for NERMN navigation stack.</p>
          </header>
          <div className="specs-table" role="table" aria-label="NERMN Specifications">
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

      <SectionWrapper id="nermn-cta" dark>
        <header className="section-header">
          <h2>Evaluate NERMN</h2>
          <p>
            Request access to navigation stack documentation, integration guides,
            and deployment specifications.
          </p>
        </header>
        <div className="cta-actions">
          <Button variant="primary" href="/contact">Request Evaluation Access</Button>
          <Button variant="secondary" href="/products">All Products</Button>
        </div>
      </SectionWrapper>
    </main>
  );
}
