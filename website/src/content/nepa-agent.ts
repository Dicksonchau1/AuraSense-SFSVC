/* ── Module workflow data ── */

export interface WorkflowStage {
  label: string;
  detail: string;
}

export interface ModuleWorkflow {
  id: string;
  title: string;
  stages: WorkflowStage[];
}

export const moduleWorkflows: ModuleWorkflow[] = [
  {
    id: 'nssim',
    title: 'NSSIM',
    stages: [
      {
        label: 'Runtime Signal',
        detail: 'Restricted zone breach detected on CAM-03',
      },
      {
        label: 'NEPA Agent',
        detail: 'Packages alert context and incident summary',
      },
      {
        label: 'Operator Review',
        detail: 'Review incident and confirm escalation',
      },
      {
        label: 'Evidence / Replay',
        detail: 'Replay clip and audit trail available',
      },
      {
        label: 'Routed Action',
        detail: 'Notify shift lead and log patrol dispatch',
      },
    ],
  },
  {
    id: 'sfsvc',
    title: 'SFSVC',
    stages: [
      {
        label: 'Runtime Signal',
        detail: 'Facade anomaly detected on sector B14',
      },
      {
        label: 'NEPA Agent',
        detail: 'Groups finding by severity and structural category',
      },
      {
        label: 'Operator Review',
        detail: 'Engineer reviews evidence and confirms escalation class',
      },
      {
        label: 'Evidence / Replay',
        detail: 'Image evidence, audit trace, repeatability context',
      },
      {
        label: 'Routed Action',
        detail: 'Create structured engineering review record',
      },
    ],
  },
  {
    id: 'nermn',
    title: 'NERMN',
    stages: [
      {
        label: 'Runtime Signal',
        detail: 'Navigation confidence degraded near waypoint 12',
      },
      {
        label: 'NEPA Agent',
        detail: 'Flags degraded-safe state and frames next valid maneuver',
      },
      {
        label: 'Operator Review',
        detail: 'Supervisor confirms fallback path',
      },
      {
        label: 'Evidence / Replay',
        detail: 'Mission replay and telemetry trace',
      },
      {
        label: 'Routed Action',
        detail: 'Handoff to safeguarded route execution',
      },
    ],
  },
];

export const capabilityChips = [
  'Multi-step orchestration',
  'Evidence-backed actions',
  'Operator-in-the-loop',
  'Verifiable workflow',
  'Bounded response path',
];
