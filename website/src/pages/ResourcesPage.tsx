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
