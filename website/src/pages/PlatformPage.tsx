import { useEffect } from 'react';
import { Hero } from '../components/sections/Hero';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { FeatureGrid } from '../components/sections/FeatureGrid';
import { Button } from '../components/ui/Button';
import { platformContent } from '../content/platform';

export function PlatformPage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  const { hero, sections } = platformContent;

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

      <SectionWrapper id="platform-cta" dark>
        <header className="section-header">
          <h2>Evaluate the Platform</h2>
          <p>
            Request access to technical documentation, deployment specifications,
            and SDK evaluation packages.
          </p>
        </header>
        <div className="cta-actions">
          <Button variant="primary" href="/contact">Request Access</Button>
          <Button variant="secondary" href="/products">View Products</Button>
        </div>
      </SectionWrapper>
    </main>
  );
}
