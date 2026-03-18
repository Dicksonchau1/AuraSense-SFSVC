# ADR-0001: NEPA Hybrid STDP Map + Off-Chip TD Critic

| Field | Value |
|---|---|
| ADR | 0001 |
| Title | NEPA Hybrid STDP Map + Off-Chip TD Critic |
| Status | **Accepted** — design baseline for NEPA hybrid learning |
| Created | 2026-03-19 |
| Authors | NEPA Engineering |
| Supersedes | — |
| Superseded by | — |
| Related | [ADR-0002] Shadow Anomaly Layer (pending), [ADR-0003] SR Calibration Pipeline (pending) |

---

## Context

NEPA needs a neuromorphic learning stack that can:

- (a) form long-horizon structural-risk maps over facade zones,
- (b) adapt coverage and control policies online across missions,
- (c) remain auditable, bounded, and compatible with SFSVC/CPSE governance.

Existing neuromorphic drone work focuses on low-level ego-motion control. Hippocampal-inspired mapping work targets spatial navigation, not facade-specific risk maps. Commercial inspection stacks use conventional deep learning without neuromorphic substrates.

We must avoid designs that:
- Push TD arithmetic into the neuromorphic core (hard to audit, hard to stabilize under varied facade regimes).
- Allow unbounded plasticity or opaque drift in structural-risk estimates.
- Couple learning and deployment without human-approval gates on bundle promotion.

---

## Decision

We adopt a **hybrid two-layer architecture**:

### Layer 1 — On-edge STDP substrate (neuromorphic core)

A recurrent spiking neural network learns a successor-representation-like weight matrix `W` over discrete NEPA states `s = (zone_bucket, risk_mode)` using local eligibility traces with time constants `τ_e ∈ [5s, 30s]`. This captures predictive reachability and structural-risk structure at mission timescales. Theta-phase modulation (global inhibitory oscillation) compresses zone-transition sequences into STDP plasticity windows (~10–20ms burst) to accelerate SR-like map formation.

State encoding: `zone_bucket = SHA256(zone_id)[0:2]` (256 deterministic buckets) × `risk_mode ∈ {clear, low_risk, medium_risk, high_risk}` = 1024-state space.

### Layer 2 — Off-chip TD critic (host CPU sidecar)

Tabular `V(s)` updated with standard TD(λ) as missions run. Reward function:

```
r_t = α·coverage(s_t) − β·risk(s_t) − γ_time·Δt
```

TD error: `δ_t = r_t + γ·V(s_{t+1}) − V(s_t)`, clipped to `[-δ_max, +δ_max]` with reward normalization. Scalar `δ_t` is the only signal sent from critic to neuromorphic core.

### Layer 3 — Three-factor R-STDP modulation interface

All synaptic updates follow:

```
Δw_ij = η · δ_t · e_ij
```

where:
- `e_ij` = eligibility trace from local STDP (pre × post spike timing), decaying with `τ_e`
- `δ_t` = scalar TD error (clipped)
- `η` = gated learning rate (see shadow STDP spec for anomaly-gating rule)

Explicit bounds enforced:
- `|δ_t| ≤ δ_max` (clipped before transmission)
- `|Δw_ij| ≤ Δw_max` per update
- `w_ij ∈ [w_min, w_max]` hard weight clamp
- Per-second global update budget: max N_updates/s to prevent burst plasticity

### Interface separation

Two channels only between neuromorphic layer and host CPU:

| Direction | Content |
|---|---|
| Neuromorphic → Host | Sparse spike/event stream (zone-transition events, timestamped) |
| Host → Neuromorphic | Scalar `δ_t` + plasticity control commands: `{freeze, set_eta, map_realign, reset_traces}` |

Policy outputs and bundle promotion remain governed by SFSVC/CPSE artifact pipelines with human approval. No autonomous bundle promotion.

---

## Status

**Accepted** — design baseline for NEPA hybrid learning, pending incremental implementation behind feature flags and `.github/copilot-instructions.md` guardrails.

Implementation is gated into four roadmap phases (see `docs/adr/ROADMAP.md`).

---

## Consequences

### Positive

- The STDP substrate forms and maintains predictive, hippocampal-like structural-risk maps under real mission timing, while the TD critic remains simple, stable, and inspectable on conventional hardware.
- Bounded plasticity (clipped `δ`, `Δw`, and `w`, plus update budgets) and explicit eligibility decay reduce the risk of runaway weight growth or catastrophic drift in the risk atlas.
- Clear two-channel interfaces (events up, scalar modulation + commands down) make it easy to log, replay, and audit learning events, and to integrate with existing SFSVC/CPSE governance for bundle promotion and release tagging.
- The shadow anomaly layer (D=256, cosine, K=8, STDP-on-embeddings) provides a parallel, per-context anomaly signal that can be cross-validated against the TD critic's value estimates without coupling the two subsystems.

### Negative / Trade-offs

- Maintaining a separate TD critic and replay/realignment pipeline increases host-side complexity and requires careful engineering of reward normalization, `δ` clipping, and periodic SR calibration.
- Treating the neuromorphic substrate as an advisory map that must be promoted through human-approved artifacts slows fully autonomous adaptation, but aligns with the project's safety and procurement constraints (MBIS, BVLOS regulatory context).
- The engineered SR approximation via STDP requires ongoing validation against host-side SR estimates and may need tuning (eligibility constants, learning rates, regularization) as flight cadences and facade regimes change. BTSP benchmarks suggest ~10–30 trials to approximate the SR; standard STDP needs ~50–100 — this sets calibration interval expectations.
- First-pass implementation keeps TD entirely off-chip; embedding a minimal TD error circuit on neuromorphic hardware is a second-generation decision point gated on autonomous operation requirements.

---

## Implementation constraints (normative)

These constraints are reproduced in `.github/copilot-instructions.md` and govern all AI-agent-generated code:

- `τ_e ∈ [5s, 30s]` — eligibility trace time constant
- `zone_bucket = SHA256(zone_id)[0:2]` — deterministic, stable across runs
- No TD arithmetic on the neuromorphic core in phase 1
- `δ_t` transmitted as a single scalar per zone-transition event over SPI/UART
- SR calibration: after every N=10 missions early on, compare `W` against `M̂ = (I − γT̂)⁻¹` using Frobenius distance; apply slow weight interpolation `W ← (1−α_reg)W + α_reg·M̂` with `α_reg = 0.05` if drift exceeds threshold
- Prioritized replay: sample sequences by `|δ_t|` from host priority queue; stream as pregenerated spike trains to neuromorphic core during hover/charging

---

## References

- Stachenfeld et al. (2017) — Hippocampus as predictive map
- Frémaux & Gerstner (2016) — Neuromodulated STDP and reward-based learning
- Izhikevich (2007) — Solving the distal reward problem through linkage of STDP and dopamine signaling
- Copilot instructions: `.github/copilot-instructions.md`
- Shadow anomaly layer spec: `shadow_model/` + Section 3 of copilot-instructions.md
