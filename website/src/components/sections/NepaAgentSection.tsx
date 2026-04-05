import { useState, useEffect, useRef, useCallback } from 'react';
import { SectionWrapper } from '../ui/SectionWrapper';
import { Button } from '../ui/Button';
import {
  moduleWorkflows,
  capabilityChips,
  type ModuleWorkflow,
} from '../../content/nepa-agent';

/* ── StatusChip ── */

function StatusChip({ label }: { label: string }) {
  return (
    <span className="inline-block rounded-md border border-white/10 bg-white/[0.03] px-2.5 py-1 text-[11px] font-medium tracking-wide text-slate-400">
      {label}
    </span>
  );
}

/* ── WorkflowStageCard ── */

function WorkflowStageCard({
  label,
  detail,
  index,
  isActive,
  isAgent,
}: {
  label: string;
  detail: string;
  index: number;
  isActive: boolean;
  isAgent: boolean;
}) {
  return (
    <div
      className={`
        relative rounded-lg border p-3.5 transition-all duration-500 ease-out
        ${isActive
          ? isAgent
            ? 'border-teal-400/40 bg-teal-400/[0.06] shadow-[0_0_12px_rgba(20,184,166,0.08)]'
            : 'border-white/15 bg-white/[0.04]'
          : 'border-white/5 bg-white/[0.01] opacity-40'
        }
      `}
      style={{
        transform: isActive ? 'translateY(0)' : 'translateY(4px)',
        transitionDelay: `${index * 120}ms`,
      }}
    >
      {/* stage index */}
      <div className="mb-2 flex items-center gap-2">
        <span
          className={`flex h-5 w-5 items-center justify-center rounded text-[10px] font-bold ${
            isAgent
              ? 'bg-teal-400/20 text-teal-300'
              : 'bg-white/10 text-slate-400'
          }`}
        >
          {index + 1}
        </span>
        <span
          className={`text-[10px] font-semibold uppercase tracking-[0.12em] ${
            isAgent ? 'text-teal-400' : 'text-slate-500'
          }`}
        >
          {label}
        </span>
      </div>
      <p className="text-[12.5px] leading-[1.5] text-slate-300">{detail}</p>

      {/* connector line to next stage */}
      {index < 4 && (
        <div
          className="absolute -bottom-3 left-1/2 h-3 w-px bg-white/10"
          aria-hidden="true"
        />
      )}
    </div>
  );
}

/* ── ModuleWorkflowTabs ── */

function ModuleWorkflowTabs({
  workflows,
  activeId,
  onSelect,
}: {
  workflows: ModuleWorkflow[];
  activeId: string;
  onSelect: (id: string) => void;
}) {
  return (
    <div className="flex gap-1 rounded-lg border border-white/10 bg-white/[0.02] p-1" role="tablist">
      {workflows.map((w) => (
        <button
          key={w.id}
          role="tab"
          aria-selected={w.id === activeId}
          aria-pressed={w.id === activeId}
          onClick={() => onSelect(w.id)}
          className={`
            rounded-md px-3.5 py-1.5 text-[11px] font-semibold uppercase tracking-[0.1em] transition-colors duration-200
            ${w.id === activeId
              ? 'bg-teal-400/15 text-teal-300 border border-teal-400/25'
              : 'text-slate-500 hover:text-slate-300 border border-transparent'
            }
          `}
        >
          {w.title}
        </button>
      ))}
    </div>
  );
}

/* ── AgentSimulationPanel ── */

