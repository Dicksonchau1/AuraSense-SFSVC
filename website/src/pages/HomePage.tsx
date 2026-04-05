import { HeroSection } from '../components/sections/HeroSection';
import { ProductCardsSection } from '../components/sections/ProductCardsSection';
import { GovernanceSection } from '../components/sections/GovernanceSection';
import { ContactCTASection } from '../components/sections/ContactCTASection';
import { ProductPillars } from '../components/product/ProductPillars';
import { PlatformDiagram } from '../components/diagrams/PlatformDiagram';
import { PageContainer } from '../components/layout/PageContainer';
import { SectionHeading } from '../components/ui/SectionHeading';
import { platformConfig } from '../content/platform';
import { allProducts } from '../content/products';

export function HomePage() {
  return (
    <>
      <HeroSection
        tag="NEPA — Neuromorphic Edge Platform"
        title="Deterministic Edge Intelligence"
        subtitle="AuraSense NEPA is a real-time neuromorphic inference platform built on spike-based temporal contrast processing with sub-millisecond latency. Deployable at the edge — on drones, cameras, and robots."
        primaryCta={{ label: 'Explore Platform', href: '/platform' }}
        secondaryCta={{ label: 'View Products', href: '/products' }}
        metrics={[
          { value: 'OPERATIONAL', label: 'Engine' },
          { value: '<2ms', label: 'P95 Latency' },
          { value: '8 ACTIVE', label: 'Lanes' },
          { value: '0', label: 'Shared State' },
          { value: 'INTACT', label: 'Chain Seal' },
          { value: 'VERIFIED', label: 'Replay Cert' },
        ]}
      />

      {/* Why Deterministic Edge Intelligence */}
      <section className="py-20 bg-bg-surface/50">
        <PageContainer>
          <SectionHeading
            tag="Purpose"
            title="Purpose-Built for Infrastructure Environments"
            subtitle="Inspection outcomes influence asset valuation, maintenance decisions, regulatory reporting, and operational safety. NEPA is designed for environments where inspection systems must do more than detect — they must withstand scrutiny."
          />
          <div className="bg-bg-surface border border-border-default rounded-sm p-6 md:p-8 max-w-4xl">
            <p className="text-sm text-text-secondary leading-relaxed">
              NEPA is a deterministic inspection infrastructure platform engineered for auditability, replay verification, and governance control. This is infrastructure software, not exploratory analytics.
            </p>
          </div>
        </PageContainer>
      </section>

      {/* Platform Architecture */}
      <section className="py-20">
        <PageContainer>
          <SectionHeading
            tag="Architecture"
            title="NEPA Platform Architecture"
            subtitle="Three product modules built on a shared deterministic runtime with full audit, replay, and governance infrastructure."
            align="center"
          />
          <PlatformDiagram />
        </PageContainer>
      </section>

      <ProductPillars
        tag="Capabilities"
        title="Real-Time Operational Discipline"
        pillars={platformConfig.pillars.slice(0, 3)}
      />

      <ProductCardsSection products={allProducts} />

      <GovernanceSection items={platformConfig.governanceItems} />

      <ContactCTASection />
    </>
  );
}
