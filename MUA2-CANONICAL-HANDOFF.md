# MUA2 CANONICAL HANDOFF — Wii Marvel: Ultimate Alliance 2 native PC recompilation

**Project:** Wii Marvel: Ultimate Alliance 2 USA (RMSE52) native-PC recompilation  
**Repository:** `GTTeancum/OpenMUA2`  
**Canonical branch:** `main`  
**Date:** 2026-09-24

## Mandatory workflow

- GitHub `main` is authoritative; inspect it before editing.
- **Multiplatform performance is the active priority.**
- Preserve SMC/hash/audit/REL eligibility/verification protections.
- Use moderately short turns because resume-stream failures occur: one focused merge/implementation plus validation, then update/attach this handoff.
- At the end of every turn, update this file, commit it to `main`, and attach `MUA2-CANONICAL-HANDOFF.md` in chat.
- Never commit proprietary RMSE52 data, extracted files, generated proprietary translation output, saves, logs, screenshots, or RAM captures.
- Never claim FPS/gameplay validation without an actual RMSE52 run.

## Important locations

- Static recomp runtime: `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/`
- DolRecomp: `project/lib/DolRecomp/`
- Vendored DolRecomp: `project/lib/ModernGekko/vendor/dolphin/DolRecomp/`
- Module template/export: `project/lib/ModernGekko/vendor/dolphin/module-template/`
- Tooling: `tools/`
- Tests: `tests/`
- CI: `.github/workflows/`
- Current status: `docs/CURRENT-STATUS.md`
- Work log: `docs/WORKLOG-2026-09-23.md`
- Recovery plan: `docs/RECOVERY-PLAN.md`
- Windows workspace target: `D:\\Programming\\GitHub\\OpenMUA2\\`
- Local proprietary/generated data: `.local/` only

## Accepted runtime/performance state

Native DOL + REL execution is present. SMC/chunk-hash protection remains enabled.

Key accepted performance commits:

- `a6328f40bb0c98a58c8f50e52a30d4da55390b7e` — `O2 + indexed` default.
- `bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc` — page-index combined DOL+REL dispatch.
- `f4b7f991deb4a9e787b97c3bac2a475faadfc1f6` — empty/single-chunk merged-dispatch fast paths.
- `405b81de4136a7532e966218185a190f6eb9230d` — page-local ordinary indexed dispatch.
- `917a5d893087736a74566dbaf71de5f3989a6d90` — cached x86-64-v3 feature detection.
- `4e677ac8491bc3b5256998ba3688893f61d07695` — module-load baseline/v3 chassis binding.
- `5e6ad207c77affbf500bf5327ce6222e9e7fd7c1` — reuse module-load v3 choice for indirect dispatch.
- `2776fa0a3e80136495a32552b9d909e16dcfcd5e` — gate host-call probes by cached per-chunk coverage.
- `af7938bdb63d4530ea43a6ba445800fc4171a153` — reuse runtime→linked PC between eligibility and dispatch.
- `7b7e412f671039e1d29e2cdff7b2e51509bc046e` — same-section linked→runtime REL hint.
- `7a9cdc0165257ec19301972785150e4961f7f65e` — same-section runtime→linked continuation hint.
- `12a75b2db225678368be5e2b6340218ba2aafa2f` — skip empty forced-fallback range scans.
- `5b87d08b8f6e4daff2ca64bab75d67632ec28721` — read cached host-call chunk state directly in the burst path.
- `8f8c32a9c90e039c898d78eb8313c7d9d64f28c5` — generated chassis-only dispatch skips duplicate host-call dispatch while preserving replacements and physical alias fallback.

Other important accepted correctness/runtime commits:

- `67d75dc63455b47e34159fefae4ed39fca5e5efc` — absolute linked REL section-table support.
- `770db1089526cae9d0fc1b48fbd65c760e75efe6` — cache-control ops remain in native bursts.
- `7e0b48660f90e93e7333eaf4a127c3afb7b9ceb6` — scalar FMA semantics repair.
- `26334ad651567bf098401f781d1189e5f75c296c` — MEM2 lockstep journaling/restoration.
- `46091e1a2394e052f116838727471cbed974aa2d` — merged DOL+REL dispatch eligibility guards.

## Latest accepted work — PR #21

PR #21 `Skip duplicate chassis host-call dispatch` is **MERGED**.

Merge commit:

- `8f8c32a9c90e039c898d78eb8313c7d9d64f28c5`

Behavior:

- Generator emits chassis-only dispatch helpers.
- Chassis-only helpers preserve replacement dispatch and physical MEM1 alias fallback.
- Chassis-only helpers skip `ppc_host_call()` because StaticRecomp already rejected host-call PCs.
- Normal/generated indirect dispatch remains host-call-aware.
- Baseline and x86-64-v3 chassis descriptors bind directly to the chassis-only helper.

Validation:

- OpenMUA2 tooling run `36042200541`: PASS on Ubuntu + Windows.
- DolRecomp run `36042200581`: PASS on Ubuntu + Windows.
- ModernGekko run `36042200621`: standalone + full build/test PASS on Ubuntu + Windows, including MSVC/Ninja.

Status doc updated in:

- `34991fac838de555642b48a93ff177c04c9c1ee3` — `Record merged chassis host-call dispatch fast path`

## Current pending work — PR #22

**PR:** #22 — `Remove redundant module-active burst check`  
**Branch:** `perf/remove-redundant-module-active-backedge`  
**Head:** `99878e7dabbafc7554c40312121eab02463c01e7`  
**State:** OPEN / UNMERGED

Focused behavior:

- Removes the explicit `m_module_active &&` from the native burst back-edge.
- This is redundant because every `fast_native_continue()` path already rejects an inactive module:
  - REL/forced-fallback path reaches `FastDispatchableAt() -> ChunkIndexOf()`, whose first guard rejects `!m_module_active`.
  - direct lookup path checks `!m_module_active || m_chunk_lookup_table.empty()` before table lookup.
- Timing, exception, host-call, downcount, CPU-state, and dispatchability conditions remain unchanged.
- Regression pins the single module-active gate and both underlying inactive-module guards.

Files changed:

- `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore_Run.cpp`
- `tests/test_staticrecomp_rel_dispatch_reuse_perf.py`

Diff size:

- runtime: 1 addition / 2 deletions
- regression: 22 additions

## PR #22 validation state

OpenMUA2 tooling run `36047001104`:

- Ubuntu: **PASS**
- Windows: **PASS**

ModernGekko run `36047001119`:

- Standalone Ubuntu: `in_progress` (Configure)
- Full Ubuntu: `in_progress` (Configure)
- Standalone Windows: `in_progress` (Configure)
- Full Windows: `in_progress` (Configure)

No CI failure has appeared.

PR #22 must remain unmerged until required Windows/Ubuntu validation completes.

## Historical RMSE52 performance boundary

Historical accepted native-REL run, predating recent optimizations:

- reported FPS ~`10.319`
- guest-frame FPS ~`2.062`
- speed ~`0.1404`
- native dispatches `174,476,293`
- native exceptions `5,959`
- hook fallback `1,299,055`
- hook fast cache `1,295,291`
- hook slow `3,764`
- JIT fallback runs `11,023`

This proves native REL progression, **not current performance**.

## Current blockers

1. **Immediate integration gate:** PR #22 CI must complete successfully.
2. **Game-performance gate:** this environment does not have the proprietary RMSE52 workspace/image, so current-main FPS cannot be measured here.

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
2. Check PR #22 and workflow runs `36047001104` / `36047001119`.
3. If required CI jobs PASS:
   - merge PR #22,
   - update `docs/CURRENT-STATUS.md`,
   - inspect one next genuinely recurring burst/chassis cost,
   - update/attach this handoff,
   - stop.
4. If CI fails:
   - leave PR #22 unmerged,
   - fix only the failing issue,
   - rerun validation,
   - update/attach this handoff,
   - stop.

## Last turn update — 2026-09-24

Latest source/status commits produced or observed this turn:

- `8f8c32a9c90e039c898d78eb8313c7d9d64f28c5` — PR #21 merged
- `34991fac838de555642b48a93ff177c04c9c1ee3` — current status updated for PR #21

What happened:

- Reconciled parallel work and found PR #21 pending.
- Confirmed PR #21 tooling, DolRecomp, and full ModernGekko CI PASS on Windows + Ubuntu.
- Merged PR #21.
- Updated `docs/CURRENT-STATUS.md`.
- Inspected the remaining burst loop and verified the explicit `m_module_active` back-edge check was redundant with `fast_native_continue()`.
- Implemented the single-branch cleanup on a new branch.
- Added targeted source regression coverage.
- Opened PR #22.
- PR #22 tooling PASS on Windows and Ubuntu; all four ModernGekko jobs are in progress at Configure; no failure observed.
- No RMSE52 game-side run occurred.
