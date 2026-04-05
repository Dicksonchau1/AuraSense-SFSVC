import { useEffect } from 'react';
import { SectionWrapper } from '../components/ui/SectionWrapper';

interface LegalPageProps {
  title: string;
}

/** TODO: Replace placeholder content with finalized legal copy. */
export function LegalPage({ title }: LegalPageProps) {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  return (
    <main>
      <SectionWrapper id="legal-hero" dark>
        <div style={{ paddingTop: '3rem', paddingBottom: '2rem', maxWidth: '720px' }}>
          <h1 style={{ color: '#ffffff', fontSize: '2.5rem', fontWeight: 700, marginBottom: '1rem' }}>
            {title}
          </h1>
        </div>
      </SectionWrapper>

      <SectionWrapper id="legal-content">
        <div style={{ maxWidth: '720px', margin: '0 auto', padding: '2rem 0' }}>
          {/* TODO: Publish finalized legal content */}
          <p style={{ color: '#94a3b8', fontSize: '1rem', lineHeight: 1.8 }}>
            This page is under review and will be published shortly. For questions,
            contact{' '}
            <a
              href="mailto:legal@aurasensehk.com"
              style={{ color: '#14b8a6', textDecoration: 'none' }}
            >
              legal@aurasensehk.com
            </a>
            .
          </p>
        </div>
      </SectionWrapper>
    </main>
  );
}
