import { useEffect } from 'react';
import { Hero } from '../components/sections/Hero';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { Card } from '../components/ui/Card';
import { Button } from '../components/ui/Button';
import { resourcesPageContent, resourceCategories, featuredResources } from '../content/resources';
import type { ResourceEntry } from '../types/content';

export function ResourcesPage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  const { hero, sections } = resourcesPageContent;

  return (
    <main>
      <Hero content={hero} />

      {/* Resource Categories */}
      <SectionWrapper id="categories" dark>
        <header className="section-header">
          <h2>Resource Categories</h2>
          <p>Documentation, specifications, and evaluation materials organized by domain.</p>
        </header>
        <div
          className="category-grid"
          style={{
            display: 'flex',
            flexWrap: 'wrap',
            gap: '0.75rem',
            marginTop: '2rem',
          }}
        >
          {resourceCategories.map((category) => (
            <span
              key={category}
              style={{
                padding: '0.5rem 1rem',
                border: '1px solid #1e293b',
                borderRadius: '4px',
                color: '#14b8a6',
                fontSize: '0.875rem',
                fontWeight: 500,
                backgroundColor: 'rgba(20, 184, 166, 0.05)',
              }}
            >
              {category}
            </span>
          ))}
        </div>
      </SectionWrapper>

      {/* Featured Resources */}
      <SectionWrapper id="featured-resources">
        <header className="section-header">
          <h2>Featured Resources</h2>
          <p>Selected documentation and evaluation materials for technical teams.</p>
        </header>
        <div
          className="resource-grid"
          style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(auto-fill, minmax(340px, 1fr))',
            gap: '1.5rem',
            marginTop: '2rem',
          }}
        >
          {featuredResources.map((resource: ResourceEntry) => (
            <div key={resource.title} className="resource-card" style={{ position: 'relative' }}>
              <div
                style={{
                  display: 'flex',
                  gap: '0.5rem',
                  marginBottom: '0.75rem',
                }}
              >
                <span
                  style={{
                    padding: '0.25rem 0.625rem',
                    fontSize: '0.75rem',
                    fontWeight: 600,
                    color: '#14b8a6',
                    backgroundColor: 'rgba(20, 184, 166, 0.1)',
                    borderRadius: '3px',
                    textTransform: 'uppercase',
                    letterSpacing: '0.025em',
                  }}
                >
                  {resource.type}
                </span>
                <span
                  style={{
                    padding: '0.25rem 0.625rem',
                    fontSize: '0.75rem',
                    fontWeight: 600,
                    color: '#94a3b8',
                    backgroundColor: 'rgba(148, 163, 184, 0.1)',
                    borderRadius: '3px',
                    textTransform: 'uppercase',
                    letterSpacing: '0.025em',
                  }}
                >
                  {resource.audience}
                </span>
              </div>
              <Card
                title={resource.title}
                description={resource.description}
                cta={resource.cta}
              />
            </div>
          ))}
        </div>
      </SectionWrapper>

      {/* Audience Segmentation */}
      <SectionWrapper id="sdk-quickstart" dark>
        <header className="section-header">
          <h2>SDK Quickstart</h2>
          <p>Get the SFSVC runtime running in under a minute with Docker.</p>
        </header>
        <div
          style={{
            maxWidth: '720px',
            margin: '2rem auto 0',
            backgroundColor: 'rgba(15, 23, 42, 0.6)',
            border: '1px solid #1e293b',
            borderRadius: '8px',
            padding: '1.5rem',
          }}
        >
          <pre
            style={{
              color: '#e2e8f0',
              fontSize: '0.875rem',
              lineHeight: 1.8,
              overflowX: 'auto',
              margin: 0,
              fontFamily: '"JetBrains Mono", "Fira Code", monospace',
            }}
          >
{`# Pull the runtime image
docker pull aurasensehk/sfsvc:latest

# Run with your license key
docker run -d \\
  -e SFSVC_LICENSE_KEY=<your-key> \\
  -p 8000:8000 \\
  aurasensehk/sfsvc:latest`}
          </pre>
        </div>
      </SectionWrapper>

      {/* Requirements */}
      <SectionWrapper id="sdk-requirements">
        <header className="section-header">
          <h2>System Requirements</h2>
          <p>Minimum and recommended specifications for SFSVC deployment.</p>
        </header>
        <div
          style={{
            maxWidth: '720px',
            margin: '2rem auto 0',
            overflowX: 'auto',
          }}
        >
          <table
            style={{
              width: '100%',
              borderCollapse: 'collapse',
              fontSize: '0.9rem',
            }}
          >
            <thead>
              <tr>
                {['Component', 'Minimum', 'Recommended'].map((h) => (
                  <th
                    key={h}
                    style={{
                      textAlign: 'left',
                      padding: '0.75rem 1rem',
                      borderBottom: '2px solid #14b8a6',
                      color: '#14b8a6',
                      fontWeight: 700,
                      fontSize: '0.8rem',
                      textTransform: 'uppercase',
                      letterSpacing: '0.05em',
                    }}
                  >
                    {h}
                  </th>
                ))}
              </tr>
            </thead>
            <tbody>
              {[
                ['CPU', 'x86-64 with AVX2', '4+ cores, Zen 2 / 10th-gen Intel'],
                ['RAM', '4 GB', '8 GB'],
                ['GPU', 'Not required', 'Not required'],
                ['OS', 'Linux (kernel 5.4+)', 'Ubuntu 22.04 LTS'],
                ['Docker', '20.10+', '24.0+'],
                ['Camera', 'USB / CSI / RTSP', 'Global shutter ≥ 720p'],
              ].map(([component, min, rec]) => (
                <tr key={component}>
                  <td
                    style={{
                      padding: '0.75rem 1rem',
                      color: '#ffffff',
                      fontWeight: 600,
                      borderBottom: '1px solid #1e293b',
                    }}
                  >
                    {component}
                  </td>
                  <td
                    style={{
                      padding: '0.75rem 1rem',
                      color: '#94a3b8',
                      borderBottom: '1px solid #1e293b',
                    }}
                  >
                    {min}
                  </td>
                  <td
                    style={{
                      padding: '0.75rem 1rem',
                      color: '#94a3b8',
                      borderBottom: '1px solid #1e293b',
                    }}
                  >
                    {rec}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </SectionWrapper>

      {/* Configuration */}
      <SectionWrapper id="sdk-configuration" dark>
        <header className="section-header">
          <h2>Configuration</h2>
          <p>Environment variables accepted by the SFSVC Docker container.</p>
        </header>
        <div
          style={{
            maxWidth: '720px',
            margin: '2rem auto 0',
            overflowX: 'auto',
          }}
        >
          <table
            style={{
              width: '100%',
              borderCollapse: 'collapse',
              fontSize: '0.9rem',
            }}
          >
            <thead>
              <tr>
                {['Variable', 'Default', 'Description'].map((h) => (
                  <th
                    key={h}
                    style={{
                      textAlign: 'left',
                      padding: '0.75rem 1rem',
                      borderBottom: '2px solid #14b8a6',
                      color: '#14b8a6',
                      fontWeight: 700,
                      fontSize: '0.8rem',
                      textTransform: 'uppercase',
                      letterSpacing: '0.05em',
                    }}
                  >
                    {h}
                  </th>
                ))}
              </tr>
            </thead>
            <tbody>
              {[
                ['SFSVC_LICENSE_KEY', '(none)', 'Ed25519 license token — required'],
                ['MISSION_PROFILE', 'facade', 'facade | bridge | runway'],
                ['PORT', '8000', 'HTTP listen port'],
                ['ENABLE_THERMAL', 'false', 'Enable thermal overlay fusion'],
                ['STDP_ENABLED', 'true', 'Online STDP adaptation'],
                ['LOG_LEVEL', 'info', 'debug | info | warn | error'],
              ].map(([v, def, desc]) => (
                <tr key={v}>
                  <td
                    style={{
                      padding: '0.75rem 1rem',
                      color: '#e2e8f0',
                      fontFamily: '"JetBrains Mono", "Fira Code", monospace',
                      fontSize: '0.825rem',
                      borderBottom: '1px solid #1e293b',
                    }}
                  >
                    {v}
                  </td>
                  <td
                    style={{
                      padding: '0.75rem 1rem',
                      color: '#94a3b8',
                      fontFamily: '"JetBrains Mono", "Fira Code", monospace',
                      fontSize: '0.825rem',
                      borderBottom: '1px solid #1e293b',
                    }}
                  >
                    {def}
                  </td>
                  <td
                    style={{
                      padding: '0.75rem 1rem',
                      color: '#94a3b8',
                      borderBottom: '1px solid #1e293b',
                    }}
                  >
                    {desc}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </SectionWrapper>

      {/* API Reference */}
      <SectionWrapper id="api-reference">
        <header className="section-header">
          <h2>API Reference</h2>
          <p>REST API served on localhost:8000. All payloads are JSON.</p>
        </header>
        <div
          style={{
            maxWidth: '720px',
            margin: '2rem auto 0',
            display: 'flex',
            flexDirection: 'column',
            gap: '1.5rem',
          }}
        >
          {/* POST /v1/frames */}
          <div
            style={{
              backgroundColor: 'rgba(15, 23, 42, 0.4)',
              border: '1px solid #1e293b',
              borderRadius: '8px',
              padding: '1.5rem',
            }}
          >
            <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem', marginBottom: '0.75rem' }}>
              <span
                style={{
                  padding: '0.25rem 0.625rem',
                  fontSize: '0.75rem',
                  fontWeight: 700,
                  color: '#14b8a6',
                  backgroundColor: 'rgba(20, 184, 166, 0.15)',
                  borderRadius: '3px',
                  fontFamily: 'monospace',
                }}
              >
                POST
              </span>
              <code style={{ color: '#e2e8f0', fontSize: '0.9rem' }}>/v1/frames</code>
            </div>
            <p style={{ color: '#94a3b8', fontSize: '0.875rem', marginBottom: '1rem' }}>
              Submit a frame for processing. Returns crack detections, anomaly scores, and evidence metadata.
            </p>
            <div style={{ fontSize: '0.825rem', color: '#64748b' }}>
              <strong style={{ color: '#94a3b8' }}>Request:</strong>{' '}
              <code style={{ color: '#e2e8f0' }}>{'{ "image_b64": "...", "zone_id": "A-12", "timestamp": "..." }'}</code>
            </div>
            <div style={{ fontSize: '0.825rem', color: '#64748b', marginTop: '0.5rem' }}>
              <strong style={{ color: '#94a3b8' }}>Response:</strong>{' '}
              <code style={{ color: '#e2e8f0' }}>{'{ "cracks": [...], "anomaly_score": 0.42, "evidence_id": "..." }'}</code>
            </div>
          </div>

          {/* GET /v1/events */}
          <div
            style={{
              backgroundColor: 'rgba(15, 23, 42, 0.4)',
              border: '1px solid #1e293b',
              borderRadius: '8px',
              padding: '1.5rem',
            }}
          >
            <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem', marginBottom: '0.75rem' }}>
              <span
                style={{
                  padding: '0.25rem 0.625rem',
                  fontSize: '0.75rem',
                  fontWeight: 700,
                  color: '#38bdf8',
                  backgroundColor: 'rgba(56, 189, 248, 0.15)',
                  borderRadius: '3px',
                  fontFamily: 'monospace',
                }}
              >
                GET
              </span>
              <code style={{ color: '#e2e8f0', fontSize: '0.9rem' }}>/v1/events</code>
            </div>
            <p style={{ color: '#94a3b8', fontSize: '0.875rem', marginBottom: '1rem' }}>
              Retrieve the audit event stream. Supports filtering by zone, severity, and time range.
            </p>
            <div style={{ fontSize: '0.825rem', color: '#64748b' }}>
              <strong style={{ color: '#94a3b8' }}>Params:</strong>{' '}
              <code style={{ color: '#e2e8f0' }}>?zone=A-12&severity=high&since=2024-01-01T00:00:00Z</code>
            </div>
          </div>

          {/* GET /v1/health */}
          <div
            style={{
              backgroundColor: 'rgba(15, 23, 42, 0.4)',
              border: '1px solid #1e293b',
              borderRadius: '8px',
              padding: '1.5rem',
            }}
          >
            <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem', marginBottom: '0.75rem' }}>
              <span
                style={{
                  padding: '0.25rem 0.625rem',
                  fontSize: '0.75rem',
                  fontWeight: 700,
                  color: '#38bdf8',
                  backgroundColor: 'rgba(56, 189, 248, 0.15)',
                  borderRadius: '3px',
                  fontFamily: 'monospace',
                }}
              >
                GET
              </span>
              <code style={{ color: '#e2e8f0', fontSize: '0.9rem' }}>/v1/health</code>
            </div>
            <p style={{ color: '#94a3b8', fontSize: '0.875rem' }}>
              Health check endpoint. Returns runtime version, uptime, and license status.
            </p>
          </div>
        </div>
      </SectionWrapper>

      {/* Audience Segmentation */}
      <SectionWrapper id="audience" dark>
        <header className="section-header">
          <h2>Resources by Audience</h2>
          <p>Access materials tailored to your role and evaluation stage.</p>
        </header>
        <div
          style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(auto-fill, minmax(280px, 1fr))',
            gap: '1.5rem',
            marginTop: '2rem',
          }}
        >
          {[
            {
              title: 'Engineering Teams',
              description:
                'SDK documentation, API references, integration guides, and deployment specifications for development and systems teams.',
            },
            {
              title: 'Technical Evaluators',
              description:
                'Architecture briefs, benchmark reports, comparison matrices, and compliance documentation for technical decision-makers.',
            },
            {
              title: 'Operations & Deployment',
              description:
                'Deployment playbooks, configuration references, monitoring guides, and edge infrastructure documentation.',
            },
            {
              title: 'Procurement & Legal',
              description:
                'Licensing terms, data governance documentation, regulatory compliance materials, and commercial evaluation packages.',
            },
          ].map((audience) => (
            <Card key={audience.title} title={audience.title} description={audience.description} />
          ))}
        </div>
      </SectionWrapper>

      {/* Support & Access Model */}
      <SectionWrapper id="access-model">
        <header className="section-header">
          <h2>Access & Support Model</h2>
          <p>All resources are available through controlled evaluation access.</p>
        </header>
        <div
          style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(auto-fill, minmax(280px, 1fr))',
            gap: '1.5rem',
            marginTop: '2rem',
          }}
        >
          {[
            {
              title: 'Evaluation Access',
              description:
                'Request access to SDK packages, documentation portals, and integration environments. Available to qualified engineering teams.',
            },
            {
              title: 'Technical Support',
              description:
                'Direct engineering support for integration, deployment, and operational questions. Available to evaluation and licensed partners.',
            },
            {
              title: 'Documentation Portal',
              description:
                'Comprehensive technical documentation, API references, and deployment guides. Access granted with evaluation or license agreement.',
            },
          ].map((item) => (
            <Card key={item.title} title={item.title} description={item.description} />
          ))}
        </div>
      </SectionWrapper>

      {/* Content sections from data */}
      {sections.map((section, index) => (
        <SectionWrapper key={section.id} id={section.id} dark={index % 2 === 0}>
          <header className="section-header">
            {section.eyebrow && (
              <span className="section-eyebrow">{section.eyebrow}</span>
            )}
            <h2>{section.title}</h2>
            <p>{section.description}</p>
          </header>
        </SectionWrapper>
      ))}

      <SectionWrapper id="resources-cta" dark>
        <header className="section-header">
          <h2>Request Evaluation Access</h2>
          <p>
            Access the full documentation library, SDK packages, and technical support
            through our evaluation program.
          </p>
        </header>
        <div className="cta-actions">
          <Button variant="primary" href="/contact">Request Access</Button>
          <Button variant="secondary" href="/platform">View Platform</Button>
        </div>
      </SectionWrapper>
    </main>
  );
}
