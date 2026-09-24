# MUA2 CANONICAL HANDOFF — Wii Marvel: Ultimate Alliance 2 native PC recompilation

**Project:** Wii Marvel: Ultimate Alliance 2 (USA, RMSE52) native-PC recompilation  
**Repository:** `GTTeancum/OpenMUA2`  
**Canonical branch:** `main`  
**Source state summarized through:** `8e2baf16efb0022b8910ddefa0130843c1b7da0b` (`Record REL dispatch resolution reuse`)  
**Date:** 2026-09-24

> ## MANDATORY END-OF-TURN UPDATE RULE
>
> At the end of **every future development turn**, update this file **before reporting back**, commit the refreshed handoff to GitHub `main`, and **post/attach the refreshed `MUA2-CANONICAL-HANDOFF.md` in the chat** so the user can download it. A development turn is not complete until the updated handoff file has been posted in chat.
>
> Record the latest `main` SHA actually inspected, code/document/test changes, tests/builds/CI/game runs actually performed and their real results, failures/rejected experiments, the exact blocker/next step, Windows/Linux portability status, and any new file/location a successor needs.
>
> A new chat should need only the **posted copy of this file** plus access to `GTTeancum/OpenMUA2`. Read this file, then inspect current GitHub `main` because it may have advanced.

---

## 1. Mission and non-negotiable rules

The goal is a **real native PC recompilation/port** of Wii Marvel: Ultimate Alliance 2 USA, not a Dolphin wrapper and not disguised emulation.

- GitHub `main` is authoritative.
- Continue in small, technically justified increments, with **multiplatform performance as the active priority**.
- Performance work must target shared Windows/Linux paths wherever practical; neither platform is secondary for optimization decisions.
- Prefer changes that benefit MSVC and GCC/Clang together and retain explicit A/B controls when an optimization needs game-side measurement.
- Never weaken chunk hashes, SMC checks, REL eligibility validation, verifier checks, or game timing merely to advance farther.
- Preserve failures as failures and reduce them to focused regressions.
- Never fabricate build, boot, gameplay, controller, save, audio, or performance results.
- The original extracted `sys/main.dol` is the real boot DOL. Generated/merged DOL+REL material is code-generation input only and must never replace it.
- Proprietary WBFS/game data, generated game-derived translation, logs, saves, screenshots, RAM captures, and other local outputs stay outside tracked Git source.

---

## 2. Important locations

| Purpose | Path |
|---|---|
| DolRecomp | `project/lib/DolRecomp/` |
| ModernGekko | `project/lib/ModernGekko/` |
| GXRuntime | `project/lib/ModernGekko/vendor/dolphin/GXRuntime/` |
| Static recomp runtime | `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/` |
| Interpreter reference FP | `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/Interpreter/` |
| Module template/export | `project/lib/ModernGekko/vendor/dolphin/module-template/` |
| Project tooling | `tools/` |
| Project tests | `tests/` |
| CI | `.github/workflows/` |
| Current status | `docs/CURRENT-STATUS.md` |
| Detailed work log | `docs/WORKLOG-2026-09-23.md` |
| Recovery plan | `docs/RECOVERY-PLAN.md` |
| Source transformation records | `locks/` |
| Historical recovered patches | `patches/` |

