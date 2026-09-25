# MUA2 CANONICAL HANDOFF — Wii Marvel: Ultimate Alliance 2 native PC recompilation

**Project:** Wii Marvel: Ultimate Alliance 2 USA (RMSE52) native-PC recompilation  
**Repository:** `GTTeancum/OpenMUA2`  
**Canonical branch:** `main`  
**Date:** 2026-09-25

## Mandatory workflow

- GitHub `main` is authoritative; inspect it before editing.
- **Multiplatform performance is the active priority.**
- Preserve SMC/hash/audit/REL eligibility/verification protections.
- Use moderately short turns because resume-stream failures occur: one focused merge/implementation plus validation, then update/attach this handoff.
- At the end of every turn, update this file, commit it to `main`, and attach `MUA2-CANONICAL-HANDOFF.md` in chat.
- Never commit proprietary RMSE52 data, extracted files, generated proprietary translation output, saves, logs, screenshots, or RAM captures.
- When RMSE52 game assets are available, keep their persistent copies in `/MUA2/RMSE52-Game-Files` and materialize working copies under `.local/game`; do not use GitHub checkpoints as substitutes for the original game payload.
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
- Persistent private game-file Library: `/MUA2/RMSE52-Game-Files`

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
- `d3aeab80a3ae44c2d18265ad9f1e5868bf369e50` — check cheap burst termination conditions before continuation eligibility.
- `b78431f21625ad61b4f66855f5f94b859f04cf7a` — check fallback-slice termination before dispatchability/host-call probes.

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

## Latest accepted work — PR #25

PR #25 `Check cheap burst termination before continuation lookup` is **MERGED**.

Merge commit:

- `d3aeab80a3ae44c2d18265ad9f1e5868bf369e50`

Behavior:

- Reorders the native burst back-edge to check `ppc.downcount > 0` and CPU running state before `fast_native_continue()`.
- When the burst must already terminate, it skips one chunk/REL/host-call continuation eligibility probe.
- When the burst can continue, the existing `fast_native_continue()` path is unchanged.
- Synchronous/external exception breaks remain before the back-edge condition.
- Skipped REL refresh and lazy host-call-cache population are eligibility/cache work only and are re-established on the next eligible entry.

Validation:

- OpenMUA2 tooling run `36074380247`: **PASS on Ubuntu + Windows**.
- ModernGekko run `36074380288`: **PASS all four jobs**:
  - standalone Windows: PASS;
  - standalone Ubuntu: PASS;
  - full Windows build/test: PASS;
  - full Ubuntu build/test: PASS.
- No RMSE52 game-side FPS claim is made from CI.

Status doc:

- `3417a27ee6a0d3b032650020724c9758b2191973` — `Record merged cheap burst termination gate`

## Latest accepted work — PR #27

PR #27 `Check fallback termination before dispatch probes` is **MERGED**.

Merge commit:

- `b78431f21625ad61b4f66855f5f94b859f04cf7a`

Behavior:

- Reorders the interpreter-only fallback loop so `ppc.downcount > 0` and CPU running state are checked before `DispatchableAt(ppc.pc)` and `IsHostCallAddress(ppc.pc)`.
- An exhausted/stopped fallback slice no longer performs chunk verification, REL eligibility refresh, or an exact host-call lookup only to discover that it cannot continue.
- If the slice can continue, the same dispatchability and host-call stop conditions still run before another interpreted instruction.
- `IsHostCallAddress()` is read-only.
- Deferred `DispatchableAt()` work is eligibility preparation only; after a downcount exit the next outer-loop entry runs `core_timing.Advance()` and checks dispatchability before any native execution. After CPU stop there is no next execution to prepare.
- Forced-fallback, exception delivery, timing, SMC/hash protection, lockstep, and native re-entry semantics otherwise remain unchanged.

Validation:

- OpenMUA2 tooling run `36130494685`: **PASS on Ubuntu + Windows**.
- ModernGekko run `36130494695`: **PASS all four jobs**:
  - standalone Windows: PASS;
  - standalone Ubuntu: PASS;
  - full Windows build/test: PASS;
  - full Ubuntu build/test: PASS.
- No RMSE52 game-side FPS claim is made from CI.

Status doc:

- `c1ee8eba08ba06d2a08b687e26a8615f5c63ebfc` — `Record merged fallback termination ordering`

## Current blockers

1. **Game-performance gate:** this environment does not have the proprietary RMSE52 workspace/image, so current FPS cannot be measured here.
2. There is no current source/CI integration blocker after PR #27.

## Next exact turn

1. Inspect current `main` after PR #27 and the status/handoff commits.
2. Implement one focused multiplatform performance cleanup at native-entry host-call handoff:
   - current native-entry condition calls `host_call_at(ppc.pc, entry_chunk_index)`;
   - when that returns true, the immediately following fallback branch calls `IsHostCallAddress(ppc.pc)` again for the same PC;
   - preserve the proven exact-host-call result from the entry gate and reuse it in the fallback branch, eliminating that duplicate exact lookup.
3. Preserve all other cases:
   - if the module is inactive or `DispatchableAt()` fails, the fallback branch must still perform its direct `m_guest.host_call && IsHostCallAddress(ppc.pc)` check;
   - candidate-chunk coverage and exact-address semantics remain unchanged;
   - host-call handler, passthrough-JIT invalidation, timing, forced-fallback, SMC/hash, and exception behavior remain unchanged.
4. Add a focused source regression proving the exact host-call result is reused only on the dispatchable-entry rejection path.
5. Run OpenMUA2 tooling CI and ModernGekko Windows/Ubuntu validation; merge only if green.
6. Update/attach this handoff and stop.
7. Do not claim FPS improvement without a fresh RMSE52 benchmark.

## Last turn update — 2026-09-25

What happened:

- Created private persistent Library folder `/MUA2/RMSE52-Game-Files`.
- Added `/MUA2/RMSE52-Game-Files/README.md` with RMSE52 verification metadata for the original `sys/main.dol` and known REL.
- Audited persistent Library storage for the original five split 7z volumes, WBFS/ISO, extracted `sys/main.dol`, and `Marvel-rev-fin-plf2.rel`.
- The actual proprietary RMSE52 game payload is not currently present in Library storage.
- Inspected `OpenMUA2_LOCAL01.zip` and `MUA2_MG01_Checkpoint.zip`; neither contains the original extracted RMSE52 game payload. The MG01 archive contains a generated recompilation `main.dol`, which must not be substituted for the original `sys/main.dol`.
- Future uploads of the original game payload should be saved directly to `/MUA2/RMSE52-Game-Files` so later chats can retrieve them without re-upload.
- No proprietary game data was committed to GitHub.
