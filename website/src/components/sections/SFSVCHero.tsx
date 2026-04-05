export function SFSVCHero() {
  return (
    <section
      style={{
        position: 'relative',
        width: '100%',
        height: '100vh',
        display: 'flex',
        alignItems: 'center',
        overflow: 'hidden',
        background: '#080c14',
      }}
    >
      {/* Background video */}
      <video
        src="/assets/sfsvc/sfsvc-hero-video.mp4"
        autoPlay
        muted
        loop
        playsInline
        style={{
          position: 'absolute',
          inset: 0,
          width: '100%',
          height: '100%',
          objectFit: 'cover',
          zIndex: 0,
        }}
      />

      {/* Dark gradient overlay */}
      <div
        style={{
          position: 'absolute',
          inset: 0,
          background:
            'linear-gradient(to bottom, rgba(0,0,0,0.55) 0%, rgba(0,0,0,0.75) 100%)',
          zIndex: 0,
        }}
      />

      {/* Content */}
      <div className="sfsvc-hero-content" style={{ position: 'relative', zIndex: 1, width: '100%' }}>
        <div
          style={{
            maxWidth: '1280px',
            margin: '0 auto',
            padding: '0 1.5rem',
          }}
        >
          {/* Eyebrow */}
          <span
            style={{
              display: 'inline-block',
              color: '#14b8a6',
              fontSize: '0.75rem',
              fontWeight: 600,
              letterSpacing: '0.1em',
              textTransform: 'uppercase',
              marginBottom: '1rem',
              background: 'rgba(20, 184, 166, 0.1)',
              border: '1px solid rgba(20, 184, 166, 0.3)',
              borderRadius: '9999px',
              padding: '0.35rem 1rem',
            }}
          >
            AuraSense · SFSVC
          </span>

          {/* Heading */}
          <h1
            className="sfsvc-hero-heading"
            style={{
              color: '#ffffff',
              fontWeight: 700,
              lineHeight: 1.1,
              marginBottom: '1.25rem',
            }}
          >
            Structural Facade Scanning
            <br />
            &amp; Vectorisation Core
          </h1>

          {/* Subheading */}
          <p
            className="sfsvc-hero-subheading"
            style={{
              color: '#94a3b8',
              lineHeight: 1.7,
              marginBottom: '2rem',
            }}
          >
            Neuromorphic event-driven crack detection and facade defect
            vectorisation for autonomous drone inspection at sub-millimetre
            resolution.
          </p>

          {/* CTA buttons */}
          <div style={{ display: 'flex', flexWrap: 'wrap', gap: '1rem' }}>
            <a
              href="/contact"
              style={{
                display: 'inline-flex',
                alignItems: 'center',
                justifyContent: 'center',
                padding: '0.875rem 2rem',
                backgroundColor: '#14b8a6',
                color: '#080c14',
                fontWeight: 600,
                fontSize: '0.9375rem',
                borderRadius: '0.5rem',
                textDecoration: 'none',
                transition: 'background-color 0.3s ease',
              }}
              onMouseEnter={(e) =>
                (e.currentTarget.style.backgroundColor = '#0d9488')
              }
              onMouseLeave={(e) =>
                (e.currentTarget.style.backgroundColor = '#14b8a6')
              }
            >
              Request SDK Access
            </a>
            <a
              href="#specs"
              style={{
                display: 'inline-flex',
                alignItems: 'center',
                justifyContent: 'center',
                padding: '0.875rem 2rem',
                backgroundColor: 'transparent',
                color: '#ffffff',
                fontWeight: 600,
                fontSize: '0.9375rem',
                borderRadius: '0.5rem',
                border: '1px solid rgba(255,255,255,0.2)',
                textDecoration: 'none',
                transition: 'border-color 0.3s ease',
              }}
              onMouseEnter={(e) =>
                (e.currentTarget.style.borderColor = 'rgba(20,184,166,0.5)')
              }
              onMouseLeave={(e) =>
                (e.currentTarget.style.borderColor = 'rgba(255,255,255,0.2)')
              }
            >
              View Technical Specs
            </a>
          </div>
        </div>
      </div>

      {/* Scroll chevron */}
      <div
        style={{
          position: 'absolute',
          bottom: '2rem',
          left: '50%',
          transform: 'translateX(-50%)',
          zIndex: 1,
          animation: 'sfsvcPulse 2s ease-in-out infinite',
        }}
      >
        <svg
          width="24"
          height="24"
          viewBox="0 0 24 24"
          fill="none"
          stroke="#94a3b8"
          strokeWidth="2"
          strokeLinecap="round"
          strokeLinejoin="round"
        >
          <polyline points="6 9 12 15 18 9" />
        </svg>
      </div>

      {/* Keyframes + responsive styles */}
      <style>{`
        @keyframes sfsvcPulse {
          0%, 100% { opacity: 0.4; transform: translateX(-50%) translateY(0); }
          50% { opacity: 1; transform: translateX(-50%) translateY(6px); }
        }
        .sfsvc-hero-heading {
          font-size: clamp(2rem, 5vw, 3.5rem);
        }
        .sfsvc-hero-subheading {
          font-size: clamp(1rem, 2vw, 1.25rem);
          max-width: 640px;
        }
        @media (max-width: 767px) {
          .sfsvc-hero-content {
            text-align: center;
          }
          .sfsvc-hero-content > div > div:last-of-type {
            justify-content: center;
          }
        }
      `}</style>
    </section>
  );
}
