import { ProductHero } from '../components/product/ProductHero';
import { ProductPillars } from '../components/product/ProductPillars';
import { ProductDeploymentSection } from '../components/product/ProductDeploymentSection';
import { ProductSpecsTable } from '../components/product/ProductSpecsTable';
import { ProductFAQ } from '../components/product/ProductFAQ';
import { ContactCTASection } from '../components/sections/ContactCTASection';
import { ArchitecturePipeline } from '../components/diagrams/ArchitecturePipeline';
import { PageContainer } from '../components/layout/PageContainer';
import { SectionHeading } from '../components/ui/SectionHeading';
import { sfsvcConfig } from '../content/products';

export function ProductSFSVCPage() {
  return (
    <>
      <ProductHero product={sfsvcConfig} />

      <ProductPillars
        tag="Core Capabilities"
        title="Why SFSVC"
        subtitle="Neuromorphic compression and perception for edge inspection workflows."
        pillars={sfsvcConfig.pillars}
      />

      <section id="architecture" className="py-20 bg-bg-surface/50">
        <PageContainer>
          <SectionHeading
            tag="Pipeline"
            title="Spike Encoding Pipeline"
            subtitle="From UAV video ingest through spike encoding to deterministic detection and auditable output."
          />
          <ArchitecturePipeline nodes={sfsvcConfig.architectureNodes} title="SFSVC Processing Pipeline" />
        </PageContainer>
      </section>

      {sfsvcConfig.specs && <ProductSpecsTable specs={sfsvcConfig.specs} />}

      <ProductDeploymentSection
        bullets={sfsvcConfig.deploymentBullets}
        governanceBullets={sfsvcConfig.governanceBullets}
      />

      {sfsvcConfig.faqItems && <ProductFAQ items={sfsvcConfig.faqItems} />}

      <ContactCTASection
        title="Ready to evaluate SFSVC?"
        subtitle="Request SDK access, technical documentation, or deployment planning support."
        primaryLabel="Request SDK Access"
        primaryHref="/contact"
        secondaryLabel="View All Products"
        secondaryHref="/products"
      />
    </>
  );
}
