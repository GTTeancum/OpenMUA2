# Original Xbox compatibility: survey and prioritized plan

Branch: `original-xbox` (created from `main` @ `e0f402e2`). Status: **planning, plus a
game-free nxdk smoke XBE that builds in CI** (run 42723871079, commit `74f82177`). Every
number below comes from files in this repo unless it is marked *unverified*.

## 1. What OpenMUA2 actually is

OpenMUA2 is **not a source port**. No MUA2 game source exists in this repo. It runs
the original Wii PowerPC executable (`main.dol` plus a REL module) inside an emulator
runtime:

| Layer | Location | Role |
|---|---|---|
| ModernGekko-Template | `project/Makefile` | Extract disc, recompile, build, run |
| DolRecomp | `project/lib/DolRecomp` | Static recompiler: DOL to C or LLVM objects |
| ModernGekko runtime | `project/lib/ModernGekko/src`, `include` | Recomp runtime, GX/audio/input glue, MUA2 HLE patches (`src/runtime/mua2_*.inl`) |
| RecompCore (Dolphin fork) | `project/lib/ModernGekko/vendor/dolphin` (904 MB) | CPU (Jit64, interpreter, StaticRecomp), GPU (VideoBackends), DSP/audio, DiscIO, input, netplay |
| Windows launcher | release `OpenMUA2.exe` (WinForms) | First-run disc extraction and content pack |
| Tooling | `tools/*.py`, `tests/` | Asset conversion, content pack, analysis |

The shipping configuration is **full Dolphin JIT + Vulkan** (`docs/PERFORMANCE-HISTORY.md`).
The README states `"native_rel_integrated": false`.

## 2. Platform-specific code map

| Area | Current implementation | Xbox (nxdk) equivalent |
|---|---|---|
| CPU | Dolphin Jit64 (x86-64 only), interpreter, StaticRecomp C modules | None. A 32-bit x86 guest-CPU backend would have to be written. |
| Rendering | GX emulation (`src/gpu/gx_*`, Dolphin `VideoCommon`) to Vulkan/D3D11/D3D12/OGL/Metal | pbkit/NV2A push buffers. No D3D8 in nxdk. GX TEV (16 stages) vs NV2A register combiners (8 stages). |
| Audio | DSP HLE plus `AudioCommon` (Cubeb, WASAPI, ALSA, Pulse, OpenAL) | nxdk `xaudio`/AC97 PCM output |
| Input | `src/input/controller.cpp`, `InputCommon` (XInput, SDL, DInput, Wiimote emulation) | nxdk SDL2 gamecontroller or USB XID |
| File I/O | `DiscIO` (WBFS/ISO/RVZ), extraction to `GameData`, `disc_interface.cpp` | FATX HDD / DVD via nxdk `fileapi` |
| Memory | `src/memory/address_space.cpp`: MEM1 `0x01800000` + MEM2 `0x04000000`; Dolphin `Memmap`/fastmem | 64 MiB unified, shared with the GPU |
| Threading | Dolphin CPU/GPU/audio threads (shipping config uses one logical CPU) | nxdk threads on a single core |
| Networking | Dolphin netplay, mod-loader fingerprints | Not needed. Drop it. |
| Launcher/UI | WinForms, ImGui OSD, Win32 windowing | Dashboard-launched XBE, no windowing |

## 3. Feasibility findings (blocking)

1. **Guest memory exceeds the console.** The Wii address space is 24 MiB + 64 MiB
   = 88 MiB (`address_space.hpp`). The original Xbox has 64 MiB **total**, and that
   64 MiB also holds the framebuffer and textures (UMA). MUA2 uses MEM2 actively. The
   costume work in `mua2_costumes.inl` allocates from MEM2 arenas. Even a perfect
   recompile cannot fit a guest image that is larger than the host's physical RAM.
2. **Code size.** The executable PPC text is about 8.57 MB: DOL text 5,316,160 B plus
   REL text 3,255,744 B (`docs/recovery/game-audit.json`). That is roughly 2.1 M
   instructions. Recompiled C, with its byte-swaps, CR/XER modelling and dispatch
   tables, typically expands this several times over (*expansion factor unverified for
   this game*). That native code competes for the same 64 MiB.
3. **CPU budget.** On a Ryzen 7 8745HS, full JIT averages 26.7 to 29.97 FPS with a
   23 FPS minimum, which is still not a solid 30 (`PERFORMANCE-HISTORY.md`). A 733 MHz
   Coppermine-class core is more than an order of magnitude slower. It also has
   **SSE1 only**: Gekko double-precision FPRs and paired singles would fall back to
   x87.
