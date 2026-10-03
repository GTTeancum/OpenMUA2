# OpenMUA2 Xbox controls: MUA1 Xbox 360 target

## Default Xbox layout v5 and profile migration - 2026-10-01

Generated profiles now use the MUA1 Xbox 360 common layout with MUA2 LT+face
fusion selection. Matching profile sections automatically enable the guarded
adapters per controller port, without experimental environment flags. Modified
or extra bindings do not opt a port in. Title/evaluator/candidate hash guards
remain. Unmodified generated and runtime-persisted v1-v4 files upgrade with an
exact original backup and rollback on installation failure. Customized files
remain unchanged; stored device IDs win over unrelated current selections.

Supported Windows JIT build passed 45/45 tests in 5.12s, exit 0; no compiler/linker
warnings/errors, existing CMake warnings remain. An initial build was deliberately
stopped to correct a v4-template replacement length; it is not a passing build.
Tests cover real expression evaluation, trigger threshold, modifier/release cases,
legacy backup/custom preservation and per-port recognition. An independently
saved v4 fixture guards against a self-consistent but incorrect migration template.

Five plaza runs completed without override flags: all direct hero slots, expected
LT+B fusion partner and idle cleanup, statue mission flag 1->3, persisted-v5
camera/release, and a mixed managed/custom controller profile. Only managed port0
was rebound in the mixed run; custom port1 kept its native use descriptor and
profile settings. Original v4 backups were byte-identical; reopening persisted v5
did not add another backup. One logical CPU, normal clocks, Vulkan3x, Null audio;
no screenshots/host input. Some functional probes overlapped build tail; no FPS claim.

Back's menu readiness/return timing, visual/physical-controller/audio acceptance,
fusion cancellation/revival, vertical camera and other pointer interactions remain
open. Actual untouched user profiles upgrade on the next normal launch; customized
profiles require deliberate selection of the new layout. Performance acceptance
is unchanged. Evidence/binary hashes: evidence/windows-20260930/XBOX-V5-MIGRATION.json.


## Experimental Back / Hero Management - 2026-10-01

With OPENMUA2_HERO_BUTTONS=1 and the matching experimental profile, Back now
requests native HeroManagement40. The adapter consumes its camera/pause markers
and shared menu aliases. The profile excludes Start from the camera-enable source,
so Start + right stick remains pause/resume and cannot synthesize Hero Management.
Defaults remain v4; this is not an installed physical-controller migration.

Windows JIT build passed 45/45 tests in 9.07s, exit 0; no compiler/linker warnings
or errors, existing CMake warnings remain. Live Back probes show only action40
besides the standing cursor baseline; release and port isolation pass. Gameplay
entity updates stop after Back. B alone did NOT resume, and early A then B also
failed. After an eight-second neutral wait, A then B restored entity updates;
a separate longer sequence also returned. This is evidence of a timing-sensitive
return path, not proof of menu contents or a particular confirmation dialog.
No fixed delay or forced menu exit is added. Start+right-stick regression passed.

Six routes completed with exit 0; one earlier exit route was rejected for a
duplicate output filename, then corrected in a separate run. Preserve those
functional failures; command success alone is not menu acceptance. First two
probes overlapped build tail. All use one logical CPU, normal clocks, Vulkan3x,
Null audio, no screenshots/host input. Visual menu readiness, broader button
combinations, physical controls/audio, default migration and FPS remain open.
Evidence/profile/binary hashes: evidence/windows-20260930/BACK-HERO-MANAGEMENT.json.


## Experimental camera/menu separation - 2026-10-01

The experimental hero profile restores horizontal right-stick camera input;
LT reserves that stick for aiming. With OPENMUA2_HERO_BUTTONS=1, the guarded
adapter consumes MenuViewBoosts99, MenuSwitchToPrevHero104 and MenuReallocate122
from the shared camera-enable source, preserving signed CameraX and turn inputs.
The initial private probe exposed those aliases; it was not shipped as a default.

Windows JIT build exited 0: 45/45 tests, 9.04s; no compiler/linker warnings/errors,
existing CMake warnings remain. Three subsequent plaza probes exited 0. Two
11-snapshot camera probes preserve positive/negative camera values, LT suppression,
release and port isolation without the three menu aliases. The 11-snapshot paused
probe keeps entity position/health frozen during stick input, then resumes their
updates after Start. The final run used the corrected checked-in profile; parsed
persisted bindings match (comments/whitespace are normalized on serialization).

