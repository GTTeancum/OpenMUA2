# Keyboard/mouse and native PC options implementation

TODO #1 remains open. This document records implementation evidence, not player
acceptance. Work and private assets remain inside this repository.

## Required delivery

1. Route the game window's keyboard and mouse events into MUA2's existing native
   gamepad action path. Preserve powers, partner selection, block, QTE release
   edges, menu context and single-core game execution.
2. Provide primary/secondary keyboard and controller bindings, three presets,
   four saved profiles, device assignment, sensitivity and inversion. Apply and
   Cancel must not lose the last usable settings when writing fails.
3. Adapt OpenXML1xbox's actual XML2 Options/Advanced assets and native mouse
   navigation. Author red menu assets while retaining MUA2's background and
   animation. Do not substitute an unrelated overlay or XML1 guest addresses.
4. Build the Windows JIT runtime, validate input/rebinding and inspect the native
   menu's actual appearance and behavior. Stage only a coherent candidate.
   Remove TODO #1 only after all these requirements are evidenced.

## October 22 input core checkpoint

Added `mua2_pc_controls.hpp` and `mua2_pc_settings.hpp`, with the profile,
primary/secondary, preset and draft/apply model adapted from the read-only
OpenXML1xbox reference. MUA2 actions replace XML1 health/energy/allies actions.
Direct power and fusion shortcuts feed the existing MUA2 gamepad action builder.
No game addresses, input interception or proprietary assets were added.

Input events are buffered for a single game input-frame consumer: short taps
survive polling, autorepeat is ignored, focus loss clears held/queued state,
and mouse deltas are consumed once. The runtime must share that snapshot among
all guest input readers rather than advance the queue at every read.

Settings use stable action names. Decode rejects invalid, duplicate, unknown
or conflicting fields without changing live settings. The draft model retains
applied settings on save failure. Atomic filesystem persistence remains to be
connected by the runtime adapter.

Windows Release target `moderngekko_mua2_pc_controls_test` built with the
repository's MSVC/CMake toolchain. Three focused CTests passed:
`moderngekko.mua2_pc_controls`, `moderngekko.mua2_gamepad_actions`, and
`moderngekko.button_qte`. The new test covers presets, conflict rejection,
modifier separation, all power/partner slots, menu confirm/back, focus and tap
lifetime, sensitivity/inversion, profile isolation, settings round trips,
malformed settings and failed Apply. No gameplay or visual validation yet.

CMake regeneration reported existing dependency policy deprecations, unavailable
optional Unix libraries/Wayland, and object-path length warnings. The focused
compile/link completed without errors or compiler warnings.

## Next integration steps

- Add a window-local event callback to DolphinNoGUI PlatformWin32. Current
  Space fast-forward and Escape shutdown shortcuts conflict with gameplay;
  route consumed game bindings before those shortcuts. Preserve F11 fullscreen.
- Connect a locked input snapshot and settings store to `ReadGamepadSample`.
  Preserve process-local test-device isolation, hotplug release arming and
  physical controller assignment. Do not require an XInput device for keyboard.
- Adapt native options commands, rebinding capture, pointer hit testing and
  settings Apply/Cancel to MUA2's recovered menu objects.
- Reuse the XML2 menu authoring pipeline in OpenXML1xbox, retaining package paths
  and using private copies under `.local`; convert/author the assets for MUA2's
  runtime and red palette, then validate their native rendering.
- Run the full supported Windows build and relevant regressions after integration.

## Window integration checkpoint (in progress)

PlatformWin32 now forwards its own keyboard, focus, mouse button and pointer
events to the runtime. It never injects OS input. F11 retains fullscreen; PC
bindings own Space/Escape/F1 before emulator shortcuts. Focus loss clears the
keyboard state, and pointer leave releases mouse buttons.

`ReadGamepadSample` now supports keyboard-only play, profile mapping and separate
device assignment. Physical hotplug arming happens before keyboard merging.
An explicit process-local Xbox test device still overrides only its own port.
The native provider update and pre-join query share a 30 Hz sample interval;
gameplay consumers read that snapshot. This timing still needs a running-game check.

The runtime loads `Config/OpenMUA2Controls.ini` beneath its configured user
directory; malformed settings produce a log message and use defaults. Native
menu Apply and atomic file writing are still pending. The automation protocol
has a `pc_key` command (`key=1..255`, `down=0|1`) calling the same local consumer;
it neither posts window messages nor changes desktop focus.

