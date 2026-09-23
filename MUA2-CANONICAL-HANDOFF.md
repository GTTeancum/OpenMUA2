# MUA2 CANONICAL HANDOFF — Wii Marvel: Ultimate Alliance 2 native PC recompilation

**Project:** Wii Marvel: Ultimate Alliance 2 (USA, RMSE52) native-PC recompilation  
**Repository:** `GTTeancum/OpenMUA2`  
**Canonical branch:** `main`  
**Source state summarized by this handoff:** `07fd13cca74370c8f27bb291ac30267a028de120` (`Fix merged overlap regression fixture`)  
**Date:** 2026-09-23

> ## MANDATORY END-OF-TURN UPDATE RULE
>
> **At the end of EVERY future development turn, update this file before giving the user the final response.**
>
> The update must record:
> 1. the latest `main` commit SHA actually inspected,
> 2. every code/document/test change made that turn,
> 3. every test/build/CI/game run actually performed and its real result,
> 4. any failed or rejected experiment,
> 5. the exact current blocker or next technically justified step,
> 6. any change to Windows portability status,
> 7. any new file/location a successor must know about.
>
> Do not let this handoff drift behind the repository. If code changes, update this file in the same turn. If no code changes but the blocker/validation status changes, update it anyway.
>
> **A new chat should need only this file plus access to `GTTeancum/OpenMUA2`.** Start by reading this file, then inspect the latest `main` before acting because `main` may have advanced after the SHA recorded above.

---

## 1. Mission and non-negotiable rules

The goal is a **real native PC recompilation/port** of the Wii USA build of Marvel: Ultimate Alliance 2, not a Dolphin wrapper and not a disguised emulator port.

Development policy:

- Use GitHub `main` as the source of truth.
- Continue in small, conservative, technically justified increments.
- Linux is the active development path.
- Keep shared code/build tooling portable and avoid unnecessary POSIX-only assumptions so Windows remains practical.
- Do not replace native recompilation with emulation.
- Do not weaken chunk hashes, SMC checks, REL eligibility validation, verifier checks, or game timing merely to advance farther.
- Do not fabricate boot/gameplay/save/audio/controller/performance results.
- Preserve failures as failures and reduce them to focused regressions.
- The original extracted `sys/main.dol` remains the real boot DOL. Generated/merged DOL+REL code is code-generation input only and must never replace the original boot DOL.
- Proprietary WBFS, extracted game data, generated game-derived translation, logs, saves, screenshots, RAM captures, and other local outputs belong outside tracked Git source.

---

## 2. Where everything lives

### Canonical source repository

`GTTeancum/OpenMUA2`

The intended Windows checkout from the original handoff is:

