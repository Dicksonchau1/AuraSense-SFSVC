import { HeroSection } from '../components/sections/HeroSection';
import { ProductCardsSection } from '../components/sections/ProductCardsSection';
import { ContactCTASection } from '../components/sections/ContactCTASection';
import { PlatformDiagram } from '../components/diagrams/PlatformDiagram';
import { PageContainer } from '../components/layout/PageContainer';
import { SectionHeading } from '../components/ui/SectionHeading';
import { allProducts } from '../content/products';

export function ProductsPage() {
  return (
    <>
      <HeroSection
        tag="Products"
        title="NEPA Product Modules"
        subtitle="Three purpose-built modules operating on the NEPA deterministic runtime. Each module addresses a distinct operational domain while sharing infrastructure for audit, replay, and governance."
        primaryCta={{ label: 'Contact Engineering', href: '/contact' }}
      />

      <section className="py-20 bg-bg-surface/50">
        <PageContainer>
          <SectionHeading
            tag="Architecture"
            title="Platform + Products"
            subtitle="All product modules execute within the NEPA platform boundary, inheriting deterministic guarantees, audit chain, and governance controls."
            align="center"
          />
          <PlatformDiagram />
        </PageContainer>
      </section>

      <ProductCardsSection products={allProducts} />

      {/* Comparison */}
      <section className="py-20 bg-bg-surface/50">
        <PageContainer>
          <SectionHeading
            tag="Comparison"
            title="Module Comparison"
            align="center"
          />
          <div className="overflow-x-auto">
            <table className="w-full border border-border-default rounded-sm text-sm">
              <thead>
                <tr className="bg-bg-surface">
                  <th className="px-6 py-4 text-left text-text-muted font-mono text-xs tracking-wide uppercase border-b border-border-default">Capability</th>
                  <th className="px-6 py-4 text-center text-accent-primary font-mono text-xs tracking-wide border-b border-border-default">SFSVC</th>
                  <th className="px-6 py-4 text-center text-accent-primary font-mono text-xs tracking-wide border-b border-border-default">NERMN</th>
                  <th className="px-6 py-4 text-center text-accent-primary font-mono text-xs tracking-wide border-b border-border-default">NSSIM</th>
                </tr>
              </thead>
              <tbody className="text-text-secondary">
                {[
                  { cap: 'Primary Domain', sfsvc: 'Inspection', nermn: 'Navigation', nssim: 'Surveillance' },
                  { cap: 'Spike Encoding', sfsvc: '✓', nermn: '✓', nssim: '✓' },
                  { cap: 'CPU-Only', sfsvc: '✓', nermn: '✓', nssim: '✓' },
                  { cap: 'Audit Chain', sfsvc: '✓', nermn: '✓', nssim: '✓' },
                  { cap: 'Replay Verification', sfsvc: '✓', nermn: '✓', nssim: '✓' },
                  { cap: 'Offline Operation', sfsvc: '✓', nermn: '✓', nssim: '—' },
                  { cap: 'Multi-Site Control', sfsvc: '—', nermn: '—', nssim: '✓' },
                  { cap: 'Spatial Mapping', sfsvc: '—', nermn: '✓', nssim: '—' },
                  { cap: 'Status', sfsvc: 'Active', nermn: 'Early Access', nssim: 'Early Access' },
                ].map((row, idx) => (
                  <tr key={row.cap} className={idx % 2 === 0 ? '' : 'bg-bg-surface/30'}>
                    <td className="px-6 py-3 text-text-primary font-medium border-b border-border-default">{row.cap}</td>
                    <td className="px-6 py-3 text-center border-b border-border-default">{row.sfsvc}</td>
                    <td className="px-6 py-3 text-center border-b border-border-default">{row.nermn}</td>
                    <td className="px-6 py-3 text-center border-b border-border-default">{row.nssim}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </PageContainer>
      </section>

      <ContactCTASection />
    </>
  );
}