`tools/import_pc_menu_reference.py` copied 29 declared files (2,107,190 bytes)
from the authorized OpenXML1xbox/XBOXgame reference into
`.local/pc-controls/xml2-menu-reference`. All imported hashes were verified.
Paths and package associations are preserved. These are private authoring
inputs, including reference-only XML1 models, not installable MUA2 replacements.
Red authoring, MUA2-compatible resource conversion and native menu hooks remain.

## Build and private menu conversion checkpoint

Both the Windows Release runtime build and final incremental build completed
successfully. Binary: `.local/build/windows-x64/runtime/moderngekko-run.exe`.
Logs: `.local/pc-controls/runtime-build.log` and `incremental-build.log`.
The first build reported dependency warnings (including SDL warning-level
overrides and LZ4 inline notices); the incremental compile/link had no warnings
or errors. Four focused CTests passed: PC controls, gamepad actions, button QTE
and automation protocol. Test temporary files were confined under
`.local/pc-controls/temp`. This is not a live-input or gameplay validation claim.

`tools/convert_pc_menu_assets.py` uses the retained igb-blender complete graph
reader/writer, DXT3 decoder and Wii RGB5A3 encoder. It produced 29 candidate files
under `.local/pc-controls/red-menu-validated`, converting 46 image nodes and
recoloring 482 UI vertex colors. Original names, geometry, animation, object
identities and package paths are retained. Reference-only controller/button
artwork retains its colors. All serialized object fields and memory blocks were
read back and compared against the authored graph. Native rendering is unverified.

The first conversion stopped because its validation compared Python field
descriptor identities. The corrected check compares field slots and values.
Automatic approval review blocked deletion of that failed partial output with
"blocked by policy"; `.local/pc-controls/red-menu-candidate` remains. It is not
the candidate to test or install. No deletion workaround was attempted.

The original MUA2 options package was inspected read-only. Its native scene
contains named `background_red`, `background_white`, `Stars_And_Stripes` and
other background/animation nodes. Their private inventory and source models are
under `.local/pc-controls/retail-scene-nodes.json` and `retail-menu-models` for
integrating the new panel while retaining MUA2's background. No menu package
has been installed or tested in-game yet. The user's requested screenshot must
come from actual native rendering after that integration, not the asset files.

The installed executable and game data remain unchanged. No source backups
were created. TODO #1 remains open.

## Native preview checkpoint - 2026-10-22

Normalized the PC igTransform animation reference from field 11 to Wii field 10.
The preview packer transfers five original MUA2 background branches and animation
to the red Options scene. The 29-resource FB passes structural readback.

An isolated `.local/pc-controls/preview-game` fixture contains a separate WAD
with only Options replaced. Other immutable files are hardlinks: never edit them
through this fixture. Config, caches, logs and native captures remain in the repo.
The runtime used one logical processor, Vulkan, null audio and a 180-second
watchdog. Process-local keyboard events navigated normal main/profile menus;
native screenshots confirmed those screens. No menu was forced.

Returning from Profiles and selecting Options exited 70: invalid read 0x32a0ca78
at guest PC 0x802e8d3c. The converted panel is NOT visually verified. The last
capture, provisionally named `red-options.png`, actually shows Profiles and must
not be presented as conversion proof. One prior command-file read failed;
subsequent navigation commands succeeded. Evidence is under
`.local/pc-controls/preview-run` (runtime.log, launch.json, commands, captures).
Do not install this candidate. Diagnose native scene loading next, then inspect
the red panels and preserved background before completing menu hooks.
Installed files and real saves remain unchanged. TODO #1 remains open.

## Native asset-loader diagnosis - 2026-10-22

Windows runtime rebuilt successfully with CPU-thread register logging on fatal
automation alerts (`fault-diagnostic-build.log`, exit 0). Run 2 reproduced the
same 0x802e8d3c failure and captured registers. Converted menu assets now use
native v8 pooled strings, reserializing all objects, and normalize both the
igTransform animation slot and declared slot count. The packer rejects a scene
whose version/endian differs from the original MUA2 layout.

