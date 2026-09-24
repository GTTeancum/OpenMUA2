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
- `12a75b2db225678368be5e2b6340218ba2aafa2f` — skip empty forced-fallback range scans in native eligibility.
- `5b87d08b8f6e4daff2ca64bab75d67632ec28721` — read cached host-call chunk state directly in the burst path.
- `8f8c32a9c90e039c898d78eb8313c7d9d64f28c5` — chassis-only generated dispatch skips duplicate host-call dispatch.
- `dc43d362d425134222d00aa02dd4dbec99fcf222` — remove redundant explicit module-active check from the native burst back-edge.

Important accepted correctness/runtime commits include absolute REL section-table support (`67d75dc6...`), native cache-control codegen (`770db108...`), scalar FMA repair (`7e0b4866...`), MEM2 lockstep journaling (`26334ad6...`), and merged DOL+REL eligibility guards (`46091e1a...`).

## Latest accepted work — PR #22

PR #22 `Remove redundant module-active burst check` is **MERGED**.

Merge commit:

- `dc43d362d425134222d00aa02dd4dbec99fcf222`

Behavior:

- Removes the explicit `m_module_active &&` from the native burst back-edge.
- `fast_native_continue()` already rejects inactive modules on both paths:
  - REL/forced-fallback path: `FastDispatchableAt() -> ChunkIndexOf()` rejects `!m_module_active`.
  - direct lookup path: rejects `!m_module_active || m_chunk_lookup_table.empty()`.
- Host-call, downcount, CPU-state, exception, timing, and dispatchability behavior remain unchanged.

Validation:

- OpenMUA2 tooling run `36047001104`: PASS on Ubuntu + Windows.
- ModernGekko run `36047001119`: standalone + full build/test PASS on Ubuntu + Windows, including MSVC/Ninja.

Status doc:

- `039782520c25933f7c6653797332bd7e8d0715cd` — `Record merged module-active burst cleanup`

## Current pending work — PR #23

**PR:** #23 — `Skip empty forced-fallback scan in interpreter path`  
**Branch:** `perf/skip-empty-fallback-interpreter-scan`  
**Head:** `0732b54cd91c465213b79255db5a45c6c7684ed2`  
**State:** OPEN / UNMERGED

Focused behavior:

- Extends the accepted PR #19 empty-range short circuit to the interpreter/fallback branch in `Run()`.
- When `m_forced_fallback_ranges` is empty, the fallback path no longer calls `IsForcedFallbackAddress(ppc.pc)`.
- When ranges exist, the exact prior forced-fallback behavior is preserved.
- No native dispatch, REL, SMC, host-call, timing, or exception semantics change.

Files changed:

- `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore_Run.cpp`
- `tests/test_staticrecomp_rel_dispatch_reuse_perf.py`

Diff size:

- runtime: 3 changed lines
- regression: 13 additions

## PR #23 validation state

OpenMUA2 tooling run `36055136834`:

- Ubuntu: **PASS**
- Windows: **PASS**

ModernGekko run `36055136817`:

- Standalone Ubuntu: `in_progress` (Configure)
- Full Ubuntu: `in_progress` (Configure)
- Standalone Windows: `in_progress` (Configure)
- Full Windows: `in_progress` (Configure)

No CI failure has appeared.

PR #23 must remain unmerged until required Windows/Ubuntu validation completes.

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

1. **Immediate integration gate:** PR #23 CI must complete successfully.
2. **Game-performance gate:** this environment does not have the proprietary RMSE52 workspace/image, so current-main FPS cannot be measured here.

## Next performance direction

After PR #23, avoid another speculative micro-cleanup unless the source evidence is strong. The remaining meaningful work is increasingly in:

- cross-section/cross-chunk transfer overhead;
- preserving the generated linked result across the post-dispatch runtime translation so continuation can avoid a linked→runtime→linked round trip, if that can be proven safe across REL section changes and physical aliases;
- or fresh profiling from the actual RMSE52 route when the proprietary workspace is available.

## Next exact turn

1. Inspect current `main`.
2. Check PR #23 and workflow runs `36055136834` / `36055136817`.
3. If required CI jobs PASS:
   - merge PR #23,
   - update `docs/CURRENT-STATUS.md`,
   - investigate one next genuinely recurring transfer cost (prefer linked-result preservation or profiling evidence over another trivial branch),
   - update/attach this handoff,
   - stop.
4. If CI fails:
   - leave PR #23 unmerged,
   - fix only the failing issue,
   - rerun validation,
   - update/attach this handoff,
   - stop.

## Last turn update — 2026-09-24

What happened:

- Reconciled substantial parallel progress: PR #20 and PR #21 were already merged and fully validated.
- PR #22 tooling and full ModernGekko CI were fully green on Windows + Ubuntu.
- Merged PR #22 as `dc43d362d425134222d00aa02dd4dbec99fcf222`.
- Updated `docs/CURRENT-STATUS.md` as `039782520c25933f7c6653797332bd7e8d0715cd`.
- Inspected the remaining burst/fallback path.
- Implemented the empty forced-fallback-range short circuit in the interpreter/fallback path.
- Added a targeted regression.
- Opened PR #23.
- PR #23 tooling PASS on Windows + Ubuntu; all four ModernGekko jobs are in progress at Configure; no failure observed.
- No RMSE52 game-side run occurred.
