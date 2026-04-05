import { useEffect } from 'react';
import { Hero } from '../components/sections/Hero';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { FeatureGrid } from '../components/sections/FeatureGrid';
import { Button } from '../components/ui/Button';
import { careersPageContent } from '../content/careers';

export function CareersPage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  const { hero, sections } = careersPageContent;
  const workOnSection = sections.find((s) => s.id?.includes('work-on') || s.id?.includes('what-we'));
  const remainingSections = sections.filter((s) => s !== workOnSection);

  return (
    <main>
      <Hero content={hero} />

      {/* What We Work On */}
      {workOnSection && (
        <SectionWrapper id={workOnSection.id} dark>
          <header className="section-header">
            {workOnSection.eyebrow && (
              <span className="section-eyebrow">{workOnSection.eyebrow}</span>
            )}
            <h2>{workOnSection.title}</h2>
            <p>{workOnSection.description}</p>
          </header>
          {workOnSection.items && workOnSection.items.length > 0 ? (
            <FeatureGrid items={workOnSection.items} columns={3} />
          ) : (
            <div
              style={{
                display: 'flex',
                flexWrap: 'wrap',
                gap: '0.75rem',
                marginTop: '2rem',
              }}
            >
              {[
                'Neuromorphic Computing',
                'Edge Vision Systems',
                'Deterministic Runtimes',
                'Structural Inspection',
                'SIMD / AVX2 Optimization',
                'Infrastructure AI',
                'Real-time Perception',
                'Autonomous Navigation',
              ].map((tag) => (
                <span
                  key={tag}
                  style={{
                    padding: '0.5rem 1rem',
                    border: '1px solid #1e293b',
                    borderRadius: '4px',
                    color: '#14b8a6',
                    fontSize: '0.875rem',
                    fontWeight: 500,
                    backgroundColor: 'rgba(20, 184, 166, 0.05)',
                  }}
                >
                  {tag}
                </span>
              ))}
            </div>
          )}
        </SectionWrapper>
      )}

      {/* Remaining sections */}
      {remainingSections.map((section, index) => (
        <SectionWrapper key={section.id} id={section.id} dark={index % 2 === 0}>
          <header className="section-header">
            {section.eyebrow && (
              <span className="section-eyebrow">{section.eyebrow}</span>
            )}
            <h2>{section.title}</h2>
            <p>{section.description}</p>
          </header>
          {section.items && section.items.length > 0 && (
            <FeatureGrid items={section.items} columns={3} />
          )}
        </SectionWrapper>
      ))}

      {/* Open Roles / CTA */}
      <SectionWrapper id="open-roles" dark>
        <header className="section-header">
          <h2>Open Roles</h2>
          <p>
            We hire selectively. If you build systems that work under real-world
            constraints, we want to hear from you.
          </p>
        </header>
        <div className="cta-actions">
          <Button variant="primary" href="mailto:careers@aurasensehk.com">
            Contact Careers
          </Button>
          <Button variant="secondary" href="/contact">General Inquiries</Button>
        </div>
      </SectionWrapper>
    </main>
  );
}
