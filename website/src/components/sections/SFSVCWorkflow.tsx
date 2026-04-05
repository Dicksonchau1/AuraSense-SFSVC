const pipelineStages = [
  'Event Sensors',
  'Spike Encoder',
  'SNN Inference Core',
  'Defect Classifier',
  'Vector Output',
];

export function SFSVCWorkflow() {
  return (
    <section
      style={{
        width: '100%',
        background: '#0a0f1a',
        padding: '5rem 0',
      }}
    >
      <div style={{ maxWidth: '1280px', margin: '0 auto', padding: '0 1.5rem' }}>
        {/* Section header */}
        <header style={{ textAlign: 'center', marginBottom: '3rem' }}>
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
            System Architecture
          </span>
          <h2
            style={{
              color: '#ffffff',
              fontSize: 'clamp(1.75rem, 3vw, 2.5rem)',
              fontWeight: 700,
              lineHeight: 1.2,
              marginBottom: '1rem',
            }}
          >
            SFSVC Inference Pipeline
          </h2>
          <p
            style={{
              color: '#94a3b8',
              fontSize: '1.0625rem',
              maxWidth: '640px',
              margin: '0 auto',
              lineHeight: 1.7,
            }}
          >
            From raw sensor event stream to vectorised defect map — the complete
            neuromorphic processing chain.
          </p>
        </header>

        {/* Workflow diagram image */}
        <div style={{ textAlign: 'center', marginBottom: '3rem' }}>
          <img
            src="/assets/sfsvc/sfsvc-workflow-diagram.png"
            alt="SFSVC inference pipeline architecture diagram"
            loading="lazy"
            decoding="async"
            className="sfsvc-workflow-img"
            style={{
              maxWidth: '960px',
              width: '100%',
              borderRadius: '12px',
              border: '1px solid rgba(255,255,255,0.08)',
              transition: 'box-shadow 0.3s ease',
            }}
          />
        </div>

        {/* Pipeline stage pills */}
        <div
          className="sfsvc-pipeline-stages"
          style={{
            display: 'flex',
            flexWrap: 'wrap',
            justifyContent: 'center',
            alignItems: 'center',
            gap: '0.5rem',
          }}
        >
          {pipelineStages.map((stage, index) => (
            <div
              key={stage}
              style={{
                display: 'flex',
                alignItems: 'center',
                gap: '0.5rem',
              }}
            >
              <span
                style={{
                  display: 'inline-block',
                  padding: '0.5rem 1rem',
                  background: '#0f172a',
                  border: '1px solid rgba(20, 184, 166, 0.3)',
                  borderRadius: '8px',
                  color: '#14b8a6',
                  fontSize: '0.8125rem',
                  fontWeight: 600,
                  whiteSpace: 'nowrap',
                }}
              >
                {stage}
              </span>
              {index < pipelineStages.length - 1 && (
                <span
                  style={{
                    color: '#475569',
                    fontSize: '1.25rem',
                    fontWeight: 400,
                    userSelect: 'none',
                  }}
                >
                  →
                </span>
              )}
            </div>
          ))}
        </div>
      </div>

      {/* Hover glow for the diagram image */}
      <style>{`
        .sfsvc-workflow-img:hover {
          box-shadow: 0 0 40px rgba(20, 184, 166, 0.15);
        }
      `}</style>
    </section>
  );
}
