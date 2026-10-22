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
The native provider update latches keyboard events once; its query and gameplay
consumers read that snapshot. This timing still needs a running-game check.

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

A Windows runtime build is in progress, with its log at
`.local/pc-controls/runtime-build.log`. No completed runtime-build or live-input
claim is made at this checkpoint. The installed executable and game data remain
unchanged. No source backups were created.
