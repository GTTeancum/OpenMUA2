# Marvel: Ultimate Alliance 2 — MG01

**September 21, 2026 · Wii USA · RMSE52 · DolRecomp + ModernGekko**

## What this checkpoint actually achieves

The uploaded dependency collection has been restored and used to build DolRecomp, ModernGekko's Linux runner/launcher/port tool, and a native x86-64 module for MUA2's main DOL. A bounded run of the real extracted game rendered its Wii wrist-strap advisory. This was runtime output with native DOL dispatch, not an offline asset render.

**This is a diagnostic Linux checkpoint, not a finished PC port.** The native module contains DOL coverage only. REL and other uncovered addresses require the runtime's fallback path. The title screen, gameplay, audible audio, saving and a Windows build are not verified.

## Results and evidence

| Check | Observed result | Evidence |
|---|---|---|
| Upload integrity | All 3 parts and 1,246 payload files verified | `logs/restore.log`, `source-pins/MANIFEST.json` |
| Source restoration | All 42 pinned repository nodes restored | `logs/prepare.log`, `source-pins/SOURCE_SNAPSHOT.json` |
| Linux dependencies | 557 packages authenticated against the pre-existing Debian archive keyring | `logs/deps-audit.log` |
| DolRecomp | Builds; 19 tests pass | `logs/dolrecomp-build.log`, `logs/dolrecomp-tests.log` |
| DOL translation | 325 C chunks; 5,316,160 executable bytes; no unknown decoded instructions reported | `logs/mua2-dol-codegen.log`, `generated-dol/` |
| Native module | Builds with Clang 19, O0, IPO disabled; ABI and all 325 chunk hashes pass | `logs/mua2-clang-build2.log`, `evidence/native-module-verification.json` |
| ModernGekko | Runner, launcher and port tool build; 30 targeted tests pass after EGL correction | `logs/moderngekko-egl-build.log`, `evidence/moderngekko-tests-after-egl.txt` |
| Checkpoint audits | 20 tests pass, including live REL replay and corruption/range cases | `evidence/checkpoint-tests.txt` |
| Graphical boot | Wrist-strap advisory captured; 177 game frames reported | `evidence/boot-ogl-fixed/automation/screen-40.png`, `boot-result.json` in its parent directory |
| Native execution | 523,228,655 native dispatches recorded in the graphical probe | `evidence/boot-ogl-fixed/runtime.log` |
| REL loading | Module 1 observed at 0x80E4A080; its 3,255,744 text bytes exactly reconstructed | `evidence/live-rel-audit.json` |

The graphical probe had a 140-second deadline and shut down cleanly after 144.474 seconds including shutdown. Its final reported speed was approximately 0.094 and its FPS approximately 2.03. This was an **O0 diagnostic module using Mesa llvmpipe software OpenGL in a constrained scratch container**, not a benchmark of a release port. The captures at 40, 75 and 110 active wall-clock seconds all show the advisory; no progression beyond that screen is claimed.

The native counter counts runtime dispatch units, not individual PowerPC instructions. The `fallback` counter does not count all fallback JIT work. Neither a native percentage nor an all-native execution claim can be derived from those counters.

The unfiltered vendored CTest tree also listed zstd test executables that were not built. That unfiltered invocation failed and its log is retained. It is **not** included as a passing suite. The passing counts above are the explicitly tested 19 DolRecomp + 30 ModernGekko + 20 checkpoint tests.

## Changes made

The cumulative patch is `patches/MG01-all-build-fixes.patch`. It changes build configuration, not guest game logic:

- Makes the private SDK include directory available to Linux hidapi, fixing the actual `libudev.h` build failure.
- Enables EGL on Linux. In the pinned runtime, Linux's OpenGL context factory uses EGL; disabling it caused the first graphical probe to fail despite working system OpenGL.
- Scopes `HAVE_EGL` to its sole source-file consumer, avoiding unrelated rebuilds while retaining the compiled EGL context implementation.

`tools/apply_sdk_fix.py` and `tools/apply_egl_fix.py` apply these changes strictly and idempotently, preserving the source files' line-ending convention. The normalized-LF patch is also supplied for inspection. Exact resulting CMake files are included under `patched-source-files/`.