4. **Toolchain.** RecompCore is C++23 and hard-fails on 32-bit hosts:
   `vendor/dolphin/CMakeLists.txt` stops with `FATAL_ERROR` unless the pointer size is
   8 bytes and the CPU is x86-64 or arm64 (or `ENABLE_GENERIC`). It also assumes
   `-msse2`. The template's `make check` requires 8-byte pointers. nxdk is a
   clang/lld i386 toolchain with a partial C++ runtime. A 904 MB vendored Dolphin
   tree does not plausibly reduce to something nxdk can build.
5. **GPU.** No GX-to-NV2A translator exists. EFB copies, TEV and the indirect-texture
   paths MUA2 uses would all need new code on fixed-function-plus-combiners hardware.

**Conclusion: "make OpenMUA2's runtime run on an original Xbox" is not a viable
target.** The rest of this plan avoids sinking effort into it before cheap gates
confirm that.

## 4. Prioritized plan

### P0: Feasibility gates (cheap; decide before any porting work)
1. **Memory census (implemented; needs a run on your Windows machine).** It is an
   opt-in sampler in the runtime (`tools/memory_census.hpp`, wired into
   `dolphin_runtime.cpp`). It counts the 4 KiB pages of MEM1/MEM2 that are
   non-zero (an upper bound) and the pages that change between samples (the
   working set), plus a per-MiB occupancy map. Run it like this:

   ```bat
   set OPENMUA2_MEMORY_CENSUS=%CD%\.local\xbox\census-latveria.csv
   rem optional, default 1000, clamped 100..600000
   set OPENMUA2_MEMORY_CENSUS_INTERVAL_MS=1000
   Run.cmd
   ```

   Play from boot through the main menu and into a Latveria fight (ideally with a
   4-hero team and fusions), then exit normally. The file is written at shutdown.
   Its `# peak` header lines give the answer. One sample takes about 40 ms of
   memory reads on a separate thread, so it doesn't change game timing.
   **Decision rule:** if peak `ever_nonzero` for MEM1+MEM2 is above about 40 MiB,
   emulation and recompilation routes are closed. That figure is the Xbox's 64 MiB
   minus room for the kernel, framebuffers, textures and the recompiled code itself.
   The census measures guest RAM only. Wii textures in guest RAM double as GPU memory
   on the Wii, but on the Xbox they would be converted into a second copy unless
   aliased.
2. **Code-size probe, against a hard limit: the XBE must be under 10 MB** (maintainer
   requirement, 2026-11-09). The Xbox CI enforces this on every XBE it builds. Run
   DolRecomp's C backend (`BACKEND=c`) and compile the output with
   `clang --target=i386-pc-win32 -Os`. Record the `.text` size. This needs a disc,
   so it runs locally, not in CI. No game-derived output may be committed.
   *Expected outcome:* the input PPC text alone is about 8.57 MB. Recompiled x86
   code is never smaller than that, and is usually several times larger. With a
   runtime, initialized data (DOL data about 0.8 MB plus REL data about 0.36 MB) and
   the 10 MB cap, this gate is very likely to **fail**. Run it anyway, because one
   measured number settles the question.
3. **nxdk smoke target.** `.github/workflows/xbox-nxdk.yml` builds pinned nxdk and
   `xbox/smoke`, a game-free XBE that prints the kernel's physical memory statistics.
   It uploads `default.xbe` as an artifact. Running it on hardware or xemu gives the
   real free-RAM figure to compare with P0.1.
4. **Decide** on the outcome of steps 1 and 2.

### Decision (2026-11-10): pivot to a non-matching decomp of MUA1 Xbox
The maintainer chose to pivot: MUA1's Xbox engine becomes the base, and it is
being decompiled (non-matching: readable C, not byte-identical) so that it can be
changed to carry MUA2 content and mechanics. A matching decomp was ruled out on
effort. It would also need XDK 5849's proprietary MSVC 7.1 compiler.

- **Where:** a *private* repository of the maintainer's, not this public repo. The
  decompiled code is derived from Activision/Raven code, so it is not GPL and
  must not be published. This repo receives no game-derived code, data or
  symbols, only plan-level notes like this one.
- **How:** Ghidra 12.2 headless with custom scripts (GPL-3.0, original work).
  They handle XBE mapping, kernel imports, MSVC RTTI classes and vtables, Alchemy
  "ARK" class registrations and reflected fields, constructor/method/singleton
  recovery, XDK and third-party library separation, and the script-binding
  tables. These tools contain no game data and could be published separately,
  after a licence review, if the maintainer wants.
