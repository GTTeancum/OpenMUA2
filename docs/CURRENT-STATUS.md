# Current status — GitHub main

September 23, 2026. This file tracks the current reconstructed source on `main`; the older LOCAL01 recovery boundary is no longer an accurate description of the checked-in implementation.

## Current reconstructed source

The repository still originates from the recoverable MG01 + FPC01 workspace, but the following missing pieces have now been reimplemented from the retained source/evidence and committed as new work rather than presented as recovered MR01 bytes:

- Native DOL + REL packaging and runtime eligibility are present. RMSE52's absolute linked REL section-table pointer is accepted with guest-RAM bounds checks, and native REL code remains protected by the existing chunk-hash / SMC verification path. Different or modified code is not made native merely to advance boot.
- REL metadata generation now binds the tracked live audit to the exact REL bytes it validated. Commit `2a86452d2aafe5156f9c4015ad7b2a3ec64b65de` requires audit status `LIVE_TEXT_MATCH` and an exact SHA-256 match against `source_rel_sha256` before replaying relocations or emitting native REL metadata, so a stale audit cannot silently authorize changed REL bytes.
- REL relocation replay is now bound to the retained live-text audit result as well. Commit `eb48bfa68c20ea2d9e9c8731a13cf311d72b3771` requires the zero-adjust comparison to be an exact match, requires well-formed and equal expected/observed text SHA-256 values, and verifies the replayed executable text bytes against that hash before native REL metadata is emitted. Same-size but incorrectly replayed text is therefore rejected.
- DolRecomp emits `dcbf`, `dcbst`, `dcbi` and `icbi` through the dedicated cache-control hook instead of the generic instruction-fallback callback. The Dolphin cache invalidation semantics and generated cycle accounting remain intact.
- The generated scalar FMA helper has been repaired to use the same instruction-shaped arithmetic/rounding path as the runtime floating-point implementation. Targeted regression coverage compares result bits, FPSCR state and write/no-write behavior across rounding/NI/VE/FI/FR cases and edge values.
- The lockstep checker contains local-loop boundary alignment: when native execution yields back at an inlined loop header, the interpreter shadow continues until it has performed the native charged work rather than comparing different loop iterations at the same PC.
- The runtime floating compare path preserves the fifth FPRF classification bit while replacing only FPCC, with a targeted runtime regression.
- MEM2 is now included in lockstep memory journaling/restoration. MEM1 and MEM2 use disjoint physical journal keys; native and interpreter MMU writes feed the same diagnostic journal, the shadow receives the pre-block image, and the native post-image is restored afterward.
- Merged DOL+REL dispatch keeps native eligibility bounded to generated code chunks. Alignment and overlap guards are covered, and commit `67db5f86875f6e40dca4f70b65fce63996a251ad` adds a regression proving an uncovered guest-address hole between generated ranges is not bridged into native eligibility. Commit `b777ddbe0d2c556fc00ee632bd273d0c1ee13621` additionally rejects unaligned generated chunk boundaries before emitting the merged table, and `5e7fc98e09f53c33f6c383682973af8824889d13` pins that PowerPC instruction-alignment invariant with a regression.
- Multiplatform performance is now the active development priority. Commit `a6328f40bb0c98a58c8f50e52a30d4da55390b7e` makes DolRecomp's existing page-indexed native dispatch the default and changes the OpenMUA2 workspace native-module optimization default from O0 to O2 on both Windows and Linux. `--dispatch-lookup linear` remains available for controlled A/B comparison, and generation cache keys/receipts record the dispatch mode so measurements cannot accidentally reuse output from the other mode.

## Active development priority — multiplatform performance

Performance on both Windows and Linux is the primary focus. Existing correctness, SMC, audit, and eligibility guards remain enabled, but expanding lockstep/correctness coverage is deferred unless a performance change produces a concrete failure that blocks measurement or execution.

Current performance baseline on `main`:

- native module optimization default: `O2`
- generated dispatch lookup default: `indexed`
- explicit A/B control: `--dispatch-lookup indexed|linear`
- build receipts record `module_opt` and `dispatch_lookup`
- DOL and REL generation cache identities include the dispatch mode
- combined DOL+REL dispatch now uses a 4 KiB guest-page index to narrow each lookup before binary search (`bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc`), with empty/single-chunk page fast paths from `f4b7f991deb4a9e787b97c3bac2a475faadfc1f6`
- ordinary DolRecomp indexed dispatch now emits an exact page-local run window; empty pages return immediately, single-run pages skip the scan, and multi-run scans cannot walk beyond the current 4 KiB page (`405b81de4136a7532e966218185a190f6eb9230d`)
- x86-64-v3 capability probing is cached (`917a5d893087736a74566dbaf71de5f3989a6d90`), normal chassis dispatch is bound to the selected baseline/v3 function at module load (`4e677ac8491bc3b5256998ba3688893f61d07695`), and generated indirect dispatch reuses the same module-load choice (`5e6ad207c77affbf500bf5327ce6222e9e7fd7c1`)
- native burst host-call checks now reuse the verified chunk index and consult cached per-chunk host-call coverage before invoking exact-address lookup (`2776fa0a3e80136495a32552b9d909e16dcfcd5e`); chunks known clean skip the per-block host-call probe while candidate chunks retain the exact check
- native dispatchability now returns the runtime→linked PC it already resolved and the burst loop reuses that value for the immediately following dispatch (`af7938bdb63d4530ea43a6ba445800fc4171a153`); continuation eligibility similarly carries the next linked PC, removing the duplicate `ResolveNativeAddress()` immediately before every native dispatch while preserving linked→runtime translation afterward
- post-dispatch REL return translation now carries the active REL section index and fast-paths returns that remain in the same section (`7b7e412f671039e1d29e2cdff7b2e51509bc046e`); cross-section and DOL returns retain the existing full `ResolveRuntimeAddress()` scan
- no current-main game-side speedup is claimed until the proprietary RMSE52 route is measured

Historical profiling showed very high native-dispatch counts and concentrated time in tiny runtime/cross-chunk entries. The accepted dispatch work now reduces lookup cost in both the ordinary DolRecomp path and the merged native DOL+REL path, removes repeated host-feature selection from normal/indirect dispatch, skips host-call address probes in chunks already known clean, reuses runtime→linked REL resolution across eligibility and dispatch, and fast-paths same-section linked→runtime returns. The next optimization work should attack the continuation-side runtime→linked section scan and remaining cross-section/cross-chunk transfer overhead before revisiting lower-volume correctness work.

## Fresh validation of current work

For the MEM2 repair, a focused Linux validation workflow first applied only the six intended source-file changes, configured/built GXRuntime, ran its tests, configured/built the ModernGekko Linux runtime and ran the ModernGekko tests. Every step completed successfully before the source repair was integrated into `main` as commit `26334ad651567bf098401f781d1189e5f75c296c` (`Journal MEM2 in lockstep verification`).

After that integration, the normal GXRuntime matrix completed successfully on both `ubuntu-latest` and `windows-latest`. This is useful Windows portability coverage for the shared runtime change, but it is **not** a full Windows game execution test.

The cross-platform ModernGekko workflow introduced by `c1eeb1a2fb8d9ea31954c818fe21d9269b7edc25` completed successfully. Its standalone tests and full configure/build/test jobs all passed on both Ubuntu and Windows, including the MSVC/Ninja Windows path. This remains source/runtime validation rather than proprietary RMSE52 execution.

The project tooling workflow for `67db5f86875f6e40dca4f70b65fce63996a251ad` completed successfully on both Ubuntu and Windows. The later tooling run for `5e7fc98e09f53c33f6c383682973af8824889d13` also passed on both `ubuntu-latest` and `windows-latest`; that regression verifies unaligned generated chunk metadata is rejected instead of entering the merged native dispatch table.

Tooling Actions run `35938476536` for `2a86452d2aafe5156f9c4015ad7b2a3ec64b65de` also passed on both `ubuntu-latest` and `windows-latest`. Its new regression verifies that exact audited REL bytes are accepted, changed REL bytes are rejected by SHA-256, and a non-matching live-audit status is rejected.

Pull-request tooling Actions run `35939460805` for the replayed-text guard passed on both `ubuntu-latest` and `windows-latest` before merge to `main` as `eb48bfa68c20ea2d9e9c8731a13cf311d72b3771`. The regression accepts correctly replayed text, rejects same-size replayed bytes whose SHA-256 differs from the live audit, rejects inconsistent expected/observed audit hashes, and rejects a text-section mismatch count.

Performance PR #2 (`a6328f40bb0c98a58c8f50e52a30d4da55390b7e`) was validated before merge by OpenMUA2 tooling run `35940341273` and DolRecomp run `35940341449`: both Ubuntu and Windows jobs passed. ModernGekko run `35940341332` had both standalone Windows and Ubuntu tests passing at merge time; its two larger full build/test jobs were still compiling and are not counted here as completed results.

