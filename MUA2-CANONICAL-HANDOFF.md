# MUA2 CANONICAL HANDOFF — Wii Marvel: Ultimate Alliance 2 native PC recompilation

**Project:** Wii Marvel: Ultimate Alliance 2 USA (RMSE52) native-PC recompilation  
**Repository:** `GTTeancum/OpenMUA2`  
**Canonical branch:** `main`  
**Date:** 2026-09-24

## Mandatory workflow rules

- GitHub `main` is authoritative. Inspect current `main` before every change.
- **Multiplatform performance is the active priority.** Correctness expansion comes later unless a concrete failure blocks performance work.
- Preserve SMC/hash/audit/REL eligibility/verification protections.
- Use **moderately short turns** because resume-stream failures occur: one focused implementation/merge plus validation is appropriate; do not batch unrelated optimizations.
- At the end of **every turn**, update this file, commit it to `main`, and attach `MUA2-CANONICAL-HANDOFF.md` in chat.
- Never fabricate build, game, controller, save, audio, or performance results.
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

## Latest accepted work — PR #18

PR #18 `Reuse REL section for native continuation lookup` is merged as:

- `7a9cdc0165257ec19301972785150e4961f7f65e`

Validation:

- OpenMUA2 tooling run `36030032034`: PASS on Ubuntu and Windows.
- ModernGekko run `36030032243`: standalone + full build/test PASS on Ubuntu and Windows.
- Windows full build used MSVC/Ninja.

Status documentation:

- `41ac96c8fe9b6feee8daecd73d0ec2342cce4f2f` — `Record merged REL continuation hint`

## Current pending work — PR #19

**PR:** #19 — `Skip empty forced-fallback scans`  
**Branch:** `perf/skip-empty-forced-fallback-scan`  
**Head:** `00cdde0bacb89106168c18354fe3adf639ffe388`  
**State:** OPEN / UNMERGED

Focused behavior:

- `FastDispatchableAt()` now calls `IsForcedFallbackAddress(address)` only when `m_forced_fallback_ranges` is non-empty.
- `DispatchableAt()` uses the same short-circuit.
- Default/normal configuration therefore avoids an out-of-line helper call plus empty range-loop on every native eligibility test.
- When forced-fallback ranges are configured, the exact existing helper and range semantics remain unchanged.
- No REL, chunk verification, host-call, refresh, or dispatch lookup behavior is otherwise changed.

Files changed:

- `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore_SMC.cpp`
- `tests/test_staticrecomp_rel_dispatch_reuse_perf.py`

Diff size:

- runtime: 2 changed guards
- regression: 17 added lines

## PR #19 validation state

OpenMUA2 tooling Actions run `36035472440`:

- Ubuntu Python tests: **PASS**
- Windows Python tests: **PASS**

ModernGekko Actions run `36035472344`:

- Standalone Ubuntu: **PASS**
- Standalone Windows: **PASS**
- Full Ubuntu: still `in_progress`, Build step
- Full Windows: still `in_progress`, Build step

No CI failure has appeared. PR #19 remains unmerged until both full integration jobs finish successfully.

## Current blockers

1. **Immediate integration gate:** PR #19 CI must complete successfully.
2. **Game-performance gate:** this environment still lacks the proprietary RMSE52 workspace/image, so current-main FPS cannot be measured here.

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

## Next performance target — investigated, not yet changed

The next likely per-block chassis cost is the cached host-call chunk query.

Verified source facts:

- `host_call_at` runs on burst entry and continuation.
- It currently calls `ChunkContainsHostCall(chunk_index)` every time before exact `IsHostCallAddress(address)`.
- `ChunkContainsHostCall()` caches its result in `m_chunk_host_call_state` using states 0=unknown, 1=clean, 2=contains host call.
- When `host_call_range_contains` is available, `LoadModule()` precomputes that cache for every chunk.
- Therefore the normal hot path still pays an out-of-line helper call even when the chunk state is already known.
- A likely safe follow-up is to read the cached state directly in the run-loop fast path and only call `ChunkContainsHostCall()` when the state is still 0, preserving exact address checks for state 2 and lazy behavior for unknown chunks.

Do not implement this until PR #19 is accepted.

## Next exact turn

1. Inspect current `main`.
2. Check PR #19 and ModernGekko run `36035472344`.
3. If both full jobs PASS:
   - merge PR #19,
   - update `docs/CURRENT-STATUS.md`,
   - implement only the cached host-call-state fast path described above if source still supports it,
   - open focused CI,
   - update/attach this handoff,
   - stop.
4. If either full job FAILS:
   - leave PR #19 unmerged,
   - fix only that failure,
   - rerun validation,
   - update/attach this handoff,
   - stop.

## Last turn update — 2026-09-24

Latest `main` inspected at turn start:

- `5085175b6cd8d00b04f35cc93593d82de421bb38` — `Update MUA2 handoff for pending PR19`

Changes this turn:

- No new source code was added beyond the already-open PR #19 implementation.
- Re-checked PR #19 CI.
- Tooling PASS on Windows and Ubuntu.
- Standalone ModernGekko PASS on Windows and Ubuntu.
- Full ModernGekko Ubuntu remains `in_progress` in Build.
- Full ModernGekko Windows remains `in_progress` in Build.
- Investigated the next per-block chassis cost and identified the cached `ChunkContainsHostCall()` call as a likely follow-up optimization.
- PR #19 remains open/unmerged until both full integration jobs pass.
- No RMSE52 game-side run occurred.
