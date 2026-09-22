# Marvel: Ultimate Alliance 2 — MR01 recovery checkpoint

Wii USA / RMSE52 · DolRecomp + ModernGekko · September 22, 2026

## Result and provenance

**The missing work has been reconstructed and actual native DOL + REL execution is back in the opening Latveria mission.** Movement, camera following, hero switching, light-attack input and charged-heavy-attack input/animation were exercised. Captures show the first Doombot fight and defeated enemies. AI companions also attack, so individual enemy damage is not attributed exclusively to the player.

This is a **new MR01 reconstruction**, not a recovered copy of the missing MG02 archive. It starts from the complete MG01 checkpoint and original pinned game/dependency uploads, retaining the separately recovered FPC01 helper repair. The final archive includes actual Linux binaries, complete generated code, a cumulative 31-file source overlay, reproduction tools and evidence. It is not a finished or glitch-free release.

## Fresh sessions actually run

| Session | Result | Wall-clock seconds |
|---|---|---:|
| native-cold-01 | No initial state: advisory, logos, title, story/difficulty and Guest setup; clean stop | 818.011 |
| native-continuation-01 | Loaded that native-created setup state; entered Latveria, moved through streets, switched Captain America to Iron Man and entered first fight; clean stop | 903.733 |
| native-combat-01 | Native-created first-fight state; light/charged-heavy controls and captures; clean stop | 112.314 |
| packaged-combat-01 | **Final corrected module and packaged runner/Sys/Run-Linux.sh**: first-fight state, movement and light/heavy controls; clean stop | 212.08 |
| native-lockstep-fma-01 | Final module: bounded differential replay, then rendered gameplay; clean stop | 438.541 |

The cold-to-mission chain uses two processes joined by a **native-created diagnostic state**, not a reference-emulator state. Cold boot and the first two gameplay sessions used the pre-FMA module (`b255e543...`); the final corrected module (`5def4a3e...`) was separately tested from the native-created first-fight state. A new uninterrupted cold boot through combat with the final module is **not** claimed.

The continuation recorded **129,808,814 native REL dispatches**; the final packaged combat session recorded **28,712,319**. These establish actual REL execution, not coverage percentages, instruction counts or elapsed game time. Hook/JIT fallback remains active. Guest native-exception counters are not host-crash counts.

No host crash or forced termination was observed in these ordinary native sessions or the final passing differential session. The pre-fix diagnostic failure below is explicitly excluded. Gameplay rates were approximately 4–6 FPS in the final software-rendered combat test; this is neither a release benchmark nor acceptable final performance. Audio was muted with the null backend throughout.

## Live correctness result and newly repaired FMA bug

The final replay reports **12,389 candidate checks**, including **11,832 actual comparisons**, **7 fallback skips** and **550 zero-charge skips**. It reports **zero mismatches, zero step-cap hits, zero filtered reports and zero undercharge reports**. Its native-dispatch selection window ended at 2,000,000; that is not two million comparisons. Complete logs and invocation variables are retained under `evidence/runs/native-lockstep-fma-01`.

Before the final fix, a broader requested replay was stopped at its deadline after 11,390 candidates and **42 mismatches**, all reported as FPSCR differences. That attempt needed SIGTERM after its graceful-stop timeout and is **not a passing run**, even though its process ultimately returned zero. Its requested dispatch window was larger than the completed final window; these are not identical-duration or identical-count sweeps.

Inspection found that generated multiply-add instructions called `GXRuntime::ppc_fma`, which rounded before preserving the unrounded result and did not implement the reference `fmadds` FI/FR update. That path also differed from existing production `ppc_fmadd_op` in rounding/exception handling. The final helper now uses the existing `ni_madd_msub`, single/double rounding helpers and matching instruction-specific flags. No FPSCR bits are masked in comparisons.

The corrected helper passes **3,027,456 cases across eight multiply-add/subtract variants**, covering the four rounding modes, NI/VE controls, initial FI/FR states, signed zero, finite/extreme values, infinities and NaNs. All result bits, FPSCR bits and write/no-write decisions match the mechanically adapted, pinned interpreter instruction bodies. The original helper fails **608,086** of those cases. A separate run linked the **exact Clang 19 O1 release CPU objects used in the final game module** and also passed all 3,027,456 cases. These are reference-code comparisons, not physical-Wii verification. Host FTZ/DAZ and guest FP-trap delivery are outside this fixture's scope.

