import { useEffect } from 'react';
import { Hero } from '../components/sections/Hero';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { FeatureGrid } from '../components/sections/FeatureGrid';
import { ProductCard } from '../components/sections/ProductCard';
import { Button } from '../components/ui/Button';
import { homepageContent } from '../content/homepage';
import { productSummaries } from '../content/products';

export function HomePage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  const { hero, sections } = homepageContent;
  const ctaSection = sections.find((s) => s.id === 'cta');
  const contentSections = sections.filter((s) => s.id !== 'cta' && s.id !== 'product-modules');
  const productSection = sections.find((s) => s.id === 'product-modules');

  return (
    <main>
      <Hero content={hero} large />

      {contentSections.map((section, index) => (
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

      {productSection && (
        <SectionWrapper id={productSection.id} dark>
          <header className="section-header">
            {productSection.eyebrow && (
              <span className="section-eyebrow">{productSection.eyebrow}</span>
            )}
            <h2>{productSection.title}</h2>
            <p>{productSection.description}</p>
          </header>
          <div className="product-grid">
            {productSummaries.map((product) => (
              <ProductCard key={product.id} product={product} />
            ))}
          </div>
        </SectionWrapper>
      )}

      {ctaSection && (
        <SectionWrapper id="cta" dark>
          <header className="section-header">
            <h2>{ctaSection.title}</h2>
            <p>{ctaSection.description}</p>
          </header>
          <div className="cta-actions">
            <Button variant="primary" href="/contact">Request Technical Briefing</Button>
            <Button variant="secondary" href="/platform">View Platform</Button>
          </div>
        </SectionWrapper>
      )}
    </main>
  );
}
