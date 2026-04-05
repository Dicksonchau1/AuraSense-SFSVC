import { ProductHero } from '../components/product/ProductHero';
import { ProductPillars } from '../components/product/ProductPillars';
import { ProductDeploymentSection } from '../components/product/ProductDeploymentSection';
import { ContactCTASection } from '../components/sections/ContactCTASection';
import { ArchitecturePipeline } from '../components/diagrams/ArchitecturePipeline';
import { PageContainer } from '../components/layout/PageContainer';
import { SectionHeading } from '../components/ui/SectionHeading';
import { SurfaceCard } from '../components/ui/SurfaceCard';
import { nssimConfig } from '../content/products';

export function ProductNSSIMPage() {
  return (
    <>
      <ProductHero product={nssimConfig} />

      <ProductPillars
        tag="Capabilities"
        title="Sensor Ingest and Temporal Intelligence"
        subtitle="Enterprise CCTV and multi-sensor security intelligence with temporal event reasoning."
        pillars={nssimConfig.pillars}
      />

      <section id="architecture" className="py-20 bg-bg-surface/50">
        <PageContainer>
          <SectionHeading
            tag="Architecture"
            title="Intelligence Pipeline"
            subtitle="End-to-end evidence chain from temporal intelligence through risk detection to operator dashboards and compliance reporting."
          />
          <ArchitecturePipeline nodes={nssimConfig.architectureNodes} title="NSSIM Intelligence Pipeline" />
        </PageContainer>
      </section>

      {/* Alert Orchestration & Audit */}
      <section className="py-20">
        <PageContainer>
          <SectionHeading
            tag="Operations"
            title="Alert Orchestration and Audit Model"
            subtitle="Risk scoring, evidence retention, and multi-site operational orchestration."
          />
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <SurfaceCard>
              <h3 className="text-base font-semibold text-text-primary mb-2">Alert Orchestration</h3>
              <p className="text-sm text-text-secondary leading-relaxed">
                Risk scoring and alert routing with configurable severity bands. Escalation policies ensure critical events reach operators within defined SLA windows.
              </p>
            </SurfaceCard>
            <SurfaceCard>
              <h3 className="text-base font-semibold text-text-primary mb-2">Evidence Retention</h3>
              <p className="text-sm text-text-secondary leading-relaxed">
                All sensor events, alerts, and operator actions stored in tamper-evident audit chains. Evidence retention policies configurable per deployment and regulatory context.
              </p>
            </SurfaceCard>
            <SurfaceCard>
              <h3 className="text-base font-semibold text-text-primary mb-2">Multi-Site Operations</h3>
              <p className="text-sm text-text-secondary leading-relaxed">
                Centralised visibility across distributed deployments. Cross-site event correlation, fleet health monitoring, and compliance dashboards for security operations.
              </p>
            </SurfaceCard>
            <SurfaceCard>
              <h3 className="text-base font-semibold text-text-primary mb-2">Operator Dashboards</h3>
              <p className="text-sm text-text-secondary leading-relaxed">
                Real-time temporal event timelines, risk heatmaps, and alert status views designed for security operations centres and enterprise control rooms.
              </p>
            </SurfaceCard>
          </div>
        </PageContainer>
      </section>

      <ProductDeploymentSection
        bullets={nssimConfig.deploymentBullets}
        governanceBullets={nssimConfig.governanceBullets}
      />

      <ContactCTASection
        title="Interested in NSSIM early access?"
        subtitle="Contact our engineering team for early access program details and deployment planning."
        primaryLabel="Join Early Access"
        primaryHref="/contact"
        secondaryLabel="View All Products"
        secondaryHref="/products"
      />
    </>
  );
}