## Other reconstructed fixes and passing tests

The production REL loader writes 16-bit relocation fields at their actual halfword destinations, supports the observed BSS placement, and checks module alignment. Replaying **95,772 relocations** exactly reproduces **3,255,744 executable bytes** and all 19 captured section entries. The fresh cold-boot linked header matches MG01's captured 228-byte header exactly.

Native REL eligibility requires the complete observed layout: module 1/version 3, base `0x80E4A080`, text `0x80E4A164`, BSS `0x811BCAC0` and all 19 sections. The runtime recognizes the linked absolute section-table pointer. Different layouts are rejected for native REL execution and fall back; general dynamic rebasing is not implemented.

Diagnostic-only MEM2 isolation restores entry memory for shadow execution and the native post-image afterward, including early exits. The local-loop comparison helper accounts for native yield points at return joins inside an inlined loop. It was tested against the actual code around the historical `0x802A1C08 -> 0x802A1C1C` report. The passing new replay does not prove that every historical input or every later path has been reproduced.

| Test | Fresh result |
|---|---|
| DolRecomp C backend | 19 CTests pass; LLVM was disabled |
| ModernGekko runtime | 30 targeted CTests pass |
| Production REL fixtures | 2,281 assertions pass |
| Linked-layout validator | 20,011 assertions, including 10,000 mutated headers |
| MEM2 isolation | 110,009 assertions, including 10,000 randomized trials |
| Local-loop boundary | 512 assertions, including the actual captured chunk |
| Float comparison helpers | 1,536 finite cases pass; original negative control fails 1,344 |
| FCTIW conversion | 1,298,576 cases pass, including a separate exact-final-release-object test |
| Source overlay | 12 Python tests; 31 exact pinned originals apply cleanly; reapply changes zero files |
| Existing pre-integration upgrade | Actual earlier ZIP applies, upgrades to all 31 final hashes, and is idempotent |
| Control preflight | Seven tests reject offline/invalid commands without writing a command |
| Final native module | All 16,743 chunk hashes, 8,571,904 text bytes and ABI checks pass |

Standalone helper fixtures use ASan/UBSan; the reference vptr-only check is excluded because no Interpreter object is instantiated. Exact release CPU objects are intentionally tested **without** sanitizer instrumentation. Runtime common support libraries are prebuilt. No whole-game sanitizer run is claimed. Failed link/configuration attempts and the failed pre-FMA live diagnostic remain separately identifiable in the evidence.

Code-hash checks remain enabled. The cold chain rejected a genuinely self-modified DOL chunk at `0x804AE500–0x804AE700`; fallback handled it. The real game's `sys/main.dol` is unchanged. **`generated/main.dol` is a code-generation-only merged DOL/REL reference, never a boot replacement.**

## Remaining limits

No complete level, later mission, long-session stability, multiplayer, audible output, physical controller, Windows build/execution or game-owned save/load round trip has been verified. Diagnostic `.sav` files, generated game save files and an Auto Saving message are not proof of a successful game-save reload. The reference core also logs an unavailable WiiConnect24 hostname in this offline environment; online services are not validated.

The final live sweep is a strong bounded CPU/memory check, not a guarantee of flawless rendering, timing, physics, audio or full-game behavior. The native module still depends on the ModernGekko/RecompCore runtime and its fallback implementation; this is not a pure all-AOT replacement for every guest address.

## Restore and rebuild

Keep all five original game volumes and all three dependency volumes, with their upload index/checksums. **MG01's separate ZIP is no longer required for this cumulative package**: the small original MEM1 capture pieces needed for regeneration are included. The full game, private SDK and upstream source collection are not duplicated.

The tested host was Debian 13 amd64 with Python, GCC 14, CMake, Ninja, 7-Zip, Xvfb/xauth, dpkg-deb, gpgv and the existing Debian archive keyring. The uploaded collection supplies private Clang 19 and the runtime development dependencies. No packages were installed system-wide.

