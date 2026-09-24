# MUA2 CANONICAL HANDOFF — Wii Marvel: Ultimate Alliance 2 native PC recompilation

**Project:** Wii Marvel: Ultimate Alliance 2 USA (RMSE52) native-PC recompilation  
**Repository:** `GTTeancum/OpenMUA2`  
**Canonical branch:** `main`  
**Date:** 2026-09-24

## Mandatory workflow rules

- GitHub `main` is authoritative. Inspect current `main` before every change.
- **Multiplatform performance is the active priority.** Correctness expansion comes later unless a concrete failure blocks performance work.
- Preserve SMC/hash/audit/REL eligibility/verification protections.
- Use **moderately short turns** because resume-stream failures occur: one focused merge/implementation plus validation is appropriate; do not batch unrelated work.
- At the end of **every turn**, update this file, commit it to `main`, and attach `MUA2-CANONICAL-HANDOFF.md` in chat.
- Never fabricate build, gameplay, controller, save, audio, or performance results.
- Never commit RMSE52 proprietary game data, extracted files, generated proprietary translation output, saves, logs, screenshots, or RAM captures.

## Important locations

- DolRecomp: `project/lib/DolRecomp/`
- ModernGekko: `project/lib/ModernGekko/`
- GXRuntime: `project/lib/ModernGekko/vendor/dolphin/GXRuntime/`
- Static recomp runtime: `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/`
- Tooling: `tools/`
- Tests: `tests/`
- CI: `.github/workflows/`
- Current status: `docs/CURRENT-STATUS.md`
- Work log: `docs/WORKLOG-2026-09-23.md`
- Recovery plan: `docs/RECOVERY-PLAN.md`
- Windows workspace target: `D:\\Programming\\GitHub\\OpenMUA2\\`
- Local proprietary/generated data: `.local/` only.

## Accepted runtime/performance state

Native DOL and native REL execution are present. SMC/chunk-hash protection remains enabled.

Key accepted commits:

- `67d75dc63455b47e34159fefae4ed39fca5e5efc` — absolute linked REL section-table support.
- `770db1089526cae9d0fc1b48fbd65c760e75efe6` — cache-control ops stay in native bursts.
- `7e0b48660f90e93e7333eaf4a127c3afb7b9ceb6` — scalar FMA semantics repair.
- `26334ad651567bf098401f781d1189e5f75c296c` — MEM2 lockstep journaling/restoration.
- `46091e1a2394e052f116838727471cbed974aa2d` — merged DOL+REL dispatch eligibility guards.
- `a6328f40bb0c98a58c8f50e52a30d4da55390b7e` — `O2 + indexed` default.
- `bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc` — page-index combined DOL+REL dispatch.
- `f4b7f991deb4a9e787b97c3bac2a475faadfc1f6` — empty/single-chunk merged-dispatch fast paths.
- `405b81de4136a7532e966218185a190f6eb9230d` — page-local ordinary indexed dispatch.
- `917a5d893087736a74566dbaf71de5f3989a6d90` — cached x86-64-v3 feature detection.
- `4e677ac8491bc3b5256998ba3688893f61d07695` — module-load baseline/v3 dispatch selection.
- `5e6ad207c77affbf500bf5327ce6222e9e7fd7c1` — reuse module-load v3 choice for indirect dispatch.
- `2776fa0a3e80136495a32552b9d909e16dcfcd5e` — gate exact host-call probes by cached chunk coverage.
- `af7938bdb63d4530ea43a6ba445800fc4171a153` — reuse runtime→linked PC resolution across eligibility and dispatch.
- `7b7e412f671039e1d29e2cdff7b2e51509bc046e` — fast-path same-section linked→runtime REL return translation.
- `7a9cdc0165257ec19301972785150e4961f7f65e` — fast-path same-section runtime→linked REL continuation resolution.
- `12a75b2db225678368be5e2b6340218ba2aafa2f` — skip forced-fallback helper/range scan when no forced-fallback ranges are configured.

## Historical RMSE52 boundary

Retained historical facts:

- module ID `1`
- REL version `3`
- header around `0x80E4A080`
- linked text around `0x80E4A164`
- accepted native REL text coverage `3,255,744` bytes
- historical accepted run advanced real game frames

Historical performance, predating recent optimizations:

- reported FPS ~`10.319`
- guest-frame FPS ~`2.062`
- speed ~`0.1404`
- native dispatches `174,476,293`
- native exceptions `5,959`
- hook fallback `1,299,055`
- hook fast cache `1,295,291`
- hook slow `3,764`
- JIT fallback runs `11,023`

Do not claim current FPS improvement until RMSE52 is rerun.