Adapted imported volume-slider items to MUA2's ordinary label plus native
increment/decrement commands and a gamevar-driven model. MENU_ITEM_SLIDER is
absent from the MUA2 DOL/REL strings; MENU_ITEM_LIST_CYCLE is present in the REL
and retained. Mapped XML2 text styles to MUA2 title/selectable/small-white styles.
The entire 29-file v8 conversion passes field/memory readback; the isolated WAD
was checked to contain the exact rebuilt FB. These checks do not prove rendering.

Fresh-boot runs 4, 7 and 8 still failed with a solid red screen when opening
Options. Their normal menu navigation used only process-local keyboard events.
The original invalid-read alert no longer occurs with v8, but the UI is NOT fixed.
Run 3's restored main-menu state was not used as acceptance because the archive
had changed. Runs 5/6 were stopped diagnostic profiling attempts, not FPS tests.
The profiler was disabled after process exit before runs 7/8.

The saved failure from run 8 was inspected read-only in runs 9/10, without an
archive change. Its thread context and stack show an allocation of 0xfffffffc
inside the Alchemy loader (80317630/8031b74c via 802fd150 and 802da3cc).
The loader's buffer sizes/counts exactly match `m_pda_option_focus.igb`:
entry bytes 1680/count 69, metadata bytes 3364/count 50, object bytes 1924/count
57, memory bytes 66092/count 28. This identifies the imported highlight model
as the failing load, rather than an ordinary menu navigation or memory-pool
capacity failure. The responsible serialized array field is not identified yet.
That model still uses legacy igVertexArray1_1/igGeometryAttr1_5, unlike the native
Wii menu models; investigate its array/schema conversion next.

Evidence: `.local/pc-controls/preview-run-8/failure.state`, context-5.bin;
`preview-run-9/stack.bin`; `preview-run-10/0x920ffb68.bin` and 0x9210a390.bin.
Current candidate: `red-menu-v8`, `native-options-v8`. Do not install it.
No valid converted-UI screenshot existed at that checkpoint. All test runtimes were closed.

## First visible red PC panels - 2026-10-22

Current candidates are `red-menu-native-arrays` and `native-options-native-arrays`.
Legacy arrays now convert to native typed geometry; directory string and pool
references are remapped during graph transfer. Independent readback verifies
124 reachable meshes / 2,534 triangles, original matrices and animation payloads,
with only the intended red tint differences. The original loader failure is
resolved in fresh boots.