No code-hash checks were disabled, no game memory was written by the probe, and no fabricated device acknowledgements or startup bypasses were added. The probe used the runtime's input-override protocol for brief Plus/A button presses.

## REL findings: verified, but not yet integrated natively

The runtime log reports the REL base as **0x80E4A080**. The memory capture confirms module ID 1, version 3, and 19 sections. Its executable section starts at **0x80E4A164**. BSS starts at **0x811BCAC0**, where the game reuses the discarded relocation area, not after the complete on-disc REL file.

`tools/audit_live_rel.py` replays all **95,772** relocation operations using the section addresses actually recorded in RAM. All **3,255,744 executable bytes** match, with SHA-256:

```
05a41edb0df90f3bdf3de80c12a4b68e59786bf5c214f5d877945bffe20605f0
```

That comparison establishes three concrete problems to address before native REL integration:

1. **Halfword relocations:** all 30,913 observed 16-bit relocation offsets already point at an immediate halfword (offset modulo 4 is 2). The pinned DolRecomp REL loader writes at `patch + 2` instead. Replaying that behavior produces **121,737 mismatching text bytes**; writing at the recorded offset produces zero. The analysis tool includes the correct replay and tests; the upstream C loader has **not** been patched in MG01.
2. **Placement/alignment:** DolRecomp's default BSS-after-full-file layout does not match this game's linked layout. Its executable-section alignment check also conflicts with the observed section address (4-byte aligned, not 8-byte aligned). These remain integration work rather than silently forced assumptions.
3. **Linked header:** the game rewrites the section-table field to the absolute pointer **0x80E4A0CC** and clears the import-table size. The pinned runtime's REL discovery checks for the original relative section-table offset. That discovery logic needs to accommodate the actual linked header before it can safely track this module.

These findings concern this pinned source and captured boot. The recorded base is **not** a promise that future boots allocate the REL at the same address. A fixed-address native patch is not supplied. The module descriptor still reports **zero native REL modules**.

## Modified DOL code: protection worked

The runtime rejected native coverage for chunk **0x804AA900–0x804AE900** because guest memory differed from the generated module. The final RAM capture confirms **24 changed instruction words** in that chunk (`evidence/smc-memory-diff-final.json`). The mismatch is real; it was not worked around by disabling verification. Uncovered/mismatching code is left to runtime fallback. An earlier capture preceded these modifications and had no differences; both audit receipts are retained to avoid conflating their capture times.

## Contents and prerequisites

`linux-x86_64/` contains the actual built runner, launcher, port tool, DolRecomp and `gRMSE52_recomp.so`. `generated-dol/` preserves the generated C, input DOL and generation receipt. `evidence/` includes actual screenshots, logs, command/status records and the final MEM1/MEM2 capture. `BUILD_STATE.json` records measured status and binary hashes. `SHA256SUMS` covers every packaged file except itself.

This archive deliberately does **not** duplicate the full game asset tree, private SDK, original dependency archive, or complete runtime `Sys` asset directory. Those are recoverable from the original uploads and pinned dependency sources. **Keep the original game volumes and all three dependency volumes with this checkpoint.**

The provided binaries are Debian 13 x86-64 scratch builds and carry build-tree/private-SDK paths. They are not a universal portable Linux bundle and are not Windows executables. Use the rebuild recipe below in a matching Linux environment; do not expect double-clicking the launcher on Windows to work.

The upstream sources and their original license files are preserved in the previously supplied dependency collection. `source-pins/SOURCE_SNAPSHOT.json` identifies their exact commits and archive hashes. The generated game code and memory evidence derive from the user's supplied game and are not independent open-source assets.

## Rebuild in a Linux scratch environment

The machine used here had Debian 13 amd64, Python 3.13.5, GCC 14, CMake 3.31.6, Ninja, `dpkg-deb`, `gpgv`, the existing Debian archive keyring, 7-Zip, Xvfb and xauth. The private SDK supplies Clang 19 and frontend development dependencies. No network access is needed once the original uploads are present. Set the three paths below to new, dedicated locations:

```bash
CHECKPOINT=/path/to/MUA2_MG01_Checkpoint
UPLOADS=/path/to/original-upload-folder
ROOT=/path/to/new-mua2-scratch

python3 "$UPLOADS/restore_uploads.py" \
  --input-dir "$UPLOADS" --output-dir "$ROOT/dependencies"
PAYLOAD="$ROOT/dependencies/payload"
python3 "$PAYLOAD/helpers/prepare_scratch.py" \
  --payload "$PAYLOAD" --workspace "$ROOT/scratch"
python3 "$CHECKPOINT/tools/extract_private_sdk.py" \
  --payload "$PAYLOAD" --sdk "$ROOT/sdk"

# Extract the WBFS from the original five-volume 7z using an existing 7-Zip.
# Every original volume must be beside .001; the destination must be new.
7z x -y -bsp0 "-o$ROOT/game-image" \
  "$UPLOADS/Marvel - Ultimate Alliance 2 (USA).7z.001" '*.wbfs'

# The verified wit executable is restored by prepare_scratch.py.
WIT="$ROOT/scratch/tools/wit-linux/wit-v3.05a-r8638-x86_64/bin/wit"
# A rebuilt DolRecomp provides the tested disc extraction entry point.
cmake -S "$ROOT/scratch/project/lib/DolRecomp" \
  -B "$ROOT/scratch/build/dolrecomp" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DDOLRECOMP_ENABLE_LLVM=OFF -DBUILD_TESTING=ON
cmake --build "$ROOT/scratch/build/dolrecomp" --parallel 2
"$ROOT/scratch/build/dolrecomp/dolrecomp" extract --wit "$WIT" \
  "$ROOT/game-image/Marvel - Ultimate Alliance 2 (USA).wbfs" "$ROOT/scratch/game"

JOBS=2 bash "$CHECKPOINT/build-linux.sh" \
  "$ROOT/scratch" "$ROOT/sdk" "$ROOT/scratch/game"
```

The original optional APT installation path was stopped in simulation because it would remove existing scratch packages. The replacement helper above authenticates and extracts packages privately; it does not install, upgrade or remove host packages. It refuses a nonempty unowned destination.

The build components and commands were executed successfully in this response. The consolidated `build-linux.sh` is assembled from those commands and syntax-checked, but a second clean **end-to-end invocation of that new wrapper** has not been performed. Some local tools were already present in this scratch environment; this is not a bootstrap script for a bare unrelated distribution.

## Repeat the bounded graphical probe

After building, CMake supplies the runtime's `Sys` directory beside its runner. The probe creates a separate user profile and requires a new output directory.

```bash
xvfb-run -a -s '-screen 0 1280x720x24 -nolisten tcp' \
  python3 "$CHECKPOINT/tools/boot_probe.py" \
  --runtime "$ROOT/scratch/build/moderngekko/moderngekko-run" \
  --module "$ROOT/scratch/build/mua2-module/gRMSE52_recomp.so" \
  --game "$ROOT/scratch/game" --output "$ROOT/probe-01" \
  --seconds 140 --press-start --capture-memory
```

The probe checks for native dispatch and video-initialization errors, not just process exit status. This matters because the initial failed video boot returned exit code zero. A passing probe still does not prove gameplay; inspect its screenshots and game state. Memory reads are deliberately limited to captured guest RAM, and no memory-write command is used.

## Replay the audits

```bash
export MUA2_GAME_ROOT="$ROOT/scratch/game"
export MUA2_RAM_ROOT="$CHECKPOINT/evidence/boot-ogl-fixed/automation"
python3 -m unittest discover -s "$CHECKPOINT/tools" -p 'test*.py' -v

python3 "$CHECKPOINT/tools/audit_live_rel.py" \
  --rel "$MUA2_GAME_ROOT/files/Marvel-rev-fin-plf2.rel" \
  --memory "$MUA2_RAM_ROOT" --base 0x80E4A080 \
  --output "$ROOT/live-rel-replay.json"
```

Use 0x80E4A080 only for the **included capture**, whose base was observed and verified. A new boot must establish its own loaded base.

## Next development target

Correct the REL loader/discovery integration using the proven halfword, BSS and linked-header behavior, add native REL coverage with validation, then progress through title/menu/gameplay. Input, audio and saves still need real tests. An optimized Windows build follows that integration; none is supplied or claimed here.
