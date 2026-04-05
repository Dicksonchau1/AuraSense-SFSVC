import { HeroSection } from '../components/sections/HeroSection';
import { GovernanceSection } from '../components/sections/GovernanceSection';
import { ContactCTASection } from '../components/sections/ContactCTASection';
import { ProductPillars } from '../components/product/ProductPillars';
import { PageContainer } from '../components/layout/PageContainer';
import { SectionHeading } from '../components/ui/SectionHeading';
import { MetricCard } from '../components/ui/MetricCard';
import { SurfaceCard } from '../components/ui/SurfaceCard';
import { platformConfig } from '../content/platform';

export function PlatformPage() {
  return (
    <>
      <HeroSection
        tag="Platform"
        title={platformConfig.headline}
        subtitle={platformConfig.subheadline}
        primaryCta={{ label: 'View Products', href: '/products' }}
        secondaryCta={{ label: 'Contact Engineering', href: '/contact' }}
      />

      {/* System Overview */}
      <section className="py-20 bg-bg-surface/50">
        <PageContainer>
          <SectionHeading
            tag="System Overview"
            title="Deterministic Inspection Infrastructure"
            subtitle={platformConfig.summary}
          />
          <div className="grid grid-cols-2 md:grid-cols-4 gap-px bg-border-default rounded-sm overflow-hidden border border-border-default">
            {platformConfig.metrics.map((m) => (
              <div key={m.label} className="bg-bg-surface">
                <MetricCard value={m.value} label={m.label} detail={m.detail} />
              </div>
            ))}
          </div>
        </PageContainer>
      </section>

      {/* Real-Time Operational Discipline */}
      <ProductPillars
        tag="Capabilities"
        title="Real-Time Operational Discipline"
        subtitle="Eight independent processing lanes operate in parallel, each handling a dedicated sensor modality or processing function."
        pillars={platformConfig.pillars}
      />

      {/* Evidence & Replay */}
      <section className="py-20 bg-bg-surface/50">
        <PageContainer>
          <SectionHeading
            tag="Evidence Framework"
            title="Immutable Evidence and Replay Verification"
            subtitle="Every finding is reproducible, attributable, and defensible. The evidence framework ensures findings withstand independent audit."
          />
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            {platformConfig.evidenceCapabilities.map((cap) => (
              <SurfaceCard key={cap.title}>
                <h3 className="text-base font-semibold text-text-primary mb-2">{cap.title}</h3>
                <p className="text-sm text-text-secondary leading-relaxed">{cap.description}</p>
              </SurfaceCard>
            ))}
          </div>
        </PageContainer>
      </section>

      {/* Deployment Model */}
      <section id="architecture" className="py-20">
        <PageContainer>
          <SectionHeading
            tag="Deployment"
            title="Enterprise Deployment Model"
            subtitle="NEPA integrates into existing camera-to-edge-to-cloud architectures. Standard hardware. Standard interfaces."
          />
          <div className="space-y-4">
            {platformConfig.deploymentLayers.map((layer) => (
              <div key={layer.id} className="flex items-start gap-4 bg-bg-surface border border-border-default rounded-sm p-5">
                <span className="text-xs font-mono font-medium text-accent-primary bg-accent-primary/10 px-2.5 py-1 rounded-sm shrink-0">
                  {layer.id}
                </span>
                <div>
                  <h3 className="text-sm font-semibold text-text-primary">{layer.label}</h3>
                  <p className="text-sm text-text-secondary mt-1">{layer.description}</p>
                </div>
              </div>
            ))}
          </div>
        </PageContainer>
      </section>

      {/* Integration Benefits */}
      <section className="py-20 bg-bg-surface/50">
        <PageContainer>
          <SectionHeading
            tag="Integration"
            title="Designed to Integrate. Not to Replace."
            subtitle="The platform aligns with enterprise risk management, compliance, and infrastructure lifecycle oversight requirements without requiring changes to upstream sensor or downstream reporting systems."
          />
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            {platformConfig.integrationBenefits.map((benefit) => (
              <SurfaceCard key={benefit.title}>
                <h3 className="text-base font-semibold text-text-primary mb-2">{benefit.title}</h3>
                <p className="text-sm text-text-secondary leading-relaxed">{benefit.description}</p>
              </SurfaceCard>
            ))}
          </div>
        </PageContainer>
      </section>

      <GovernanceSection items={platformConfig.governanceItems} />

      <ContactCTASection />
    </>
  );
}
