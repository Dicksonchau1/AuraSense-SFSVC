const stats = [
  { value: '<1ms', label: 'Inference Latency' },
  { value: '99.2%', label: 'Detection Recall' },
  { value: '0.3mm', label: 'Minimum Crack Width' },
];

export function SFSVCInferenceOutput() {
  return (
    <section
      style={{
        width: '100%',
        background: '#080c14',
        padding: '5rem 0',
      }}
    >
      <div
        className="sfsvc-inference-grid"
        style={{
          maxWidth: '1280px',
          margin: '0 auto',
          padding: '0 1.5rem',
          display: 'grid',
          gap: '3rem',
          alignItems: 'center',
        }}
      >
        {/* Left column — text */}
        <div>
          <span
            style={{
              display: 'inline-block',
              color: '#14b8a6',
              fontSize: '0.75rem',
              fontWeight: 600,
              letterSpacing: '0.1em',
              textTransform: 'uppercase',
              marginBottom: '0.75rem',
            }}
          >
            Live Output
          </span>
          <h2
            style={{
              color: '#ffffff',
              fontSize: 'clamp(1.75rem, 3vw, 2.5rem)',
              fontWeight: 700,
              lineHeight: 1.2,
              marginBottom: '1.25rem',
            }}
          >
            Real-World Inference Results
          </h2>
          <p
            style={{
              color: '#94a3b8',
              fontSize: '1.0625rem',
              lineHeight: 1.7,
              marginBottom: '2rem',
              maxWidth: '540px',
            }}
          >
            The SFSVC inference engine processes event camera streams at
            sub-millisecond latency, producing classified defect vectors with
            severity grading, spatial coordinates, and confidence scores directly
            on the Jetson Nano edge device — no cloud dependency.
          </p>

          {/* Stat callouts */}
          <div
            style={{
              display: 'grid',
              gridTemplateColumns: 'repeat(3, 1fr)',
              gap: '1.5rem',
            }}
          >
            {stats.map((stat) => (
              <div key={stat.label}>
                <span
                  style={{
                    display: 'block',
                    color: '#14b8a6',
                    fontSize: '2.5rem',
                    fontWeight: 700,
                    lineHeight: 1.1,
                    marginBottom: '0.25rem',
                  }}
                >
                  {stat.value}
                </span>
                <span
                  style={{
                    color: '#94a3b8',
                    fontSize: '0.8125rem',
                    fontWeight: 500,
                  }}
                >
                  {stat.label}
                </span>
              </div>
            ))}
          </div>
        </div>

        {/* Right column — image */}
        <div>
          <img
            src="/assets/sfsvc/sfsvc-inference-output.png"
            alt="SFSVC live inference output showing crack detection overlays"
            loading="lazy"
            decoding="async"
            className="sfsvc-inference-img"
            style={{
              width: '100%',
              borderRadius: '12px',
              border: '1px solid rgba(255,255,255,0.08)',
              transition: 'transform 0.3s ease',
            }}
          />
        </div>
      </div>

      {/* Responsive grid + hover effect */}
      <style>{`
        .sfsvc-inference-grid {
          grid-template-columns: 1fr;
        }
        @media (min-width: 768px) {
          .sfsvc-inference-grid {
            grid-template-columns: 3fr 2fr;
          }
        }
        .sfsvc-inference-img:hover {
          transform: scale(1.01);
        }
      `}</style>
    </section>
  );
}