```bash
CHECKPOINT=/absolute/path/to/MUA2_MR01_Checkpoint
UPLOADS=/absolute/path/to/original-uploads
ROOT=/absolute/path/to/new-mua2-work

python3 "$CHECKPOINT/tools/restore_uploads.py" \
  --input-dir "$UPLOADS" --output-dir "$ROOT/dependencies"
PAYLOAD="$ROOT/dependencies/payload"
python3 "$PAYLOAD/helpers/prepare_scratch.py" \
  --payload "$PAYLOAD" --workspace "$ROOT/scratch"
python3 "$CHECKPOINT/tools/extract_private_sdk.py" \
  --payload "$PAYLOAD" --sdk "$ROOT/sdk"

7z x -y -bsp0 "-o$ROOT/game-image" \
  "$UPLOADS/Marvel - Ultimate Alliance 2 (USA).7z.001" '*.wbfs'
cmake -S "$ROOT/scratch/project/lib/DolRecomp" \
  -B "$ROOT/scratch/build/dolrecomp" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DDOLRECOMP_ENABLE_LLVM=OFF
cmake --build "$ROOT/scratch/build/dolrecomp" --parallel 2
WIT="$ROOT/scratch/tools/wit-linux/wit-v3.05a-r8638-x86_64/bin/wit"
"$ROOT/scratch/build/dolrecomp/dolrecomp" extract --wit "$WIT" \
  "$ROOT/game-image/Marvel - Ultimate Alliance 2 (USA).wbfs" "$ROOT/scratch/game"
JOBS=2 bash "$CHECKPOINT/build-linux.sh" \
  "$ROOT/scratch" "$ROOT/sdk" "$ROOT/scratch/game"
```

The constituent build/generation/test commands were executed; the consolidated build wrapper was syntax-checked but has **not** been rerun end-to-end in a second clean workspace. It requires a new generation directory and rejects unrecognized source edits. It must not overwrite the actual game DOL.

## Repeat the tested packaged gameplay route

The packaged binaries are Debian 13/private-SDK scratch builds with build-tree library paths, not universal portable Linux executables. The final packaged runner and its included Sys assets were actually used in `packaged-combat-01`. No font files are supplied; original dependency sources can restore the complete Sys assets during a rebuild.

```bash
xvfb-run -a -s '-screen 0 1280x720x24 -nolisten tcp' \
  bash "$CHECKPOINT/Run-Linux.sh" \
  "$ROOT/scratch/game" "$ROOT/sdk" "$ROOT/replay-combat" \
  --profile "$CHECKPOINT/evidence/runs/native-continuation-01/user" \
  --load-state "$CHECKPOINT/evidence/runs/native-continuation-01/automation/native-first-fight.sav" \
  --seconds 300 --screenshot-every 20
```

From another terminal, while status is running, use `tools/control_probe.py --session "$ROOT/replay-combat" pad --frames 12 wii_a=1`. `wii_b` is the chargeable heavy-attack button, `wii_plus` switches heroes in gameplay, and `nunchuk_x`/`nunchuk_y` are the movement axes. `pad_frames` acknowledges after the requested number of **game frames**; at software-rendered speed this takes several wall-clock seconds. `save` captures a diagnostic state only. The controller helper never writes game memory.

For a fresh boot, omit `--profile` and `--load-state`. To repeat the passing differential window, use a new output folder and add `--lockstep-limit 2000000 --seconds 510`. The diagnostic copies MEM2 and is extremely slow during its selected window; it is disabled for ordinary sessions.

## Audit reproduction and contents

`tools/run_unit_audits.py` accepts `--workspace`, `--payload`, `--game`, `--capture` and a new `--output`; use `evidence/mg01-rel-capture` for the capture. After it creates `OUTPUT/fpc`, `verify_linked_fctiw.py` and `verify_fma.py` accept that support directory and the final module build directory for exact-object comparisons. Their complete commands and source/object hashes are recorded with the results.

`source-overlay` contains exact cumulative source bytes, patch and pin hashes. `generated` preserves every generated C chunk unchanged; `evidence/module-groups.json` records compilation grouping. `evidence/runs` holds native-origin states, captures, input receipts, status histories and passing/failed logs. `RESULTS.json` and `BUILD_STATE.json` record binary hashes and measured scope. `SHA256SUMS` covers all packaged payload files except itself; the external validation receipt covers the completed ZIP.
