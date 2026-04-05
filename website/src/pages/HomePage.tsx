import { SectionWrapper } from '../components/ui/SectionWrapper';

export function HomePage() {
  return (
    <main>
      <SectionWrapper>
        <div style={{ maxWidth: '640px' }}>
          <span className="module-label">NEPA</span>
          <h1 style={{
            fontSize: 'clamp(2.5rem, 5vw, 3.5rem)',
            fontWeight: 700,
            lineHeight: 1.1,
            letterSpacing: '-0.03em',
            color: 'var(--color-text-primary)',
            marginBottom: '1.5rem',
          }}>
            Deterministic Edge<br />Infrastructure Runtime
          </h1>
          <p style={{
            fontSize: '1rem',
            lineHeight: 1.65,
            color: 'var(--color-text-secondary)',
            maxWidth: '50ch',
          }}>
            A bounded-latency perception layer for safety-critical inspection
            and monitoring systems. CPU-only. Offline. Deployable.
          </p>
        </div>
      </SectionWrapper>
    </main>
  );
}
