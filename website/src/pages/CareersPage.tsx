import { useEffect } from 'react';
import { Hero } from '../components/sections/Hero';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { FeatureGrid } from '../components/sections/FeatureGrid';
import { Card } from '../components/ui/Card';
import { Button } from '../components/ui/Button';
import { careersPageContent, roleCategories } from '../content/careers';
import type { RoleCategory } from '../types/content';

export function CareersPage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  const { hero, sections } = careersPageContent;
  const workOnSection = sections.find((s) => s.id?.includes('work-on') || s.id?.includes('what-we'));
  const cultureSection = sections.find((s) => s.id?.includes('culture') || s.id?.includes('how-we'));
  const expectSection = sections.find((s) => s.id?.includes('expect') || s.id?.includes('candidate'));
  const remainingSections = sections.filter(
    (s) => s !== workOnSection && s !== cultureSection && s !== expectSection
  );

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

      {/* Who We Hire */}
      <SectionWrapper id="who-we-hire">
        <header className="section-header">
          <h2>Who We Hire</h2>
          <p>
            We look for engineers who build systems that work under constraints —
            not demos that work under ideal conditions.
          </p>
        </header>
        <div
          style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(auto-fill, minmax(300px, 1fr))',
            gap: '1.5rem',
            marginTop: '2rem',
          }}
        >
          {roleCategories.map((role: RoleCategory) => (
            <Card key={role.title} title={role.title} description={role.description} />
          ))}
        </div>
      </SectionWrapper>

      {/* How We Work */}
      {cultureSection && (
        <SectionWrapper id={cultureSection.id} dark>
          <header className="section-header">
            {cultureSection.eyebrow && (
              <span className="section-eyebrow">{cultureSection.eyebrow}</span>
            )}
            <h2>{cultureSection.title}</h2>
            <p>{cultureSection.description}</p>
          </header>
          {cultureSection.items && cultureSection.items.length > 0 ? (
            <FeatureGrid items={cultureSection.items} columns={3} />
          ) : (
            <div
              style={{
                display: 'grid',
                gridTemplateColumns: 'repeat(auto-fill, minmax(280px, 1fr))',
                gap: '1.5rem',
                marginTop: '2rem',
              }}
            >
              {[
                {
                  title: 'Deterministic by Default',
                  description:
                    'We build systems with bounded latency and reproducible behavior. Every output is auditable.',
                },
                {
                  title: 'Small Teams, Full Ownership',
                  description:
                    'Engineers own their modules end-to-end — from algorithm design through deployment and support.',
                },
                {
                  title: 'Evidence Over Opinion',
                  description:
                    'Technical decisions are backed by benchmarks, profiling data, and field validation. Not slide decks.',
                },
              ].map((principle) => (
                <Card
                  key={principle.title}
                  title={principle.title}
                  description={principle.description}
                />
              ))}
            </div>
          )}
        </SectionWrapper>
      )}

      {/* What Candidates Should Expect */}
      {expectSection && (
        <SectionWrapper id={expectSection.id}>
          <header className="section-header">
            {expectSection.eyebrow && (
              <span className="section-eyebrow">{expectSection.eyebrow}</span>
            )}
            <h2>{expectSection.title}</h2>
            <p>{expectSection.description}</p>
          </header>
          {expectSection.items && expectSection.items.length > 0 && (
            <FeatureGrid items={expectSection.items} columns={3} />
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