Original intended Windows workspace: `D:\Programming\GitHub\OpenMUA2\`.

Local generated/game data belongs under `.local/` (including `.local/game`, `.local/logs`, `.local/benchmarks`, `.local/automation`, `.local/user`). Never commit the RMSE52 image or extracted proprietary files.

---

## 3. Current reconstructed native state

The repository originated from the recovered **MG01 + FPC01** source baseline. Missing later behavior was reimplemented and tested in current source rather than claimed as recovered MR01 bytes.

### Native DOL + REL

Native DOL packaging/execution is present. Native REL packaging/runtime eligibility is also present.

RMSE52 uses an **absolute linked REL section-table pointer** in the observed live module. Commit:

`67d75dc63455b47e34159fefae4ed39fca5e5efc` — `Support absolute REL section tables`

`StaticRecompCore::RefreshRelSections()` accepts relative, absolute guest virtual, and physical guest section-table pointers with guest-RAM bounds checks.

Observed retained RMSE52 diagnostics:

- module ID `1`
- REL version `3`
- module/header around `0x80E4A080`
- linked text around `0x80E4A164`
- accepted native REL text coverage `3,255,744` bytes
- one live REL module discovered in the accepted build

The accepted post-fix historical run advanced real game frames. Do not treat that as fresh current-main validation.

### Hash / SMC protection

Native code remains under chunk-hash/SMC verification. Different or modified guest code must not become native merely to advance boot.

`2a86452d2aafe5156f9c4015ad7b2a3ec64b65de` — `Require exact REL audit source`

`tools/merge_mua2_generated.py` now refuses to emit REL metadata unless the retained live audit reports `LIVE_TEXT_MATCH` and the supplied REL's SHA-256 exactly matches the audit's `source_rel_sha256`. This prevents a stale audit from being paired with changed REL bytes before native metadata is emitted.

`eb48bfa68c20ea2d9e9c8731a13cf311d72b3771` — `Verify replayed REL text against live audit`

After relocation replay, the merge tool now also requires the zero-adjust live comparison to be exact, validates matching expected/observed text SHA-256 values, and verifies the replayed executable text bytes against that retained live hash before emitting native REL metadata. A relocation-replay regression that produces the wrong bytes at the correct size is rejected.

### Active performance baseline

`a6328f40bb0c98a58c8f50e52a30d4da55390b7e` — `Make indexed O2 builds the multiplatform performance default`

This is the current default performance configuration for both Windows and Linux:

- native module optimization: `O2`
- generated dispatch lookup: `indexed`
- explicit comparison mode: `--dispatch-lookup linear`
- DOL and REL generation cache identities include the dispatch mode
- build receipts record `module_opt` and `dispatch_lookup`
- combined native DOL+REL lookup is page-indexed as of `bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc`
- single-chunk merged-dispatch pages are fast-pathed as of `f4b7f991deb4a9e787b97c3bac2a475faadfc1f6`: empty indexed pages return immediately, pages overlapping exactly one generated chunk skip the binary-search loop, and only multi-chunk boundary pages use the search loop
- ordinary DolRecomp indexed dispatch is page-bounded as of `405b81de4136a7532e966218185a190f6eb9230d`: each 4 KiB page has an exact candidate-run `[first,end)` window, empty pages return immediately, single-run pages skip the scan, and multi-run scans cannot walk into a later page
- x86-64-v3 feature detection is cached once per process as of `917a5d893087736a74566dbaf71de5f3989a6d90`
- normal chassis dispatch is bound directly to baseline or x86-64-v3 at module load as of `4e677ac8491bc3b5256998ba3688893f61d07695`, removing host-feature selection from each normal native block
- generated indirect dispatch reuses the module-load x86-64-v3 decision as of `5e6ad207c77affbf500bf5327ce6222e9e7fd7c1`, so indirect guest transfers no longer re-enter the host-feature probe
- native bursts gate exact-address host-call lookup with cached per-chunk host-call coverage as of `2776fa0a3e80136495a32552b9d909e16dcfcd5e`: verified chunks known clean skip the per-block `IsHostCallAddress()` probe, while candidate chunks retain the exact address-level check
- dispatchability carries the runtime→linked PC it already resolved as of `af7938bdb63d4530ea43a6ba445800fc4171a153`; the native burst reuses that PC for the immediately following dispatch and continuation carries the next linked PC forward, eliminating the duplicate `ResolveNativeAddress()` before dispatch while preserving linked→runtime translation afterward

DolRecomp's indexed dispatcher uses its page/run lookup rather than the older linear range chain. The current `405b81de...` implementation also bounds that lookup to the runs overlapping the current guest page and fast-paths zero/one-run pages. The linear path remains intentionally available so indexed-vs-linear can be measured on the exact same game route. The merge tool emits a 4 KiB guest-page index for the combined DOL+REL module, narrowing its binary search to chunks overlapping the current page rather than the full merged chunk table; the current `f4b7f991...` fast path avoids even that binary search on the common zero- or one-chunk page cases.

**Priority directive:** performance is now the focus. Existing SMC/hash/audit/correctness guards stay enabled, but do not spend turns expanding lockstep or correctness coverage unless a concrete failure prevents further performance measurement or execution. Correctness expansion comes later.

### Cache-control codegen

`770db1089526cae9d0fc1b48fbd65c760e75efe6` — `Keep cache-control ops in native bursts`

DolRecomp emits `dcbf`, `dcbst`, `dcbi`, and `icbi` through the dedicated `cpu->cache_control` ABI hook rather than generic instruction fallback, while retaining Dolphin invalidation semantics and cycle accounting.

Regression follow-up: `24cd9677cc260eea19101b676c5d3c2b5d142cf4`.

### Scalar FMA correctness

`7e0b48660f90e93e7333eaf4a127c3afb7b9ceb6` — `Repair generated scalar FMA helper semantics`

Coverage compares result bits, FPSCR, write/no-write behavior, rounding modes, NI/VE, FI/FR, and representative edge values.

### Floating compare correctness

The runtime preserves the fifth FPRF classification bit while replacing only FPCC. Targeted regression coverage exists.

### Local-loop lockstep alignment

The verifier aligns interpreter-shadow work with native charged work when native execution returns to an inlined loop header. Historical sensitive boundary: `0x802A1C08 -> 0x802A1C1C`.

Do not remove this as an apparent extra interpreter step without reproducing the original divergence.

### MEM2 lockstep journaling/restoration

`26334ad651567bf098401f781d1189e5f75c296c` — `Journal MEM2 in lockstep verification`

The verifier journals MEM1 and MEM2 with disjoint physical keys, captures native/interpreter MMU writes consistently, restores the shadow pre-block MEM2 image before reference execution, and restores native post-state afterward.

### Merged DOL+REL dispatch eligibility

`46091e1a2394e052f116838727471cbed974aa2d` — `Preserve merged dispatch eligibility checks`

`tools/merge_mua2_generated.py` rejects overlapping generated chunks and preserves 4-byte instruction alignment in merged binary-search dispatch.

Regression / guard commits:

- `2490b3c408441ad26c10abc4586b7e086f5af3d9` — eligibility guards
- `07fd13cca74370c8f27bb291ac30267a028de120` — corrected overlap fixture
- `67db5f86875f6e40dca4f70b65fce63996a251ad` — proves an uncovered guest-address hole between generated DOL/REL ranges remains non-native
- `b777ddbe0d2c556fc00ee632bd273d0c1ee13621` — rejects unaligned generated chunk boundaries before merged dispatch emission
- `5e7fc98e09f53c33f6c383682973af8824889d13` — regression for unaligned generated chunk rejection

The gap regression changes no native execution behavior; it pins an existing eligibility invariant so future merge-tool changes cannot silently bridge ungenerated guest code. The alignment guard closes a separate metadata hole: a malformed unaligned generated function/chunk boundary can no longer cause an aligned guest PC to enter the wrong native chunk.

### Runtime shutdown/platform teardown

Relevant accepted fixes:

- `a44dfe6df328599ed7e2bc09f980d3286750fb40` — runtime stop path
- `723ca66ccdcc22348ab0ee83c5e1fc579a567776` — platform teardown order

### Fallback / hotspot diagnostics

`aa4ee6bbcffd05ea828913ab9d8cf3e698061200` — fallback hotspot diagnostics

Optional detailed sampling: `STATICRECOMP_FALLBACK_SAMPLES=1`.

Historical profiling showed generic slow interpreter fallback was much smaller than cache-control callback traffic; use fresh measurements before choosing another optimization.

---

## 4. Performance boundary

The latest retained live native-REL benchmark **predates** direct cache-control codegen and later correctness repairs.

Recorded historical values:

- average reported FPS about `10.319`
- guest-frame FPS about `2.062`
- average speed about `0.1404`
- native dispatches `174,476,293`
- generic `fallback=0` in shutdown summary
- native exceptions `5,959`
- hook fallback `1,299,055`
- hook fast cache `1,295,291`
- hook slow `3,764`
- JIT fallback runs `11,023`

This proved native REL progression, **not acceptable performance**. No current-main speedup/regression may be claimed until a fresh proprietary-game benchmark is run.

The current source has now moved to the `O2 + indexed` default, but **no FPS/speed improvement is claimed yet** because the proprietary RMSE52 route has not been rerun from this environment. Historical profiling showed very high native dispatch counts and hot tiny runtime/cross-chunk entries, so dispatch/chassis overhead is the first optimization target.

Previously rejected/not-useful performance experiments must not be silently resurrected without new evidence: cache affinity, module IPO, MSVC chunk optimization, and multiword emission experiments recorded in history.

---

## 5. Current CI / source validation

### DolRecomp

Cross-platform DolRecomp CI passed after the cache-control regression update at `24cd9677...`.

### GXRuntime

GXRuntime CI passed on Ubuntu and Windows after portability fixes and the MEM2 change (`26334ad6...`). Relevant portability commits include portable DVD seeks (`b0aaebef...`) and savestate test handling (`ce2c426e...`).

### ModernGekko

Workflow `.github/workflows/moderngekko-ci.yml` introduced at:

`c1eeb1a2fb8d9ea31954c818fe21d9269b7edc25`

The previously pending first run is now confirmed **successful**. GitHub Actions showed all four jobs passing:

- Full build and test — Ubuntu
- Standalone tests — Ubuntu
- Full build and test — Windows
- Standalone tests — Windows

The Windows full job included MSVC environment setup and the Ninja build path. This is meaningful portability validation, but **not** a full Windows game execution test.

### Project tooling

Workflow `.github/workflows/tooling-ci.yml` runs `python -m unittest discover -s tests -v` on Ubuntu and Windows.

`67db5f86875f6e40dca4f70b65fce63996a251ad` added `tests/test_merge_mua2_dispatch_holes.py`.

The resulting tooling run completed successfully on both Ubuntu and Windows. The later tooling run for `5e7fc98e09f53c33f6c383682973af8824889d13` also completed successfully on both `ubuntu-latest` and `windows-latest`, covering the new unaligned-chunk rejection regression.

Tooling Actions run `35938476536` for `2a86452d2aafe5156f9c4015ad7b2a3ec64b65de` completed successfully on both `ubuntu-latest` and `windows-latest`. The added regression accepts the exact audited REL bytes, rejects changed REL bytes by SHA-256, and rejects a non-`LIVE_TEXT_MATCH` audit status.

Pull-request tooling Actions run `35939460805` for the replayed-text audit guard completed successfully on both `ubuntu-latest` and `windows-latest` before merge to `main` as `eb48bfa68c20ea2d9e9c8731a13cf311d72b3771`. The integration regression accepts correctly replayed text and rejects same-size wrong replay bytes, inconsistent expected/observed audit hashes, and nonzero text mismatch counts.

Performance PR #2 was merged as `a6328f40bb0c98a58c8f50e52a30d4da55390b7e`. Its direct cross-platform validation:

- OpenMUA2 tooling run `35940341273`: PASS on Ubuntu and Windows.
- DolRecomp run `35940341449`: PASS configure/build/test on Ubuntu and Windows.
- ModernGekko run `35940341332`: standalone tests PASS on Ubuntu and Windows. The two full build/test jobs were still compiling when this handoff was updated; do not record them as passed unless a later turn observes completion.

Performance PR #3 / commit `bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc`:

- OpenMUA2 tooling run `35941158719`: PASS on Ubuntu.
- OpenMUA2 tooling run `35941158719`: PASS on Windows.
- The merged-dispatch tests cover the page-indexed path, uncovered-page gaps, instruction alignment, overlap rejection, and existing native-eligibility holes.

Performance PR #9 / merge commit `405b81de4136a7532e966218185a190f6eb9230d`:

- DolRecomp Actions run `36000036463`: PASS configure/build/test on Ubuntu and Windows.
- The Ubuntu generated `c_execute` test compiled and executed an indexed case reporting 11 chunks in 6 runs across 2 pages, with up to 5 runs on a page; all 19 DolRecomp tests passed.
- ModernGekko Actions run `36000035131`: standalone and full build/test jobs PASS on Ubuntu and Windows.

Performance PR #13 / merge commit `2776fa0a3e80136495a32552b9d909e16dcfcd5e`:

- OpenMUA2 tooling Actions run `36003571111`: PASS on Ubuntu and Windows.
- ModernGekko Actions run `36003570930`: standalone tests PASS on Ubuntu and Windows.
- ModernGekko Actions run `36003570930`: full build/test PASS on Ubuntu and Windows, including the Windows MSVC/Ninja path that compiles the modified `StaticRecompCore::Run()` code.
- The change reuses dispatchability's verified chunk index and consults cached `ChunkContainsHostCall()` coverage before exact `IsHostCallAddress()` lookup; exact host-call behavior remains for chunks that may contain a host call.

Performance PR #14 / merge commit `af7938bdb63d4530ea43a6ba445800fc4171a153`:

- OpenMUA2 tooling Actions run `36008961778`: PASS on Ubuntu and Windows.
- ModernGekko Actions run `36008961830`: standalone tests PASS on Ubuntu and Windows.
- ModernGekko Actions run `36008961830`: full build/test PASS on Ubuntu and Windows, including the Windows MSVC/Ninja path compiling the modified StaticRecomp runtime.
- Dispatchability/continuation now return the already-resolved linked PC, so the immediately following native dispatch no longer performs the same runtime→linked lookup again; post-dispatch linked→runtime translation remains intact.

### What is still not freshly validated

Do not claim from current `main` without a real run:

- complete boot through title/menu
- opening gameplay/combat correctness
- a complete level
- long-session stability
- multiplayer
- audible/accurate audio
- physical-controller gameplay correctness
- game-owned save/reload across process restart
- full Windows game build and execution
- current-main performance after direct cache-control codegen
- a fresh large lockstep total equivalent to historical `11,832`
- overall crash-free/glitch-free/full completion

---

## 6. Exact blocker and next work

The immediate measurement blocker is **fresh proprietary RMSE52 game-side execution from current `main`**. GitHub/CI intentionally does not contain the game image, so this environment can improve and cross-platform-test performance code but cannot claim game FPS gains.

Performance-first sequence:

1. Build current `main` with the default `O2 + indexed` settings against the exact USA RMSE52 image.
2. Benchmark the same route on Windows and Linux using identical warmup/sample windows and equivalent graphics/audio settings.
3. A/B `--dispatch-lookup indexed` and `--dispatch-lookup linear` with all other settings held constant.
4. On the faster baseline, collect native dispatch count/hot PCs, burst length, host-call check cost, REL translation cost, native exceptions, and JIT fallback.
5. Optimize shared dispatch/chassis and generated cross-chunk transfer paths that benefit MSVC and GCC/Clang together.
6. Rebuild and re-measure on both platforms after each accepted performance change.
7. Defer additional lockstep/correctness expansion until performance reaches a useful plateau or a concrete failure blocks measurement. Existing guards remain enabled.

When proprietary game-side execution is unavailable, continue **performance-oriented** source work, microbenchmarks, generated-code improvements, profiling instrumentation, and Windows/Linux CI. Do not substitute new correctness projects for the performance priority.

---

## 7. Windows-awareness requirements

- Prefer portable C/C++/CMake/Python constructs in shared source.
- Avoid unnecessary hard-coded POSIX process/filesystem APIs.
- Preserve MSVC compatibility.
- Keep path handling tolerant of Windows separators and drive-letter paths.
- Maintain Windows CI where feasible.
- Passing shared-library/tests is not the same as a successful full Windows game build.

Root Windows entry points include `OpenMUA2.cmd`, `Setup.cmd`, `Build.cmd`, `Run.cmd`, `Snapshot.cmd`, and `Backup.cmd`.

---

## 8. Source discipline

Before editing, inspect current GitHub `main` and any writable checkout status. Never reset/clean/discard unknown user work.

Tracked source must not contain the WBFS, extracted original game files, generated proprietary translation output, raw RAM captures, user saves, or local runtime logs.

`FILE-MANIFEST.json` records the delivered baseline rather than current editable source; do not regenerate it merely to hide intentional changes.

Useful deeper documents:

- `docs/CURRENT-STATUS.md`
- `docs/WORKLOG-2026-09-23.md`
- `docs/RECOVERY-PLAN.md`
- `README.md`
- `AGENTS.md`
- `locks/SOURCE_SNAPSHOT.json`
- `locks/SOURCE_TRANSFORMATIONS.json`
- `patches/LOCAL01-recovered-production.patch`

---

## 9. Last turn update — 2026-09-24

Latest `main` actually inspected before this handoff edit:

- `8e2baf16efb0022b8910ddefa0130843c1b7da0b` — `Record REL dispatch resolution reuse`

User priority / workflow mandates:

- **Multiplatform performance remains the active project focus. Correctness expansion comes later.**
- Existing correctness/SMC/hash/audit guards remain enabled; do not drift into new correctness work unless a concrete failure blocks performance measurement or execution.
- **Mandatory handoff rule:** every development turn must end with this file updated, committed to GitHub `main`, and posted/attached in chat. The turn is not complete until the refreshed file is posted.

Changes made this turn:

- Inspected current GitHub `main` first; it was stable at `add931c83dec88567dc140b4861e7f37aed616a8` with no parallel work to reconcile.
- Targeted the next performance hotspot identified by the prior handoff: duplicate REL runtime→linked resolution inside the hot native burst loop.
- Created branch `perf/reuse-rel-dispatch-resolution` and PR #14 (`Reuse REL resolution across native dispatch`).
- Updated:
  - `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore.h`
  - `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore_SMC.cpp`
  - `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/StaticRecompCore_Run.cpp`
  - `tests/test_staticrecomp_host_call_gate_perf.py`
  - added `tests/test_staticrecomp_rel_dispatch_reuse_perf.py`
- `ChunkIndexOf`, `DispatchableAt`, and `FastDispatchableAt` can now return the linked PC produced by the authoritative `ResolveNativeAddress()` lookup they already perform.
- Native-burst entry reuses that linked PC for the immediately following module dispatch rather than calling `ResolveNativeAddress()` again.
- Burst continuation similarly resolves/validates the next runtime PC once, returns its linked PC, and carries it into the next dispatch iteration.
- The post-dispatch `TranslateRelAddress()` path remains intact. This optimization does **not** keep a REL mapping decision beyond the dispatchability/continuation check that immediately precedes the dispatch.
- Existing chunk verification, forced-fallback handling, REL refresh behavior, and host-call gating remain enabled.
- PR #14 was squash-merged as `af7938bdb63d4530ea43a6ba445800fc4171a153`.
- Updated `docs/CURRENT-STATUS.md` in `8e2baf16efb0022b8910ddefa0130843c1b7da0b`.
- No proprietary RMSE52 game data or game-derived output was committed.

Tests/CI actually observed:

- OpenMUA2 tooling Actions run `36008961778`: PASS on `ubuntu-latest` and `windows-latest`.
- ModernGekko Actions run `36008961830`:
  - Standalone tests — Ubuntu: PASS.
  - Standalone tests — Windows: PASS.
  - Full build and test — Ubuntu: PASS.
  - Full build and test — Windows: PASS.
- The Windows full job used the MSVC environment + Ninja path and compiled the changed StaticRecomp core as part of the full runtime.
- The new source regression specifically rejects reintroduction of `ResolveNativeAddress(runtime_dispatch_address, &linked_dispatch_address, nullptr)` in the dispatch body while requiring the linked PC to be carried from dispatchability and continuation.
- No RMSE52 game-side build, gameplay run, FPS measurement, or benchmark occurred in this environment.

Windows/Linux portability:

- PR #14 is shared C++ only; it adds no platform-specific assembly and no POSIX-only runtime dependency.
- Tooling regressions pass on Windows and Ubuntu.
- Full ModernGekko integration build/test passes on Windows/MSVC and Ubuntu.
- This is source/build portability validation, **not** a full Windows game execution result.

Failures/rejected experiments:

- No code/test failure was observed for PR #14.
- GitHub's live log-blob endpoint returned a temporary 404 while full jobs were in progress; the jobs themselves remained healthy and later passed.
- Previously rejected performance experiments (cache affinity, module IPO, MSVC chunk optimization, multiword emission) remain rejected absent fresh evidence.

Current blocker:

- Fresh proprietary RMSE52 game-side execution remains unavailable in this environment, so no FPS/speed improvement is claimed for `af7938bd...` or the other recent dispatch optimizations.
- The native-dispatch hot path now avoids duplicate runtime→linked resolution between eligibility and dispatch. Remaining REL work is primarily post-dispatch linked→runtime/cross-section translation plus any repeated lookup associated with cross-chunk/indirect transfers.

Next exact step:

1. Inspect current `main` first because parallel work may have advanced it.
2. Continue **performance work**, not correctness expansion.
3. Investigate the post-dispatch linked→runtime path in `StaticRecompCore_SMC.cpp` / `StaticRecompCore_Run.cpp`. A promising direction is to fast-path the common case where the dispatched REL block returns to the same active REL section, while retaining full section lookup for cross-section/DOL transfers and retaining REL refresh correctness.
4. Keep Windows and Ubuntu full-build validation for any change touching the StaticRecomp runtime loop.
5. When the proprietary RMSE52 workspace is available, benchmark current `O2 + indexed` against `O2 + linear` on the standard route and collect native-dispatch count/hot PCs, burst length, host-call checks, REL translation cost, native exceptions, and JIT fallback before claiming any speedup.
