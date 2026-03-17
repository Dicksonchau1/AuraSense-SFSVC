# NEPA — Neuromorphic Envelope Performance Analytics

By AuraSense (Hong Kong)

Deterministic facade inspection infrastructure.

## System Overview

NEPA is a neuromorphic perception and inspection pipeline designed for structured facade analysis.

It operates through deterministic spike processing lanes, version-bound inspection runs, and replay-verifiable outputs.

Core principles:

- Deterministic execution
- Replay verification
- Attributable outputs
- Structured parallel lanes
- Version-sealed pipelines
- No shared mutable state

The system is designed for institutional and regulatory-grade environments.

## High-Level Architecture

```
Raw Signal Input (.spk)
        │
        ▼
Spike Ingestion Layer
        │
        ▼
Clustering + Lane Processing
        │
        ▼
Versioned Spike Pipeline
        │
        ▼
Inspection Run Anchor
        │
        ▼
Output Contract (JSON)
        │
        ▼
Evidence Store + Replay Validation
```

### Architecture Components

**Raw Signal Input (.spk)**
Encoded neuromorphic sensor data. Temporal contrast events captured as sparse spike streams.

**Spike Ingestion Layer**
Validates input format, performs boundary checks, and prepares spike data for processing.

**Clustering + Lane Processing**
Groups spatially-related spikes into clusters. Processes each cluster through isolated computational lanes to maintain determinism.

**Versioned Spike Pipeline**
Version-locked processing pipeline. Each run executes under a specific pipeline version identifier.

**Inspection Run Anchor**
Immutable record of execution context: input hash, pipeline version, timestamp, and configuration snapshot.

**Output Contract (JSON)**
Structured output conforming to predefined schema. All findings are typed and validated.

**Evidence Store + Replay Validation**
Stores execution artifacts and golden hashes. Enables bit-exact replay verification against reference outputs.

## Core Components

### /engine

C++ spike processing and lane metrics logging.

Handles low-level spike stream parsing, temporal clustering, and deterministic lane execution. Produces structured metrics logs for each processing lane.

### /app

Pipeline orchestration, ingestion, clustering, replay monitoring.

Coordinates end-to-end execution flow. Manages input validation, cluster assignment, pipeline versioning, and replay verification workflows.

### /contracts

Structured output contract definitions.

JSON schema definitions for inspection outputs. Enforces output structure, type constraints, and required fields.

### /scripts

Operational and validation scripts.

Automation for pipeline deployment, replay execution, and golden hash validation. Supports operational workflows and integrity checks.

### /tests

Replay fixtures and deterministic validation tests.

Fixed input datasets and golden output hashes. Validates that pipeline execution remains deterministic across runs and versions.

### /docs

Governance, meeting, and integration documentation.

Records of design decisions, integration protocols, and governance policies. Maintains audit trail for institutional compliance.

## Deterministic Replay Model

Inspection runs are version-bound.

Each run can be replayed against fixture inputs.

Golden output hashes validate invariance.

Pipeline versioning is enforced.

No output is considered valid without replay consistency.

### Replay Workflow

1. Capture input data and pipeline version at execution time
2. Generate output and compute cryptographic hash
3. Store hash as golden reference
4. On replay, execute identical pipeline version against same input
5. Compare output hash to golden reference
6. Flag divergence as pipeline integrity failure

Replay validation ensures that outputs remain stable across infrastructure changes, compiler updates, and dependency shifts.

## Output Contract

All findings conform to sfsvc_output_contract.json.

Each finding includes:

- Zone ID
- Classification
- Confidence
- Pipeline version
- Replay hash reference

Outputs are structured, attributable, and auditable.

### Contract Enforcement

Output schema is validated at runtime. Any output that fails schema validation is rejected. This ensures that downstream consumers receive consistently-structured data.

Contract versioning allows schema evolution while maintaining backward compatibility with archived outputs.

## Governance Model

**Versioned pipeline releases**
Each pipeline release is tagged with a semantic version. Version identifiers are embedded in all outputs.

**Watchdog threshold policies**
Automated monitoring detects anomalies in processing metrics. Threshold violations trigger alerts and optional execution halts.

**Failure monitoring**
Structured logging captures all failure modes. Failures are categorized, indexed, and aggregated for operational review.

**Replay integrity validation**
Continuous validation of replay consistency. Any deviation from golden hashes triggers investigation and potential rollback.

**Structured change control**
All pipeline changes follow documented review and approval workflows. Changes are traceable to specific commits and design documents.

The system is designed to withstand external scrutiny.

## Deployment Notes

Production startup script reference: See /scripts for deployment automation.

Environment isolation expectation: Execution environments must be isolated and reproducible.

No secrets committed: API keys, credentials, and sensitive configuration are managed externally.

Deterministic build required: Builds must be reproducible across environments to ensure replay consistency.

## Development Workflow

```bash
git pull origin main
git checkout -b feature-branch
git add .
git commit -m "Structured commit message"
git push origin feature-branch
```

No direct commits to main without validation.

All changes require review and replay validation before merge.

## System Positioning Statement

NEPA is inspection infrastructure.

It is not a prototype, not a visualization layer, and not an experimental AI model.

It is a governed perception pipeline designed for structured institutional deployment.