- **Status (static analysis only, nothing run):**
  - About 24,200 functions decompiled.
  - About 65% placed in classes, libraries or inferred source files.
  - About 1,100 C++ classes and 476 Alchemy classes with exact sizes, and an
    exact-layout type header that compiles.
  - MUA1's script API (370 bindings), the surface converted MUA2 missions would
    target.
- **Memory:** the pools seem to grow on demand rather than having fixed budgets,
  so the per-level asset budget needs a runtime measurement. Unverified.

### P1: Recommended direction if P0 confirms infeasibility: content port to the MUA1 Xbox engine
The MUA1 Xbox XBE is the reference here: its size, sections and heap layout show
what a shipped Alchemy-3.2 game on this hardware fits within the 10 MB/64 MiB
budget. Measure that before planning content, using step 1 below.

#### MUA1 Xbox reference measurement (2026-11-10)
Measured with `tools/inspect_xbe.py` on the maintainer's own copy of the XBE. The
file was used inside the automation sandbox only; it is not in this repo or in CI.
Only these header-level numbers are recorded.

| Item | Value |
|---|---|
| Title / ID | Marvel: Ultimate Alliance, `0x4156005C`, retail |
| XBE file size | 6,221,824 B (5.93 MiB), under the 10 MB budget |
| Load image (`SizeOfImage`) | 9,381,344 B (8.95 MiB), leaving about 55 MiB for kernel, heap, assets and framebuffers |
| Game `.text` (engine, Alchemy, game logic) | 4,459,476 B (4.25 MiB) |
| XDK library sections (D3D, DSOUND, XONLINE, XNET, WMADEC, XGRPH, ...) | 695,008 B |
| `.rdata` | 775,396 B |
| `.data` + BSS | 3,411,644 B (3,202,768 B zero-filled at load) |
| XDK | 5849 for every library (D3D8, DSOUND, XONLINES, XVOICE, D3DX8, XGRAPHC, ...) |
| Stack commit / heap reserve | 256 KiB / 1 MiB |

What this means:
- **The recompile route (P2) fails the budget.** MUA2's PPC text (8.57 MB) is 1.92x
  MUA1's *entire* native x86 game code (4.46 MB). Even a hypothetical 1:1
  translation, before runtime, XDK-equivalent libraries and data, already uses 86%
  of the 10 MB XBE budget. Real recompiled output is larger, so P0.2 is very likely
  to confirm the failure.
- **A shipped Alchemy-3.2 title fits comfortably.** MUA1 uses 62% of the file budget,
  and its image takes about 14% of RAM. MUA2 content on MUA1's engine (P1) inherits a
  known-good code footprint, so the open question is *asset* memory per level, not
  code size.
- **Next RE targets in this XBE, for local use only:** the Alchemy/Raven heap setup
  (how the remaining ~55 MiB is split between the Alchemy pools, texture memory and
  streaming), and the per-level asset loading path (`ravenZipFile` and the `.xmlb`/
  `.igb` loaders). Together they define the per-level asset budget that converted
  MUA2 content must meet.
MUA2 Wii assets can be back-converted from Alchemy 3.5 to MUA1 Xbox's Alchemy 3.2
with igb-blender (per the brief). That makes the most realistic Xbox deliverable a
**MUA1-Xbox-engine mod that carries MUA2 content**, not OpenMUA2's runtime.
1. Ask the user for a legally dumped **MUA1 Xbox XBE** (their own copy). Use it only
   locally, for RE of heap layout, streaming budgets and per-level memory ceilings.
   It is never committed. The first measurement is ready to run:
   `python tools/inspect_xbe.py <MUA1 default.xbe> --json > .local/xbox/mua1-xbe.json`.
   This reports load-image size, code/data/zero-fill per section, the linked XDK
   library versions, and the 10 MB budget check. It only parses public header
   structures and extracts no code or data. It has been validated against a real
   cxbe-built XBE in CI; it has not been tested on a commercial XDK-built XBE yet.
2. Build `tools/xbox_budget.py`, which checks converted packages against MUA1 Xbox's
   measured per-level texture/geometry/audio budgets.
3. Extend the asset pipeline (`tools/alchemy_batch`, `build_content_pack.py`) with an
   Xbox target. Swizzled DXT1/3/5 textures for NV2A, Xbox ADPCM audio, MUA1 XMLB
   formats.
4. Recreate MUA2 gameplay (fusions, roster rules) as MUA1 data and scripts where the
   engine allows. Document what cannot be expressed. This is a design/RE task, *scope
   unverified*.
