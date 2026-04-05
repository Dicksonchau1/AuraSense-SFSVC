import type { ProductPageContent } from '../types/content';

export const nssimContent: ProductPageContent = {
  hero: {
    headline: 'NSSIM — Neuromorphic Smart Surveillance Intelligence Module',
    subheadline: 'Multi-Sensor Surveillance Intelligence for Critical Infrastructure',
    description:
      'NSSIM is a multi-sensor surveillance intelligence module that processes camera, acoustic, and environmental sensor feeds through a spike-based temporal reasoning engine. It produces actionable alerts with severity tiering, temporal event correlation, and chain-of-custody evidence packages for regulatory and operational review. Designed for perimeter security, critical asset monitoring, and multi-site surveillance operations. Edge-native. Offline-capable. Audit-ready.',
    primaryCta: { label: 'Request NSSIM Briefing', href: '/contact' },
    secondaryCta: { label: 'View All Products', href: '/products' },
  },
  sections: [
    {
      id: 'intelligence-pipeline',
      eyebrow: 'Architecture',
      title: 'Intelligence Pipeline',
      description:
        'NSSIM processes multi-sensor input through a spike-based intelligence pipeline that transforms raw sensor feeds into actionable surveillance alerts. The pipeline is deterministic — identical sensor inputs produce identical alert outputs. Every intermediate stage is logged for replay verification and audit.',
      items: [
        {
          title: 'Multi-Sensor Ingestion',
          description:
            'NSSIM ingests camera streams (RGB, thermal, IR), acoustic sensor feeds, environmental telemetry (vibration, temperature, humidity), and access control events. Sensor data is timestamped at ingestion with nanosecond precision and routed to modality-specific encoding stages. Sensor health monitoring detects degradation and triggers fallback processing.',
        },
        {
          title: 'Spike-Based Encoding',
          description:
            'Each sensor modality is encoded into a spike representation through modality-appropriate transformations. Video frames use temporal differencing — new motion triggers spike events, static scenes are suppressed. Acoustic data uses spectral change detection. Environmental sensors use threshold-crossing encoding. The result is a unified, sparse event representation across all modalities.',
        },
        {
          title: 'Feature Correlation Engine',
          description:
            'Spike events from multiple sensor modalities are correlated in a temporal feature engine that identifies coincident or sequential patterns across sensors. A perimeter camera motion event coinciding with an acoustic anomaly and an access control event produces a correlated multi-modal alert with higher confidence than any single-sensor detection.',
        },
        {
          title: 'Classification and Alert Generation',
          description:
            'Correlated features are classified using spike-timing-dependent pattern matching against learned threat profiles. Classifications carry confidence scores, severity tiers, and evidence references. Alerts are emitted with full provenance — which sensors contributed, what temporal window was analyzed, which prototype matched, and what the anomaly score was.',
        },
      ],
    },
    {
      id: 'temporal-reasoning',
      eyebrow: 'Intelligence',
      title: 'Temporal Reasoning',
      description:
        'Surveillance intelligence is fundamentally temporal. A single frame or sensor reading rarely constitutes actionable information. NSSIM implements temporal reasoning over spike event streams — detecting patterns that unfold over seconds, minutes, or hours. This enables detection of behavioral patterns, gradual environmental changes, and coordinated multi-stage events that frame-by-frame processing would miss.',
      items: [
        {
          title: 'Temporal Event Windows',
          description:
            'Spike events are analyzed within configurable temporal windows — from milliseconds (rapid motion detection) to hours (behavioral pattern analysis). Window sizes are tuned per alert type and deployment context. Multiple temporal scales operate simultaneously, enabling both immediate threat detection and slow-evolving anomaly recognition.',
        },
        {
          title: 'Sequential Pattern Detection',
          description:
            'The temporal engine detects sequential event patterns — A followed by B within a time window, repeated access attempts with increasing frequency, gradual perimeter probe sequences. Patterns are defined as configurable detection rules with temporal constraints, sensor scope, and confidence thresholds.',
        },
        {
          title: 'Behavioral Baseline Adaptation',
          description:
            'NSSIM maintains a temporal baseline of normal activity patterns for each monitored zone. Deviations from baseline — unusual activity timing, atypical sensor patterns, unexpected environmental changes — are detected as anomalies. Baseline adaptation uses gated learning: only confirmed non-threat events update the baseline, preventing adversarial pattern injection.',
        },
        {
          title: 'Cross-Sensor Temporal Correlation',
          description:
            'Events from different sensors are correlated across time — a visual motion event at T=0 correlated with an acoustic signature at T=200ms and a vibration event at T=500ms may indicate a single physical event observed across modalities. Temporal correlation produces multi-modal evidence packages with higher reliability than single-sensor detection.',
        },
      ],
    },
    {
      id: 'alert-orchestration',
      eyebrow: 'Operations',
      title: 'Alert Orchestration',
      description:
        'Raw detections are not alerts. NSSIM implements an alert orchestration layer that aggregates, deduplicates, prioritizes, and routes detections into actionable alerts with appropriate severity, context, and evidence. The orchestration layer prevents alert fatigue while ensuring that genuine threats are surfaced with sufficient information for operational response.',
      items: [
        {
          title: 'Severity Tiering',
          description:
            'Alerts are classified into configurable severity tiers — informational, advisory, elevated, critical. Tier assignment considers anomaly score, multi-sensor corroboration, temporal pattern match strength, and zone sensitivity. Critical alerts are never suppressed or delayed. Advisory alerts may be aggregated in digest form.',
        },
        {
          title: 'Alert Deduplication',
          description:
            'Multiple detections of the same physical event across sensors or temporal windows are consolidated into a single alert with composite evidence. Deduplication uses spatial proximity, temporal overlap, and classification similarity to identify duplicate detections. Each contributing detection is preserved in the evidence chain.',
        },
        {
          title: 'Routing and Escalation',
          description:
            'Alerts are routed to configured destinations — operator consoles, SIEM integrations, notification channels, and evidence archives. Escalation rules promote unacknowledged alerts to higher severity tiers after configurable timeout periods. Routing is zone-aware: different zones can have different alert routing configurations.',
        },
        {
          title: 'Alert Context Packaging',
          description:
            'Each alert is packaged with contextual information: contributing sensor data, temporal event timeline, classification details, zone metadata, and evidence references. Operators receive sufficient context to make response decisions without querying additional systems.',
        },
      ],
    },
    {
      id: 'audit-evidence',
      eyebrow: 'Governance',
      title: 'Audit and Evidence',
      description:
        'Surveillance operations in critical infrastructure environments produce evidence that may enter regulatory, legal, or operational review processes. NSSIM treats evidence integrity as a structural requirement. Every detection, every alert, every operator acknowledgment is recorded in a chain-sealed evidence log with cryptographic integrity and full provenance.',
      items: [
        {
          title: 'Chain-of-Custody Evidence',
          description:
            'Alert evidence packages include raw sensor excerpts, spike-encoded intermediate representations, classification outputs, and temporal correlation records. Every artifact is chain-sealed with SHA-256 hashing in canonical JSON form. The evidence chain is tamper-evident — any modification is independently detectable.',
        },
        {
          title: 'Operator Action Logging',
          description:
            'Operator responses to alerts — acknowledgment, dismissal, escalation, response actions — are recorded in the evidence chain with operator identity, timestamp, and action context. The audit trail captures not only what the system detected but how operators responded.',
        },
        {
          title: 'Replay Verification',
          description:
            'Surveillance sessions can be replayed from sealed session logs to verify that alerts were correctly generated from the recorded sensor data. Replay produces bit-identical alert outputs, enabling post-incident review and regulatory audit without access to the live system.',
        },
        {
          title: 'Evidence Retention and Export',
          description:
            'Evidence is retained locally on the edge device in append-only storage. Configurable retention periods govern automatic archival. Evidence export produces self-contained packages with integrity verification metadata suitable for regulatory submission, legal discovery, or operational review.',
        },
      ],
    },
    {
      id: 'multi-site-deployment',
      eyebrow: 'Deployment',
      title: 'Multi-Site Deployment',
      description:
        'Critical infrastructure surveillance typically spans multiple physical sites with centralized operational oversight. NSSIM supports multi-site deployment through site gateways that aggregate local intelligence, coordinate alert routing, and manage evidence consolidation across distributed installations.',
      items: [
        {
          title: 'Site Gateway Architecture',
          description:
            'Each site runs one or more NSSIM instances on edge devices. A site gateway aggregates local alerts, manages model bundles, and provides the uplink to centralized operations. Gateways operate independently during connectivity outages — local alerting and evidence retention continue uninterrupted.',
        },
        {
          title: 'Cross-Site Alert Correlation',
          description:
            'Site gateways forward alerts to a central coordination layer that correlates events across sites. Coordinated activity spanning multiple locations — simultaneous perimeter probes, sequential access attempts at different facilities — is detected at the cross-site level and escalated as correlated multi-site events.',
        },
        {
          title: 'Centralized Configuration Management',
          description:
            'Detection rules, severity thresholds, alert routing, and model bundles are managed centrally and distributed to sites through the governance pipeline. Configuration changes are versioned, auditable, and require approval before deployment. Sites can operate with local configuration overrides for site-specific requirements.',
        },
        {
          title: 'Bandwidth-Efficient Uplink',
          description:
            'NSSIM transmits structured alert metadata and evidence references — not raw sensor streams — to the central coordination layer. Full sensor data and evidence packages are retained locally and retrieved on demand for investigation. This enables effective multi-site coordination over bandwidth-constrained or intermittent connectivity.',
        },
      ],
    },
  ],
  specs: [
    { label: 'Sensor Modalities', value: 'Video (RGB/thermal/IR), Acoustic, Environmental, Access Control' },
    { label: 'Temporal Window Range', value: 'Milliseconds to hours (configurable)' },
    { label: 'Alert Severity Tiers', value: 'Informational, Advisory, Elevated, Critical' },
    { label: 'Evidence Format', value: 'Chain-sealed canonical JSON with SHA-256' },
    { label: 'Replay Verification', value: 'Bit-identical alert reproduction from session logs' },
    { label: 'Multi-Site Support', value: 'Gateway-mediated with cross-site correlation' },
    { label: 'Connectivity Requirement', value: 'None (offline-capable with local retention)' },
    { label: 'Platform', value: 'Linux x86_64, Linux ARM64' },
    { label: 'Integration', value: 'REST/gRPC API, SIEM webhook, SNMP traps' },
    { label: 'License Verification', value: 'Ed25519 offline activation' },
  ],
};
