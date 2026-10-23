# Keyboard/mouse and PC menu acceptance

Scope: the former TODO #1, completed against the Windows JIT runtime. Evidence
below covers the implemented input/menu integration, not a full-game playthrough,
physical-device acceptance, combat FPS or audible audio.

| Requirement | Evidence |
| --- | --- |
| MUA2 keyboard/mouse gameplay actions | Run 93: 17 published-action and release checks in Latveria; inspected native capture shows shield power activation. Run 97: inspected settled before/after gameplay views show mouse-drag camera movement. |
| Powers, fusions, block and repeated-use QTE mapping | PC-controls and gamepad-action tests exercise all power/partner slots, modifier separation and context guards. Button-QTE test covers press edges and holding/release behavior. No new fusion/QTE encounter completion is claimed. |
| Primary/secondary bindings, presets, profiles, sensitivity and device assignment | PC-controls tests cover validation, mapping, sensitivity/inversion, profile isolation, assignment, serialization and failure retention. Runs 27/29 verify native rebinding, live use, three presets, Apply/Cancel and saved-file preservation. |
| 720p, 1080p, 1440p and 4K | Runs 67-70 verify native selection/save/restart and actual GPU output dimensions. Runs 70-73 verify final internal scaling (2x/3x/4x/6x). Native HD captures were inspected, including the user-approved red layout. |
| Native XML2 menu adaptation | Approved Options and Advanced packages preserve native MUA2 backgrounds/animation and use recovered MUA2 callbacks. Run 99 verifies the combined installed-data candidate renders Options and completes mouse save/acknowledgment, rejects empty clicks and preserves keyboard selection. |
| Windows build and verification | Latest runner build succeeds without compiler warnings/errors; four focused CTests pass. Launcher passes 9 profile and 67 UI-gate checks; final embedded-package SHA256/help verification passes. Earlier supported suite passed 72 available enabled tests, with three unavailable dependency executables excluded and one disabled test. |

Private evidence is under `.local/pc-controls/preview-run-*`, with build/test
logs and candidate package/installation receipts alongside them. The final menu
inputs are `native-options-native-arrays` and `native-advanced-preview`; the
older `native-options-preview` is rejected and must not be packaged.

Candidate archive verification compares every installed member, replacing only
Options and adding Advanced; roster, costumes and all other data are preserved.
Installation status and hashes are authoritative in `installation-receipt.json`.
Wii terminology/glyph cleanup and Wiimote-screen replacement remain separate
user TODOs. The approved visual design is unchanged.
