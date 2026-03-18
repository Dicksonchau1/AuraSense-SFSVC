# NEPA Enhancement Roadmap

Derived from ADR-0001. Each phase is a distinct milestone with acceptance criteria, target PRs, and agent guardrails.

---

## Phase 1 — Stable foundation (current sprint)

**Goal:** Compilable, testable SFSVC core + shadow anomaly layer bootstrap.

| # | Enhancement | Source PR / ADR | Acceptance criteria |
|---|---|---|---|
| 1.1 | Fix all C++ compilation errors in SFSVC engine | PR #1 | `cmake --build` exits 0; `ctest` passes; no ODR violations |
| 1.2 | Replace simulated crack detection with OpenCV CV pipeline | PR #2 | `detect_cracks_cv()` present; no `random.Random` seeded simulation; ~20ms/frame on 720p |
| 1.3 | Add C++ architecture docs + review prompts | PR #3 | `docs/CPP_ARCHITECTURE_REVIEW.md` merged; AVX2 SIMD patterns documented |
| 1.4 | Assemble Python SDK package with Docker + benchmarks | PR #4 | `pip install -e .` works; `aurasense-benchmark` CLI runs; 0.45ms median benchmark |
| 1.5 | Consolidate GitHub Pages to single `docs/` workflow | PR #10 | `static.yml` removed; only `pages.yml` deploys from `docs/`; no race condition |
| 1.6 | NEPA brand rewrite + site content | PR #9 | Site reflects NEPA identity; no drone/retina language; P99 claims verified |
| 1.7 | Add `CODEOWNERS` for shadow_model/, types.h, bundle_schema/ | New PR | Human review required for all shadow layer and struct changes |

**Phase 1 exit gate:** PR #1 merged and CI green. All others can merge in parallel after.

---

## Phase 2 — Shadow anomaly layer (next sprint)

**Goal:** Working on-edge shadow anomaly layer with online STDP adaptation and audit chain.

| # | Enhancement | ADR ref | Acceptance criteria |
|---|---|---|---|
| 2.1 | Implement `shadow_proto_io.hpp` binary reader/writer | ADR-0001 §impl | `static_assert(sizeof(ProtoFileHeader)==64)`; round-trip test: write→read→compare |
| 2.2 | Implement AVX2 `cosine_dot_avx2()` + `argmax_cosine()` | ADR-0001 §impl | `_mm256_fmadd_ps` 4-way unroll; `alignas(32)` enforced; scalar fallback under `#ifndef __AVX2__` |
| 2.3 | Implement `stdp_update()` online adaptation step | ADR-0001 §3.3 | Gating thresholds exact (0.20, 0.60); `‖δ_k‖₂ ≤ 0.25` invariant; decay 0.0005/event |
| 2.4 | Shadow anomaly event emitter + `shadow_audit.log` writer | ADR-0001 §3.6 | `prev_entry_hash` chain; `shadow_replay_verify.py` passes on every test session |
| 2.5 | Bundle bootstrap: emit `bundle.json` + `prototypes.bin` from first N missions | — | Magic bytes correct; context records lexicographically sorted; `bundle_hash_hex` verified |
| 2.6 | Checkpoint every 512 events: persist `adaptation.bin` + hash | ADR-0001 §3.3 | Hash matches `sha256(adaptation.bin)` in `adaptation.json`; survives process restart |

**Phase 2 exit gate:** `shadow_replay_verify.py` passes on a synthetic 2000-event test session; AVX2 benchmark < 1µs per 8-prototype context lookup on target CPU.

---

## Phase 3 — TD critic + three-factor modulation

**Goal:** Off-chip TD critic running in parallel with STDP core; eligibility-trace-gated weight updates; prioritized replay.

