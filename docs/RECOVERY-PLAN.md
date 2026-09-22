# Resume development from actual files

The shipped baseline is MG01 + FPC01. MR01's records are useful leads but not recoverable source. Preserve this first local commit, then implement new work in small separate commits with code, a regression test, a command log and a current status update.

## Source map

- CPU translation and REL loader: `project/lib/DolRecomp/src/`; a second pinned copy is under `project/lib/ModernGekko/vendor/dolphin/DolRecomp`.
- Native runtime CPU helpers: `project/lib/ModernGekko/vendor/dolphin/GXRuntime/src/core`.
- Native module build/export: `project/lib/ModernGekko/vendor/dolphin/module-template`.
- Native eligibility/dispatch and shadow diagnostics: `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp`.
- Reference floating-point behavior: `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/Interpreter`.
- Source pins: `locks/SOURCE_SNAPSHOT.json`. Actual delivered byte hashes: `FILE-MANIFEST.json`.

## Native REL recovery

The retained audits describe `Marvel-rev-fin-plf2.rel`, module 1/version 3, 19 sections and 95,772 relocations. In the captured linked layout, module base was `0x80E4A080`, text `0x80E4A164` and BSS `0x811BCAC0`. These are observed addresses, not universal allocation promises.

The pinned loader still needs correct halfword destinations, explicit observed BSS placement and module-level alignment. The game's linked header rewrites the section-table offset to an absolute pointer. Restore precise header/layout validation before enabling native REL eligibility. Different layouts must reject native execution and retain fallback, not silently relocate to assumed addresses. The lost reconstruction reportedly matched 3,255,744 text bytes; reproduce that comparison from new captured runtime memory before claiming the same result.

Metadata audits are included under `docs/recovery/`, but raw captured RAM and the missing native-origin states are not part of this source-only package. A new capture is required where an audit needs actual memory bytes. Do not fabricate fixtures from a report hash or distribute game memory in a public Git history.

## Floating-point and verifier recovery

The recovered FPC01 conversion mask patch is already applied to both DolRecomp copies. The GXRuntime module uses a separate floating-point helper path. Do not assume the conversion fix also restores the missing multiply-add repair.

Historical MR01 records identify `GXRuntime::ppc_fma` behavior and the reference `fmadds` FI/FR/rounding semantics. Reimplement any repair against the actual pinned interpreter and test all result bits, FPSCR fields and suppressed-write decisions. Then rebuild the module and replay live inputs. Historical matrix totals are not fresh evidence.

Wii shadow execution needs correct **MEM2** snapshot restoration both before the interpreter shadow and after early exits. The native local-loop yield/return boundary around `0x802A1C08 -> 0x802A1C1C` also needs exact alignment. The floating-point reference comparison helper's mask issue is another separate historical lead. Do not whitelist addresses, mask status bits, skip comparisons or disable hash checks to turn a failing diagnostic green.

## Acceptance after reconstruction

Validate relocation/text/ABI tests, then boot the exact final local module from a clean start through menus and into movement/combat. Record the tested controls, entry conditions, elapsed wall-clock time versus guest frames, clean exit and failures. Test the game's own save/reload with a process restart, not just runtime save states. No full-game “crash-free/glitch-free” label follows from a bounded opening-scene test.

Keep engine timing unchanged unless an override is justified from original code and measured behavior. Native dispatch counts are not percentages of all-native game execution. Maintain hook/JIT fallback until coverage and semantics justify a different policy. Windows, hardware controllers, audio, saves and later missions need their own tests.
