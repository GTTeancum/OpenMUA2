# Marvel: Ultimate Alliance 2 — MG02

**Wii USA / RMSE52 · DolRecomp + ModernGekko · Linux x86-64 development checkpoint**  
Packaged September 22, 2026 (UTC).

## Result

**Actual native DOL + REL execution now reaches the opening Latveria mission and the first Doombot fight.** Captures show movement, the camera following the party, light attacks, a charged heavy attack, enemy damage, and switching from Captain America to Iron Man. These are live runtime captures, not offline renders.

**This is not a stable or finished PC release.** Three differential-check reports remain unresolved, software-rendered performance is poor, and the complete game has not been tested. No Windows executable is supplied. Audio was muted using the null backend and is not verified.

## What was run

| Session | Actual result | Wall-clock duration |
|---|---|---:|
| `native-mission-01` | Opening mission and combat, using a reference-created diagnostic state; 3,907 frames progressed; clean exit | 756.635 seconds |
| `native-cold-01` | Fresh native startup, wrist strap, logos, title, Begin Story, Normal difficulty, Guest setup; no initial save state; clean exit | 1,004.539 seconds |
| `native-cold-02` | Continued the **native-created** setup state; start game, intro/loading, opening mission, movement, first enemy fight, heavy attack, hero switch, damage; clean exit | 596.216 seconds |

The native-only startup chain is split across two processes using the runtime's own diagnostic save state. It is **not** represented as one uninterrupted cold-to-combat process, and the second chain does not rely on the JIT reference's mission state. The native module is identical across all three runs. The latest runtime binary was used for `native-cold-02` and `native-lockstep-04`.

The first mission run recorded **104,194,798 native REL dispatches**. The native-origin continuation recorded **79,710,518**. These establish real REL execution; dispatch counts are not instruction counts, percentages of native coverage, or elapsed game time. Hook/JIT fallback remains active for uncovered or modified code. The `fallback_steps=0` field does not count all fallback activity.

No host crash, forced termination or unhandled guest error was observed in those three ordinary native sessions. This bounded observation does not establish full-game crash freedom or absence of graphical, timing or logic glitches. The sampled software OpenGL rates were roughly 4–9 FPS; these runs used Mesa software rendering in the scratch container, not a release-performance benchmark.

## The remaining differential failures

The original verifier did not roll back Wii MEM2 stores. Its first diagnostic run (`native-lockstep-01`) corrupted/stalled the guest, reported 19 mismatches and needed forced termination. That failure is retained, not counted as success.

MG02 adds diagnostic-only full-MEM2 isolation, including restoration on early exits. A 512-check replay then reported zero mismatches. A broader 1,924-check replay reported six. Inspection and a production-helper fixture proved an additional reference-interpreter FPCC masking bug: a shifted mask was applied to the unshifted FPRF field. In 1,536 finite comparison cases the original helper failed 1,344 cases; the corrected helper failed zero.

**The latest broad replay still reports three mismatches out of 1,924 checks:**

- `0x80388908 -> 0x80388B00`: FPSCR differs by `0x00020000` (FI).
- `0x80387390 -> 0x80251DE0`: FPSCR differs by `0x00020000` (FI).
- `0x802A1C08 -> 0x802A1C1C`: registers and one MEM2 byte differ. The generated chunk inlines a local loop and local return-dispatch continuation. The first direct-branch join correction does **not** resolve this case. Exact boundary alignment versus an actual execution bug needs further verification.

The latest replay had 7 fallback-skipped checks, 192 zero-charge-skipped checks and no step-cap hits. It exited cleanly, but **is not a passing correctness sweep**. No register bits are masked out, no addresses are whitelisted, and no code-hash protections are disabled to turn these reports green. Final tested runtime SHA-256 is `ba6bab6c1e5a1b3967df631e64646c2f17ecb9d1bd49ba98a1fb8110df2bac81`.

## Native code and REL integration

The combined module contains **16,743 generated chunks**, covering 8,571,904 DOL-plus-REL text bytes, compiled with Clang 19 at O1 and IPO off. All chunk hashes, text tiling, module/CPU ABIs and the REL descriptor pass the supplied native verifier. Module SHA-256 is `d637b11e1ebeaab493a382b7b27ab27f77788c516455d517b2b13767262a50e4`.

The production REL loader now writes 16-bit relocations at the actual halfword offset, supports explicit BSS placement, and checks module rather than every file-section alignment. Its replay of **95,772 relocations matches all 3,255,744 captured executable bytes**.

Native REL eligibility is deliberately restricted to the complete observed linked layout: module 1/version 3, base `0x80E4A080`, text `0x80E4A164`, BSS `0x811BCAC0`, and all 19 section entries. The runtime recognizes the linked absolute section-table pointer. Different layouts are rejected for native execution and use fallback; general dynamic rebasing is not claimed.