`D:\Programming\GitHub\OpenMUA2\`

Linux may use any normal clone path. Always inspect the current GitHub `main` before editing.

### Main source areas

| Purpose | Path |
|---|---|
| DolRecomp source | `project/lib/DolRecomp/` |
| Second pinned DolRecomp copy used by ModernGekko/Dolphin | `project/lib/ModernGekko/vendor/dolphin/DolRecomp/` |
| ModernGekko | `project/lib/ModernGekko/` |
| GXRuntime | `project/lib/ModernGekko/vendor/dolphin/GXRuntime/` |
| Static native recomp CPU/runtime integration | `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/` |
| Dolphin reference interpreter FP behavior | `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/Interpreter/` |
| Native module template/export | `project/lib/ModernGekko/vendor/dolphin/module-template/` |
| Project Python/build/merge tooling | `tools/` |
| Project Python regression tests | `tests/` |
| CI workflows | `.github/workflows/` |
| Current status | `docs/CURRENT-STATUS.md` |
| Detailed September 23 work log | `docs/WORKLOG-2026-09-23.md` |
| Recovery/development plan | `docs/RECOVERY-PLAN.md` |
| Source transformation records | `locks/` |
| Recovered historical patches | `patches/` |
| Fresh/local validation evidence | `evidence/` where tracked; local runtime receipts remain under `.local/` |

### Local-only game/build/test data

The original USA RMSE52 `.wbfs` is expected at the project root in the user's local Windows workspace. It must **never** be committed or modified.

Generated/extracted/test output belongs under:

- `.local/game/`
- `.local/logs/`
- `.local/benchmarks/`
- `.local/automation/`
- `.local/user/`
- other `.local/` build/generated subdirectories

Source backups may live under `.backups/`.

---

## 3. Current project state

The repository originated from a recovered **MG01 + FPC01** source baseline. Missing later work was not reconstructed from prose and falsely presented as recovered bytes; it has instead been reimplemented and tested incrementally in current source.

### Native DOL and REL execution

Native DOL packaging/execution is present.

Native REL packaging and runtime eligibility are also present. The RMSE52 live REL used an **absolute linked section-table pointer**, not merely the REL-relative `0x4C` form. Commit:

`67d75dc63455b47e34159fefae4ed39fca5e5efc` — `Support absolute REL section tables`

`StaticRecompCore::RefreshRelSections()` now resolves:

- normal relative section table locations,
- absolute guest virtual pointers,
- physical guest pointers,

with guest-RAM bounds checks.

Observed live RMSE52 REL information from the retained diagnostics:

- module ID: `1`
- REL version: `3`
- observed module/header area: approximately `0x80E4A080`
- observed linked text start: `0x80E4A164`
- native covered REL text: `3,255,744` bytes in the successful audit
- one live REL module was discovered in the accepted build

The post-fix run advanced real game frames rather than freezing at the former native REL blocker.

### Hash/SMC protection

Native code remains under chunk-hash/SMC verification. Different or modified guest code is not made native simply to advance the game.

Do not disable or relax this.

### Cache-control code generation

Originally, very high-frequency `dcbf`, `dcbst`, `dcbi`, and `icbi` instructions were reaching the generic `instruction_fallback` hook. Profiling showed roughly 1.3–1.4 million fast cache-control fallback callbacks in the benchmark path.

Commit:

`770db1089526cae9d0fc1b48fbd65c760e75efe6` — `Keep cache-control ops in native bursts`

DolRecomp now emits these through the existing dedicated `cpu->cache_control` ABI hook, preserving Dolphin's cache invalidation behavior and cycle accounting while avoiding the generic fallback path.

Regression follow-up:

`24cd9677cc260eea19101b676c5d3c2b5d142cf4` — `Update cache-control codegen regression`

### Scalar FMA correctness

Commit:

`7e0b48660f90e93e7333eaf4a127c3afb7b9ceb6` — `Repair generated scalar FMA helper semantics`

The generated scalar FMA helper was repaired to use the instruction-shaped arithmetic/rounding behavior expected by the runtime/reference path.

Regression coverage compares:

- result bits,
- FPSCR state,
- write/no-write behavior,
- rounding modes,
- NI/VE behavior,
- FI/FR behavior,
- representative edge values.

Do not assume older FMA behavior is correct merely because gameplay boots.

### Floating compare correctness

Current runtime code preserves the fifth FPRF classification bit while replacing only the FPCC portion. There is targeted regression coverage for this path.

### Inlined/local loop verifier alignment

Current lockstep verification includes the reconstructed local-loop alignment behavior. When native execution returns to an inlined loop header, the interpreter shadow is allowed to execute enough work to match the native charged work before comparison, rather than comparing different iterations that happen to share the same PC.

This is important around the historically observed loop boundary near:

`0x802A1C08 -> 0x802A1C1C`

Do not remove this as an apparent "extra interpreter step" without reproducing the original divergence.

### MEM2 lockstep journaling/restoration

Commit:

`26334ad651567bf098401f781d1189e5f75c296c` — `Journal MEM2 in lockstep verification`

This was a major correctness repair.

The verifier now:

- journals MEM1 and MEM2 using non-overlapping physical keys,
- captures native and interpreter MMU writes consistently,
- restores the shadow's pre-block MEM2 state before reference execution,
- restores the native post-state after shadow comparison/cleanup.

This prevents shadow-reference execution from corrupting or comparing against the wrong MEM2 image.

### Merged DOL+REL dispatch eligibility

The generated merged dispatch table must not accidentally broaden what is considered valid native code.

Commit:

`46091e1a2394e052f116838727471cbed974aa2d` — `Preserve merged dispatch eligibility checks`

`tools/merge_mua2_generated.py` now:

- rejects invalid zero/negative code chunks,
- rejects overlapping generated chunks,
- preserves 4-byte instruction alignment before merged binary-search dispatch.

Regression:

`2490b3c408441ad26c10abc4586b7e086f5af3d9` — `Test merged dispatch eligibility guards`

Fixture correction:

`07fd13cca74370c8f27bb291ac30267a028de120` — `Fix merged overlap regression fixture`

The first overlap-test fixture was incorrect and caused tooling CI failure. The fixture was corrected so it actually overlaps the generated chunk range. This was a test-fixture failure, not justification to remove the production overlap guard.

### Runtime shutdown/platform teardown

Earlier native REL work exposed shutdown/teardown issues. Relevant commits:

- `a44dfe6df328599ed7e2bc09f980d3286750fb40` — `Trace and fix runtime stop path`
- `723ca66ccdcc22348ab0ee83c5e1fc579a567776` — `Fix runtime platform teardown order`

Keep this behavior in mind if a later Windows or Linux shutdown crash reappears.

### Fallback/hotspot diagnostics

Commit:

`aa4ee6bbcffd05ea828913ab9d8cf3e698061200` — `Add fallback hotspot diagnostics`

Optional detailed sampling is enabled by:

`STATICRECOMP_FALLBACK_SAMPLES=1`

Normal builds keep cheaper aggregate counters.

The profiling established that generic slow interpreter fallback was much smaller than cache-control callback traffic; performance work should therefore use fresh measurements instead of assuming all fallback is equally expensive.

### Input/controller state

The repository has an Xbox/Wii controller profile path and diagnostic tooling.

At the recorded XInput/SDL diagnostic:

- SDL enumerated `Xbox One Controller`
- sticks/triggers/A/B/X/Y/LB/RB/Start/Back/D-pad/stick clicks/Guide were exposed
- rumble was requested
- physical transitions were not observed during that sample window
- Guide/Home remained intentionally unmapped to Wii Close Game

Relevant earlier commit:

`9252dc7b79bf378a0db0bc05b07e424e437bddf8` — `Map Xbox B as Wii B alternate`

Do not claim physical gameplay controls are fully validated until a hands-on run records transitions.

---

## 4. Performance history and what it does / does not prove

### Pre-direct-cache-control native REL benchmark

The latest recorded live benchmark cited in current status **predates** the direct cache-control codegen change.

Recorded values from the fixed absolute-REL build:

- average reported FPS: about `10.319`
- guest-frame FPS: about `2.062`
- average speed: about `0.1404`
- native dispatches: `174,476,293`
- generic `fallback=0` in that shutdown summary
- native exceptions: `5,959`
- hook fallback: `1,299,055`
- hook fast cache: `1,295,291`
- hook slow: `3,764`
- JIT fallback runs: `11,023`
- verified chunks/REL were active and game frames advanced

This proved native REL progression, **not acceptable performance**.

Because cache-control codegen, FMA, MEM2, and later dispatch-guard changes happened afterward, do not claim a speedup or regression until a new game-side benchmark is run.

### Rejected experiments

Several performance experiments were explicitly recorded as rejected or not useful. Do not silently resurrect them as accepted improvements without new evidence:

- `621879e9...` — rejected cache-affinity benchmark
- `ab94726c...` — rejected module IPO benchmark
- `962dfbc0...` — rejected MSVC chunk optimization
- `f01a51c1...` — rejected multiword-emission experiment

There were also targeted scheduler/REL-discovery improvements, including:

- `4072d19a...` — scheduler idle performance
- `07915170...` — avoid REL discovery during cache invalidation
- `afbfff39...` — avoid REL discovery for physical fallback PCs
- `b5f4cae7...` — fallback JIT diagnostics for native REL

Treat the current tree, not historical speculation, as authoritative.

---

## 5. Current validation status

### Confirmed successful CI / build validation

#### DolRecomp

Cross-platform workflow:

`.github/workflows/dolrecomp-ci.yml`

Commands:

```text
cmake -S project/lib/DolRecomp -B build/dolrecomp -DDOLRECOMP_ENABLE_LLVM=OFF
cmake --build build/dolrecomp --config Release --parallel 2
ctest --test-dir build/dolrecomp -C Release --output-on-failure
```

The initial workflow-add commit failed, then the cache-control regression follow-up passed on both supported CI OSes.

Latest known successful DolRecomp workflow:
`24cd9677...` — success.

#### GXRuntime

Workflow:

`.github/workflows/gxruntime-ci.yml`

Commands:

```text
cmake \
  -S project/lib/ModernGekko/vendor/dolphin/GXRuntime \
  -B build/gxruntime \
  -DCMAKE_BUILD_TYPE=Release \
  -DGXRUNTIME_ENABLE_AURORA=OFF \
  -DGXRUNTIME_ENABLE_AURORA_RECOMP=OFF

