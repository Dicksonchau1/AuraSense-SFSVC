import { useEffect } from 'react';
import { Hero } from '../components/sections/Hero';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { FeatureGrid } from '../components/sections/FeatureGrid';
import { ProductCard } from '../components/sections/ProductCard';
import { Button } from '../components/ui/Button';
import { productsPageContent, productSummaries } from '../content/products';

export function ProductsPage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  const { hero, sections } = productsPageContent;
  const comparisonSection = sections.find((s) => s.id?.includes('comparison'));
  const deploymentSection = sections.find((s) => s.id?.includes('deployment'));
  const remainingSections = sections.filter(
    (s) => s !== comparisonSection && s !== deploymentSection
  );

  return (
    <main>
      <Hero content={hero} />

      <SectionWrapper id="product-modules" dark>
        <header className="section-header">
          <h2>Product Modules</h2>
          <p>
            Each module operates independently or as part of the integrated NEPA stack.
            CPU-native. Deterministic. Offline-capable.
          </p>
        </header>
        <div className="product-grid">
          {productSummaries.map((product) => (
            <ProductCard key={product.id} product={product} />
          ))}
        </div>
      </SectionWrapper>

      {remainingSections.map((section, index) => (
        <SectionWrapper key={section.id} id={section.id} dark={index % 2 !== 0}>
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

      {comparisonSection && (
        <SectionWrapper id={comparisonSection.id} dark>
          <header className="section-header">
            {comparisonSection.eyebrow && (
              <span className="section-eyebrow">{comparisonSection.eyebrow}</span>
            )}
            <h2>{comparisonSection.title}</h2>
            <p>{comparisonSection.description}</p>
          </header>
          {comparisonSection.items && comparisonSection.items.length > 0 && (
            <FeatureGrid items={comparisonSection.items} columns={3} />
          )}
        </SectionWrapper>
      )}

      {deploymentSection && (
        <SectionWrapper id={deploymentSection.id}>
          <header className="section-header">
            {deploymentSection.eyebrow && (
              <span className="section-eyebrow">{deploymentSection.eyebrow}</span>
            )}
            <h2>{deploymentSection.title}</h2>
            <p>{deploymentSection.description}</p>
          </header>
          {deploymentSection.items && deploymentSection.items.length > 0 && (
            <FeatureGrid items={deploymentSection.items} columns={3} />
          )}
        </SectionWrapper>
      )}

      <SectionWrapper id="products-cta" dark>
        <header className="section-header">
          <h2>Start Your Evaluation</h2>
          <p>
            Access SDK packages, deployment specifications, and integration documentation.
          </p>
        </header>
        <div className="cta-actions">
          <Button variant="primary" href="/contact">Request Evaluation Access</Button>
          <Button variant="secondary" href="/platform">View Platform Architecture</Button>
        </div>
      </SectionWrapper>
    </main>
  );
}