| # | Enhancement | ADR ref | Acceptance criteria |
|---|---|---|---|
| 3.1 | Tabular TD(λ) critic over discretized facade states | ADR-0001 §Layer 2 | `V(s)` table initialized; updates logged with `δ_t` values; reward normalization + clipping active |
| 3.2 | Eligibility trace manager (τ_e ∈ [5s,30s], per synapse) | ADR-0001 §Layer 3 | Traces decay correctly; `Δw_ij = η·δ_t·e_ij` applied at zone-transition boundaries |
| 3.3 | Two-channel critic↔core interface (SPI/UART or IPC mock) | ADR-0001 §interface | Inbound: sparse spike events; outbound: scalar δ_t + {freeze,set_eta,map_realign,reset_traces} |
| 3.4 | Plasticity bounds enforcement | ADR-0001 §Decision | `|δ_t|≤δ_max`; `|Δw_ij|≤Δw_max`; `w_ij∈[w_min,w_max]`; per-second update budget |
| 3.5 | Prioritized replay generator: sample by `|δ_t|`, stream spike trains during hover | ADR-0001 §impl | Priority queue on host; replay sessions logged with `session_type=replay` in audit log |
| 3.6 | SR calibration step: `M̂=(I−γT̂)⁻¹` vs `W`; Frobenius distance; interpolation | ADR-0001 §impl | Runs after N=10 missions; `α_reg=0.05`; drift threshold configurable; calibration event logged |

**Phase 3 exit gate:** Simulated 50-mission run shows TD critic converges on value estimates; STDP map Frobenius distance from `M̂` decreases monotonically over calibration cycles.

---

## Phase 4 — Operator visibility + governance hardening

**Goal:** Dashboard surfaces shadow layer health; governance pipeline enforces human approval on all bundle promotions.

| # | Enhancement | Acceptance criteria |
|---|---|---|
| 4.1 | Dashy / operator UI: anomaly count per session, top contexts by anomaly rate | Live data from `shadow_audit.log`; auto-refreshes per session |
| 4.2 | Bundle version + hash display: current `bundle_version`, `bundle_hash`, `adaptation_hash`, drift stats | Visible in `dashboard.html`; "chain verified" badge from `shadow_replay_verify.py` |
| 4.3 | `CODEOWNERS` + branch protection: shadow_model/, types.h require human review | No merge without owner approval on protected paths |
| 4.4 | Signed bundle release tagging: `shadow_consolidate.py` → candidate bundle → human signs → tagged release | Release workflow in CI; `bundle_hash_hex` matches tagged artifact |
| 4.5 | Demo hub re-integration: facade risk map, inspection dashboard, VR placeholder (from closed PR #7) | Fresh branch on top of post-Phase-1 main; NEPA brand correct; Pages deploy clean |
| 4.6 | ADR-0002: Shadow Anomaly Layer formal ADR | Documents D=256/cosine/K=8 decisions, alternatives considered, and zone_bucket scheme |
| 4.7 | ADR-0003: SR Calibration Pipeline formal ADR | Documents M̂ computation, Frobenius threshold, α_reg choice, and trial-count benchmark |

**Phase 4 exit gate:** Operator can open `dashboard.html`, see live anomaly stats, verify chain status, and promote a bundle entirely through the governed UI + release pipeline without touching code.

---

## Dependency graph

```
Phase 1 (PRs #1→#3→#2→#4→#10→#9)
    │
    ▼
Phase 2 (shadow layer: 2.1→2.2→2.3→2.4→2.5→2.6)
    │
    ▼
Phase 3 (TD critic + modulation: 3.1→3.2→3.3→3.4 in parallel, then 3.5→3.6)
    │
    ▼
Phase 4 (governance + UI: all can proceed in parallel after Phase 3 green)
```

---

## Agent instruction cross-reference

All agents operating in this repo must consult `.github/copilot-instructions.md` before generating code for any phase. Phase-specific constraints:

- **Phase 1:** Never reintroduce `random.Random` crack simulation. Never add a second Pages workflow.
- **Phase 2:** All shadow layer binary I/O must match `shadow_proto_io.hpp` exactly. D=256, K=8, cosine locked.
- **Phase 3:** TD critic stays off-chip. Interface is two channels only. No TD arithmetic on neuromorphic core.
- **Phase 4:** Bundle promotion requires human sign-off. No agent may self-tag a bundle release.