One logical CPU, normal clocks, Vulkan3x, Null audio; no screenshots or host input.
The first two runs overlapped build tail; none is FPS evidence. Native routing and
entity observations do not establish visual camera/menu correctness or physical
controller/audio acceptance. Defaults remain v4. Back/hero management, vertical
camera support, broader menu/fusion interactions and default migration remain open.
Evidence/profile/binary hashes: evidence/windows-20260930/CAMERA-MENU-ADAPTER.json.


The user's 2026-10-01 correction supersedes the earlier PS2-derived target and
v4 profile. Successful input-routing probes do not establish a correct layout.
Generated profiles now use v5 and matching ports enable guarded adapters automatically.
The dated checkpoints below retain the migration history and remaining acceptance work.

## Required common controls

| Input | Required behavior |
| --- | --- |
| Left stick | Move |
| Right stick | Camera, with context-specific aiming only when appropriate |
| A | Light attack; menu confirm |
| B | Heavy attack, hold to charge; menu back |
| X | Grab/pick up or contextual use, selected by game context |
| Y | Jump and character traversal |
| LB | Block/dodge; must not activate contextual use |
| RT + A/B/X/Y | Four powers; suppress ordinary face-button actions |
| D-pad | Direct hero selection in gameplay; navigation in menus |
| Start/Menu | Pause and resume |
| Back/View | Hero management target; verify the available game action |

