import { useEffect } from 'react';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { Button } from '../components/ui/Button';

const OPEN_ROLES = [
  {
    title: 'Neuromorphic Systems Engineer',
    meta: 'Hong Kong · Full-time · Engineering',
    tag: 'Active',
  },
  {
    title: 'Edge AI Inference Engineer (C++/CUDA)',
    meta: 'Hong Kong · Full-time · Engineering',
    tag: 'Active',
  },
  {
    title: 'Computer Vision Engineer — Structural Inspection',
    meta: 'Hong Kong · Full-time · Engineering',
    tag: 'Active',
  },
  {
    title: 'Frontend Engineer (React / TypeScript)',
    meta: 'Hong Kong · Full-time · Product',
    tag: 'Active',
  },
  {
    title: 'Drone Systems Integration Engineer',
    meta: 'Hong Kong · Full-time · Engineering',
    tag: 'Active',
  },
];

export function CareersPage() {
  useEffect(() => { window.scrollTo(0, 0); }, []);

  return (
    <main>
      {/* Hero */}
      <SectionWrapper compact>
        <div style={{ maxWidth: '560px' }}>
          <span className="module-label">Careers</span>
          <h1 style={{
            fontSize: 'clamp(2rem, 4vw, 3rem)',
            fontWeight: 700,
            lineHeight: 1.15,
            letterSpacing: '-0.03em',
            color: 'var(--color-text-primary)',
            marginBottom: '1rem',
          }}>
            Build the infrastructure that sees.
          </h1>
          <p style={{
            fontSize: '0.95rem',
            lineHeight: 1.65,
            color: 'var(--color-text-secondary)',
            maxWidth: '48ch',
          }}>
            We are actively recruiting engineers who build systems that work under
            real-world constraints — at the edge, under power limits, in the field.
          </p>
        </div>
      </SectionWrapper>

      <hr className="divider" />

      {/* Open Roles */}
      <SectionWrapper dark compact>
        <header className="section-header">
          <span className="section-eyebrow">Open Positions</span>
          <h2>Active Recruiting</h2>
          <p>
            All roles are based in Hong Kong. We hire on technical substance —
            bring evidence, not portfolios.
          </p>
        </header>
        <div style={{ display: 'flex', flexDirection: 'column', gap: '0.75rem' }}>
          {OPEN_ROLES.map((role) => (
            <div key={role.title} className="role-card">
              <div>
                <div className="role-title">{role.title}</div>
                <div className="role-meta">{role.meta}</div>
              </div>
              <span className="role-tag">{role.tag}</span>
            </div>
          ))}
        </div>
        <div className="cta-actions">
          <Button variant="primary" href="mailto:careers@aurasensehk.com">
            Apply via Email
          </Button>
        </div>
      </SectionWrapper>
    </main>
  );
}
