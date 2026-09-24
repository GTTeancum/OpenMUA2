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
- `de411096f5986980ac58e1f3a7373d8f5351dc67` — reuse generated linked results across eligible native burst continuations.

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

## Latest accepted work — PR #24

PR #24 `Reuse generated linked result on burst continuation` is **MERGED**.

Merge commit:

- `de411096f5986980ac58e1f3a7373d8f5351dc67`

Behavior:

- Preserves the linked PC returned by generated dispatch while keeping `m_guest.pc` in runtime form for host-side semantics.
- `TranslateRelAddress()` reports the active REL section selected during linked→runtime translation.
- `FastDispatchableLinkedAt()` uses a preserved linked result for the next verified chunk lookup when its invariants hold, avoiding the normal runtime→linked conversion.
- Forced-fallback and exact host-call checks remain on the runtime address.
- REL chunks must still belong to the resolved active REL section; DOL chunks require the non-REL sentinel.
- Lockstep-checked blocks and native exception returns disable preserved-result reuse.
- Any direct-linked eligibility miss falls back to `FastDispatchableAt() -> ResolveNativeAddress() -> RefreshRelSections()`.

Validation:

- OpenMUA2 tooling run `36071914753`: **PASS on Ubuntu + Windows**.
- ModernGekko run `36071914841`: **PASS all four jobs**:
  - standalone Windows: PASS;
  - standalone Ubuntu: PASS;
  - full Windows build/test: PASS;
  - full Ubuntu build/test: PASS.
- No RMSE52 game-side FPS claim is made from CI.

Status doc:

- `1256b9ff7e59dd48750fdc93ebcfec63a900ab22` — `Record merged linked continuation fast path`

## Current pending work — PR #25

**PR:** #25 — `Check cheap burst termination before continuation lookup`  
**Branch:** `perf/cheap-burst-termination-first`  
**Head:** `f0e8f0c1d130c49343ac427b2ad99015abfc3fa7`  
**State:** OPEN / UNMERGED

Focused behavior:

- Reorders the native burst back-edge from:
  - `fast_native_continue(...) && ppc.downcount > 0 && CPU running`
- To:
  - `ppc.downcount > 0 && CPU running && fast_native_continue(...)`
- When the burst must already terminate for exhausted cycle budget or stopped CPU state, the runtime now skips one continuation eligibility probe.
- When the burst can continue, the same `fast_native_continue()` path still runs unchanged.
- Synchronous/external exception breaks remain before the back-edge condition.

Safety proof:

- `FastDispatchableLinkedAt()` only reads eligibility/forced-fallback/chunk/REL state.
- The fallback `FastDispatchableAt() -> ChunkIndexOf() -> ResolveNativeAddress()` may call `RefreshRelSections()`, but that refresh exists to prepare eligibility; after a downcount exit the next outer-loop entry calls `DispatchableAt()` again after timing advance, and after CPU stop there is no continuation.
- Lazy `ChunkContainsHostCall()` discovery only populates the host-call coverage cache; skipping it on an already-terminating edge does not change guest-visible state and discovery occurs on the next eligible entry.
- Forced-fallback, exception, timing, SMC/hash verification, lockstep, and host-call semantics are otherwise unchanged.

Files changed:

- `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore_Run.cpp`
- `tests/test_staticrecomp_rel_dispatch_reuse_perf.py`

Validation state:

- OpenMUA2 tooling run `36074380247`: **PASS on Ubuntu + Windows**.
- ModernGekko run `36074380288`: **IN PROGRESS** at handoff-update time.
- No RMSE52 game-side run occurred.

## Current blockers

1. **Immediate integration gate:** PR #25 requires current-head ModernGekko Windows/Ubuntu validation before merge.
2. **Game-performance gate:** this environment does not have the proprietary RMSE52 workspace/image, so current FPS cannot be measured here.

## Next exact turn

1. Inspect current `main` and PR #25 head `f0e8f0c1d130c49343ac427b2ad99015abfc3fa7`.
2. Check ModernGekko run `36074380288`.
3. If standalone + full build/test PASS on Ubuntu + Windows:
   - merge PR #25,
   - update `docs/CURRENT-STATUS.md`,
   - update/attach this handoff,
   - stop.
4. If any current-head job fails:
   - keep PR #25 unmerged,
   - fix only that failure,
   - rerun validation,
   - update/attach this handoff,
   - stop.
5. Do not claim FPS improvement without a fresh RMSE52 benchmark.

## Last turn update — 2026-09-24

What happened:

- Started from current `main` at `c1cfa52e89f1f7daeb578612e66f0f01987d2cd1`.
- Implemented the next multiplatform performance cleanup on branch `perf/cheap-burst-termination-first`.
- Reordered the native burst back-edge so `ppc.downcount > 0` and CPU running state are checked before `fast_native_continue()`.
- Added a focused regression proving the cheap termination checks precede the continuation probe while retaining the no-duplicate-`m_module_active` invariant.
- Verified the branch diff contains only the intended runtime and test files.
- Opened PR #25 at head `f0e8f0c1d130c49343ac427b2ad99015abfc3fa7`.
- OpenMUA2 tooling run `36074380247`: PASS on Ubuntu + Windows.
- ModernGekko run `36074380288`: in progress.
- No RMSE52 game-side run occurred.