## Latest accepted work — PR #19

PR #19 `Skip empty forced-fallback scans` is **MERGED**.

Merge commit:

- `12a75b2db225678368be5e2b6340218ba2aafa2f`

Behavior:

- `FastDispatchableAt()` and `DispatchableAt()` now call `IsForcedFallbackAddress()` only when `m_forced_fallback_ranges` is non-empty.
- Default/no-range configuration avoids an out-of-line helper call plus empty range-loop on every native eligibility test.
- Configured forced-fallback ranges retain the exact prior range behavior.
- No REL, SMC/chunk verification, host-call, or dispatch lookup semantics changed.

Validation:

OpenMUA2 tooling run `36035472440`:

- Ubuntu Python tests: **PASS**
- Windows Python tests: **PASS**

ModernGekko run `36035472344`:

- Standalone Ubuntu: **PASS**
- Standalone Windows: **PASS**
- Full build/test Ubuntu: **PASS**
- Full build/test Windows: **PASS**

Status documentation:

- `f7467d44abc1a7dbed4b015e221a6e1c23372c7a` — `Record merged empty fallback fast path`

## Current pending work — PR #20

**PR:** #20 — `Inline cached host-call chunk state`  
**Branch:** `perf/inline-cached-host-call-state`  
**Head:** `e637d409da51789b60988a99041b424bbd46d5ed`  
**State:** OPEN / UNMERGED

Focused behavior:

- `Run()` now reads `m_chunk_host_call_state[chunk_index]` directly in the native host-call eligibility path.
- Cached state meanings remain: 0=unknown, 1=clean, 2=contains host call.
- Known state 1/2 avoids the out-of-line `ChunkContainsHostCall()` call.
- Unknown state 0 still calls `ChunkContainsHostCall()`, preserving lazy discovery.
- Exact `IsHostCallAddress(address)` is still performed only when a chunk may contain a host call.
- Modules without `host_call_contains` metadata still return false from the fast predicate.
- Chunk-index bounds checks remain.
- No dispatch/REL/SMC behavior is otherwise changed.

Files changed:

- `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore_Run.cpp`
- `tests/test_staticrecomp_host_call_gate_perf.py`

Diff size:

- runtime: 9 additions / 2 deletions
- regression: 13 additions / 2 deletions

## PR #20 validation state

OpenMUA2 tooling run `36038408893`:

- Ubuntu Python tests: **PASS**
- Windows Python tests: `in_progress`, Run tooling tests

ModernGekko run `36038408875`:

- Standalone Ubuntu: `in_progress`, Configure
- Standalone Windows: `in_progress`, Configure
- Full Ubuntu: `in_progress`, Configure
- Full Windows: `in_progress`, Configure

No CI failure has appeared.

PR #20 must remain unmerged until required Windows/Ubuntu validation completes.

## Current blockers

1. **Immediate integration gate:** PR #20 CI must complete successfully.
2. **Game-performance gate:** this environment lacks the proprietary RMSE52 workspace/image, so current-main FPS cannot be measured here.

## Not freshly validated

Do not claim current:

- current FPS/speed
- complete boot/title/menu
- opening gameplay/combat
- complete level
- long-session stability
- multiplayer
- audio/controller correctness
- save/reload across restart
- full Windows game execution
- fresh large lockstep total
- crash-free/glitch-free completion

## Next exact turn

1. Inspect current `main`.
2. Check PR #20 and workflow runs `36038408893` / `36038408875`.
3. If required CI jobs PASS:
   - merge PR #20,
   - update `docs/CURRENT-STATUS.md`,
   - investigate one next shared dispatch/chassis hotspot,
   - update/attach this handoff,
   - stop.
4. If CI fails:
   - leave PR #20 unmerged,
   - fix only the failing issue,
   - rerun validation,
   - update/attach this handoff,
   - stop.

## Last turn update — 2026-09-24

Latest `main` inspected at turn start:

- `b2130a8e4ee7bfdbce0900d70507db51645c9166` — `Refresh MUA2 handoff while PR19 full CI runs`

Changes this turn:

- PR #19 full Ubuntu integration PASS.
- PR #19 full Windows/MSVC integration PASS.
- Merged PR #19 as `12a75b2db225678368be5e2b6340218ba2aafa2f`.
- Updated `docs/CURRENT-STATUS.md` as `f7467d44abc1a7dbed4b015e221a6e1c23372c7a`.
- Implemented the cached host-call-state fast path.
- Added regression coverage for known cached state plus lazy unknown-state fallback.
- Opened PR #20.
- PR #20 CI started; Ubuntu tooling already PASS, remaining jobs are running.
- No RMSE52 game-side run occurred.
