import { useEffect } from 'react';
import { Hero } from '../components/sections/Hero';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { FeatureGrid } from '../components/sections/FeatureGrid';
import { Button } from '../components/ui/Button';
import { aboutPageContent } from '../content/about';

export function AboutPage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  const { hero, sections } = aboutPageContent;

  return (
    <main>
      <Hero content={hero} large />

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

      <SectionWrapper id="about-cta" dark>
        <header className="section-header">
          <h2>Ready to Evaluate?</h2>
          <p>Schedule a technical briefing with AuraSense engineering.</p>
        </header>
        <div className="cta-actions">
          <Button variant="primary" href="/contact">Contact Engineering</Button>
          <Button variant="secondary" href="/platform">View Platform</Button>
        </div>
      </SectionWrapper>
    </main>
  );
}