function AgentSimulationPanel() {
  const [activeModule, setActiveModule] = useState('nssim');
  const [visibleStages, setVisibleStages] = useState(0);
  const timerRef = useRef<ReturnType<typeof setInterval> | null>(null);

  const workflow = moduleWorkflows.find((w) => w.id === activeModule)!;

  const startReveal = useCallback(() => {
    setVisibleStages(0);
    if (timerRef.current) clearInterval(timerRef.current);
    let count = 0;
    timerRef.current = setInterval(() => {
      count += 1;
      setVisibleStages(count);
      if (count >= 5) {
        if (timerRef.current) clearInterval(timerRef.current);
      }
    }, 400);
  }, []);

  useEffect(() => {
    startReveal();
    return () => {
      if (timerRef.current) clearInterval(timerRef.current);
    };
  }, [activeModule, startReveal]);

  const handleSelect = (id: string) => {
    if (id !== activeModule) {
      setActiveModule(id);
    }
  };

  return (
    <div className="rounded-xl border border-white/10 bg-white/[0.02] p-4 sm:p-5">
      {/* panel header */}
      <div className="mb-4 flex flex-col gap-3 sm:flex-row sm:items-center sm:justify-between">
        <p className="text-[10px] font-semibold uppercase tracking-[0.14em] text-slate-500">
          Orchestration Preview
        </p>
        <ModuleWorkflowTabs
          workflows={moduleWorkflows}
          activeId={activeModule}
          onSelect={handleSelect}
        />
      </div>

      {/* workflow path label */}
      <div className="mb-3 flex flex-wrap items-center gap-1.5">
        {['Runtime Signal', 'NEPA Agent', 'Operator Review', 'Evidence / Replay', 'Routed Action'].map(
          (stage, i) => (
            <span key={stage} className="flex items-center gap-1.5">
              {i > 0 && (
                <span className="text-[9px] text-slate-600" aria-hidden="true">→</span>
              )}
              <span
                className={`text-[9px] font-medium uppercase tracking-[0.08em] ${
                  i <= visibleStages - 1
                    ? stage === 'NEPA Agent'
                      ? 'text-teal-400'
                      : 'text-slate-400'
                    : 'text-slate-600'
                }`}
              >
                {stage}
              </span>
            </span>
          ),
        )}
      </div>

      {/* animated connector bar */}
      <div className="mb-4 h-px w-full overflow-hidden rounded bg-white/5" aria-hidden="true">
        <div
          className="h-full rounded bg-teal-400/40 transition-all duration-700 ease-out"
          style={{ width: `${(visibleStages / 5) * 100}%` }}
        />
      </div>

      {/* stage cards */}
      <div className="grid gap-2.5">
        {workflow.stages.map((stage, i) => (
          <WorkflowStageCard
            key={`${activeModule}-${i}`}
            label={stage.label}
            detail={stage.detail}
            index={i}
            isActive={i < visibleStages}
            isAgent={stage.label === 'NEPA Agent'}
          />
        ))}
      </div>

      {/* status line */}
      <div className="mt-4 flex items-center gap-2">
        <span
          className={`h-1.5 w-1.5 rounded-full transition-colors duration-500 ${
            visibleStages >= 5 ? 'bg-teal-400' : 'bg-slate-600'
          }`}
          aria-hidden="true"
        />
        <span className="text-[10px] text-slate-500">
          {visibleStages >= 5
            ? 'Workflow complete — action routed'
            : 'Processing workflow…'}
        </span>
      </div>
    </div>
  );
}

/* ── NepaAgentSection ── */

export function NepaAgentSection() {
  return (
    <SectionWrapper id="nepa-agent" dark>
      <div className="lg:grid lg:grid-cols-12 lg:gap-10 space-y-10 lg:space-y-0">
        {/* ── left: copy ── */}
        <div className="lg:col-span-5">
          <p className="text-[11px] font-semibold uppercase tracking-[0.14em] text-teal-400">
            NEPA Agent
          </p>
          <h2 className="mt-3 text-2xl font-semibold leading-tight tracking-[-0.03em] text-white sm:text-3xl">
            From runtime output to structured action.
          </h2>
          <p className="mt-4 max-w-[52ch] text-sm leading-[1.7] text-slate-400">
            NEPA Agent is a multi-step orchestration layer that turns runtime
            outputs into structured action across connected systems. It operates
            operator-in-the-loop, works in your environment, and routes
            evidence-backed actions through a verifiable workflow with a bounded
            response path. Instead of generating open-ended responses, NEPA Agent
            observes what the runtime has perceived, frames the next valid step, and
            coordinates alerts, summaries, replay, escalation, and downstream
            actions in a way that remains traceable, reviewable, and operationally
            safe.
          </p>

          {/* capability chips */}
          <div className="mt-5 flex flex-wrap gap-2">
            {capabilityChips.map((chip) => (
              <StatusChip key={chip} label={chip} />
            ))}
          </div>

          <p className="mt-4 text-[12px] italic text-slate-500">
            Works across surveillance, inspection, and robotic operations.
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

        {/* ── right: simulation ── */}
        <div className="lg:col-span-7">
          <AgentSimulationPanel />
        </div>
      </div>
    </SectionWrapper>
  );
}
