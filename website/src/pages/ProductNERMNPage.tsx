import { ProductHero } from '../components/product/ProductHero';
import { ProductPillars } from '../components/product/ProductPillars';
import { ProductDeploymentSection } from '../components/product/ProductDeploymentSection';
import { ContactCTASection } from '../components/sections/ContactCTASection';
import { ArchitecturePipeline } from '../components/diagrams/ArchitecturePipeline';
import { PageContainer } from '../components/layout/PageContainer';
import { SectionHeading } from '../components/ui/SectionHeading';
import { SurfaceCard } from '../components/ui/SurfaceCard';
import { nermnConfig } from '../content/products';

export function ProductNERMNPage() {
  return (
    <>
      <ProductHero product={nermnConfig} />

      <ProductPillars
        tag="Capabilities"
        title="Sensor-to-Decision Pipeline"
        subtitle="From event-driven sensing through neuromorphic inference to real-time navigation command output."
        pillars={nermnConfig.pillars}
      />

      <section id="architecture" className="py-20 bg-bg-surface/50">
        <PageContainer>
          <SectionHeading
            tag="Architecture"
            title="Navigation Stack Overview"
            subtitle="Mission-oriented command pipeline for autonomous flight and ground robotics."
          />
          <ArchitecturePipeline nodes={nermnConfig.architectureNodes} title="NERMN Navigation Pipeline" />
        </PageContainer>
      </section>

      {/* Safety & Supervision */}
      <section className="py-20">
        <PageContainer>
          <SectionHeading
            tag="Safety"
            title="Safety and Supervision Layer"
            subtitle="Independent watchdog monitoring with deterministic fallback policies for mission-critical autonomy."
          />
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <SurfaceCard>
              <h3 className="text-base font-semibold text-text-primary mb-2">Watchdog Supervision</h3>
              <p className="text-sm text-text-secondary leading-relaxed">
                Independent watchdog monitors inference latency, sensor health, and command validity. Violations trigger degraded-safe policy transitions with operator alerting.
              </p>
            </SurfaceCard>
            <SurfaceCard>
              <h3 className="text-base font-semibold text-text-primary mb-2">Degraded-Safe Policy</h3>
              <p className="text-sm text-text-secondary leading-relaxed">
                Predefined fallback behaviours for sensor loss, communication interruption, or inference timeout. Policy transitions are logged and auditable.
              </p>
            </SurfaceCard>
            <SurfaceCard>
              <h3 className="text-base font-semibold text-text-primary mb-2">Health Monitoring</h3>
              <p className="text-sm text-text-secondary leading-relaxed">
                Continuous health assessment of all sensors, inference cores, and communication channels. Anomalies are flagged before they impact navigation decisions.
              </p>
            </SurfaceCard>
            <SurfaceCard>
              <h3 className="text-base font-semibold text-text-primary mb-2">Telemetry and Operator Visibility</h3>
              <p className="text-sm text-text-secondary leading-relaxed">
                Real-time telemetry stream provides full operator visibility into navigation state, sensor status, and mission progress for ground control operations.
              </p>
            </SurfaceCard>
          </div>
        </PageContainer>
      </section>

      <ProductDeploymentSection
        bullets={nermnConfig.deploymentBullets}
        governanceBullets={nermnConfig.governanceBullets}
      />

      <ContactCTASection
        title="Interested in NERMN early access?"
        subtitle="Contact our engineering team for early access program details and integration planning."
        primaryLabel="Join Early Access"
        primaryHref="/contact"
        secondaryLabel="View All Products"
        secondaryHref="/products"
      />
    </>
  );
}
