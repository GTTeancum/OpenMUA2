# Current status — GitHub main

September 23, 2026. This file tracks the current reconstructed source on `main`; the older LOCAL01 recovery boundary is no longer an accurate description of the checked-in implementation.

## Current reconstructed source

The repository still originates from the recoverable MG01 + FPC01 workspace, but the following missing pieces have now been reimplemented from the retained source/evidence and committed as new work rather than presented as recovered MR01 bytes:

- Native DOL + REL packaging and runtime eligibility are present. RMSE52's absolute linked REL section-table pointer is accepted with guest-RAM bounds checks, and native REL code remains protected by the existing chunk-hash / SMC verification path. Different or modified code is not made native merely to advance boot.
- DolRecomp emits `dcbf`, `dcbst`, `dcbi` and `icbi` through the dedicated cache-control hook instead of the generic instruction-fallback callback. The Dolphin cache invalidation semantics and generated cycle accounting remain intact.
- The generated scalar FMA helper has been repaired to use the same instruction-shaped arithmetic/rounding path as the runtime floating-point implementation. Targeted regression coverage compares result bits, FPSCR state and write/no-write behavior across rounding/NI/VE/FI/FR cases and edge values.
- The lockstep checker contains local-loop boundary alignment: when native execution yields back at an inlined loop header, the interpreter shadow continues until it has performed the native charged work rather than comparing different loop iterations at the same PC.
- The runtime floating compare path preserves the fifth FPRF classification bit while replacing only FPCC, with a targeted runtime regression.
- MEM2 is now included in lockstep memory journaling/restoration. MEM1 and MEM2 use disjoint physical journal keys; native and interpreter MMU writes feed the same diagnostic journal, the shadow receives the pre-block image, and the native post-image is restored afterward.
- Merged DOL+REL dispatch keeps native eligibility bounded to generated code chunks. Alignment and overlap guards are covered, and commit `67db5f86875f6e40dca4f70b65fce63996a251ad` adds a regression proving an uncovered guest-address hole between generated ranges is not bridged into native eligibility.

## Fresh validation of current work

For the MEM2 repair, a focused Linux validation workflow first applied only the six intended source-file changes, configured/built GXRuntime, ran its tests, configured/built the ModernGekko Linux runtime and ran the ModernGekko tests. Every step completed successfully before the source repair was integrated into `main` as commit `26334ad651567bf098401f781d1189e5f75c296c` (`Journal MEM2 in lockstep verification`).

After that integration, the normal GXRuntime matrix completed successfully on both `ubuntu-latest` and `windows-latest`. This is useful Windows portability coverage for the shared runtime change, but it is **not** a full Windows game execution test.

The cross-platform ModernGekko workflow introduced by `c1eeb1a2fb8d9ea31954c818fe21d9269b7edc25` completed successfully. Its standalone tests and full configure/build/test jobs all passed on both Ubuntu and Windows, including the MSVC/Ninja Windows path. This remains source/runtime validation rather than proprietary RMSE52 execution.

The project tooling workflow for `67db5f86875f6e40dca4f70b65fce63996a251ad` also completed successfully on both Ubuntu and Windows. The added regression checks that a gap between generated DOL and REL code ranges remains non-native in the merged dispatch table.

The earlier current-source commits also include cross-platform DolRecomp CI for the cache-control generator change and cross-platform GXRuntime CI for the FMA/runtime changes.

## Validation boundary

No fresh RMSE52 WBFS boot/gameplay session has been run from GitHub CI after the latest cache-control, FMA and MEM2 changes because the proprietary game image is intentionally not in the repository. Historical MR01 gameplay and the 11,832-comparison differential pass remain useful evidence and reproduction targets, but they are **not** treated as fresh validation of current `main`.

The latest recorded live native-REL benchmark predates the direct cache-control generator change: it advanced game frames but remained far below the 30 FPS target. A speedup from the later source changes must be measured rather than inferred.

No new claim is made here for a complete level, long-session stability, multiplayer, audible audio, game-owned save/reload, physical-controller gameplay, or a full Windows game build/execution.

## Immediate priorities

1. Rebuild the exact RMSE52 native module/runtime from current `main` on the local game workspace and run a fresh baseline benchmark. This will measure the real effect of keeping cache-control operations inside native bursts.
2. Run a bounded current-main lockstep window on the opening gameplay route so MEM2 restoration, loop-boundary alignment, floating compare and FMA behavior are exercised together against the reference interpreter.
3. Preserve any divergence as a failure and reduce it to a focused regression. Do not whitelist mismatches, mask FPSCR state, disable chunk hashes/SMC protection, or change game timing to make a test pass.
4. Once correctness is re-established on the current build, profile native dispatch/exception/JIT and cache-invalidation costs again using fresh measurements rather than the pre-cache-control benchmark.
5. Keep Linux as the active development path while continuing to keep shared source/build code portable; a full Windows ModernGekko/game build remains a separate validation milestone.

The original extracted `sys/main.dol` remains the boot source. Any merged/generated DOL/REL reference is code-generation material only and must never replace the game's real boot DOL.
