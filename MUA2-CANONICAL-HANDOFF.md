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
- Original Windows workspace target: `D:\\Programming\\GitHub\\OpenMUA2\\`
- Local proprietary/generated data: `.local/` only; never commit it.

## Accepted runtime/performance state

Native DOL and native REL execution are present. SMC/chunk-hash protection remains enabled.

Key accepted performance/correctness commits:

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

## Historical RMSE52 boundary

Observed retained facts:

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

Do not claim current FPS improvement until the actual RMSE52 route is rerun.

## Latest accepted work — PR #17

PR #17 `Reuse REL section for return translation` is merged as:

- `7b7e412f671039e1d29e2cdff7b2e51509bc046e`

Validation:

- OpenMUA2 tooling run `36026447896`: PASS on Ubuntu and Windows.
- ModernGekko run `36026447968`: standalone + full build/test PASS on Ubuntu and Windows.
- Windows full build used MSVC/Ninja.

`docs/CURRENT-STATUS.md` was updated in:

- `bccc38eb90bd822beca965773e3d849d79e8eb18`

## Current pending work — PR #18

**PR:** #18 — `Reuse REL section for native continuation lookup`  
**Branch:** `perf/rel-native-section-hint`  
**Head:** `26fbbf169146d950a6fbd9bd13d63a7eaf70cd6d`  
**State:** OPEN / UNMERGED

Focused behavior:

- Adds an optional `rel_section_hint` to runtime→linked native address resolution.
- On burst continuation, the previous active REL section index is used as the first runtime-range check.
- Same-section continuation can therefore map runtime→linked directly instead of linearly scanning every active REL section.
- Hint miss preserves the existing full active-section scan.
- Direct DOL lookup remains after the active REL scan.
- `RefreshRelSections()` remains the final fallback when allowed.
- Initial dispatch and callers without a hint retain the old behavior through the default sentinel `0xffffffffu`.
- PR #17's linked→runtime same-section fast path is unchanged.

Files changed in PR #18:

- `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore.h`
- `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore_SMC.cpp`
- `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore_Run.cpp`
- `tests/test_staticrecomp_rel_dispatch_reuse_perf.py`
- `tests/test_staticrecomp_host_call_gate_perf.py`

Implementation details:

- `ResolveNativeAddress(..., rel_section_hint)` factors a `resolve_section(i)` helper.
- It checks `rel_section_hint` first when valid.
- On a miss, it scans all other active sections, then performs the original direct-DOL lookup, then the original refresh-and-rescan fallback.
- `ChunkIndexOf`, `FastDispatchableAt`, and `DispatchableAt` carry the optional hint.
- `fast_native_continue()` snapshots the current section index as the hint before the call overwrites the output section index.

## PR #18 validation state

OpenMUA2 tooling Actions run `36030032034`:

- Ubuntu Python tests: **PASS**
- Windows Python tests: `in_progress` at end of turn; it had reached post-checkout cleanup after the tooling test step.

ModernGekko Actions run `36030032243`:

- Standalone Ubuntu: `in_progress`, Configure step.
- Standalone Windows: `in_progress`, Configure step.
- Full Ubuntu: `in_progress`, Install Linux dependencies.
- Full Windows: `in_progress`, Configure step.

No CI failure has appeared.

One combined workflow-status query timed out once; separate workflow queries succeeded. This was a connector timeout, not a CI failure.

## Current blockers

1. **Immediate integration gate:** PR #18 CI must finish. Do not merge until required Windows/Ubuntu jobs are green.
2. **Game-performance gate:** this environment still lacks the proprietary RMSE52 workspace/image, so no current-main FPS can be measured here.

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
2. Check PR #18 head and workflow runs `36030032034` / `36030032243`.
3. If all required CI jobs PASS:
   - merge PR #18,
   - update `docs/CURRENT-STATUS.md`,
   - investigate the next single dispatch/chassis hotspot,
   - update/attach this handoff,
   - stop.
4. If CI fails:
   - keep PR #18 unmerged,
   - fix only the failing issue,
   - rerun validation,
   - update/attach this handoff,
   - stop.

## Last turn update — 2026-09-24

Latest `main` inspected at turn start:

- `6af73016d480b94ca286402e0b524e67ecb2d34c` — `Update MUA2 handoff after PR17 merge`

Changes this turn:

- Implemented the reverse same-section REL hint for continuation-side runtime→linked resolution.
- Opened PR #18.
- Added/updated targeted source regressions.
- Did not merge PR #18 because CI is still running.
- No RMSE52 game-side run occurred.