A self-modified DOL chunk at `0x804AE500–0x804AE700` is rejected by hash verification in the cold-boot chain. Fallback remains enabled. The original extracted game's `sys/main.dol` is unchanged. **`generated/main.dol` is a code-generation-only DOL+REL reference, never a game boot replacement.**

## Tests actually passed

- DolRecomp with LLVM: **30 CTests**; latest ModernGekko runtime: **30 targeted CTests**.
- Production REL relocation field/bounds fixtures: **322 cases**.
- Production REL header/layout validators: **10,173 assertions**, including 10,000 mutated headers and the actual captured linked layout.
- MEM2 snapshot isolation: **60,003 assertions**, including 10,000 randomized before/native/shadow restoration trials.
- Direct local-join diagnostic helper: **10,006 assertions**. This unit result does not resolve the remaining live loop report.
- Actual production finite FP comparison helpers: **1,536 cases**, zero failures after the fix.
- Overlay utilities: **11 Python tests**. All **28 cumulative production files** apply cleanly to their exact pinned originals; applying twice changes nothing; all output bytes match the manifest.

The standalone C/C++ fixtures ran with AddressSanitizer and UndefinedBehaviorSanitizer. The finite FP fixture disables the vptr-only sanitizer because it links the production static helpers without instantiating the Interpreter object; its test-only declaration copy exposes private static helpers without changing their implementation or member layout. It intentionally does not test NaN/exception behavior or full instruction dispatch. All failed build attempts and diagnostic logs are preserved separately.

## Contents and reproduction

`linux-x86_64/` contains the actual final runner, native game module, DolRecomp and runtime Sys support assets. Font resources are not bundled; the original dependency sources provide them when CMake restores the complete Sys directory. `generated/` contains the complete generated code and grouping receipt. `source-overlay/` contains the cumulative 28-file patch, exact source bytes and input/output hashes. `evidence/` contains screenshots, status histories, acknowledged input commands, states, audits and both passing and failed run records. `RESULTS.json` and `BUILD_STATE.json` summarize measured results. `SHA256SUMS` covers the packaged files.

Keep the original game volumes and all three dependency volumes. They supply the game data, complete pinned upstream sources and private SDK; they are not duplicated here. MG01 is not needed for the new cumulative source overlay. Two original MG01 MEM1 capture pieces are included under `evidence/mg01-rel-capture/` only to reproduce the linked-layout fixture.

Restore the dependency workspace using the collector's `restore_uploads.py` and `helpers/prepare_scratch.py`. Extract the SDK privately using `tools/extract_private_sdk.py`, and extract the original game as documented by the collector/MG01. With `CHECKPOINT`, `WORKSPACE`, `SDK`, and `GAME` set to real paths:

```bash
JOBS=2 bash "$CHECKPOINT/build-linux.sh" "$WORKSPACE" "$SDK"

python3 "$CHECKPOINT/tools/run_unit_audits.py" \
  --workspace "$WORKSPACE" --output "$WORKSPACE/audit-replay" \
  --mem1-first "$CHECKPOINT/evidence/mg01-rel-capture/memory-80000000.bin" \
  --mem1-second "$CHECKPOINT/evidence/mg01-rel-capture/memory-81000000.bin"

# Actual live native replay; use a new output folder.
xvfb-run -a -s '-screen 0 1280x720x24 -nolisten tcp' \
  python3 "$CHECKPOINT/tools/session_probe.py" \
  --mode native --runtime "$WORKSPACE/build/moderngekko/moderngekko-run" \
  --module "$WORKSPACE/build/mua2-c128/gRMSE52_recomp.so" \
  --game "$GAME" --sdk "$SDK" --output "$WORKSPACE/native-replay" \
  --seconds 600 --screenshot-every 120 \
  --profile "$CHECKPOINT/test-profile-native" \
  --load-state "$CHECKPOINT/evidence/native-cold-02/automation/native-origin-mission-entry.sav"
```

Run `control_probe.py` from another shell while the bounded probe is active to send frame-based input; it never writes game memory. See `NEXT_STEPS.md` for the tested opening route and unresolved addresses. Omit `--profile` and `--load-state` for a fresh default-profile native boot. The binaries are Debian 13/private-SDK scratch builds, not universal Linux binaries. The consolidated build wrapper uses commands exercised during this work; it has not been rerun end-to-end in a second clean workspace. `regenerate.sh` regenerates from the original game and validates the proven REL text; it must not overwrite the real game DOL.

**Still unverified:** real-time performance, full-game stability/correctness, later missions and transitions, audible audio, controller hardware/Windows input, the game's own save/reload system and Windows execution. Runtime `.sav` files and an onscreen Auto Saving message are not proof that the game's save system works.