5. Ship only xdelta-style patches and tools, never XBEs or game files.

### P2: Only if P0 unexpectedly passes: recompiled native XBE
1. Platform abstraction: split `moderngekko::runtime` from Dolphin through interfaces
   (`gpu_backend.hpp`, `audio_system.hpp`, `controller.hpp`, `disc_interface.hpp`
   already exist). Add `src/platform/{win32,nxdk}`.
2. Remove the Dolphin dependency from the StaticRecomp path. Generic C, 32-bit
   pointers, no fastmem, no JIT.
3. Re-implement the guest-memory model with a compacted MEM2 and lazy/paged regions.
4. Write a GX-to-NV2A subset renderer via pbkit (tests against captured GX FIFOs).
5. Bring up audio (DSP HLE to AC97) and input (XID through SDL2).
6. Add an Xbox CMake toolchain file and CI artifact upload.

### Always
- Do not change `main`. Keep these commits small, each with a status note in this doc.

## 5. Licensing concerns (flagged, not worked around)
- **Microsoft XDK** (official D3D8/XAPI) is proprietary and was never publicly
  licensed. Use nxdk only. Do not accept leaked XDK headers, libraries or samples.
- **MUA1 Xbox XBE and MUA2 disc/assets** are proprietary (Activision/Marvel). They are
  local RE input only. They are never committed, uploaded to CI artifacts or
  redistributed, and neither are converted game assets.
- **Alchemy (Vicarious Visions/Intrinsic) SDK** is proprietary. Tooling must stay
  reverse-engineered format readers. Do not use leaked SDK binaries.
- **igb-blender** license: *not verified*. Check before vendoring or redistributing
  any of its code.
- **nxdk** (pinned `004fefbf`) declares `Apache-2.0 AND BSD-3-Clause AND CC0-1.0 AND
  GPL-2.0-or-later AND MIT` plus `LicenseRef-NVIDIA-CG`. The first five are
  GPL-3.0-compatible. The NVIDIA one covers the bundled **proprietary NVIDIA Cg
  compiler** (`tools/cg/*/cgc`), which only compiles shaders. The smoke XBE doesn't
  use it. Any future NV2A shader work must either accept its terms (it is
  redistributable freeware, *terms not reviewed here*) or use an open shader path.
  Don't vendor `cgc` into this repo.
- **GPL obligations.** Any XBE built from RecompCore/Dolphin code is GPL-3.0 and needs
  corresponding source.
- The repo's `AGENTS.md` says never to push without authorization. Pushing to
  `original-xbox` only is authorized by the maintainer's task brief.

## 6. Not verified in this pass
- Only `xbox/smoke` was built, in GitHub Actions: Ubuntu clang 18.1.3, nxdk `004fefbf`,
  `default.xbe` 114,688 bytes, sha256
  `3cd48b59d60120108a069dbeb44cb034d350f34f9b75dc5e0dbaefbf1fa68d98`. The only build
  warning is nxdk's own lld `.edata` merge notice. The XBE has **not been run** on
  hardware or xemu.
- The OpenMUA2 runtime was not built for any platform. There is no disc and no
  Windows host in this environment.
- Raw Actions logs and artifacts are served from blob storage that the automation
  environment can't reach. The workflow therefore mirrors diagnostics into
  annotations, which are read via the REST API.
- The workflow uses Node 24 action majors (`checkout@v7`, `cache@v6`,
  `upload-artifact@v7`). These are the latest tags seen; see the latest run for
  verification.
- The PIII/NV2A performance gap is an estimate, not a measurement.
- The real MUA2 memory footprint is unknown (P0.1).
- The expansion factor for recompiled code is unknown (P0.2).
- Whether MUA1's engine can express MUA2 mechanics is unknown.

## 7. CI baseline on this branch (failures inherited from `main`)
Dispatched runs on `original-xbox` reproduce the same failures that `main` already
had. None of them involve Xbox changes:
- ModernGekko CI, Standalone tests (Ubuntu): `moderngekko.frontend_config` fails.
  The new `moderngekko.memory_census` test passes on both OSes.
- ModernGekko CI, Full build (Ubuntu): the build step fails, as on `main`.
  The Windows full build and tests pass with the census change.
- OpenMUA2 tooling CI (Ubuntu): `test_scm_revision` fails 3 times. Dolphin's
  `CMake/ScmRevGen.cmake` exits 1, which also reproduces locally. On Windows
  everything passes, including `test_inspect_xbe`.
- The ModernGekko and tooling workflows now mirror failing tests and compiler errors
  into annotations, so they can be read through the REST API.
