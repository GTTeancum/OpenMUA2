# MUA2 CANONICAL HANDOFF — Wii Marvel: Ultimate Alliance 2 native PC recompilation

**Project:** Wii Marvel: Ultimate Alliance 2 USA (RMSE52) native-PC recompilation  
**Repository:** `GTTeancum/OpenMUA2`  
**Canonical branch:** `main`  
**Date:** 2026-09-24

## Mandatory workflow rules

- GitHub `main` is authoritative. Always inspect current `main` before editing because parallel work may have advanced it.
- **Multiplatform performance is the active priority.** Correctness expansion comes later unless a concrete failure blocks performance measurement or execution.
- Preserve existing SMC/hash/audit/REL eligibility/verification protections. Never weaken them just to advance farther.
- **Short turns are mandatory** because resume-stream failures are occurring: one focused code change or CI gate, validate it, update this handoff, attach it in chat, stop.
- At the end of **every** turn, update this file, commit it to `main`, and attach `MUA2-CANONICAL-HANDOFF.md` directly in chat.
- Never fabricate build, boot, gameplay, controller, save, audio, or performance results.
- Never commit the RMSE52 WBFS, extracted proprietary game files, generated proprietary translation output, saves, logs, screenshots, or RAM captures.

## Important locations

| Purpose | Path |
|---|---|
| DolRecomp | `project/lib/DolRecomp/` |
| ModernGekko | `project/lib/ModernGekko/` |
| GXRuntime | `project/lib/ModernGekko/vendor/dolphin/GXRuntime/` |
| Static recomp runtime | `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/` |
| Module template/export | `project/lib/ModernGekko/vendor/dolphin/module-template/` |
| Tooling | `tools/` |
| Tests | `tests/` |
| CI | `.github/workflows/` |
| Current status | `docs/CURRENT-STATUS.md` |
| Work log | `docs/WORKLOG-2026-09-23.md` |
| Recovery plan | `docs/RECOVERY-PLAN.md` |
| Transform records | `locks/` |
| Historical recovered patches | `patches/` |

Original intended Windows workspace: `D:\\Programming\\GitHub\\OpenMUA2\\`

Local generated/game data belongs under `.local/` and must stay untracked.

## Current accepted runtime state

Native DOL packaging/execution is present. Native REL packaging/runtime eligibility is present.

Important accepted work:

- `67d75dc63455b47e34159fefae4ed39fca5e5efc` — support absolute linked REL section-table pointers observed in RMSE52.
- `770db1089526cae9d0fc1b48fbd65c760e75efe6` — keep high-frequency cache-control ops in native bursts through the dedicated cache-control hook.
- `7e0b48660f90e93e7333eaf4a127c3afb7b9ceb6` — repair generated scalar FMA helper semantics.
- `26334ad651567bf098401f781d1189e5f75c296c` — journal/restore MEM2 in lockstep verification.
- `46091e1a2394e052f116838727471cbed974aa2d` — preserve merged DOL+REL dispatch eligibility checks.
- `a6328f40bb0c98a58c8f50e52a30d4da55390b7e` — make `O2 + indexed` the default multiplatform performance configuration.
- `bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc` — page-index combined DOL+REL dispatch.
- `f4b7f991deb4a9e787b97c3bac2a475faadfc1f6` — fast-path empty/single-chunk merged-dispatch pages.
- `405b81de4136a7532e966218185a190f6eb9230d` — bound ordinary indexed dispatch to page-local runs; fast-path empty/single-run pages.
- `917a5d893087736a74566dbaf71de5f3989a6d90` — cache x86-64-v3 feature detection once per process.
- `4e677ac8491bc3b5256998ba3688893f61d07695` — bind normal chassis dispatch to baseline/v3 at module load.
- `5e6ad207c77affbf500bf5327ce6222e9e7fd7c1` — reuse module-load x86-64-v3 selection for generated indirect dispatch.
- `2776fa0a3e80136495a32552b9d909e16dcfcd5e` — gate exact host-call probes by cached per-chunk host-call coverage.
- `af7938bdb63d4530ea43a6ba445800fc4171a153` — reuse the runtime→linked PC already resolved by dispatchability for the immediately following native dispatch and burst continuation.

SMC/chunk hash protection remains enabled. Different guest code must not become native merely to advance the game.

Observed retained RMSE52 REL facts from historical diagnostics:

- module ID `1`
- REL version `3`
- module/header around `0x80E4A080`
- linked text around `0x80E4A164`
- accepted native REL text coverage `3,255,744` bytes
- one live REL module discovered
- the accepted historical run advanced real game frames

Do **not** treat that historical run as fresh current-main validation.

## Performance boundary

The retained live native-REL benchmark predates the recent dispatch optimizations:

- average reported FPS: about `10.319`
- guest-frame FPS: about `2.062`
- average speed: about `0.1404`
- native dispatches: `174,476,293`
- native exceptions: `5,959`
- hook fallback: `1,299,055`
- hook fast cache: `1,295,291`
- hook slow: `3,764`
- JIT fallback runs: `11,023`

This proves native REL progression, **not acceptable performance**. Do not claim a current FPS improvement until the actual RMSE52 route is rerun.

## Current pending work — PR #17

**PR:** #17 — `Reuse REL section for return translation`  
**Branch:** `perf/rel-runtime-section-hint`  
**Head:** `5aff006a6d8949c4692dce68b4127627c0829300`  
**State:** OPEN / UNMERGED

Focused behavior:

- `ChunkIndexOf`, `DispatchableAt`, and `FastDispatchableAt` can return the active REL section index alongside the already-carried linked PC.
- Native-burst entry and continuation carry that section index into the dispatch iteration.
- `TranslateRelAddress(linked_address, rel_section_hint)` checks the hinted REL section first.
- If the returned linked PC is still inside the hinted section, translation becomes one bounds check plus arithmetic rather than scanning every active REL section.
- If the result leaves that section, the code falls back to the existing `ResolveRuntimeAddress()` full scan.
- Cross-section and DOL transfers therefore retain the old behavior.
- Non-REL dispatch writes sentinel `0xffffffffu`.
- REL refresh, chunk verification, forced fallback, host-call gating, and previous runtime→linked reuse remain enabled.

Files changed in PR #17:

- `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore.h`
- `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore_SMC.cpp`
- `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore_Run.cpp`
- `tests/test_staticrecomp_rel_dispatch_reuse_perf.py`
- `tests/test_staticrecomp_host_call_gate_perf.py`

## Exact validation state for PR #17

OpenMUA2 tooling Actions run `36026447896`:

- Ubuntu Python tests: **PASS**
- Windows Python tests: **PASS**

ModernGekko Actions run `36026447968`:

- Standalone tests — Ubuntu: **PASS**
- Standalone tests — Windows: **PASS**
- Full build and test — Ubuntu: **IN PROGRESS**, Build step
- Full build and test — Windows: **IN PROGRESS**, Build step

No failed CI job has appeared.

PR #17 must remain unmerged until both full ModernGekko jobs reach final conclusions.

## Current blockers

1. **Immediate source-integration gate:** wait for the two full ModernGekko jobs in run `36026447968`.
2. **Game-performance measurement gate:** this environment does not have the proprietary RMSE52 workspace/image, so current-main FPS cannot be measured here.

## What is not freshly validated

Do not claim any of these from current `main` without a real game-side run:

- complete boot through title/menu
- opening gameplay/combat correctness
- complete level
- long-session stability
- multiplayer
- audio correctness
- physical-controller gameplay correctness
- save/reload across process restart
- full Windows game build and execution
- current-main FPS/speed
- current large lockstep total equivalent to historical `11,832`
- overall crash-free/glitch-free/full completion

## Next exact short turn

1. Inspect current GitHub `main`.
2. Re-check PR #17 head and ModernGekko run `36026447968`.
3. If full Ubuntu and Windows jobs both PASS:
   - merge PR #17,
   - update `docs/CURRENT-STATUS.md`,
   - update this handoff,
   - attach `MUA2-CANONICAL-HANDOFF.md`,
   - stop.
4. If either full job FAILS:
   - keep PR #17 unmerged,
   - fix only that failure,
   - rerun validation,
   - update/attach this handoff,
   - stop.

## Last turn update — 2026-09-24

Latest `main` inspected before this handoff update:

- `8fac9453858010326e0f7641355bd4e3b6ca3e94` — `Condense MUA2 handoff and record PR17 CI gate`

This was a **CI-gate-only short turn**. No source code was changed.

PR #17 state:

- PR: #17 — `Reuse REL section for return translation`
- Branch: `perf/rel-runtime-section-hint`
- Head: `5aff006a6d8949c4692dce68b4127627c0829300`
- State: OPEN / UNMERGED

Validation observed this turn:

- OpenMUA2 tooling run `36026447896`: completed **PASS**
  - Ubuntu Python tests: PASS
  - Windows Python tests: PASS
- ModernGekko run `36026447968`:
  - Standalone Ubuntu: PASS
  - Standalone Windows: PASS
  - Full Ubuntu: still `in_progress`, Build step
  - Full Windows: still `in_progress`, Build step
- No failed CI job has appeared.
- No RMSE52 game-side run occurred.

Current blocker:

- PR #17 cannot merge until both full ModernGekko jobs finish successfully.

Next exact short turn:

1. Inspect current `main`.
2. Re-check ModernGekko run `36026447968`.
3. If both full jobs PASS, merge PR #17, update `docs/CURRENT-STATUS.md`, update/attach this handoff, stop.
4. If either fails, keep PR #17 unmerged, fix only that failure, rerun validation, update/attach this handoff, stop.
