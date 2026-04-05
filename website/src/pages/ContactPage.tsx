import { useEffect } from 'react';
import { Hero } from '../components/sections/Hero';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { FeatureGrid } from '../components/sections/FeatureGrid';
import { Card } from '../components/ui/Card';
import { Button } from '../components/ui/Button';
import { contactPageContent, contactEmails } from '../content/contact';

export function ContactPage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  const { hero, sections } = contactPageContent;
  const supportSection = sections.find((s) => s.id?.includes('support'));
  const inquirySections = sections.filter((s) => s !== supportSection);

  return (
    <main>
      <Hero content={hero} />

      {/* Inquiry Paths */}
      <SectionWrapper id="inquiry-paths" dark>
        <header className="section-header">
          <h2>How to Reach Us</h2>
          <p>Select the inquiry path that best matches your needs.</p>
        </header>
        <div
          style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(auto-fill, minmax(300px, 1fr))',
            gap: '1.5rem',
            marginTop: '2rem',
          }}
        >
          {inquirySections.map((section) => (
            <div key={section.id} className="inquiry-card">
              <Card
                title={section.title}
                description={section.description}
              />
              {section.items && section.items.length > 0 && (
                <div style={{ marginTop: '1rem' }}>
                  {section.items.map((item) => (
                    <p
                      key={item.title}
                      style={{
                        color: '#94a3b8',
                        fontSize: '0.875rem',
                        lineHeight: 1.6,
                        marginBottom: '0.5rem',
                      }}
                    >
                      <strong style={{ color: '#e2e8f0' }}>{item.title}:</strong>{' '}
                      {item.description}
                    </p>
                  ))}
                </div>
              )}
            </div>
          ))}
        </div>
      </SectionWrapper>

      {/* Support Section */}
      {supportSection && (
        <SectionWrapper id={supportSection.id}>
          <header className="section-header">
            {supportSection.eyebrow && (
              <span className="section-eyebrow">{supportSection.eyebrow}</span>
            )}
            <h2>{supportSection.title}</h2>
            <p>{supportSection.description}</p>
          </header>
          {supportSection.items && supportSection.items.length > 0 && (
            <FeatureGrid items={supportSection.items} columns={3} />
          )}
        </SectionWrapper>
      )}

      {/* Direct Contact */}
      <SectionWrapper id="direct-contact" dark>
        <header className="section-header">
          <h2>Direct Contact</h2>
          <p>Reach the appropriate team directly.</p>
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
              label: 'General Inquiries',
              email: contactEmails.general,
            },
            {
              label: 'Technical Support',
              email: contactEmails.support,
            },
            {
              label: 'Careers',
              email: contactEmails.careers,
            },
          ].map((contact) => (
            <div
              key={contact.label}
              style={{
                padding: '1.5rem',
                border: '1px solid #1e293b',
                borderRadius: '6px',
                backgroundColor: 'rgba(15, 23, 42, 0.5)',
              }}
            >
              <h3 style={{ color: '#ffffff', fontSize: '1rem', marginBottom: '0.5rem' }}>
                {contact.label}
              </h3>
              <a
                href={`mailto:${contact.email}`}
                style={{
                  color: '#14b8a6',
                  fontSize: '0.875rem',
                  textDecoration: 'none',
                }}
              >
                {contact.email}
              </a>
            </div>
          ))}
        </div>
      </SectionWrapper>

      <SectionWrapper id="contact-cta">
        <header className="section-header">
          <h2>Start a Conversation</h2>
          <p>
            Whether you are evaluating the platform, exploring partnership, or
            considering a role — we respond to qualified inquiries promptly.
          </p>
        </header>
        <div className="cta-actions">
          <Button variant="primary" href={`mailto:${contactEmails.general}`}>
            Email Us
          </Button>
        </div>
      </SectionWrapper>
    </main>
  );
}
