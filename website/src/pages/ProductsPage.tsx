import { SectionWrapper } from '../components/ui/SectionWrapper';

export function ProductsPage() {
  return (
    <main>
      <SectionWrapper compact>
        <span className="module-label">Products</span>
        <h1 style={{
          fontSize: 'clamp(2rem, 4vw, 3rem)',
          fontWeight: 700, lineHeight: 1.15, letterSpacing: '-0.03em',
          color: 'var(--color-text-primary)', marginBottom: '1rem',
        }}>Edge Modules</h1>
        <p style={{ fontSize: '0.95rem', color: 'var(--color-text-secondary)' }}>
          Purpose-built modules running on the NEPA runtime.
        </p>
      </SectionWrapper>
    </main>
  );
}
