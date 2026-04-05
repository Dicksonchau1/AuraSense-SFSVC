import { SectionWrapper } from '../ui/SectionWrapper';

interface SubBlock {
  title: string;
  body: string;
  points: string[];
}

const subBlocks: SubBlock[] = [
  {
    title: 'Parallel Lanes',
    body: 'Each stream runs in an isolated, pre-allocated lane with no heap allocation in the hot path, no shared mutable state, and no GPU dependency. SIMD-accelerated frame differencing and lock-free queues keep latency bounded under fixed execution budgets.',
    points: [
      'Pre-allocated buffers',
      'SIMD inner loops',
      'Lock-free SPSC queues',
      'No shared mutable state',
    ],
  },
  {
    title: 'Replay',
    body: 'Every session is sealed with input digests, model references, runtime configuration, and outputs. The replay tool re-executes the same pipeline and confirms bit-identical results for audit, dispute review, and operational verification.',
    points: [
      'Sealed session logs',
      'Bit-identical replay',
      'Portable verification package',
    ],
  },
  {
    title: 'Governance',
    body: 'Evidence and model decisions are written to an append-only audit chain with cryptographic integrity. Provenance remains traceable from capture to archive, with retention and disposal enforced at the storage layer.',
    points: [
      'Append-only hash chain',
      'Model provenance',
      'Immutable evidence lifecycle',
    ],
  },
];

export function RuntimeEvidenceSection() {
  return (
    <SectionWrapper id="runtime-evidence" dark>
      <header className="section-header">
        <span className="text-[11px] font-semibold uppercase tracking-[0.14em] text-teal-400">
          Execution Layer
        </span>
        <h2 className="mt-3 text-2xl font-semibold leading-tight tracking-[-0.02em] text-white sm:text-3xl">
          Runtime and Evidence Guarantees
        </h2>
        <p className="mt-4 max-w-[60ch] text-sm leading-6 text-slate-400">
          NEPA runs as a deterministic edge runtime: bounded-latency lanes on the
          hot path, and a replay-verifiable evidence trail off it. Runtime behavior
          and evidence integrity are engineered, not inferred.
        </p>
      </header>

      <div className="mt-10 grid gap-6 sm:grid-cols-2 lg:grid-cols-3">
        {subBlocks.map((block) => (
          <div
            key={block.title}
            className="rounded-xl border border-white/10 bg-white/[0.02] p-5 transition-colors hover:border-white/20"
          >
            <h3 className="text-base font-semibold text-white">{block.title}</h3>
            <p className="mt-2 text-[13px] leading-[1.6] text-slate-400">
              {block.body}
            </p>
            <ul className="mt-4 space-y-1.5">
              {block.points.map((pt) => (
                <li
                  key={pt}
                  className="flex items-start gap-2 text-[12px] text-slate-400"
                >
                  <span className="mt-1.5 h-1 w-1 shrink-0 rounded-full bg-teal-400" aria-hidden="true" />
                  {pt}
                </li>
              ))}
            </ul>
          </div>
        ))}
      </div>
    </SectionWrapper>
  );
}