Performance PR #3 (`bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc`) page-indexes the combined native DOL+REL dispatcher. OpenMUA2 tooling run `35941158719` passed on both Ubuntu and Windows before merge. Its tests cover page-index emission, empty uncovered pages, alignment, overlap rejection, and the existing dispatch-hole invariant.

Performance PR #9 was merged as `405b81de4136a7532e966218185a190f6eb9230d`. It adds an exact page-local run end to the ordinary DolRecomp indexed dispatcher, fast-paths empty and single-run pages, and bounds multi-run scans to the current page. DolRecomp Actions run `36000036463` passed configure/build/test on both Ubuntu and Windows. ModernGekko Actions run `36000035131` subsequently completed successfully: standalone and full build/test jobs all passed on Ubuntu and Windows.

Performance PR #13 was merged as `2776fa0a3e80136495a32552b9d909e16dcfcd5e`. It returns the already-resolved verified chunk index from dispatchability checks and gates `IsHostCallAddress()` behind cached `ChunkContainsHostCall()` coverage in native bursts, preserving exact host-call checks only for candidate chunks. OpenMUA2 tooling run `36003571111` passed on Ubuntu and Windows. ModernGekko run `36003570930` passed all four jobs: standalone and full build/test on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #14 was merged as `af7938bdb63d4530ea43a6ba445800fc4171a153`. It carries the runtime→linked PC already produced by `DispatchableAt`/`FastDispatchableAt` into the immediately following module dispatch and through native-burst continuation, removing the duplicate `ResolveNativeAddress()` call before dispatch while keeping post-dispatch linked→runtime translation intact. OpenMUA2 tooling run `36008961778` passed on Ubuntu and Windows. ModernGekko run `36008961830` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #17 was merged as `7b7e412f671039e1d29e2cdff7b2e51509bc046e`. It carries the active REL section index alongside the linked PC and uses that section as a hint for post-dispatch linked→runtime translation. Same-section returns now translate directly; cross-section and DOL returns still use the full `ResolveRuntimeAddress()` scan. OpenMUA2 tooling run `36026447896` passed on Ubuntu and Windows. ModernGekko run `36026447968` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

The earlier current-source commits also include cross-platform DolRecomp CI for the cache-control generator change and cross-platform GXRuntime CI for the FMA/runtime changes.

## Validation boundary

No fresh RMSE52 WBFS boot/gameplay session has been run from GitHub CI after the latest cache-control, FMA and MEM2 changes because the proprietary game image is intentionally not in the repository. Historical MR01 gameplay and the 11,832-comparison differential pass remain useful evidence and reproduction targets, but they are **not** treated as fresh validation of current `main`.

The latest recorded live native-REL benchmark predates the direct cache-control generator change: it advanced game frames but remained far below the 30 FPS target. A speedup from the later source changes must be measured rather than inferred.

No new claim is made here for a complete level, long-session stability, multiplayer, audible audio, game-owned save/reload, physical-controller gameplay, or a full Windows game build/execution.

## Immediate priorities

1. Build the current `O2 + indexed` native module/runtime against the exact RMSE52 image and benchmark the same route on Windows and Linux wherever the local game workspace is available.
2. A/B `--dispatch-lookup indexed` against `--dispatch-lookup linear` with the same compiler, optimization level, route, warmup, graphics/audio settings, and sample window. Keep both raw results.
3. Profile native-dispatch/chassis overhead on the faster baseline: dispatch count, hottest dispatch PCs, burst length, host-call checks, REL address translation, native exceptions, and JIT fallback. Ordinary/merged dispatch lookup, host-feature selection, clean-chunk host-call probing, and duplicate runtime→linked resolution have now been reduced, so measure after these changes.
4. Optimize shared generated/native transfer paths that benefit MSVC and GCC/Clang together. With same-section linked→runtime translation now fast-pathed, the next source-level focus is continuation-side runtime→linked resolution: use the previously active REL section as a hint before the full section scan, while preserving refresh and cross-section/DOL fallback behavior.
5. Rebuild and re-measure on both platforms after each accepted performance change. Do not infer a speedup from source structure or CI.
6. Defer additional lockstep/correctness expansion until performance work reaches a useful plateau or a concrete failure blocks further performance measurement. Existing correctness/SMC/audit guards stay enabled.

The original extracted `sys/main.dol` remains the boot source. Any merged/generated DOL/REL reference is code-generation material only and must never replace the game's real boot DOL.
