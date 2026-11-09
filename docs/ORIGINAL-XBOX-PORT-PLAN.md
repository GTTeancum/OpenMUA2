# Original Xbox compatibility: survey and prioritized plan

Branch: `original-xbox` (created from `main` @ `e0f402e2`). Status: **planning only, no
Xbox code yet.** Every number below comes from files in this repo unless it is marked
*unverified*.

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
1. **Memory census.** Add an opt-in runtime counter that records the high-water mark
   of touched MEM1/MEM2 pages and the arena use during a Latveria level, on Windows.
   This shows the game's real footprint against the 88 MiB maximum. If it is above
   about 40 MiB, emulation and recompilation paths are closed.
2. **Code-size probe.** Run DolRecomp's C backend (`BACKEND=c`) and compile the
   output with `clang --target=i386-pc-win32 -Os`. Record the `.text` size. This needs
   a disc, so it runs locally, not in CI. No game-derived output may be committed.
3. **nxdk smoke target.** `.github/workflows/xbox-nxdk.yml` builds pinned nxdk and
   `xbox/smoke`, a game-free XBE that prints the kernel's physical memory statistics.
   It uploads `default.xbe` as an artifact. Running it on hardware or xemu gives the
   real free-RAM figure to compare with P0.1.
4. **Decide** on the outcome of steps 1 and 2.

### P1: Recommended direction if P0 confirms infeasibility: content port to the MUA1 Xbox engine
MUA2 Wii assets can be back-converted from Alchemy 3.5 to MUA1 Xbox's Alchemy 3.2
with igb-blender (per the brief). That makes the most realistic Xbox deliverable a
**MUA1-Xbox-engine mod that carries MUA2 content**, not OpenMUA2's runtime.
1. Ask the user for a legally dumped **MUA1 Xbox XBE** (their own copy). Use it only
   locally, for RE of heap layout, streaming budgets and per-level memory ceilings.
   It is never committed.
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
- Nothing was built or run. There is no disc and no Windows host, and GitHub push
  auth was unavailable when this was written.
- The PIII/NV2A performance gap is an estimate, not a measurement.
- The real MUA2 memory footprint is unknown (P0.1).
- The expansion factor for recompiled code is unknown (P0.2).
- Whether MUA1's engine can express MUA2 mechanics is unknown.
