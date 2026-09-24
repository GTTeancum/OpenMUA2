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
- `20dd90851c3a2625ddb973f8850e2494bb33ca9f` — skip the forced-fallback helper in the interpreter/fallback branch when no forced-fallback ranges are configured.

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

## Latest accepted work — PR #23

PR #23 `Skip empty forced-fallback scan in interpreter path` is **MERGED**.

Merge commit:

- `20dd90851c3a2625ddb973f8850e2494bb33ca9f`

Behavior:

- Extends PR #19's empty forced-fallback-range short circuit to the interpreter/fallback branch in `Run()`.
- When `m_forced_fallback_ranges` is empty, the fallback path no longer calls `IsForcedFallbackAddress(ppc.pc)`.
- Configured forced-fallback ranges retain the exact prior behavior.
- Native dispatch, REL, SMC, host-call, timing, and exception semantics are unchanged.

Validation:

- OpenMUA2 tooling run `36055136834`: PASS on Ubuntu + Windows.
- ModernGekko run `36055136817`: standalone + full build/test PASS on Ubuntu + Windows, including MSVC/Ninja.

Status doc:

- `2de1dee811932d3d010dc606abf02cb7ab749769` — `Record merged interpreter fallback fast path`

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

1. **Game-performance gate:** this environment does not have the proprietary RMSE52 workspace/image, so current-main FPS cannot be measured here.
2. There is no pending source-integration PR at the end of this turn.

## Next performance direction — investigated, not yet changed

The next genuinely recurring transfer cost is the post-dispatch linked→runtime→linked round trip before burst continuation.

Current source behavior:

1. Generated dispatch returns a **linked** PC in `m_guest.pc`.
2. `TranslateRelAddress()` converts it to the runtime PC for tracing, lockstep, idle-loop, exception and host-side checks.
3. `fast_native_continue()` then calls dispatchability, which converts that runtime PC back to linked form before the next generated dispatch.

A future optimization could preserve the generated linked result in a separate local while keeping `m_guest.pc` in runtime form for all host-side semantics, then use the preserved linked result for the continuation chunk lookup.

Do **not** implement that blindly. Before changing it, prove all of the following:

- lockstep verification cannot legitimately replace the post-dispatch PC before continuation;
- exception/interrupt handling cannot require a different linked result;
- REL section refresh/unload/relocation semantics remain equivalent;
- physical aliases and DOL/REL cross-section transfers still take the correct fallback;
- forced-fallback and exact host-call checks continue to use the runtime address.

Because the recent source cleanups have removed many obvious per-block calls/branches, prefer this transfer optimization only if the invariants above can be pinned by targeted tests. Otherwise the next meaningful step is fresh RMSE52 profiling rather than another speculative micro-cleanup.

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

## Next exact turn

1. Inspect current `main`.
2. Continue the linked-result-preservation investigation above.
3. If the invariants can be proven from source and targeted regression coverage, implement one focused PR that preserves the linked result across host-side runtime-PC work and uses it for continuation lookup.
4. If those invariants cannot be proven safely, do not force the optimization; record the blocker and wait for fresh RMSE52 profiling evidence.
5. Update/attach this handoff and stop.

## Last turn update — 2026-09-24

What happened:

- Reconciled parallel progress through PR #23.
- PR #23 tooling and all four ModernGekko jobs were fully green on Windows + Ubuntu.
- Merged PR #23 as `20dd90851c3a2625ddb973f8850e2494bb33ca9f`.
- Updated `docs/CURRENT-STATUS.md` as `2de1dee811932d3d010dc606abf02cb7ab749769`.
- Inspected the remaining native burst transfer path.
- Confirmed the next recurring cost is a linked→runtime translation after generated dispatch followed by runtime→linked resolution for continuation.
- Did not implement linked-result preservation yet because lockstep, exception/interrupt, REL refresh, physical-alias, and cross-section invariants must be proven first.
- No RMSE52 game-side run occurred.
