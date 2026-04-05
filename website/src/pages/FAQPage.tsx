import { useEffect, useState } from 'react';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { Button } from '../components/ui/Button';

interface FAQEntry {
  question: string;
  answer: string;
}

interface FAQGroup {
  title: string;
  entries: FAQEntry[];
}

const faqGroups: FAQGroup[] = [
  {
    title: 'Trial & Billing',
    entries: [
      {
        question: 'Is there a free trial?',
        answer:
          'Yes. Every new account starts with a 7-day full-access trial of the SFSVC SDK. No credit card required — just sign up, pull the Docker image, and start processing frames.',
      },
      {
        question: 'Do I need a credit card to start the trial?',
        answer:
          'No. The trial activates automatically when you create an account. We only require payment information when you choose to continue after the trial period.',
      },
      {
        question: 'Can I cancel during the trial?',
        answer:
          'Absolutely. If you decide the SDK is not a fit, simply let the trial expire. No charges, no cancellation fees, no questions asked.',
      },
      {
        question: 'What happens after the trial ends?',
        answer:
          'Your SDK access pauses until you select a plan. All configuration and telemetry data is retained for 30 days so you can resume where you left off.',
      },
      {
        question: 'Can I upgrade my tier mid-cycle?',
        answer:
          'Yes. Upgrades take effect immediately and are prorated for the remainder of your billing cycle. Downgrades apply at the start of the next cycle.',
      },
    ],
  },
  {
    title: 'Licensing',
    entries: [
      {
        question: 'How does the Ed25519 license key work?',
        answer:
          'Each license key is an Ed25519-signed token bound to your organization ID and tier. The SDK verifies the signature locally at startup — no phone-home required during normal operation.',
      },
      {
        question: 'Can I deploy on multiple drones at once?',
        answer:
          'Yes, within the concurrency limits of your tier. Starter allows 1 concurrent instance, Professional allows up to 5, and Enterprise supports unlimited concurrent deployments.',
      },
      {
        question: 'What happens when a license key expires?',
        answer:
          'The SDK enters a grace period of 72 hours, during which it continues to operate normally while warning via the telemetry channel. After the grace period, processing pauses until reactivation.',
      },
      {
        question: 'Does the SDK work offline and in BVLOS scenarios?',
        answer:
          'Yes. After initial activation, the SDK operates fully offline. License validation is cached locally, and all perception, classification, and audit-chain functions work without network access — critical for BVLOS (beyond visual line of sight) operations.',
      },
      {
        question: 'Which platforms are supported?',
        answer:
          'Linux x86-64 with AVX2 is the primary target. ARM64 (Jetson, RPi 5) support is in active development. Docker images are published for both architectures. Windows and macOS are supported for development via Docker Desktop.',
      },
    ],
  },
  {
    title: 'Technical',
    entries: [
      {
        question: 'What are the minimum hardware requirements?',
        answer:
          'x86-64 CPU with AVX2 (Intel Haswell or newer / AMD Zen+), 4 GB RAM, 2 GB disk, Docker 20.10+. A 4-core CPU is recommended for sustained 125+ FPS throughput.',
      },
      {
        question: 'Does NEPA require a GPU?',
        answer:
          'No. The entire SFSVC pipeline — spike encoding, STDP learning, anomaly scoring — runs on CPU using AVX2 SIMD intrinsics. GPU acceleration is neither required nor used in the production runtime.',
      },
      {
        question: 'How does this compare to OpenCV-based approaches?',
        answer:
          'Traditional OpenCV pipelines (bilateral filter → Canny → contour analysis) typically run at 15–40 FPS with P95 latency of 8–25 ms. SFSVC achieves 125+ FPS with P95 < 0.5 ms by replacing the classical pipeline with a neuromorphic spike codec optimized for edge hardware.',
      },
      {
        question: 'Can I integrate with ROS 2?',
        answer:
          'Yes. The SDK publishes a standard ROS 2 topic interface. Crack events, anomaly scores, and evidence frames are available as ROS messages. See the integration guide in the SDK documentation.',
      },
      {
        question: 'Where can I find detailed API documentation?',
        answer:
          'Full API reference is available at /resources. The SDK also ships with OpenAPI specs and example scripts in the Docker image under /opt/sfsvc/examples.',
      },
    ],
  },
];

export function FAQPage() {
  useEffect(() => {
    window.scrollTo(0, 0);
  }, []);

  return (
    <main>
      {/* Hero */}
      <SectionWrapper id="faq-hero" dark>
        <div style={{ maxWidth: '720px', paddingTop: '3rem', paddingBottom: '2rem' }}>
          <h1 style={{ color: '#ffffff', fontSize: '2.5rem', fontWeight: 700, marginBottom: '1rem' }}>
            Frequently Asked Questions
          </h1>
          <p style={{ color: '#94a3b8', fontSize: '1.125rem', lineHeight: 1.6 }}>
            Answers for engineering teams, evaluators, and procurement stakeholders.
          </p>
        </div>
      </SectionWrapper>

      {/* FAQ Groups */}
      <SectionWrapper id="faq-list">
        <div style={{ maxWidth: '720px', margin: '0 auto' }}>
          {faqGroups.map((group) => (
            <div key={group.title} style={{ marginBottom: '2.5rem' }}>
              <h2
                style={{
                  color: '#14b8a6',
                  fontSize: '1.25rem',
                  fontWeight: 700,
                  letterSpacing: '0.05em',
                  textTransform: 'uppercase',
                  marginBottom: '1rem',
                  paddingBottom: '0.5rem',
                  borderBottom: '2px solid #14b8a6',
                }}
              >
                {group.title}
              </h2>
              {group.entries.map((entry) => (
                <FAQItem key={entry.question} question={entry.question} answer={entry.answer} />
              ))}
            </div>
          ))}
        </div>
      </SectionWrapper>

      {/* CTA */}
      <SectionWrapper id="faq-cta" dark>
        <header className="section-header">
          <h2>Still have questions?</h2>
          <p>Reach out to our team for technical or commercial inquiries.</p>
        </header>
        <div className="cta-actions">
          <Button variant="primary" href="/contact">Contact Us</Button>
          <Button variant="secondary" href="/resources">View Resources</Button>
        </div>
      </SectionWrapper>
    </main>
  );
}

function FAQItem({ question, answer }: FAQEntry) {
  const [open, setOpen] = useState(false);

  return (
    <div
      style={{
        borderBottom: '1px solid var(--border-subtle, #1e293b)',
        padding: '1.25rem 0',
      }}
    >
      <button
        onClick={() => setOpen(!open)}
        aria-expanded={open}
        style={{
          all: 'unset',
          cursor: 'pointer',
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
          width: '100%',
          color: '#ffffff',
          fontWeight: 600,
          fontSize: '1rem',
        }}
      >
        <span>{question}</span>
        <span style={{ color: '#14b8a6', fontSize: '1.25rem', marginLeft: '1rem' }}>
          {open ? '−' : '+'}
        </span>
      </button>
      {open && (
        <p style={{ color: '#94a3b8', marginTop: '0.75rem', lineHeight: 1.6 }}>
          {answer}
        </p>
      )}
    </div>
  );
}
