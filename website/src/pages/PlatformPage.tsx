import { useEffect } from 'react';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { Button } from '../components/ui/Button';

export function PlatformPage() {
  useEffect(() => { window.scrollTo(0, 0); }, []);

  return (
    <main>
      {/* Hero */}
      <SectionWrapper compact>
        <div style={{ maxWidth: '640px' }}>
          <span className="module-label">NEPA Platform</span>
          <h1 style={{
            fontSize: 'clamp(2rem, 4vw, 3rem)',
            fontWeight: 700,
            lineHeight: 1.15,
            letterSpacing: '-0.03em',
            color: 'var(--color-text-primary)',
            marginBottom: '1rem',
          }}>
            Neuromorphic Edge Perception Architecture
          </h1>
          <p style={{
            fontSize: '0.95rem',
            lineHeight: 1.65,
            color: 'var(--color-text-secondary)',
            maxWidth: '50ch',
          }}>
            A unified runtime that connects spiking neural network inference,
            autonomous planning, and human-readable intelligence into a
            single deterministic edge platform.
          </p>
        </div>
      </SectionWrapper>

      <hr className="divider" />

      {/* NEPA Agent + HRI */}
      <SectionWrapper dark>
        <div className="split-layout">
          <div>
            <header className="section-header" style={{ marginBottom: '1.5rem' }}>
              <span className="section-eyebrow">NEPA Agent</span>
              <h2>AI Agent with Human-Robot Interface</h2>
              <p>
                The NEPA agent runs a persistent neural language model at the edge,
                enabling operators to issue natural-language commands while simultaneously
                receiving structured video-frame timestamp collections from the
                inspection mission. Commands like "flag all cracks wider than 0.5mm
                from the last run" are parsed, resolved to timestamped frame ranges,
                and surfaced as annotated clips — no cloud dependency.
              </p>
            </header>
            <div style={{ display: 'flex', flexDirection: 'column', gap: '0.6rem' }}>
              {[
                'Natural language command parsing at the edge',
                'Video frame timestamp collection and annotation',
                'Bidirectional HRI — speak to system, receive structured output',
                'Zero cloud round-trip — all inference on Jetson Nano',
                'Persistent session memory across inspection missions',
              ].map((item) => (
                <div key={item} style={{
                  display: 'flex', gap: '0.6rem', alignItems: 'flex-start',
                  fontSize: '0.85rem', color: 'var(--color-text-secondary)',
                }}>
                  <span style={{ color: 'var(--color-accent)', marginTop: '2px', flexShrink: 0 }}>▸</span>
                  {item}
                </div>
              ))}
            </div>
          </div>
          {/* Agent diagram — SVG */}
          <div className="media-frame" style={{ padding: '2rem', background: 'var(--color-surface-overlay)' }}>
            <svg viewBox="0 0 400 300" fill="none" xmlns="http://www.w3.org/2000/svg"
              style={{ width: '100%', height: 'auto' }}>
              {/* Operator node */}
              <rect x="20" y="110" width="100" height="36" rx="4"
                stroke="rgba(20,184,166,0.5)" strokeWidth="1" fill="rgba(20,184,166,0.06)" />
              <text x="70" y="132" textAnchor="middle" fill="#14b8a6" fontSize="10" fontFamily="Inter,sans-serif">Operator</text>
              {/* Arrow → NLP */}
              <line x1="120" y1="128" x2="150" y2="128" stroke="#334155" strokeWidth="1" markerEnd="url(#arr)" />
              {/* NLP Core */}
              <rect x="150" y="98" width="100" height="60" rx="4"
                stroke="rgba(20,184,166,0.7)" strokeWidth="1.5" fill="rgba(20,184,166,0.1)" />
              <text x="200" y="124" textAnchor="middle" fill="#f0f2f5" fontSize="10" fontWeight="600" fontFamily="Inter,sans-serif">NEPA Agent</text>
              <text x="200" y="140" textAnchor="middle" fill="#8a919e" fontSize="8.5" fontFamily="Inter,sans-serif">NLU + HRI Core</text>
              {/* Arrow → Timestamp */}
              <line x1="250" y1="118" x2="280" y2="104" stroke="#334155" strokeWidth="1" markerEnd="url(#arr)" />
              <rect x="280" y="80" width="100" height="36" rx="4"
                stroke="rgba(20,184,166,0.4)" strokeWidth="1" fill="rgba(20,184,166,0.05)" />
              <text x="330" y="102" textAnchor="middle" fill="#14b8a6" fontSize="9" fontFamily="Inter,sans-serif">Frame Timestamps</text>
              {/* Arrow → Inference */}
              <line x1="250" y1="138" x2="280" y2="152" stroke="#334155" strokeWidth="1" markerEnd="url(#arr)" />
              <rect x="280" y="140" width="100" height="36" rx="4"
                stroke="rgba(20,184,166,0.4)" strokeWidth="1" fill="rgba(20,184,166,0.05)" />
              <text x="330" y="162" textAnchor="middle" fill="#14b8a6" fontSize="9" fontFamily="Inter,sans-serif">SNN Inference Core</text>
              {/* Return arrow */}
              <line x1="200" y1="158" x2="200" y2="200" stroke="#334155" strokeWidth="1" />
              <line x1="200" y1="200" x2="70" y2="200" stroke="#334155" strokeWidth="1" />
              <line x1="70" y1="200" x2="70" y2="148" stroke="#334155" strokeWidth="1" markerEnd="url(#arr)" />
              <text x="135" y="216" textAnchor="middle" fill="#555d6b" fontSize="8" fontFamily="Inter,sans-serif">Annotated response</text>
              <defs>
                <marker id="arr" markerWidth="6" markerHeight="6" refX="3" refY="3" orient="auto">
                  <path d="M0,0 L0,6 L6,3 z" fill="#334155" />
                </marker>
              </defs>
            </svg>
          </div>
        </div>
      </SectionWrapper>

      <hr className="divider" />

      {/* NSSIM — CCTV Overlay */}
      <SectionWrapper>
        <div className="split-layout">
          <div className="media-frame" style={{ minHeight: '260px', display: 'flex', alignItems: 'center', justifyContent: 'center', background: 'var(--color-surface-overlay)' }}>
            {/* Placeholder: replace with actual CCTV overlay screenshot once asset is available */}
            <svg viewBox="0 0 480 300" fill="none" xmlns="http://www.w3.org/2000/svg" style={{ width: '100%', height: 'auto' }}>
              {/* Simulated CCTV frame */}
              <rect x="0" y="0" width="480" height="300" fill="#0c0e12" />
              <rect x="12" y="12" width="456" height="276" rx="2" stroke="rgba(20,184,166,0.3)" strokeWidth="1" />
              {/* Simulated floor plan lines */}
              <line x1="0" y1="150" x2="480" y2="150" stroke="rgba(255,255,255,0.03)" strokeWidth="1" />
              <line x1="240" y1="0" x2="240" y2="300" stroke="rgba(255,255,255,0.03)" strokeWidth="1" />
              {/* Detection overlays */}
              <rect x="60" y="90" width="80" height="90" rx="2" stroke="#14b8a6" strokeWidth="1.5" fill="rgba(20,184,166,0.06)" />
              <text x="62" y="86" fill="#14b8a6" fontSize="8" fontFamily="monospace">PERSON 0.94</text>
              <rect x="290" y="70" width="70" height="100" rx="2" stroke="#14b8a6" strokeWidth="1.5" fill="rgba(20,184,166,0.06)" />
              <text x="292" y="66" fill="#14b8a6" fontSize="8" fontFamily="monospace">PERSON 0.88</text>
              <rect x="170" y="130" width="90" height="50" rx="2" stroke="rgba(251,191,36,0.7)" strokeWidth="1.5" fill="rgba(251,191,36,0.04)" />
              <text x="172" y="126" fill="rgba(251,191,36,0.9)" fontSize="8" fontFamily="monospace">ALERT ZONE</text>
              {/* HUD text */}
              <text x="16" y="28" fill="rgba(20,184,166,0.6)" fontSize="8" fontFamily="monospace">NSSIM v1 · SITE A · CH-04</text>
              <text x="380" y="28" fill="rgba(20,184,166,0.6)" fontSize="8" fontFamily="monospace">LIVE</text>
              <circle cx="474" cy="24" r="4" fill="#14b8a6" opacity="0.8" />
            </svg>
          </div>
          <div>
            <header className="section-header" style={{ marginBottom: '1.5rem' }}>
              <span className="section-eyebrow">NSSIM</span>
              <h2>Surveillance Dashboard with CCTV Overlay</h2>
              <p>
                NSSIM renders temporal intelligence directly over live CCTV feeds.
                The operator sees real-time detection bounding boxes, alert zones,
                risk scores, and audit flags — all processed on-device. No video
                leaves the site.
              </p>
            </header>
            <div className="stat-grid">
              {[
                { value: '<8ms', label: 'Frame latency' },
                { value: '16+', label: 'Camera feeds' },
                { value: '99.1%', label: 'Alert precision' },
              ].map(({ value, label }) => (
                <div key={label} className="stat-item">
                  <div className="stat-value">{value}</div>
                  <div className="stat-label">{label}</div>
                </div>
              ))}
            </div>
          </div>
        </div>
      </SectionWrapper>

      <hr className="divider" />

      {/* NERMN — Radar / Avoidance */}
      <SectionWrapper dark>
        <div className="split-layout">
          <div>
            <header className="section-header" style={{ marginBottom: '1.5rem' }}>
              <span className="section-eyebrow">NERMN</span>
              <h2>Physics + Behavioural Avoidance</h2>
              <p>
                NERMN combines a physics-based potential field model with a
                biologically-inspired behavioural layer derived from vertebrate
                lateral-inhibition circuits. The robot continuously updates a
                real-time occupancy radar and selects trajectories that satisfy
                both kinematic constraints and learned collision-aversion patterns —
                without a discrete planning step.
              </p>
            </header>
            <div style={{ display: 'flex', flexDirection: 'column', gap: '0.5rem' }}>
              {[
                'Physics layer: potential fields + kinematic hard constraints',
                'Behavioural layer: SNN lateral-inhibition avoidance model',
                'Radar-fused occupancy map updated at 1kHz event rate',
                'Zero discrete planning — continuous trajectory refinement',
                'Degrades gracefully under sensor noise and partial occlusion',
              ].map((item) => (
                <div key={item} style={{
                  display: 'flex', gap: '0.6rem', alignItems: 'flex-start',
                  fontSize: '0.85rem', color: 'var(--color-text-secondary)',
                }}>
                  <span style={{ color: 'var(--color-accent)', marginTop: '2px', flexShrink: 0 }}>▸</span>
                  {item}
                </div>
              ))}
            </div>
          </div>
          {/* Radar SVG */}
          <div style={{ display: 'flex', justifyContent: 'center' }}>
            <div className="radar-canvas" style={{ maxWidth: '360px' }}>
              <svg viewBox="0 0 360 360" fill="none" xmlns="http://www.w3.org/2000/svg"
                style={{ width: '100%', height: 'auto' }}>
                {/* Concentric rings */}
                {[30, 70, 110, 150].map((r) => (
                  <circle key={r} cx="180" cy="180" r={r}
                    stroke="rgba(20,184,166,0.15)" strokeWidth="1" />
                ))}
                {/* Cross hairs */}
                <line x1="180" y1="30" x2="180" y2="330" stroke="rgba(20,184,166,0.08)" strokeWidth="1" />
                <line x1="30" y1="180" x2="330" y2="180" stroke="rgba(20,184,166,0.08)" strokeWidth="1" />
                {/* Sweep arc */}
                <path d="M180,180 L180,50 A130,130 0 0,1 295,243 Z"
                  fill="rgba(20,184,166,0.06)" stroke="rgba(20,184,166,0.25)" strokeWidth="1" />
                {/* Sweep line */}
                <line x1="180" y1="180" x2="295" y2="243" stroke="rgba(20,184,166,0.6)" strokeWidth="1.5" />
                {/* Robot */}
                <circle cx="180" cy="180" r="8" fill="rgba(20,184,166,0.9)" />
                <circle cx="180" cy="180" r="14" stroke="rgba(20,184,166,0.4)" strokeWidth="1" />
                {/* Obstacles */}
                <circle cx="130" cy="120" r="6" fill="rgba(251,191,36,0.7)" />
                <circle cx="250" cy="150" r="8" fill="rgba(239,68,68,0.6)" />
                <circle cx="220" cy="240" r="5" fill="rgba(251,191,36,0.5)" />
                {/* Avoidance vector */}
                <line x1="180" y1="180" x2="155" y2="215"
                  stroke="rgba(20,184,166,0.9)" strokeWidth="2" markerEnd="url(#vec)" />
                <defs>
                  <marker id="vec" markerWidth="6" markerHeight="6" refX="3" refY="3" orient="auto">
                    <path d="M0,0 L0,6 L6,3 z" fill="rgba(20,184,166,0.9)" />
                  </marker>
                </defs>
                {/* Range labels */}
                <text x="184" y="116" fill="rgba(20,184,166,0.4)" fontSize="8" fontFamily="monospace">60m</text>
                <text x="184" y="76" fill="rgba(20,184,166,0.4)" fontSize="8" fontFamily="monospace">110m</text>
                <text x="184" y="36" fill="rgba(20,184,166,0.4)" fontSize="8" fontFamily="monospace">150m</text>
                {/* Legend */}
                <circle cx="20" cy="340" r="4" fill="rgba(20,184,166,0.9)" />
                <text x="30" y="344" fill="rgba(20,184,166,0.6)" fontSize="8" fontFamily="Inter,sans-serif">Robot</text>
                <circle cx="80" cy="340" r="4" fill="rgba(239,68,68,0.7)" />
                <text x="90" y="344" fill="rgba(239,68,68,0.6)" fontSize="8" fontFamily="Inter,sans-serif">Critical</text>
                <circle cx="145" cy="340" r="4" fill="rgba(251,191,36,0.7)" />
                <text x="155" y="344" fill="rgba(251,191,36,0.6)" fontSize="8" fontFamily="Inter,sans-serif">Proximity</text>
              </svg>
            </div>
          </div>
        </div>
      </SectionWrapper>

      <hr className="divider" />

      {/* CTA */}
      <SectionWrapper compact>
        <header className="section-header">
          <h2>Request Platform Access</h2>
          <p>SDK documentation, deployment specs, and evaluation packages available on request.</p>
        </header>
        <div className="cta-actions">
          <Button variant="primary" href="/contact">Request Access</Button>
          <Button variant="secondary" href="/products">View Products</Button>
        </div>
      </SectionWrapper>
    </main>
  );
}
