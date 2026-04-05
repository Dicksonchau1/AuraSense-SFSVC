import { SectionWrapper } from '../ui/SectionWrapper';
import { Button } from '../ui/Button';

/* ── data ── */

const operatorLayer = ['Operator Console', 'Dashboard', 'SDK / API'];
const middlewareCore = [
  'Temporal Encoder',
  'SNN Inference',
  'Agent Runtime',
  'HRI Interface',
  'Safety Arbitration',
];
const inputLayer = [
  'Event Camera',
  'CCTV Streams',
  'Radar',
  'Drone Telemetry',
  'Robot Sensors',
];
const outputLayer = ['SFSVC', 'NSSIM', 'NERMN', 'Future Modules'];
const metaRail = [
  'Latency Bound',
  'Replay',
  'Audit Log',
  'State Integrity',
  'Deployment Target',
];

const evidenceCards = [
  {
    title: 'Deterministic Runtime',
    body: 'Bounded latency under edge constraints',
  },
  {
    title: 'Human-Robot Interface',
    body: 'Natural language commands with timestamped outputs',
  },
  {
    title: 'Multi-Modal Inputs',
    body: 'Cameras, radar, telemetry, and event streams',
  },
  {
    title: 'Reusable Product Layer',
    body: 'One middleware core across SFSVC, NSSIM, and NERMN',
  },
];

const signalStages: { label: string; active: boolean }[] = [
  { label: 'Sensors', active: false },
  { label: 'Encoding', active: true },
  { label: 'Inference', active: true },
  { label: 'Agent Reasoning', active: true },
  { label: 'Operator / Command Output', active: false },
];

/* ── sub-components ── */

function LayerLabel({ children }: { children: string }) {
  return (
    <p className="mb-2 text-[10px] font-semibold uppercase tracking-[0.12em] text-slate-500">
      {children}
    </p>
  );
}

function Node({
  label,
  variant = 'normal',
}: {
  label: string;
  variant?: 'normal' | 'core';
}) {
  const base =
    variant === 'core'
      ? 'rounded-md border border-teal-400/30 bg-teal-400/[0.06] px-3 py-2 text-[12px] font-medium text-teal-300 transition-colors hover:border-teal-400/50'
      : 'rounded-md border border-white/10 bg-black/20 px-3 py-2 text-[12px] font-medium text-slate-300 transition-colors hover:border-white/20';
  return <span className={base}>{label}</span>;
}

function ConnectorLine() {
  return (
    <div className="flex justify-center py-1" aria-hidden="true">
      <div className="h-4 w-px bg-white/10" />
    </div>
  );
}

/* ── main component ── */

export function NepaMiddleLayerSection() {
  return (
    <SectionWrapper id="nepa-middle-layer" dark>
      <div className="lg:grid lg:grid-cols-12 lg:gap-8 space-y-8 lg:space-y-0">
        {/* ── left: intro ── */}
        <div className="lg:col-span-3">
          <p className="text-[11px] font-semibold uppercase tracking-[0.14em] text-teal-400">
            NEPA Infrastructure
          </p>
          <h2 className="mt-3 text-2xl font-semibold leading-tight tracking-[-0.03em] text-white">
            The middle layer between sensing, reasoning, and action
          </h2>
          <p className="mt-4 max-w-[34ch] text-sm leading-6 text-slate-400">
            NEPA is the runtime layer that ingests sensor streams, executes
            temporal inference, coordinates agent reasoning, and exposes
            structured outputs to operators, robots, and downstream products.
          </p>
          <div className="mt-6 flex flex-wrap gap-3">
            <Button variant="primary" size="sm" href="/contact">
              Request Access
            </Button>
            <Button variant="secondary" size="sm" href="/products">
              View Products
            </Button>
          </div>
        </div>

        {/* ── center: architecture diagram ── */}
        <div className="lg:col-span-6">
          <div className="rounded-xl border border-white/10 bg-white/[0.02] p-4 sm:p-5 lg:p-6">
            <div className="relative">
              {/* Operator Layer */}
              <div>
                <LayerLabel>Operator Layer</LayerLabel>
                <div className="flex flex-wrap gap-2">
                  {operatorLayer.map((n) => (
                    <Node key={n} label={n} />
                  ))}
                </div>
              </div>

              <ConnectorLine />

              {/* NEPA Middleware Core — visually strongest */}
              <div className="rounded-lg border border-teal-400/25 bg-teal-400/[0.04] p-3">
                <LayerLabel>Middleware Core</LayerLabel>
                <div className="flex flex-wrap gap-2">
                  {middlewareCore.map((n) => (
                    <Node key={n} label={n} variant="core" />
                  ))}
                </div>
              </div>

              <ConnectorLine />

              {/* Input Layer */}
              <div>
                <LayerLabel>Input Layer</LayerLabel>
                <div className="flex flex-wrap gap-2">
                  {inputLayer.map((n) => (
                    <Node key={n} label={n} />
                  ))}
                </div>
              </div>

              <ConnectorLine />

              {/* Output / Application Layer */}
              <div>
                <LayerLabel>Output / Application Layer</LayerLabel>
                <div className="flex flex-wrap gap-2">
                  {outputLayer.map((n) => (
                    <Node key={n} label={n} />
                  ))}
                </div>
              </div>

              {/* Meta rail — inline at bottom on narrow, side-strip on wider cards */}
              <div className="mt-4 pt-3 border-t border-white/5">
                <LayerLabel>Infrastructure Properties</LayerLabel>
                <div className="flex flex-wrap gap-2">
                  {metaRail.map((n) => (
                    <Node key={n} label={n} />
                  ))}
                </div>
              </div>
            </div>
          </div>

          {/* Signal-flow strip */}
          <div className="mt-4 flex flex-wrap items-center gap-2">
            {signalStages.map((stage, i) => (
              <span key={stage.label} className="flex items-center gap-2">
                {i > 0 && (
                  <span className="text-[10px] text-slate-600" aria-hidden="true">
                    →
                  </span>
                )}
                <span
                  className={
                    stage.active
                      ? 'rounded-md border border-teal-400/30 bg-teal-400/[0.06] px-2.5 py-1 text-[11px] text-teal-300'
                      : 'rounded-md border border-white/10 px-2.5 py-1 text-[11px] text-slate-400'
                  }
                >
                  {stage.label}
                </span>
              </span>
            ))}
          </div>
        </div>

        {/* ── right: evidence rail ── */}
        <div className="lg:col-span-3 space-y-3">
          {evidenceCards.map((card) => (
            <div
              key={card.title}
              className="rounded-lg border border-white/10 bg-white/[0.02] p-4 transition-colors hover:border-white/20"
            >
              <p className="text-sm font-medium text-white">{card.title}</p>
              <p className="mt-1 text-[13px] leading-5 text-slate-400">
                {card.body}
              </p>
            </div>
          ))}
        </div>
      </div>
    </SectionWrapper>
  );
}