MUA2 also needs explicit model attributes on menu items; bindings come from the
original XML2 scene Model properties. Hidden named items must remain for native
initialization (removing them caused run 13's null read). Empty text and zero
opacity suppress legacy labels. Setting animate on all model items did not fix
the missing faces and was reverted after run 15.

The actual missing-face cause was background depth. Imported panel faces around
Y=-9 were behind the native background planes at Y=-11..-94. A common +96 Y
translation of the five native backdrop anchors and their position tracks fixes
the overlap while keeping all background resources and relative animation.
Run 17's `options-result.png` shows full red PC panels with the original MUA2
background. This is visual progress, not completed PC settings: values, command
hooks, mouse navigation, advanced-page integration and HD renderer application
still require implementation and validation. Installed game remains unchanged.

## Native pointer routing - 2026-10-22 (in progress)

The game window supplies client mouse coordinates and dimensions. The rendering
thread publishes its actual viewport under the input mutex; letterbox/pillarbox
clicks are rejected. Headless tests use normalized native-capture image space
because there is no client window. This does not verify Windows client scaling.

The process-local automation command `pc_pointer` takes only `x=0..1` and
`y=0..1`; `pc_key` with `key=1` presses/releases the local left mouse button.
Neither command moves the desktop cursor or sends OS/window messages.

Pointer focus/activation currently targets Options and Advanced. It uses the
normal native menu input callback, current world bounds/camera, native focus
setter and native item Use callback. Decorative objects without commands are
excluded. Modal dialog/save state blocks the underlying menu; keyboard/controller
dialog confirmation is retained. Left-click is suppressed as generic confirm
in these menus so clicking empty space cannot activate the last focused item.
Mouse sliders/wheel behavior, complete dialog navigation, hit-area coverage and
windowed output still require validation/implementation before TODO #1 closes.

### Wheel and backdrop follow-up

pc_wheel accepts a finite nonzero delta from -16 to 16, including fractional
notches. It feeds the game's own wheel consumer; no OS wheel messages are sent.
The native directional callback adjusts the hovered item's authored left/right
commands. Live runs 46/47 verify effects-volume increments, decrements, minimum
clamping, fractional accumulation and empty-space rejection. Mouse bar/drag and
full dialog handling remain work in progress.

Generic Advanced now uses the original ordinary-menu backdrop definitions from
cw_boosts, whose five model names are checked against the retained Options scene.
Its static opacity differs from OPTIONS_MENU's animation-scene configuration.
Run 48's inspected native capture confirms the overly bright backdrop is fixed.

### Verified HD output and legacy scaling precedence

The Windows window size limit now permits the configured client dimensions plus
borders/menu. Normal-menu Apply/restart verified 1080p, 1440p and 4K outputs.
Actual dimensions come from the rendering-thread backbuffer/viewport/EFB log.
Native captures at these sizes preserve the user-approved layout and background.

The frontend config.ini internal render preference is a fallback. An explicit
in-game output choice enables automatic integral scaling, while an explicit
runtime API scale override still wins. Final-binary startup checks verify
720p/1080p/1440p internal scales of 2x/3x/4x; a full Advanced-menu run verifies
4K with 6x EFB rendering and saves 720p through Apply. See CURRENT-STATUS and
private run receipts 67-73 for exact evidence and limitations.

Native mouse confirmation dialogs and remaining gameplay controls still need
acceptance before TODO #1 closes. Visual design was approved by the user.

### Native confirmation dialog recovery (mouse path pending)

Runs 74/75 cold-booted and reached Options with ordinary contained navigation,
changed effects volume and selected the native Apply item. Run 74's native
capture was inspected: the real save-options Yes/No dialog covers Options.
The Wii wording/glyphs seen there belong to deferred TODO #4; no replacement
screen was forced. Run 75 moved Down to No, confirmed, observed the active flag
clear and returned to cw_main. This verifies the native decline branch using
keyboard events, not mouse support or saved-data persistence.

Recovered current-build layout (never reuse XML1 addresses):
- Dialog manager global 80817374, expected vtable 811983b8.
- Active slot at manager+8a0c; slots 0..6, stride 13b8, base manager+4.
- Active flag 13a9 bit 80; choices count 13a4, visible count 13a5,
  first visible row 13a6, current selection 13a7, input player 13a8.
- Choice labels at 0201 + index*81; corresponding native commands at
  070b + index*81. Observed Yes=saveloadProcess(2), No=saveloadAbort().
- Enabled-choice bitmask at 1128. Other masks 1120/1124 control native
  closing/modal behavior; preserve the native confirmation handler.
- Renderer 810125cc caches each visible row's vertical coordinate at
  112c + index*4 (store 81013308). These are NOT normalized screen coordinates.
- Manager+8a28 points to the text-layout object. Its +40c/+410 are x/y,
  +414/+418 width/height, +408 accumulated text height, +434 scene resource,
  +438 font object. Run 75 recorded x=87, y=278, width=340, height=240.
- Renderer choice loop 81013124..810135d4 invokes font draw 80253d04
  at 81013524; selected-row highlight uses the same text metrics.
- Native direction input is 8100fc00; confirmation is 810100d4. The latter
  checks controller acceptance at 81010768, validates selection and enabled
  masks, then executes the selected native command and native closure logic.
- Font draw 80253e38 rescales coordinates using the object returned by
  80017890 (static object 8060e040), vtable methods +68/+6c, then queues
  text geometry. The dialog scene/camera transform must also be accounted
  for before deriving mouse hit regions. A row Y or screenshot percentage
  alone is insufficient; do not hard-code the observed Yes/No positions.

Private evidence: preview-run-74/dialog-object.bin, dialog-manager.bin,
dialog-vtable.bin, save-confirmation.png; preview-run-75/dialog-layout.bin,
dialog.json and decline-result.json. Focused disassembly notes are under
.local/pc-controls/dialog-{input,render,update}-disasm.txt. No guest memory was
written by these inspection tests. Both processes exited normally, CPU 2,
Vulkan, No Audio Output, 180-second watchdogs, no desktop/OS input or capture.
One native image inspected. No production edit/build or mouse acceptance yet.