cmake --build build/gxruntime --config Release --parallel 2
ctest --test-dir build/gxruntime -C Release --output-on-failure
```

After portability fixes to DVD seeks and savestate test handling, the MEM2 change passed the normal GXRuntime matrix on both Ubuntu and Windows.

Latest known successful GXRuntime workflow:
`26334ad6...` — success.

Relevant portability commits:

- `b0aaebef...` — `Make GXRuntime DVD seeks portable`
- `ce2c426e...` — `Make GXRuntime savestate test portable`

#### ModernGekko

Workflow:

`.github/workflows/moderngekko-ci.yml`

It contains:

- standalone targeted tests on Ubuntu/Windows
- full configure/build/test jobs on Ubuntu/Windows
- MSVC environment setup on Windows
- Ninja full-build path
- selected long/fuzzer tests excluded from the normal CI pass

Commit:
`c1eeb1a2...` — `Add cross-platform ModernGekko CI`

**At the moment this handoff was written, that first ModernGekko CI run was still in progress.**
A successor must check the final GitHub Actions result before calling it passed.

#### Python/project tooling

Workflow:

`.github/workflows/tooling-ci.yml`

Command:

```text
python -m unittest discover -s tests -v
```

Runs on both Ubuntu and Windows.

The first run at `3c4e5b40...` failed because the new overlap-regression fixture did not actually overlap the generated range. Commit `07fd13cc...` corrected the fixture.

**The corrected tooling CI run at `07fd13cc...` completed successfully.**

### Focused Linux validation already performed for MEM2

Before MEM2 integration, the focused validation built/tested:

- GXRuntime
- ModernGekko Linux runtime

and completed successfully.

This was source/runtime validation; it was not a proprietary RMSE52 gameplay test.

---

## 6. What is NOT currently freshly validated

Do not claim any of the following from current `main` unless a new run has actually been performed and recorded:

- complete game boot through title/menu from the latest source
- full opening gameplay/combat correctness
- a complete level
- long-session stability
- multiplayer
- audible/accurate audio
- physical-controller gameplay correctness
- game-owned save/reload across a process restart
- full Windows game build and execution
- current-main performance after direct cache-control codegen
- a fresh large lockstep comparison total equivalent to the historical `11,832`
- overall "crash-free", "glitch-free", or "fully native" completion

Historical MR01 reports remain reproduction targets, not current validation.

---

## 7. Exact next blocker / next work

The next meaningful blocker is **fresh proprietary-game-side measurement on current `main`**.

A successor should:

1. Inspect the latest `main` and check the final results of the in-progress ModernGekko/tooling CI runs noted above.
2. Build the current source against the exact USA RMSE52 image in the local game workspace.
3. Verify the native module audit still accepts the expected DOL + REL coverage with chunk hashes/SMC enabled.
4. Run a fresh baseline benchmark through the same boot/post-ready route.
5. Compare the new numbers against the pre-cache-control benchmark rather than assuming improvement.
6. Run a bounded lockstep gameplay window exercising the reconstructed correctness fixes together:
   - native REL mapping,
   - cache-control direct hooks,
   - scalar FMA behavior,
   - floating compare,
   - local-loop alignment,
   - MEM2 journaling/restoration.
7. If lockstep diverges, preserve the failure, capture the first useful divergence, and reduce it to a targeted regression before proceeding.
8. Only after current correctness is re-established should another performance optimization be accepted.
9. Continue Linux-first, but run/maintain cross-platform CI and avoid introducing unnecessary POSIX-only APIs.

If proprietary game-side execution is unavailable in the current environment, do not invent results. Continue with source-level regressions/CI/portability work and record that game-side validation remains blocked on access to the local RMSE52 workspace.

---

## 8. Windows-awareness requirements

Windows is a later target but must remain practical now.

Keep these constraints:

- shared runtime/source should prefer portable C/C++/CMake/Python constructs,
- avoid hard-coded POSIX process/filesystem APIs when standard-library or existing abstraction paths exist,
- preserve MSVC compatibility,
- keep path handling tolerant of Windows separators and drive-letter paths,
- CI should continue covering Windows where feasible,
- a passing shared-library/unit-test matrix is **not** the same as a successful full Windows game build,
- do not declare Windows game support until ModernGekko + generated module + game execution are all tested there.

The intended original Windows workspace path remains:

`D:\Programming\GitHub\OpenMUA2\`

Root Windows entry points include:

- `OpenMUA2.cmd`
- `Setup.cmd`
- `Build.cmd`
- `Run.cmd`
- `Snapshot.cmd`
- `Backup.cmd`

These should remain usable even though active development is currently Linux-first.

---

## 9. Git/source discipline

Before editing:

- inspect latest GitHub `main`,
- inspect working-tree status in any writable checkout,
- do not reset/clean/discard unknown user changes,
- make focused changes,
- add a regression for correctness changes whenever practical,
- keep source/history separate from local proprietary outputs.

Tracked source should never contain:

- the WBFS,
- extracted original game files,
- generated proprietary translation output,
- raw RAM captures,
- user saves,
- local runtime logs,
- local screenshots unless explicitly sanitized/approved.

---

## 10. Important documents to read if more detail is needed

The new chat should start with **this file**, but deeper source context is in:

- `docs/CURRENT-STATUS.md`
- `docs/WORKLOG-2026-09-23.md`
- `docs/RECOVERY-PLAN.md`
- `README.md`
- `AGENTS.md`
- `locks/SOURCE_SNAPSHOT.json`
- `locks/SOURCE_TRANSFORMATIONS.json`
- `patches/LOCAL01-recovered-production.patch`

`docs/CURRENT-STATUS.md` was refreshed at commit:

`e7cfc48953f469f578b8738a4a74a4854e55d352`

It is accurate through the main reconstructed correctness work, but this canonical handoff also includes the later ModernGekko CI / merged-dispatch / tooling-test commits.

---

## 11. Condensed chronology of the current GitHub work

Key progression, newest direction last:

1. Recovered MG01 + FPC01 source workspace established.
2. Native DOL generation/build/audit retained.
3. Native REL packaging/integration reconstructed.
4. Fallback/JIT/native performance diagnostics added.
5. Scheduler/REL-discovery hot paths improved conservatively.
6. Runtime shutdown and platform teardown repaired.
7. RMSE52 absolute REL section-table pointer support fixed native REL discovery.
8. Native REL advanced real frames; performance still poor.
9. XInput/SDL mapping diagnostic recorded.
10. Cache-control ops moved from generic instruction fallback to dedicated native hook.
11. Cross-platform DolRecomp CI added; regression updated until passing.
12. Scalar FMA helper semantics repaired and tested.
13. GXRuntime Windows portability fixes made for DVD seek and savestate tests.
14. MEM2 lockstep journaling/restoration repaired; Linux focused validation passed; GXRuntime Ubuntu/Windows CI passed.
15. Current reconstructed status document refreshed.
16. Cross-platform ModernGekko CI added.
17. Merged generated dispatch regained instruction-alignment and overlap eligibility guards.
18. Regression tests added for those merged dispatch guards.
19. Cross-platform Python tooling CI added.
20. The first overlap fixture was found faulty; `07fd13cc...` corrected the fixture.
21. Corrected tooling CI at `07fd13cc...` completed successfully; the initial ModernGekko CI was still running at handoff close and must be rechecked.

---

## 12. New-chat startup instructions

When this file is handed to a new ChatGPT chat, the first development response should:

1. Treat `GTTeancum/OpenMUA2` GitHub `main` as authoritative.
2. Read this entire file.
3. Fetch/inspect the latest `main` SHA and recent commits, because the repository may have advanced since `07fd13cc...`.
4. Check GitHub Actions status, especially any runs that were pending when this file was last updated.
5. Read only the deeper source/docs needed for the next task.
6. Continue from the exact blocker in section 7.
7. Keep turns reasonably small/conservative to reduce chat/context failures.
8. **Before ending the turn, update this file with the new state.**

No reconstruction of the prior chat should be necessary beyond this file and the repository.

---

## 13. End-of-turn update template

Append or revise this section every turn:

```text
### Last turn update — YYYY-MM-DD HH:MM local

Latest main inspected:
- <full SHA> — <commit title>

Changes made:
- <file / behavior / commit>

Tests actually run:
- <command/workflow>
- Result: PASS / FAIL / PENDING
- Do not write PASS unless the result was actually observed.

Game-side validation:
- <exact run performed, or "not run">

Windows portability:
- <what was tested or changed>

Failures/rejected experiments:
- <real failures; never omit them>

Current blocker:
- <one concrete blocker>

Next exact step:
- <one technically justified step>
```

**This update is mandatory at the end of every future turn.**