MUA1 sources: [Xbox 360 gameplay description](https://drkwaitingroom.com/2023/04/28/game-corner-marvelua/)
and [control reference](https://strategywiki.org/wiki/Marvel%3A_Ultimate_Alliance/Controls).
The latter has incomplete Xbox columns; do not use it alone to assert unverified
Xbox-specific auxiliary bindings. RB/map and stick-click behavior need a verified
Xbox reference and a matching action in this version before being assigned.

## MUA2 additions

Fusion and partner selection need an additional binding. Proposed extension:
Hold LT, then press A/B/X/Y for the teammate indicated by the button prompt. This replaces
MUA1's team-command use of LT. The user requested this MUA2 Xbox 360
pattern, confirmed by the [Xbox 360 gameplay reference](https://drkwaitingroom.com/2024/04/28/game-corner-marvel-alliance2/). Native partner selection now has opt-in numeric validation; cancellation,
revival and full-flow acceptance remain open. Do not substitute trigger cycling.

In this Wii-derived version, revival uses fusion selection with a fallen hero.
Keep it within the same selection flow; do not import the separate health-pack
mechanic from MUA2 Xbox 360. [Wii-specific revival reference](https://www.cheatcc.com/articles/marvel-ultimate-alliance-2-review-for-nintendo-wii-wii/).

Hacking and any pointer/gesture interactions require contextual stick/button
adapters, not physical motion controls. Hacking and fusion are new MUA2 features;
boosts belong in menus rather than requiring a new combat button.
[Official guide, Wii/PS2/PSP section](https://ptgmedia.pearsoncmg.com/images/9780744010879/samplepages/1087-9_MUA2.pdf).
Exact hacking and gesture replacements remain unverified.

## Implementation sequence

1. Establish a guarded per-player game-action boundary and menu/gameplay context.
   Current profile expressions combine grab from two attack inputs and share
   one input between block and use. A global remap cannot separate these.
   Never use transient stack references as ownership guards.
2. Implement X context priority and LB block independently at that boundary.
   Preserve the game's per-player action-consumption/deduplication rules.
3. Resolve ordered team slots to eligible actors and implement direct D-pad
   selection. The observed control flag and team lookup are research leads,
   not a verified multiplayer ownership contract. Handle dead/absent heroes,
   already-selected heroes and heroes controlled by another player explicitly.
4. Move powers to RT and integrate the proposed LT fusion/revival flow, including
   selection cancel/release. Preserve ordinary attacks outside modifier contexts.
   Keep menu navigation, confirm/back, pause/resume and hero management coherent.
5. Add exact v4 generated/persisted profile migration with backup and rollback;
   preserve customized profiles. Update the bottom quick reference only when
   implemented behavior is established, and label all remaining limitations.

## Validation and release gates

- Real expression-parser tests for modifier priority, threshold boundaries,
  release, simultaneous inputs and independent controller ports.
- Game-action adapter tests for context, ownership and selection eligibility;
  tests must exercise behavior rather than mirror mapping strings.
- Supported Windows build through Build.cmd --cpu jit --jobs 2 and native tests.
- Bounded process-local tutorial-plaza validation: no host input, no screenshots,
  one logical CPU, normal clocks, existing Vulkan 3x rendering configuration.
- Confirm observable game behavior, not merely active bits or rendered frames.
  Visual/menu interaction, physical controller and audible quality remain
  unverified wherever the allowed evidence cannot establish them.
- Keep proprietary code, memory dumps, states and generated output in .local.
  Do not add hard-coded player addresses without game/version and object guards.

No new FPS sweep is authorized by this plan. The performance goal remains open;
the latest concrete user priorities are audio and corrected Xbox controls.

## Experimental implementation checkpoint

OPENMUA2_CONTEXT_X=1 now guards and rebinds the native use descriptor to the
grab chord. Two plaza routing probes confirm X grab/use and LB block separation
when enabled, unchanged v4 when disabled, release and port isolation. Actual
statue interaction testing fails for experimental X while stock LB succeeds.
Do not enable by default or call step 2 complete from action bits alone.
See CONTEXT-X-EXPERIMENT.json
in evidence/windows-20260930.

Post-evaluator normalization now passes two X statue interactions and the
14-snapshot value/routing probe. Two default-off LB comparisons failed; their
cause remains unresolved. Keep the experiment off and the full remap open.
See evidence/windows-20260930/CONTEXT-X-NORMALIZED.json.

## Experimental context-aware button gestures - 2026-10-01

The opt-in OPENMUA2_CONTEXT_X=1 adapter now replaces shake/lift with A/B only
inside a verified human-owned co-op interaction. X enters using the existing
normalized grab/use chord. Repeated A presses supply shake edges; B supplies
lift. The corresponding light/heavy attacks are consumed inside this context.
The adapter checks live handles, owner, actor/node/target types and current
string generation. It remains OFF by default; v4 default controls are unchanged.

Supported Windows JIT build passed: 43/43 tests, 8.08s, exit 0. No compiler/linker
warnings or errors; existing CMake warnings remain. Two original-save plaza runs
completed the statue interaction (mission flag 1 -> 3), with context entered and
exited. Nine input snapshots per run confirm gesture routing and release. A
separate 14-snapshot probe preserves ordinary A attack outside the interaction,
release and port isolation; it does not establish the complete Xbox layout.

Only process-local input was used, on one logical CPU with normal clocks,
Vulkan 3x and Null audio. No screenshots, visual/audio acceptance or FPS sweep.
Other interactions, menus, multiplayer and holding inputs across context exit
remain unverified. Historical default-off LB failures remain unresolved.
LT + indicated face button remains the fusion target; this patch does not
implement fusion selection. Full control migration, audio and performance remain
open. Evidence and binary hashes: evidence/windows-20260930/CONTEXT-BUTTON-GESTURES.json.

## RT/LT and direct-selection research - 2026-10-01

A disposable profile routes RT+face to all four powers. The corrected 15-snapshot
probe passes modifier suppression, release and port isolation. An initial
both-trigger Block/PowerShift2 leak was found and corrected in that private
profile. These are routing results, not power animation/effect acceptance.

LT enters the native fusion-request state in two plaza runs. Neither release nor
subsequent B cancels it in the prepared tutorial save. Partner selection and
fusion completion are not established. The ordered native four-actor roster is
now identified and matched against live actor handles/types; target/sequence
encoding is located, but its complete eligibility/consumption contract remains
under investigation. Fixed pointer corners must not substitute for direct selection.

Production defaults and source binaries are unchanged. No rebuild was needed;
the preceding 43/43 result applies to dd302e0a. One logical CPU, Vulkan3x, normal
clocks, Null audio, process-local input and read-only memory probes; no screenshots,
visual/audio acceptance or new FPS claim. Next: native partner selection/cancel,
then profile integration/migration. Evidence: evidence/windows-20260930/TRIGGER-FUSION-RESEARCH.json.

## Experimental native fusion partner selection - 2026-10-01

OPENMUA2_FUSION_BUTTONS=1 now substitutes a validated roster actor at the native
picker boundary, preserving the game's subsequent eligibility/resource processing.
It requires the matching experimental marker profile; it is OFF by default and
must not be treated as a completed v4/custom-profile migration. A/B/X/Y address
ordered roster slots 0/1/2/3; self and other human-controlled actors are rejected.
No host selection cache is retained. Title/code, input ownership and live full
entity handles are checked. Runtime start hooks now support multiple addresses.

Windows build passed: 44/44 tests, 7.76s, no compiler/linker warnings or errors;
existing CMake warnings remain. In the prepared plaza team, LT+A/B/Y selected
three distinct expected teammates and returned to idle. LT+X (self) selected no
partner. All four runs passed marker/power suppression, release and port-isolation
checks. The existing X/A/B statue interaction still completed its mission flag.

The first B attempt failed because its profile suppressed tutorial confirmation.
The corrected test fixture keeps manual aiming/confirmation separate from native
partner selection. Fixture: project/lib/ModernGekko/tests/data/mua1_fusion_experimental.ini.
This is a process-local test-device profile, not the user's installed layout.

Release-to-cancel, revival, other teams, multiplayer, menu polish and default
profile migration remain open. Numeric selection/state evidence does not verify
animations, effects or audible output. One logical CPU, normal clocks, Vulkan3x,
Null audio; no screenshots, host input or new FPS claim. Evidence and binary
hashes: evidence/windows-20260930/NATIVE-FUSION-BUTTONS.json.

## Fusion timeout and retry evidence - 2026-10-01

Two new prepared-plaza runs distinguish delayed native exit from release-to-cancel.
After the tutorial Ready confirmation, releasing LT left phase 1 at the 500ms
snapshot and reached phase 0 later. Holding LT for 6500ms also reached phase 0.
Both retained request_owner; that field alone is not evidence of a stuck request.
A subsequent LT request advanced the sequence from 1 to 2, LT+B selected the
expected Iron Man actor, and the native flow returned to phase 0 with owner cleared.
Immediate release cancellation is still unimplemented/unverified. Do not clear
native globals merely because the owner remains populated during phase 0.

Both corrected routes completed with exit 0. An initial hold/retry route was
rejected for a boolean button value; the numeric-input correction used a new
output directory. No runtime source changes or rebuild: the previous 44/44
Windows result still applies to 539b16a8. Single logical CPU, normal clocks,
Vulkan 3x, Null audio, process-local input; no screenshots or visual/audio/FPS
acceptance. Full Xbox migration and the performance goal remain open.
Evidence: evidence/windows-20260930/FUSION-TIMEOUT-RETRY.json.

## Experimental direct D-pad hero selection - 2026-10-01

OPENMUA2_HERO_BUTTONS=1 connects an experimental D-pad profile to the native
single-index hero handoff. Up/Right/Down/Left select ordered roster slots 0/1/2/3.
The adapter validates title/code, per-player input ownership, live full handles,
and the current caller's actor array. It changes the requested index and limits
the native search to one attempt; the game retains eligibility and handoff logic.
No temporary actor-array pointer is retained. Default profiles remain v4.

The supported Windows JIT build passed 45/45 tests in 7.66s, exit 0, with no
compiler/linker warnings or errors; existing CMake warnings remain. The final
21-snapshot plaza probe selected all four expected heroes, left self/diagonal
requests unchanged, preserved selection during a hold, suppressed gameplay power
bits, and returned to the initial CursorY-only baseline on release. Only injected
port 0 responded. An initial synthetic-plus menu exit/skill-point leak was fixed;
those actions are absent in the repeat. LT+B fusion also selected Iron Man and
returned to idle with the request owner cleared while the new observer was enabled.

Fixture: project/lib/ModernGekko/tests/data/mua1_hero_experimental.ini (process-local
test device; requires both HERO_BUTTONS and FUSION_BUTTONS flags, not an installed
physical-controller profile). Tests used one logical CPU, normal clocks, Vulkan3x,
Null audio, no screenshots or host input. Numeric native behavior is not visual,
audible, physical-controller or full menu validation. Dead/co-op cases, default
migration, camera/menu integration, cancellation, audio and FPS acceptance remain
open. Evidence/binary hashes: evidence/windows-20260930/DIRECT-HERO-BUTTONS.json.

## Repeated-X QTE replacement - development 2026-10-03

Motion QTEs must use repeated X presses to advance and complete the interaction.
Holding X counts once; no separate B finish and no button-to-shake translation.
The opt-in runtime integration builds and passes its focused regressions, but
actual cooperative completion/cleanup is not yet verified or installed. The
provisional development quota is twelve presses. Seven main HUD motion icons
have a private Xbox X candidate; other Wii UI glyphs remain outstanding.

## Installed safeguard quick reference - 2026-10-03

Statue safeguard QTE: repeatedly press X (12 separate presses). Holding counts
once. No B finish. The Xbox X prompt and matching tutorial text are installed.
Native completion, mission cleanup and resumed movement passed in a seeded
headless fixture. Other QTE variants and full Xbox glyph conversion remain open.

## Xbox menu prompt development checkpoint - 2026-10-03

The new opt-in OPENMUA2_XBOX_GLYPHS=1 observer resolves common Xbox prompts by
semantic action instead of the original Wii descriptor. Contextual Grab/Action
uses X; menu accept/back uses A/B; block uses LB, jump uses Y and pause uses Start.
It is restricted to managed controller ports, exact code hashes and the verified
controller type. Unknown tokens retain native behavior. This is not a complete
power/fusion/gesture UI conversion and is not enabled in the installed game.

The font compiler tools/build_xbox_menu_fonts.py produces private English normal
and widescreen menu font candidates from the supplied Xbox artwork. It guards
both texture and coordinate-table hashes, preserves all other bytes and refuses
existing output directories. Six glyphs per font were decoded and individually
inspected. The output is not installed and must accompany the runtime mapping.

Windows Build.cmd --cpu jit --jobs 2 passed 48/48 tests in 9.68 seconds. An initial
compile failed on a diagnostic state.lr reference; it was removed before the
successful build. Existing vendored CMake warnings remain. A headless native
repeat exited 0 and observed MenuAccept -> A and MENU_BACK -> B. Contextual X is
covered by the focused regression, but has not yet been observed as a displayed
prompt in the native flow. One prior automation command-file open failure was
retained; it occurred before parsing or input and the fresh full repeat passed.
No screenshots, host input, physical-controller, audio or FPS acceptance.

The installed EXE remains a3740b32dd3be5f8f1cac94c4fbb2fc14395dc6f6ae6dbea477cab5978fb1d3a.
Actual saves and installed assets were not touched. Existing unrelated static
recompiler edits remain preserved and included in this local build. Evidence:
evidence/windows-20261003/XBOX-PROMPT-DEVELOPMENT.json. Removed 272,241 bytes of
verified duplicate candidate fonts; previously rejected deletions were not retried.
Continue the full controls/QTE/UI goal; this checkpoint does not satisfy it.

## Native contextual Xbox X prompt correction - 2026-10-03

A seeded statue approach exposed the actual uppercase ACTION token (action 21).
The first opt-in prompt mapping missed that alias and still returned A. The
mapping and regression now include ACTION. A repeated headless native run exited 0
and traced ACTION:2->6 (Xbox X), MenuAccept:2->2 and MENU_BACK:3->3; X entered the
QTE. The mission trigger was enabled only in diagnostic memory. An earlier
attempt changing unrelated prompt literals did not exercise Action and remains
a failed fixture, not acceptance.

The corrected Windows build passed 48/48 tests in 17.46 seconds, exit 0. Existing
vendored CMake warnings remain; no compiler/linker errors. This remains opt-in
and uninstalled: fonts are separately inspected private candidates. No in-game
visual, physical-controller, audio or FPS acceptance. Full UI/QTE scope remains
open. Evidence: evidence/windows-20261003/XBOX-ACTION-PROMPT.json.

Removed the redundant private QTE WAD only after matching its SHA256 to the
installed copy; reclaimed 281,821,811 bytes. Original data and installed WAD/saves
are preserved. Previously rejected cleanup targets were not retried.

## Xbox tutorial and combined UI candidate - 2026-10-03

The private --xbox-ui asset compiler now combines the seven QTE X cells, both
common Xbox menu fonts and twelve updated Rev tutorial instructions. Grab uses
contextual X; powers describe RT with A/X/B/Y; hero selection uses the D-pad;
fusion describes LT with a partner face button; horizontal camera control uses
the right stick. Trigger and face names in the new power/fusion text are literal
labels pending the remaining auxiliary glyph work.

Five private-data Python regressions pass. All three standalone tip tables and
both packaged copies retain their original sizes. The compiler separates a
shared Rev/PSP fusion string into reclaimed string padding without changing the
PSP entry. Initial shared-string and allocation-overflow checks failed safely;
the corrected conversion passes. Independent full archive comparison confirms
exactly seven changed members and 1,041 byte-identical members; archive CRC
validation passes. Original archive and installed EXE hashes remain unchanged.

This is NOT installed or native/rendered UI acceptance. No C++ changed, so the
previous 48/48 Windows build remains the latest runtime build. Unverified camera
reset, quick-assign and throw instructions remain open, as do the full UI/QTE and
physical-controller acceptance requirements. Evidence:
evidence/windows-20261003/XBOX-TUTORIAL-CANDIDATE.json.
