# MUA2 CANONICAL HANDOFF — Wii Marvel: Ultimate Alliance 2 native PC recompilation


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


## Experimental Start pause/resume - 2026-10-01

The guarded input observer now maps Start to native Pause39 and MenuBack90 when
OPENMUA2_HERO_BUTTONS=1. It suppresses attack/grab/use, interaction shake/lift and
hero-menu actions during Start. The experimental profile still maps only Start to
the native pause source; no synthetic B source is needed. Defaults remain v4.

Two single-logical-CPU, normal-clock, Vulkan3x plaza probes exited 0. Guarded
entity position/health changed before pause, froze between paused observations,
and changed again after the second Start. Both 200ms Start and 1200ms Start+A
worked; active input returned to CursorY-only baseline on release, other ports
remained inactive, and Start carried no attack/use or hero-menu action. Start+A
retains native MenuAccept89; the first analysis assertion was too narrow and was
corrected after inspecting that input. The discarded synthetic-B prototype leaked
SmashAttack10. Null audio, no screenshots or host input. Numeric evidence does not
verify visual menu behavior, audible quality, physical controllers or combat FPS.

Supported Windows JIT build exited 0: 45/45 native tests, 8.38s. No compiler/linker
warnings or errors; existing CMake configuration warnings remain. The first probe
overlapped the build tail and is not performance evidence. Full control migration,
other simultaneous menu/fusion/interaction inputs, audio and FPS acceptance remain
open. Evidence/binary hashes: evidence/windows-20260930/START-MENU-ADAPTER.json.

## Audio queue trims and native output log - 2026-10-01

Added opt-in counters for queue-trim events, discarded granules and Running/not-
Running attribution. The analyzer preserves older logs as lacking trim evidence
and rejects inconsistent new counters. Playback, buffer limits and defaults are
unchanged; this is diagnostic work, not an audible crackling fix.

Native application logs show WASAPI requested/mix formats both at stereo 48kHz,
a 1056-frame (22ms) render buffer, and no logged device reinitialization/failure.
The instrumented plaza route had three trims per main channel, confined to buckets
0 and 56. Buckets 1-55 had neither queue trims nor empty reads. Replacing its closing
pause/resume sequence with equal-duration neutral gameplay left only bucket-0
trims and ZERO empty reads while Core was Running. All later empty reads in that
run were not-Running. This supports a transition association, not exact timing or
an audible-clean claim. Do not enlarge buffers or change clocks from these counts.

All three runs exited 0. The no-menu run decoded 16 guarded entity probes (at least
three living heroes, 4-7 positive-health opponents), captured 57.772s of private
pre-volume PCM with zero clipped samples, and recorded maximum callback gap 11.1441ms
and work 1.6604ms. Full JIT, one logical CPU, normal clocks, Vulkan3x, muted Cubeb;
no host input, screenshots or FPS measurements. Physical output used during the
user's earlier crackling test has not yet been confirmed. Listening/device output,
full control migration and combat performance acceptance remain open.

Windows build: 45/45 native tests, 5.87s, exit 0; Python: 169 passed, 1 skipped (170 run,
42.995s). No compiler/linker warnings/errors; existing CMake warnings remain. An
initial build was deliberately stopped to correct trim-only bucket retention.
Evidence and binary hashes: evidence/windows-20260930/AUDIO-QUEUE-TRIMS.json.


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


## Experimental X value normalization - 2026-10-01

The guarded adapter now runs after evaluation and preserves the original active
bits while converting the accepted use chord to digital 1 and inactive partial
chords to 0. It remains opt-in and OFF by default. Unexpected output is rejected.
Windows build passed, 42/42 tests in 6.47s, no compiler/linker warnings or errors;
existing CMake warnings remain. No changes to the single-core rendering preset.

Two normalized X plaza runs completed the statue interaction. A 14-snapshot
probe confirmed X use value 1, zero on partial/released inputs and other ports,
with expected action routing. However, two current default-off LB comparisons
failed despite earlier stock success. This unresolved comparison prevents a
reliability/regression-free claim. Do not enable by default or call the full
Xbox remap complete. Default profiles remain v4; LT fusion target is unchanged.

No screenshots, host input, visual/audio acceptance or new FPS sweep. Null audio
was used. Details and binary hashes: evidence/windows-20260930/CONTEXT-X-NORMALIZED.json.
Audio, remaining controls and the sustained performance goal remain unfinished.


## Contextual X behavioral failure - 2026-10-01

The same statue-use-ready plaza state now has a guarded, read-only mission
completion check. Stock LB completes the interaction; experimental X does not.
Adding the original block input to X also fails. The adapter remains OFF by
default. Passing active-action bits did not establish working contextual use.

Four bounded routes completed with exit 0 using the existing 36c73706 binary:
stock LB, experimental X, experimental LB negative control, and X plus block.
Only stock LB set the script completion flag. These are behavioral results from
mission state, not visual verification. Null audio, one logical CPU, normal
clocks, Vulkan 3x; no screenshots, host input or new FPS sweep. No rebuild was
needed for this evidence/documentation checkpoint. Existing 42/42 build tests
remain scoped to the previous build. Audio and full Xbox remapping remain open.

The next control fix must resolve native context/consumption behavior; neither
simultaneous grab/use nor binding-value magnitude is yet proven as the cause.
Evidence: evidence/windows-20260930/CONTEXT-X-EXPERIMENT.json.


## Live Xbox action routing probe - 2026-10-01

## Experimental X-use separation - 2026-10-01

Added an opt-in action-descriptor adapter (OPENMUA2_CONTEXT_X=1, default OFF).
It verifies executable/live-code identity and the current input object's layout
before rebinding use to X's grab chord. The original game evaluator and action
queues still run. Unexpected descriptors are left unchanged; repeated application
is idempotent. A runtime-owned start observer is cleared at title shutdown and
retained across hook reload; settings-reload behavior is not separately tested.

Supported Windows build passed: 42/42 native tests, 6.05s, exit 0; no compiler or
linker warnings/errors. Existing CMake warnings remain. Two plaza routing probes
passed with 14 guarded snapshots each: enabled X -> grab/use, LB -> block only;
disabled retains v4. Tested actions clear on release, only injected port 0 responds.
Normal clocks, CPU2/mask4, Vulkan3x, Null audio, no screenshots or host input.
These are routing results, not completed interaction, visual, audio or FPS proof.

The first live attempt rejected the executable due to digest text case, and its
fixed guest start was rejected too. Both failures remain in private logs. Corrected
the digest comparison; successful routing runs start immediately. Full evidence
and binary hashes: evidence/windows-20260930/CONTEXT-X-EXPERIMENT.json.

Fusion target is corrected to MUA2 Xbox 360: hold LT plus the indicated partner's
A/B/X/Y button, as requested. This and RT powers/direct D-pad selection remain
unfinished. Default controls remain v4; performance goal remains unmet.

## Corrected control target - 2026-10-01

The user rejected v4's routing as the desired layout and specified MUA1 Xbox 360
as the template. RT must modify powers, D-pad must select heroes directly, X must
handle contextual use/grab, and LB must block/dodge independently. The proposed
MUA2 extension is LT fusion/partner selection, including fallen-hero revival.
The current binary remains v4; no completed-remap claim. Implementation and
validation requirements are in docs/MUA1-XBOX-CONTROL-PLAN.md. Audio/controls
remain the priority; no renewed FPS sweep. No runtime change or rebuild here.

## Hero ownership research - 2026-10-01

A bounded plaza route used eight RT presses and nine identity-guarded entity
snapshots. All four heroes remained alive. One candidate control flag followed
one hero through two identical four-hero cycles; code inspection also found it
used when counting controlled team actors. An indexed team-identity lookup was
identified, but its identities are not yet mapped to live fixed D-pad slots.
This is research evidence, not a finished ownership or direct-selection adapter.
Dead-hero and multiplayer behavior remain untested. Contextual X remains open.

Harness/native exit 0; 4.5 guest seconds, CPU2/mask4, Vulkan3x, normal clocks,
silent Null audio. No source change, rebuild, screenshots, host input, guest
memory writes, visual/audio correctness or FPS acceptance. Existing df47dfab
binary retained. Raw data stays private; aggregate evidence:
evidence/windows-20260930/XBOX-HERO-OWNERSHIP-RESEARCH.json.
Audio/controls remain the active priority; no renewed FPS sweep.

A bounded tutorial-plaza probe confirmed the input layer's active-action bits:
A -> light attack; X -> grab; LB -> block AND contextual action; RT -> next hero;
LT -> previous hero; D-pad Up -> power 4. Only the object corresponding to
injected controller port 0 responded. Tested actions cleared on every release.
Partial nonzero binding sums are not activated actions; the active bitset was
checked separately. This confirms routing, not animations or selected heroes.

Four input objects were found in the existing save and checked in execution.
The first probe cancelled on a transient saved pointer guard. A read-only
follow-up confirmed object/vtable addresses but disproved that pointer's
stability. The successful route guarded each object's vtable and first two
descriptor IDs/names before every snapshot: 14 snapshots, 12 guards each,
5.6 guest seconds. An intermediate pointer-only guest sequence was rejected
because timed input was required; corrected with a brief neutral hold.

No mapping/source changes or rebuild. Used the df47dfab Windows binary (41/41
native tests at that build). Successful harness/native exits 0. CPU2/mask4,
normal clocks, Vulkan3x, default formatter, silent Null audio, no screenshots,
host input, guest writes, frame trace or FPS acceptance. Raw research and probes
remain private. Evidence: evidence/windows-20260930/XBOX-LIVE-ACTION-ROUTING.json.

Contextual X and direct hero-slot selection remain open. The next implementation
must integrate with per-player game actions/context rather than assume a raw
D-pad remap selects a hero. No claim that the full performance goal is complete.


## Xbox rumble and persisted profile migration - 2026-10-01

Fixed a concrete output mismatch: generated profiles used Wiimote-only Motor,
while SDL/XInput gamepads expose Motor L and Motor R. Profile v4 routes rumble
to both. A process-local output test through the real expression parser verifies
strength and release on both motors; the old expression resolves zero outputs.
No physical controller is driven. Physical/game-triggered vibration is unverified.

The runtime saves INI profiles without the generated banner or empty sections.
The previous exact-template upgrade detector consequently missed its own saved
profiles. It now accepts both exact generated and known persisted forms for
v1-v3, preserving device selections and exact .pre-v4-*.bak backups. Modified
bindings or extra comments remain untouched. All six forms are regression-tested;
a copy of the actual prior run's v3 profile also upgraded with an exact backup.
Legacy templates contain only our own controller configuration, not game assets.

Replaced the bottom quick-reference table in docs/PERFORMANCE-GOAL.md with the
implemented layout. X grabs; LB handles the tested statue use; LT/RT cycle
heroes; D-pad is menus/direct powers; Start pauses and B resumes. Contextual X,
direct D-pad hero selection and full fusion/revive remain unfinished.

Supported Windows build passed: 41/41 native tests, 9.02s. Existing CMake warnings
remain; no compiler/linker warnings/errors. No game launch, screenshot, host
input, new audio/FPS measurement or acceptance claim. Full binary hashes and
scoped evidence: evidence/windows-20260930/XBOX-RUMBLE-PROFILE-V4.json.
The performance goal remains unmet; user priority remains audio/controls.


## Audio running-state attribution and B resume - 2026-10-01

Added opt-in diagnostic counters for empty mixer reads while Core is Running
versus not Running, reusing the existing state read. Playback and defaults are
unchanged. The analyzer accepts old profiles without inventing state evidence,
and requires new state counts to sum to the retained totals.

One plaza route completed: 50 DMA and 76 music empty reads in final bucket 57
were ALL observed while the core was not Running. During Running: ZERO DMA and
ONE music empty read (second 53). The prior uninstrumented run cannot be labeled
retroactively. In-game menus still count as Running; the remaining music event
cannot yet be precisely assigned to combat or a menu. No audible-clean claim.
Maximum Cubeb callback gap: 11.5406ms. Private pre-volume PCM: 57.132s, peak30846,
zero clipped samples. These numbers do not establish device playback quality.

Start paused entity activity; B resumed it (eight position and one health
changes between the resumed probes). All 15 guarded combat probes decoded,
living heroes minimum three, identified positive-health opponents 4-7. Profile
migration again retained an exact backup. Native game and harness exited 0.
No game left running. No host input, screenshots, visual/listening assessment,
physical controller operation or FPS measurements. Same CPU2/mask4, Vulkan3x,
normal clocks and default formatter; JIT budget/reserve OFF.

Supported Windows build passed 41/41 native tests (6.45s). Python: 167 passed,
one skipped (168 run, 37.962s). An initial incomplete call-site patch failed
compilation; corrected and rebuilt. Final compile/link had no warnings/errors;
existing CMake warnings remain. Binary hashes and all scoped results:
evidence/windows-20260930/AUDIO-CORE-STATE.json.

Audio is not broadly starved in this bounded run; avoid treating shutdown reads
as combat dropouts. Listening/device playback and remaining Xbox contextual-use,
hero selection and fusion/revive gaps remain open. The performance goal remains
unmet. Continue the user's audio/controls priority without FPS sweeps.


## Audio counter interpretation correction - 2026-10-01

Read-only analysis of the last plaza profile shows NO main-channel empty reads
in complete seconds 0-52. DMA and music each have one empty read in second 53.
The other 61 DMA / 92 music reads occur in final bucket 57. Prior totals were
correct, but must not be presented as that many combat dropouts. Each count is
a granule read, not an audible event. The final burst remains in the evidence;
its coincidence with the end does not prove shutdown caused it. Guest probes
lack host timestamps, so precise combat/menu/stop attribution remains unknown.

Added tools/analyze_audio_profile.py to retain total and final-bucket counts
separately, with explicit coverage checks. Missing/duplicate rows and mixed
clock origins fail rather than silently becoming zero underruns. It neither
measures FPS nor establishes audible quality. Existing PCM and profile remain
private; the aggregate and profile hash are in AUDIO-CONTROLS-CONTINUITY.json.

Python regression suite: 164 passed, one skipped (165 run, 22.662s), including
seven new audio analysis tests.

No runtime source change, rebuild, game launch, capture or FPS sweep in this
checkpoint. The prior runner hash remains verified unchanged. Audio listening,
device-path behavior and remaining Xbox control gaps are still open; the full
performance goal remains unmet. Continue audio/controls, per user direction.


## Audio continuity and Xbox profile upgrade - 2026-10-01

Current user priority: audio crackling and Xbox controls/menus. Stop the previous
FPS hiccup investigation and comparison sweeps. The 30 FPS goal remains unmet.

Continuous DMA/music audio now primes once to half the configured buffer before
consuming samples. At the default 80ms capacity this adds roughly 40ms of onset
latency. Short auxiliary sounds do not wait. No clocks/pitch/input timing changed.
The new offline test disables gap filling and verifies sample-identical output
with and without a 20ms producer interruption for both 32kHz DMA and 48kHz music.
Measured synthetic onset: 47.9062ms / 45.3229ms; these are not hardware latencies.

Unmodified generated Xbox v1/v2 profiles now upgrade automatically to v3, keeping
the selected devices and an exact .pre-v3-*.bak original. Customized profiles are
preserved. Normal startup checks existing profiles even when config.ini has no
controller selection. Tests cover old versions, devices and custom preservation;
the actual runner also upgraded a private v2 copy and retained an exact backup.

One 51.16-guest-second plaza route completed with muted Cubeb processing and
57.202 seconds of private pre-volume PCM: peak 30846, zero clipped samples.
However DMA/music still recorded 62/93 empty dequeues; maximum callback gap was
11.7508ms. This does NOT establish clean sound. Crackling remains unresolved.
All 15 guarded entity probes decoded; living heroes fell to three, opponents
with positive health ranged 4-7. Start-to-resume expectations failed.

A short targeted plaza menu check, with four heroes alive, established the
numeric behavior: active positions/health changed; Start froze them; a second
Start left them frozen; B restored position/health changes. Use Start to pause
and B to resume. This is process-local functional evidence, not menu visual QA.
No screenshots, host input, listening, physical controller or FPS measurement.
Both native game processes exited 0; no game left running. Whole process on
CPU2/mask4, Vulkan 3x/1080p preset, normal clocks; experimental formatter, JIT
budget and backpatch reserve all OFF. Raw game data/probes/audio stay in .local.

Windows Build.cmd --cpu jit --jobs 2 exited 0; 41/41 native tests passed (8.89s).
Python: 157 passed, one skipped (158 run, 23.205s). Existing CMake deprecation,
Wayland, object-path and CMP0069/lz4 warnings; no compiler/linker warnings/errors.
Runner: 16111104 bytes, SHA256
41944d85527927ffbedf466f755905506e2889b47c67ae2b75be34ff1c270523.
Other binaries, scoped results and failed checks:
evidence/windows-20260930/AUDIO-CONTROLS-CONTINUITY.json.

Next work remains audible/device-path crackling diagnosis and remaining Xbox
contextual-use, direct hero-selection and fusion/revive gaps. Do not resume FPS
sweeps without new user direction. Preserve user StaticRecomp edits and launcher.


## Guest-clock plaza sequence and matched-state repeats - 2026-10-01

Added a preloaded process-local xbox_sequence command and benchmark option
--guest-sequence-start. Timed inputs, read-only snapshots and byte guards run
on one absolute guest-time schedule; snapshots/receipts flush afterward. No
host input, screenshots, guest memory writes, clock changes or default runtime
setting changes. Guard mismatch cancels remaining sequence input and releases
its synthetic controller. See docs/TESTING-GUEST-SEQUENCES.md.

Prior receipt inspection found 3.174-4.276 seconds of inter-hold gaps in roughly
50-second routes, with individual gaps up to 419.835ms. New sequences executed
106 holds over 47.46 guest seconds with ZERO gaps. All six used the same start
tick; maximum boundary lateness was 69 guest ticks. Every run's 15 entity-pool
snapshots matched the first control byte-for-byte. All four heroes remained
alive and 5-7 identified opponents had positive health at all sampled boundaries.
Each run recorded 22 hero health decreases, 27 same-slot opponent health decreases
and 56 hero position changes. Slot reuse remains a caveat for damage inference.

Budget ON means the existing experimental 4000us soft JIT budget. Order below
is actual execution order; the last pair is reversed. Primary statistics use
identical guest frames after TWO GUEST SECONDS of warmup, not two wall seconds.

| Run | New FPS | P99 ms | Worst ms | >50ms frames |
| --- | --- | --- | --- | --- |
| off1 | 29.837607 | 36.0059 | 212.9583 | 4 |
| on1 | 29.970241 | 35.2895 | 42.8217 | 0 |
| off2 | 29.969997 | 35.1034 | 55.8543 | 1 |
| on2 | 29.970141 | 35.0907 | 41.3686 | 0 |
| on3 | 29.472984 | 54.9239 | 104.3845 | 26 |
| off3 | 29.783237 | 48.4563 | 81.2486 | 9 |

Full-route on3 was worse: 190.1731ms maximum and 36 frames over 50ms. Full-route
off3 had 11 over50ms; other maxima/counts were unchanged by warmup. All six routes
and native processes exited successfully, but pacing failures remain failures.
The intentional wrong-guard test failed the harness as expected, cancelled the
later input and stopped the native process cleanly. No probe/state assets in Git.

Supported Windows build passed 40/40 native tests; expanded sequence cases also
passed in the final 4.83s native suite. Python: 157 passed, one skipped, 19.444s.
Existing CMake warnings only; no compiler/linker warnings/errors found. Runner:
16092160 bytes, SHA256 3d549b1e8a8393710ed43e91415d29fed66d1d847596919d7aaf60d1ea63248f.
All six produced-binary hashes, full/primary timing and failures are recorded in
evidence/windows-20260930/PLAZA-GUEST-SEQUENCE.json. Preserved user static edits.

Single CPU2/mask4, full JIT, Vulkan3x/1080p preset, normal clocks, muted Cubeb
processing, experimental formatter/identity cache ON, reserve524288. Runtime
profiling OFF. No screenshots or audible/visual correctness claim. Goal UNMET;
this is not three qualifying repeats or ten-minute continuous combat acceptance.
Keep the JIT budget default-OFF. Matching sampled state and input deadlines now
support more controlled investigation, but do not prove all CPU/render behavior.
Next correlate intermittent stalls with target-process CPU measurements and
separate runtime-span diagnostics; profiling itself changes budget decisions.
Continue mixed JIT/interpreter correctness work before promoting experiments.


## Default-off soft JIT compilation budget prototype - 2026-10-01

Added OPENMUA2_JIT_BUDGET_US (decimal 250-16000 microseconds; unset/0/invalid is
OFF). It tracks host compilation time over approximately 1/30-second guest-time
windows. After the allowance is spent, a dispatcher-only path executes up to
256 cold instructions, charges their reported cycles and updates memory mapping.
Revisited PC/CPU-feature keys trigger ordinary JIT compilation immediately,
even above the allowance. Hooks, debugging, stepping and static fallback retain
ordinary compilation. Cache clearing resets budget and hints. The dispatcher
explicitly checks timing after interpretation and handles shutdown.
The initial hard 4ms allowance FAILED: 18.2773 FPS, 905.027ms worst frame and
659,574,606 interpreted instructions. Reuse promotion reduced interpreted work
to roughly half a million instructions in early revised trials. It is now part
of every enabled budget configuration; the hard-only version was rejected.

All timing below uses a 4ms soft allowance when ON; off variants disable it.
Runtime profiling is OFF. First row is the rejected initial implementation.

| Variant | New FPS | P99 ms | Worst ms | >50ms frames |
| --- | --- | --- | --- | --- |
| on1 | 18.2773 | 801.947 | 905.027 | 56 |
| promote-on1 | 29.9698 | 35.687 | 41.612 | 0 |
| off1 | 29.9707 | 35.481 | 55.136 | 1 |
| promote-on2 | 29.5728 | 53.185 | 177.381 | 18 |
| off2 | 29.8997 | 47.048 | 79.242 | 10 |
| promote-on3 | 29.8626 | 46.689 | 62.326 | 4 |
| cpu-on2 | 29.9698 | 35.959 | 45.611 | 0 |
| cpu-off2 | 29.9700 | 35.295 | 56.501 | 1 |
| final-off | 29.9703 | 35.949 | 86.612 | 2 |
| final-on | 29.9700 | 36.272 | 46.271 | 0 |

All ten routes completed with 15 guarded entity probes each. Results are MIXED:
the final pair improved worst-frame time from 86.612 to 46.271ms, but earlier
revised runs reached 177.381 and 62.326ms. Successful pairs did not improve P99.
Do not discard failed runs, claim consistent gains or enable this by default.
Probe receipt waits can vary input publication timing; enemy counts differ, so
equivalent workload is not established. Larger intermittent drops remain unexplained.

Supported Windows build passed 40/40 native tests (5.47s), including new budget
and hotness-hint regression cases. Python: 156 passed, one skipped (19.561s).
Existing CMake warnings only; no compiler/linker warnings/errors found. Final
binary OFF and ON plaza checks both exited 0. Six binary hashes and all runs:
evidence/windows-20260930/PLAZA-JIT-COMPILE-BUDGET.json.

Single logical CPU2/mask4, full JIT, Vulkan3x/1080p preset, normal clocks, muted
Cubeb processing, experimental formatter/identity cache ON, reserve524288.
No screenshots or host input. Game data, raw probes and failed launcher logs
remain private. The optional Windows counter script was blocked by execution
policy; no policy change/bypass. A corrected target-only CPU sampler completed.

Goal UNMET. This default-OFF prototype needs stronger mixed JIT/interpreter correctness
coverage and controlled-input comparisons; current tests do not exhaustively
verify MMU, exceptions, FPSCR or timing behavior. Full runtime profiling changes
allowance decisions. Numeric entity activity is not visual/audio acceptance;
three qualifying repeats and ten-minute combat validation remain outstanding.


## Read-only plaza CPU correlation - 2026-10-01

Three additional plaza runs used a target-process-only, read-only CPU sampler
at nominal 20ms intervals, with full runtime profiling OFF. All routes and
samplers exited 0, with 15 guarded entity probes each. No screenshots or host
input. Existing single-CPU, normal-clock, Vulkan 3x / 1080p preset retained;
muted Cubeb, experimental formatter/identity cache ON and reserve 524288.

| Run | New FPS | P99 ms | Worst ms | >50ms frames | Typical CPU fraction |
| --- | --- | --- | --- | --- | --- |
| 1 | 29.9702 | 35.985 | 53.825 | 2 | 69% |
| 2 | 29.9704 | 35.297 | 51.531 | 1 | 70% |
| 3 | 29.9705 | 34.969 | 58.329 | 1 | 68% |

CPU fractions bracket approximately one-second frame windows and include all
threads on the one assigned logical CPU. Windows accounting is quantized;
time not executing can mean deliberate waits or descheduling. The earlier
18/24 FPS episodes did not recur, so their cause remains unresolved. Do not
dismiss those failures or claim these diagnostics as qualifying acceptance.

No source change or performance gain. Reused the prior restored runner
(77ab0b04c79d5bcc75f11968864b60c15f781ceb4f7a76b89bb3f5dd3ac0b0bb);
its prior Windows build passed 39/39 native tests. No redundant rebuild.
Evidence: evidence/windows-20260930/PLAZA-PROCESS-CPU-CORRELATION.json.

Next investigate a bounded first-use compilation budget, with cold instructions
executed accurately until compilation can resume. Before implementation, account
for cycles, timing redispatch, exceptions, memory-base updates, hooks, stop state
and cache resets; exclude debugging and static fallback. SingleStep() cannot be
used because it overwrites timing-slice state. No tiering code is implemented or
validated at this checkpoint. Goal UNMET; audio and visual limits remain.


## Register lookahead experiment rejected; pacing failures retained - 2026-10-01

Tested sharing the JIT register allocator's forward-use scan while preserving its
lookahead boundary and scoring formula. A plaza shadow run completed without a
reported count mismatch (comparison count not recorded). Two unprofiled OFF/ON
pairs did not show a consistent worst-frame improvement:

| Run | New FPS | P99 ms | Worst ms | Frames >50 ms |
| --- | --- | --- | --- | --- |
| OFF 1 | 29.9701 | 35.125 | 52.553 | 1 |
| ON 1 | 29.9704 | 35.073 | 48.823 | 0 |
| OFF 2 | 29.9699 | 35.233 | 48.924 | 0 |
| ON 2 | 29.9700 | 35.136 | 49.935 | 0 |
| Restored original | 29.7825 | 51.774 | 132.493 | 17 |
| Restored repeat | 29.6546 | 49.642 | 69.375 | 14 |
| Restored, profiled | 29.9701 | 35.561 | 60.656 | 2 |

Rejected the candidate; patch/binary remain private. Six allocator source files
were restored byte-for-byte. Both supported Windows builds passed 39 native
tests (candidate 7.21s; restored 7.18s). Existing CMake warnings only; no compiler
or linker errors/warnings. Python was not rerun because no Python or retained
source changes resulted. The previous Python result remains 156 passed/1 skipped.

All eight routes (including shadow) completed with 15 guarded entity probes each.
The two unprofiled restoration runs FAILED pacing: rolling one-second minima
were 18 and 24 FPS. Most larger stalls in the first clustered at seconds 31-33.
The profiled follow-up's worst frame included 30.8754 ms of JIT compilation, but
it did not reproduce those larger slowdowns; their cause remains unresolved.
Do not discard the failures or attribute them solely to first-use compilation.

Plaza only, one logical CPU (2/mask 4), normal clocks, full JIT, Vulkan 3x EFB /
1920x1080 preset, muted Cubeb, formatter/identity cache enabled, reserve 524288.
No screenshots or host input. Memory probes and profiling limit timing inference;
entity counts varied, and these are not equivalent maximum-load acceptance runs.
No audible or visual correctness claim. No performance optimization shipped.
Goal UNMET. Detailed timing, entity counts, hashes and restoration results:
evidence/windows-20260930/PLAZA-REGISTER-LOOKAHEAD-REJECTED.json.


## Continued plaza combat and rejected host PGO - 2026-10-01

Three successive plaza passes in one process completed with 45 entity probes,
matching generation 13. No screenshots or host input. CPU 2 / mask 4, full JIT,
Vulkan 3x / 1920x1080 preset, normal clocks, muted Cubeb, formatter/identity cache ON,
reserve 524288. Runtime profiling and memory/timing probes make these diagnostics,
not three independent qualifying repeats.

| Pass | New FPS | P99 ms | Worst ms | >50ms | JIT compile ms | Observed living opponents |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 30.0274 | 39.149 | 53.086 | 3 | 636.078 | 4-7 |
| 2 | 29.9698 | 34.580 | 35.849 | 0 | 27.009 | 3-5 |
| 3 | 29.9699 | 34.525 | 38.491 | 0 | 49.985 | 1-4 |

Each pass covered about 52 seconds. Initial worst frame included 25.154 ms of JIT
compilation. Later passes were steadier, but enemy counts also fell: do not call
this an equivalent maximum-load comparison or attribute all improvement to
warm code. The first boundary includes slight catch-up (100.191% guest speed);
subsequent passes were approximately 100%. Zero dropped frame samples.

Tested MSVC host PGO via the existing Windows build path: /GENPROFILE:EXACT
and /FASTGENPROFILE:EXACT,COUNTER64, each with /LTCG and private databases.
Both builds passed 39 native tests (7.79s/4.92s), but both training attempts timed
out before usable gameplay: zero observed frames/probes, no collected .pgc,
failed route and runtime/harness exit 1. Elapsed 320.141s / 200.141s include shutdown
allowance. Second planned training runs did not start. No optimized USEPROFILE
candidate was built. Cause unproven; reject this experiment instead of shipping it.
Experimental source/options/tests/docs were preserved privately and removed from
the checkout; tracked CMakeLists is restored. No profiler installation or global
environment changes. Private profiles and game data remain outside Git.

Restored normal supported Windows JIT build passed: 39/39 native tests (6.91s),
Python 156 passed / 1 skipped (157 tests, 19.624s). Existing CMake warnings only;
no compiler/linker warnings/errors. PGO flags and profiler DLL dependency absent.
A fresh restored-runner plaza run exited 0 and completed 15 guarded probes with
four positive-health heroes and 6-8 identified positive-health opponents. Its
probe-instrumented timing was 29.9699 FPS, P99 34.996 ms, max 53.161 ms, one >50 ms frame;
this verifies recovery and supplies diagnostic timing, not a new FPS gain.
Runner SHA256 80cde740c3e08f72c36a1f4b46df1e672f0e5102e6388b8d76e8cab253129cb5.
Five binary hashes, failures and numeric phase evidence are recorded in
evidence/windows-20260930/PLAZA-CONTINUED-COMBAT-PGO-REJECTED.json.
Goal UNMET. First-use compilation spikes remain; sustained combat, qualifying
repeats and audio/visual limits are not resolved by this checkpoint.


## Read-only plaza entity observations - 2026-10-01

Added tools/analyze_entity_probe.py and seven synthetic regression cases. This
stdlib-only offline tool reads native memory-probe files using a private,
state-specific numeric contract. No process access, desktop input, screenshots,
game-memory writes or embedded proprietary strings/assets. Raw RAM, save states,
contracts and the diagnostic wrapper remain private under .local.

The entity-name word has an eight-bit generation tag and 24-bit identity;
searching the full word as an untagged name ID had missed the real actors.
The inspected pool has 0xa38-byte slots. Position is at +0x4c; recovery and
maximum-health normalization code supports current/recovery/maximum health at
+0x2a0/+0x2a4/+0x2a8 on the entity itself (not its attribute pointer). The tool
requires the expected generation and unique heroes, bounds all reads, rejects
non-finite classified fields, and excludes empty, stale and unclassified slots.
It does not model the whole entity lifecycle, hostile AI activity or visibility.

A live short plaza diagnostic completed: 15 probes over 51.767 guest seconds,
3-7 identified opponents with positive health, 3-4 heroes with positive health,
17 adjacent-sample hero health decreases, 29 same-slot/same-identity opponent
health decreases and 46 hero position changes. Seven same-slot/same-identity comparisons crossed
from positive to nonpositive health. These are sample deltas, NOT exact damage
or kill-event counts: same-identity slot reuse is possible. All samples remained
in generation13; maximum hero distance from the inspected plaza reference was
802.844 units, within the private wrapper's conservative1800-unit stop boundary.
One hero reached nonpositive health. Live generation-change abort was not triggered.

Start/short historical states decode to4/4 positive-health heroes and7/6
identified positive-health opponents. The old long-run endpoint uses generation14
and fails the original generation13 contract. This strengthens the reason that
it cannot count as uninterrupted combat; neither cause nor transition time is
proven. All15 live probes rechecked with the final guarded analyzer. No claim
about unsampled intervals, visual correctness or a qualifying combat repeat.

First live attempt failed in the private wrapper: read_memory has no timing
sidecar. The wrapper stopped before its first input batch; runtime shutdown
returned0, harness1. The corrected route explicitly uses read_timing and exited0.
Do not conceal or count the failed attempt as validation.

Unchanged native runner SHA256:
2f5cdc56e309d8e3514d5697a338654affa2151ed24695b318e9a271fd5bedd0.
One logical CPU2/mask4, full JIT, Vulkan3x/1920x1080 preset, normal clocks, muted
Cubeb, formatter and identity cache ON, reserve524288. Process-local input only;
no screenshots. Memory/timing reads perturb execution, so this is not a new FPS
comparison. Native code unchanged: prior Windows build/39-test result retained.
Final Python suite:156 passed,1 skipped (157 tests,22.251s), including7 entity
cases. Evidence: evidence/windows-20260930/PLAZA-ENTITY-PROBES.json.

Goal UNMET. This improves combat-state evidence, not frame rate. Remaining work
includes compile bursts/other slow frames, three qualifying repeats, ten-minute
verified plaza combat and audible quality. Retain the plaza-only/no-screenshot
constraint and do not count idle, stale, reloaded or failed-route time as combat.


## Plaza JIT metadata reservation experiment - 2026-10-01

Testing remains tutorial-plaza ONLY, with NO screenshots, host input or desktop
capture. These instructions supersede older visual-capture/varied-scene plans.
A completed input route and rendered frames do not prove continuous combat or
correct visuals. No new visual or audible correctness claim is made.

An instrumented reference found first-use JIT bursts in the slowest frames:
1,158 new blocks occupied 41.363ms of a 70.076ms frame; metadata insertion
accounted for 20.076ms inside that compile time. A separate intrusive resident
census found 36,404 executed blocks, only 1,068 executed once. Most matched
burst addresses were reused. This weakens the case for a first-visit-only
interpreter tier; no tier or speculative JIT prewarming was implemented.
Cross-run address matches are not deterministic paired execution, and invalidated
blocks are absent. Save-state WRITE does not clear this JIT; state READ does.

Added nested jit_backpatch_rehash timing and a default-off host bucket-capacity
hint: OPENMUA2_BACKPATCH_RESERVE=524288. Strict decimal values 0..1048576 are
accepted; 0 retains normal growth, and 1 aliases the initial 131072 experiment.
Invalid values retain ordinary growth. Reservations never shrink existing
capacity and preserve value references. No guest code/data is precompiled or
changed. Native tests cover pointer stability, capacity retention/rounding,
reuse, parser bounds/rejection; Python verifies nested span accounting.

Initial 131072 reservation FAILED to remove growth: 64 rehashes in both runs,
40.452ms OFF versus 37.280ms ON; worst frame 66.806 versus 88.958ms. This is a
failed trial, not evidence of a gain. The bounded revision logs actual buckets:
524288 produced 8192 buckets per shard, retained over subsequent clears.
Same-binary profiled comparison: OFF had 64 rehashes/44.968ms, ON had zero in the
measurement window. Slow metadata insertion fell from 46.945 to 2.654ms;
compile elapsed coverage 435.238 to 423.277ms (different block counts).
Worst frame 60.979 to 56.235ms. These nested spans overlap; do not add them.
Backpatch spans measure metadata insertion during compilation, not runtime faults.

Repeated comparisons WITHOUT detailed runtime profiling, all exit0:

| Order | Capacity hint | New FPS | P99 ms | Worst ms | Frames >50ms |
| --- | --- | --- | --- | --- | --- |
| 1 | OFF | 29.9706 | 35.233 | 62.933 | 1 |
| 2 | 524288 | 29.9701 | 35.283 | 51.450 | 1 |
| 3 | OFF | 29.9700 | 35.452 | 58.445 | 2 |
| 4 | 524288 | 29.9701 | 35.023 | 57.804 | 2 |

Full JIT, whole process CPU2/mask4, Vulkan3x/1920x1080 preset, normal clocks,
muted Cubeb with audio profiling, formatter ON and identity cache ON; indirect
experiments OFF. Existing short queued plaza route, two-second warmup, endpoint
before save. No screenshots. Mixed/modest frame-tail benefit; keep opt-in.
No tens-of-FPS gain or solid-30 claim. Callback counters cannot prove clean audio.

Supported Build.cmd --cpu jit --jobs 2 completed successfully with local LTO ON.
39/39 native tests passed (7.21s); Python 149 passed/1 skipped (34.877s).
Existing CMake deprecation, Wayland, object-path-length and lz4 IPO warnings;
no compiler/linker diagnostics. Five runtime executables and hashes are recorded
in evidence/windows-20260930/PLAZA-BACKPATCH-RESERVE.json. Runner SHA256:
2f5cdc56e309d8e3514d5697a338654affa2151ed24695b318e9a271fd5bedd0.
Private source data, saves, profiles and game assets remain outside Git.

Goal UNMET: >50ms frames persist; continuous combat, audio quality, three
qualifying repeats and ten-minute combat remain unverified. Investigate remaining
first-use compile bursts and establish read-only combat-state telemetry before
another long run; do not treat idle/menu/failure time as combat validation.


## Live byte-identity cache for formatter guards - 2026-10-01

Added default-off OPENMUA2_FORMAT_IDENTITY_CACHE=1 inside the existing formatter
experiment. Captured formatter-body/caller-epilogue bytes must pass the existing
SHA-1 identities, then every eligible invocation compares all live bytes against
those verified copies. Mapping, layout, ABI and fallback guards remain active;
mutations reject, including after state loads/JIT clears. No unchecked identity
result or guest pointer is cached. Captured game bytes stay in process memory.
Native coverage checks initial failure/retry, every-byte mutation/restoration,
wrong lengths and a live-source change during captured-copy validation.

Supported Windows JIT build passed with local LTO ON: 39/39 native tests (5.64s).
Existing CMake deprecation/Wayland/path-length/lz4 IPO warnings; no compiler or
linker diagnostics. Python tooling unchanged, not rerun. The initial edit script
hit a newline assertion; its accidentally started preliminary build was cancelled
and is not validation evidence. The completed build uses the full reviewed patch.
Runner SHA256 fa202fa923e7d1fcc48a5c9208f385b7638889fcbbcdaeb0df4aaa67001231a3.

Shadow: 406,667 completed comparisons, zero output/FPSCR/ABI errors, one pending
at shutdown; two hashes and813,336 live byte checks. This is scoped verification,
not full-game correctness. Same-binary alternating plaza diagnostics, all exit0:

| Order | Cache | New FPS | P99 ms | Worst ms | Frames >50ms |
| --- | --- | --- | --- | --- | --- |
| 1 | OFF | 29.7073 | 44.118 | 119.495 | 9 |
| 2 | ON | 29.9703 | 35.121 | 57.491 | 2 |
| 3 | OFF | 29.9700 | 36.288 | 62.311 | 2 |
| 4 | ON | 29.9701 | 35.340 | 56.423 | 1 |

All: whole process CPU2/mask4, JIT, Vulkan3x/1920x1080 preset, normal clocks,
muted Cubeb, formatter ON, indirect experiments OFF, no screenshots/memory probes
or detailed runtime profiling. Two-second warmup, endpoint before state save.
ON runs used two hashes each plus4,045,618/4,096,354 byte checks; OFF runs used
4,260,396/3,830,478 hashes. CPU-time counters unavailable: no measured CPU-time
saving is claimed. Both pairs favor ON, but the second gain is modest and >50ms
frames persist. Keep opt-in; no tens-of-FPS or solid-30 acceptance claim.
Combat continuity, audio quality, three qualifying repeats and ten-minute combat
remain unverified. The prior long-run transition/stall remains unresolved. Goal
UNMET. Evidence: evidence/windows-20260930/PLAZA-FORMATTER-IDENTITY-CACHE.json.


## Read-only plaza transition probes - 2026-10-01

Ran two private diagnostics with the existing runner, one logical CPU2, full JIT,
Vulkan3x/1920x1080 preset, normal clocks, muted Cubeb and formatter experiment ON.
No screenshots, host input or game-memory writes. The existing native read-memory
command samples the previously identified dialog slot; private guards reject its
failure/retry option group before publishing further input. Offline fixtures
accept baseline/short state, reject the known long-endpoint failure signature,
and reject a truncated probe (four checks). Live transition abort is unexercised.

Sequential run: exit0, 41 probes over212.296 observed guest seconds (180.190s
commanded). Batched run: exit0, 86 probes over394.388 observed guest seconds
(380.940s commanded). Both keep the baseline slot hash throughout. Neither
reproduces the prior retry signature or1.234567s guest presentation gap. Largest
guest interval33.367ms; host gaps still reach83.119/86.268ms respectively. These
are intrusive diagnostics, not acceptance or a performance improvement. Command
delivery differs from the original endurance run; no prior failure is dismissed.

The guard detects one failure signature, not enemy activity, health or every
transition. Establish actual combat continuity before another endurance acceptance
attempt, then return to measured CPU/JIT stall optimization. No fresh visual or
audible verification. No native/tooling source change or rebuild; prior build and
tests retain their recorded scope. Proprietary probe files/helpers remain private.
Evidence: evidence/windows-20260930/PLAZA-TRANSITION-PROBES.json. Goal UNMET.


## Offline plaza state audit - 2026-10-01

Investigated existing baseline, short CPU2 endpoint and long endurance endpoint
without launching the game or taking screenshots. Bounded private LZ4 decoding,
three executable anchors, the native memory serialization layout and the dialog
manager accessor/vtable identify the same dialog slot in all three states.
The long endpoint contains a newly populated failure/retry option/callback group;
the baseline and short endpoint retain their prior options. This supports a
failure/retry-path inference during the long run. Its late near-30 cadence must
not count as verified continuous combat. The slot is not the selected dialog at
the endpoints: populated historical content does not prove a currently visible
menu or timestamp/attribute the 1,220ms gap. Short-run combat continuity also
remains unverified. No fresh visual, audible, build or performance claim.

Next establish read-only health/activity or transition telemetry and stop plaza
diagnostics on failure/transition before another endurance acceptance attempt.
Keep plaza only, no screenshots, one logical CPU and existing rendering settings.
No native/tooling source changed; prior build/tests remain scoped as recorded.
Private game states, decompressed memory, strings and instructions stay outside
Git. Numeric evidence: evidence/windows-20260930/PLAZA-STATE-AUDIT.json.
Goal remains UNMET.


## Processor placement and long-run diagnostic - 2026-10-01

Added --logical-processor N to tools/run_combat_benchmark.py (Windows, current
processor group). The child inherits exactly one allowed logical CPU; the harness
is immediately restored. The runner still pins/verifies its own process, and the
benchmark now checks the requested mask, not only its bit count. No global policy,
unrelated process, native source/binary or normal launcher default changed.
Seven new tests cover setup/launch/restore failures, cleanup, allowed masks and
runtime confirmation. Tooling: 149 run, 148 passed, 1 skipped (19.762s). New option
also passed a real CPU2 plaza run. Prior native Windows build 39/39 still applies;
runner remains c5961c5d32ebeb99be9492bea5ea621757af405e0da0c754cfa0e2f4377c1311.

All runs: whole game process on one logical CPU, JIT, Vulkan3x, normal speed/clocks,
Cubeb muted, formatter experiment ON, indirect hints OFF, no screenshots. CPU
numbers below are logical processor indices, not claims about physical topology.

| Run | CPU | P99 ms | Worst ms | Frames >50ms | New FPS |
| --- | --- | --- | --- | --- | --- |
| Preliminary (harness also pinned) | 2 | 36.053 | 57.195 | 3 | 29.9702 |
| Restored-harness run | 2 | 37.519 | 49.970 | 0 | 29.9702 |
| Comparison | 4 | 36.858 | 57.384 | 2 | 29.9701 |
| Control | 0 | 45.292 | 61.772 | 6 | 29.9704 |
| Repeat 2 | 2 | 35.363 | 60.374 | 1 | 29.9698 |
| New CLI option validation | 2 | 36.471 | 60.152 | 1 | 29.9702 |

CPU2 improves the observed tail consistently enough for further diagnostics;
it is not proven universally preferable. Normal launcher stays unchanged.

A longer CPU2 run repeated the existing plaza input cycle14 times, omitting the
initial approach after cycle1; 1,458 timed holds / 628.69 commanded guest seconds.
Runtime/harness exit0. Measured after two seconds: 643.055s, 29.8948 new FPS,
P99 34.270ms, 17 frames >50ms, maximum1,220.296ms, lowest rolling second0;
no samples dropped. At138.405s a 1,220.296ms host gap coincides with1,234.567ms
guest presentation gap. At349.960s a151.890ms host gap has normal33.367ms guest
cadence. Preserve both; do not discard unverified transitions as loading.
Minutes2,4,5,7-11 have no >50ms intervals; that is NOT proof of continued combat.
Hero health, enemy activity and scene state were not observed. The long run is
NOT ten-minute combat acceptance and no clean-audio claim is made.

Next investigate the guest-side gap/possible transition and establish non-visual
activity validation before interpreting stable late cadence as combat. Keep CPU2
as an explicit diagnostic option, plaza only, no screenshots. Goal UNMET: verified
continuous combat, frame pacing and audible quality remain outstanding.
Evidence: evidence/windows-20260930/PLAZA-PROCESSOR-ENDURANCE.json.



## Guarded direct-call experiment: mixed pacing results - 2026-10-01

Added default-off OPENMUA2_INDIRECT_HINTS=origin:target1,target2,target3 for an
explicit unconditional linked bcctr call site. Runtime CTR comparisons guard
ordinary WriteExit direct-call links; mismatches retain original dispatch.
Debugging/static fallback and disabled linking exclude the experiment. No guest
addresses are baked into source defaults. Existing link management preserves
feature-context matching, downcount, BLR return prediction and SMC unlinking;
this reuse is not an exhaustive new generated-code/SMC correctness test.
Native tests cover bounded parsing, duplicate/invalid/unaligned inputs and limits.

Supported Windows JIT build passed: local LTO ON, 39/39 native tests (7.68s).
Existing CMake warnings remain; no compiler/linker diagnostics found. Python
unchanged, not rerun (prior 141 passed/1 skipped remains scoped to earlier run).
Runner SHA256 c5961c5d32ebeb99be9492bea5ea621757af405e0da0c754cfa0e2f4377c1311.
Other produced runtime executable sizes/hashes are in the evidence JSON.

Same-binary alternating plaza runs, all exit0, one logical CPU, Vulkan3x, normal
clocks/speed, Cubeb muted, formatter ON, rush/smooth OFF, no screenshots or
intrusive target counters. Two-second warmup; final input endpoint before save.

| Order | Mode | New FPS | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | Hints ON | 29.9742 | 42.497 | 61.597 | 2 | 29 |
| 2 | Hints OFF | 29.9702 | 40.687 | 57.089 | 5 | 29 |
| 3 | Hints ON | 29.9704 | 39.595 | 74.818 | 3 | 29 |
| 4 | Hints OFF | 29.9701 | 43.396 | 57.638 | 5 | 29 |

Decision: do not promote. Fewer >50ms frames came with worse maximum stalls in
both comparisons; no substantial/repeatable solid-30 gain is established. Keep
this narrowly scoped experiment default OFF. Do not add more speculative targets
without evidence. First-use compilation and non-JIT stalls remain the priorities.
No fresh visual/audible verification or clean-sound claim. Goal UNMET: three
qualifying repeats and ten-minute plaza combat acceptance remain outstanding.
Keep plaza only and no screenshots. Evidence:
evidence/windows-20260930/PLAZA-GUARDED-INDIRECT-CALLS.json.



## Selected indirect-call census - 2026-10-01

Added default-off OPENMUA2_INDIRECT_PROFILE_PC diagnostic: one aligned hexadecimal
bcctr instruction address, 64 destination/feature-flag counters, explicit total
and overflow, flushed after CPU shutdown. Disabled emits no runtime counter call.
Both taken branch forms retain ordinary dispatch and timing; guest register caches
are flushed and the destination scratch register is saved around the helper.
Selector validation, distinct translation contexts, overflow and reconciliation
are covered by native tests. Counts cover the whole process, including boot;
they are intrusive diagnostics, not an FPS benchmark or correctness proof.

Supported Windows JIT build passed, local LTO ON: 39/39 native tests in 6.21s.
Existing CMake warnings remain; no compiler/linker diagnostics found. Python
tooling unchanged and not rerun (previous 141 passed/1 skipped remains scoped).
Runner SHA256 d2e0314de0d66725d60608b6b224cbea5ffb73e82fb57675d9769e027282b849.
Runtime binary sizes/hashes are recorded in the evidence JSON.

Selected 8036f9b4 in the plaza route: 114,958,803 calls, seven destinations, all
feature flags3, zero overflow, exact count reconciliation; runtime/harness exit0.
Three busiest targets each receive about26% of calls (combined78%). Private
instruction inspection identifies short float-vertex-attribute graphics FIFO
writers. No proprietary code/data is included in the evidence or source.
This rules against a simple one-target predictor; it does not establish the
benefit of a multi-target cache over the existing direct entry-point dispatcher.
Next test a guarded polymorphic dispatch optimization only with preserved
feature flags/downcount/return prediction/cache invalidation and measured gains.
Separately address first-use JIT compilation bursts; avoid guest-cycle hacks.

Counter-disabled follow-up also exited0 and emitted no profile summary. After
two seconds: 46.581s, 29.9696 new FPS, P99 43.534ms, maximum101.092ms, five frames
>50ms, lowest rolling second28, guest speed99.9986%. No performance gain claimed.
Both runs: one logical CPU, Vulkan3x, normal speed/clocks, Cubeb muted, formatter
experiment ON, rush/smooth OFF, no screenshots. Audio crackle and fresh visual
correctness remain unverified. Goal UNMET; three qualifying repeats and ten-minute
plaza combat acceptance are still outstanding. Preserve plaza-only/no-capture.
Evidence: evidence/windows-20260930/PLAZA-INDIRECT-TARGETS.json.



## Plaza pacing hypothesis rejected; refreshed hot-block profile - 2026-10-01

Tested existing RushFramePresentation=True plus SmoothEarlyPresentation=True in
an isolated copied profile. Both no-screenshot plaza runs exited 0, with full
JIT, one logical CPU for the whole process, Vulkan3x, normal clocks/speed, Cubeb
muted and formatter explicitly ON. No source, binary or normal-profile changes.
Current Windows build/tests from the previous checkpoint still apply.

| Run | Seconds | New FPS | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Rush/smooth 1 | 46.708 | 29.973 | 45.738 | 63.824 | 6 | 29 |
| Rush/smooth 2 | 46.625 | 29.984 | 49.667 | 90.571 | 13 | 28 |

Throttle calls fell from 17,122 in the prior trace to about one per rendered
frame. Positive-deadline wait overshoot maxima were 2.108/8.715ms versus prior
2.687ms. The first run's worst frame still included 38.709ms compilation.
Fewer waits do not give a repeatable frame-pacing fix; do not promote the pair.
Interior audio buckets had DMA/stream empty-dequeue counts 1/0 and 2/3; these
muted diagnostics do not prove clean audible playback.

An additional intrusive resident-JIT-block profile completed in 217.125s,
exit0, 36,141 resident blocks. Pages 803f6000, 8036f000 and 8028e000 account for
11.70%, 11.24% and 10.37% of instrumented block time. These are not reliable
release CPU-time shares: short blocks incur disproportionate profiler overhead
and invalidated blocks are omitted. Private code inspection identifies an
indirect-call loop at 8036f998, executed 101,824,758 times in this profile;
its JIT path currently uses WriteExitDestInRSCRATCH. This is a lead, not proof
that dispatch is the next largest bottleneck. No proprietary code/data included.

Next characterize that loop's indirect targets and measure dispatch overhead;
a proposed call-site cache must preserve feature flags, checked-entry/downcount,
return prediction, SMC invalidation and fallback. Do not bypass guest scheduling
or speculatively invoke arbitrary Jit calls. First-use compilation remains a
separate hitch source. Goal UNMET: no three qualifying repeats or ten-minute
plaza combat acceptance, no fresh visual/audible verification. Keep plaza only
and no screenshots. Evidence: evidence/windows-20260930/PLAZA-PACING-HOTBLOCKS.json.



## Bounded formatter spill experiment - 2026-10-01

Plaza-only, no screenshots. Added guarded EABI stack-overflow argument reads and
bare/8-digit hexadecimal formatting to the existing DEFAULT-OFF formatter
experiment. Unsupported formats still fall back; original code/caller hashes,
float restrictions and transactional output protections remain. This avoids
some formatting work without changing clocks, effects or the one-CPU constraint.

Supported Windows JIT build succeeded with local LTO enabled: all 39 native tests
passed (8.20s); Python tooling 141 passed, 1 skipped (50.301s). Existing CMake
warnings cover deprecations, unavailable Wayland, long object paths and lz4 IPO
policy; no compiler/linker errors. Current moderngekko-run.exe is 16,064,512 bytes,
SHA256 7a4e0b314737c8ab9bf32673787ce91c8a7887323fb9efa0f0ca7175653582db.
Other runtime executable hashes/sizes are recorded in the evidence JSON.

Shadow run: 1,803,814 completed comparisons, zero output/FPSCR/preserved-ABI
mismatches, one pending at shutdown (unverified). This is scoped evidence, not
universal correctness. Three replacement-mode repeats and one trace exited 0:

| Run | Seconds | New FPS | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Spill 1 | 46.682 | 29.969 | 43.093 | 73.992 | 7 | 29 |
| Spill 2 | 46.947 | 29.970 | 43.580 | 115.656 | 4 | 28 |
| Spill 3 | 46.781 | 29.970 | 49.490 | 69.252 | 12 | 29 |
| Spill spans | 46.614 | 29.969 | 48.771 | 67.529 | 10 | 29 |

Same intact-statue plaza state/route, Vulkan3x, whole-process affinity mask1,
normal clocks/speed, Cubeb Volume0, formatter explicitly ON; two-second warmup.
These are short diagnostic runs, not qualifying acceptance runs. Earlier repeats
had 11 frames >50ms each; improvement is inconsistent and the worst new stall
is larger. The old binary control failed twice before combat with 0x80000003
(KERNELBASE breakpoint); cause unresolved. Preserve those failures. Historical
runs therefore are not a contemporaneous controlled A/B result.

Trace: 13,295 JIT compilations / 501.490ms versus prior 14,295 / 591.331ms;
measured throttle time 12.985s versus 9.088s. These observations suggest reduced
work but do not establish a causal FPS gain. The worst new trace frame includes
39.052ms JIT compilation; another 63.800ms frame includes none. Nested spans
must not be summed. First-use compilation and non-compilation stalls remain.

No fresh visual or audible validation; muted audio telemetry cannot establish
whether crackling is fixed. No further screenshots. Goal remains UNMET: robust
frame pacing and ten-minute plaza combat acceptance are outstanding. Next isolate
remaining JIT bursts and non-JIT stalls, retaining plaza-only/no-capture scope.
Evidence: evidence/windows-20260930/PLAZA-FORMATTER-SPILL.json.



## Plaza timing repeats and hitch attribution - 2026-10-01

Current runner 2824e94c0a9b4bd9c0c18110eccaf272e5f06f5c14b10534eff6ce9d48f0367d,
JIT, whole-process mask1, Vulkan 3x EFB, normal clocks/speed, Cubeb muted,
formatter experiment ON (normal launch default OFF), no screenshots. Same
xbox-timed-hero-combat route and statue-target private state; two-second warmup,
last input endpoint before save. These are short diagnostic repeats, not passes.

| Run | Seconds | New FPS | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Timing 1 | 46.621 | 29.987 | 48.517 | 101.724 | 11 | 27 |
| Timing 2 | 46.683 | 29.968 | 47.470 | 94.454 | 11 | 28 |
| Timing 3 | 46.847 | 29.970 | 46.411 | 72.992 | 11 | 28 |
| Runtime spans | 46.649 | 29.990 | 50.143 | 83.185 | 14 | 28 |

All four command runs exit0; guest speed 100.00-100.07%. The profiled worst
frame includes 49.624ms JIT compilation, 32.620ms emission, 11.325ms finalization
(nested, do not sum), and 0.0037ms GPU fence wait. Across the window 14,295 JIT
compilations cost 591.331ms elapsed. Other 60-61ms frames had zero/negligible
compilation; there are multiple causes. CPU counters remain quantized and
cannot prove frequency or scheduling causes. Next investigate first-use compile
bursts and non-compilation stalls separately in the plaza. Do not precompile
arbitrary blocks if that changes guest architectural state or cache semantics.

No source/binary changes or rebuild this checkpoint. Prior supported Windows
build 39/39 native, tooling 141 passed/1 skip still applies. No fresh visual or
audible verification; numeric audio counts are recorded, not proof of clean sound.
No further screenshots after user instruction. Earlier street exploration is
retained privately; its paused harness timed out at 600s (child 0/harness 1).
Goal unmet: frame pacing and ten-minute plaza combat remain outstanding.
Evidence: evidence/windows-20260930/PLAZA-ONLY-BASELINE.json.


## Primary benchmark scope - user direction, 2026-10-01

Use the tutorial plaza only for performance and bottleneck testing. The user
identifies it as representative of the game's maximum simultaneous enemy load,
including effects, sounds and animations. Keep the statue intact in the dense
combat baseline so its scripted completion does not remove the repeated spawns.
Later streets are not required benchmark work. Three repeat runs and ten minutes
of plaza combat with varied movement, heroes, attacks and effects remain required;
the nominal-30 cadence and correctness requirements are unchanged.

The user has requested no more screenshots. Disable capture commands for all
future runs (--no-screenshots); use frame timing, CPU/JIT and audio telemetry.
Previously inspected native evidence remains scoped to those earlier runs.
Do not imply fresh visual verification from telemetry. The latest visual plaza
run completed before this instruction, but its ten captures were not inspected;
no new visual-correctness claim is made from it. Audible playback remains
unverified. These user instructions supersede earlier route-expansion and new
screenshot requirements in this document.


## Xbox Safeguard gestures and route validation - 2026-10-01

Generated Xbox profile v3 adds Back/View + right-stick sideways for shake and
Back/View + right-stick up/down for swing. Camera rotation is suppressed while
Back/View is held; LB+RB fusion aiming suppresses these gestures. Existing user
profiles remain preserved. Real expression-parser tests cover these mappings
and separation from camera/fusion controls. Downward swing is not game-verified.

Healthy Iron Man reached the statue prompt by flying onto its pedestal. LB
starts the interaction. The complete generated-v3 route toppled the statue in
two runs, with native captures showing both prompts, the fallen statue, and
Fury's completion response. All six captures were inspected sequentially.
Use sideways input until the upward prompt, then promptly move the stick up.
The route uses 1500ms sideways and 800ms upward; 500ms sideways was too short,
while 2500ms followed by upward failed. A neutral delay and repeated upward
pulses after the long shake also failed. Midpoint restore tests alone were
insufficient; the final route completes without a midpoint reload.

Build.cmd --cpu jit --jobs 2 passed, 39/39 native tests (7.06s). Tooling:
141 passed, one skipped (59.553s). First build ran the old Xbox expectation
and failed 1/39; corrected final rebuild passed. Existing CMake warnings:
deprecation, missing Wayland, long object paths, lz4 CMP0069 IPO policy.
No compiler/linker error in final build. Produced moderngekko-run.exe,
ModernGekko.exe, moderngekko-port.exe and moderngekko-module-info.exe;
sizes and SHA-256 hashes are in the evidence JSON. Local build uses LTO ON.

The earlier paused exploration hit its 900s harness watchdog; child exit0,
harness exit1, and its incomplete route remains failed. Private screenshots,
saves and extracted mission data stay outside Git. No host input/capture used.
These are interaction diagnostics: JIT, one logical CPU, Vulkan 3x EFB, normal
speed, formatter experiment ON (normal default OFF), Cubeb muted. Audio was
not heard; screenshots do not prove continuous animation or FPS acceptance.
Three qualifying repeats and ten-minute varied combat remain outstanding.
Next extend combat beyond the statue using the verified endpoint in private
.local/automation/safeguard-v3-repeat-20261001/statue-completed.sav.
Goal unmet. Evidence: evidence/windows-20260930/XBOX-SAFEGUARD-VALIDATION.json.
Route: tools/routes/xbox-statue-safeguard.json.


## Combat-route interaction investigation - 2026-10-01

Three private route probes completed with exit0; all ten native screenshots
were inspected sequentially. Ground approach, jumps and attempted flight did
not activate the statue interaction. Two probes include Iron Man knockout;
these low-health endpoints must not become acceptance starting states.
Read-only inspection of the private mission package confirms a scripted statue
interaction whose completion stops repeating enemy spawns. A raised trigger
region is a navigation lead, not a verified input fix. Next restore an earlier
healthy state and establish the pedestal approach/use activation before changing
motion mappings. Extracted mission data and captures remain private under .local.

No source, binary or normal profile setting changed. No new FPS acceptance or
audible playback claim; Cubeb ran muted, formatter experiment ON (default OFF).
Prior build/tests remain 39/39 native and 141 tooling passed, one skipped; not
rerun for this evidence-only checkpoint. Three qualifying repeats and ten-minute
varied combat remain required. Goal unmet. Evidence:
evidence/windows-20260930/STATUE-ROUTE-EXPLORATION.json.


**Project:** Wii Marvel: Ultimate Alliance 2 USA (RMSE52) native-PC recompilation  
**Repository:** `GTTeancum/OpenMUA2`  
**Canonical branch:** `main`  
**Date:** 2026-09-30

**Latest local checkpoint:** Windows build and audit completed below. The older
Linux container's 52-object checkpoint remains historical and is not a blocker
for the Windows workspace.

## Audio buffer diagnostic, normal settings unchanged - 2026-10-01

Compared isolated copied profiles with Core.AudioBufferSize 160/80/160ms, in
that order. Existing mixer default is 80ms; Cubeb requests max(512 frames,
device minimum latency). This does not establish actual device output latency.
The mixer can replay older granules when starved, so fewer empty dequeues alone
does not prove clean playback. Increasing the queue may add audio latency.

Same verified Windows runner, JIT, one logical CPU, Vulkan 3x EFB, normal speed,
formatter experiment ON (normal launch default OFF), Cubeb muted; frame/audio
telemetry without detailed runtime profiling or screenshots. Same extended
hero route; two-second warmup and endpoint before state save:

| Queue limit ms | Measured s | FPS | Worst frame ms | Lowest rolling second | DMA/stream empties |
| --- | --- | --- | --- | --- | --- |
| 160 first | 46.6464 | 29.9702 | 62.3037 | 28 | 0/0 |
| 80 control | 46.9289 | 29.9176 | 114.7865 | 25 | 9/8 |
| 160 repeat | 46.7130 | 29.9702 | 84.9601 | 28 | 0/1 |

Empty counts cover 45/46/46 complete interior seconds respectively. All runs
exit0. Control also suffered worse frame stalls, so this is not a clean causal
comparison or a demonstrated frame-rate gain. 160ms did not eliminate starvation.
Normal launch stays 80ms; retain candidate only in private test profiles. Audio
still needs real playback verification; no new visual verification occurred.
A post-run read-only check found Balanced power plan; the reported 3801MHz WMI
snapshot cannot prove runtime frequency or throttling. No system setting changed.

No source/binary change or rebuild; existing Windows validation remains 39/39
native tests and 141 tooling passed/one skipped. User static edits/assets/saves
preserved. Numeric evidence: evidence/windows-20260930/AUDIO-BUFFER-EXPERIMENT.json.
Next broaden the combat route and correlate audio starvation with stalls across
varied gameplay, rather than treating short saved-state runs as acceptance.
Three qualifying repeats and ten-minute varied combat remain required. Goal unmet.

## Frame-boundary CPU counters and repeated hitch diagnostics - 2026-10-01

Added opt-in callback-thread/process cumulative CPU time, thread cycles and
thread identity at newly rendered frame boundaries. Runtime profiling enables
the OS queries; ordinary frame traces retain unavailable (-1) fields. Analyzer
reports whole-window and per-frame deltas, rejects unavailable/regressing or
changed-thread readings, and does not convert cycles into wall time. Windows
CPU times in these runs advance in 15.625ms increments; sequential queries are
not atomic and cannot precisely separate scheduling from execution per frame.

Build.cmd --cpu jit --jobs 2 initially failed a Windows min macro collision;
fixed NOMINMAX in the new header, then the supported build passed 39/39 native
tests in 8.87s. Final tooling: 142 run, 141 passed, one skipped in 19.929s.
No compiler/linker warning or error in final build; existing CMake warnings
remain. Local LTO ON, source default OFF. User static changes remain untouched.

Same extended hero route, JIT, one logical CPU (mask1), Vulkan, 3x EFB, normal
speed, Cubeb muted, formatter experiment ON (normal launch default remains OFF).
Two-second warmup, endpoint before save; frame and audio telemetry remain on:

| Run | Seconds | New FPS | P99 ms | Worst ms | >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Detailed profile 1 | 46.9803 | 29.9700 | 38.2873 | 66.4966 | 2 | 29 |
| Detailed profile 2 | 47.0796 | 29.9705 | 43.4118 | 96.9200 | 9 | 28 |
| Detailed profile 3 | 46.8468 | 29.9701 | 43.3577 | 80.2466 | 5 | 28 |
| No detailed runtime profile | 46.5130 | 29.9701 | 41.7470 | 62.7387 | 3 | 29 |

All completed successfully, near 100% guest speed. Average 29.97 is nominal 30;
remaining issue is intermittent hitches, not a sustained 17-18 FPS collapse in
this route. These are not three qualifying acceptance runs. Repeat2 frame14642
has 96.92ms host/33.3667ms guest, no JIT compilation, 88.53 million thread cycles
versus 97.34 million in the preceding 33.90ms frame, and 62.5ms quantized CPU time.
It is not explained by extra instruction cycles alone; scheduling/frequency and
runtime costs are not individually resolved. Other long frames contain large
compilation bursts. Hitches remain without detailed runtime instrumentation;
one comparison does not quantify profiler overhead or establish a host cause.

Interior DMA/stream empty dequeues: 0/2, 1/3, 1/1 and 1/1 respectively. No new
native visual captures or audible/device verification; prior route snapshots
do not prove current continuous visual behavior. Crackling remains unresolved.
Next narrow non-compilation host/runtime stalls and separately address cold
compilation bursts; preserve normal-speed/correctness constraints. Three
qualifying repeats and ten-minute varied combat remain outstanding. Goal active.
Evidence/binary hashes: evidence/windows-20260930/FRAME-CPU-COUNTERS.json.

## CPU graphics dispatch tracing and affinity diagnostic - 2026-10-01

Added opt-in RunGpuOnCpu timing for dispatches >=100us and analyzer coverage
regression. Short calls are omitted: recorded coverage is a lower bound, includes
nested rendering/waits/preemption, and cannot rule out total decoding cost.
Supported Windows build passed 39/39 native tests (12.69s); tooling passed
139 tests with one skipped (140 run, 50.009s). Existing CMake warnings remain;
no compiler/linker warning or error found. Local LTO ON; source default OFF.

Extended guest-timed route, JIT, one logical CPU, Vulkan, 3x EFB, experimental
formatter ON (default OFF), Cubeb muted, runtime/audio profiles, no captures:
CPU0 after two-second warmup: 46.9778s, 29.9716 new FPS, 100.0053% guest speed,
P99 40.5253ms, worst 67.0546ms, five >50ms, minimum rolling second 29.
A 52.3424ms frame has no compilation and only 1.489ms recorded slow dispatch;
a 50.0322ms frame has no compilation and 3.6289ms recorded slow dispatch.
This leaves time unexplained; it does not establish CPU execution or a GPU cause.

A diagnostic parent constrained the child to logical CPU4 (verified mask 0x10):
47.1471s, 29.9700 new FPS, 100.0000% guest speed, P99 38.3621ms, worst 67.5393ms,
four >50ms, minimum rolling second 29. Later hero cycles still reach 61.8698ms.
This single non-deterministic comparison is not a demonstrated fix; production
affinity stays unchanged. Investigate compilation bursts and non-compilation
stalls separately, with total CPU execution/scheduling coverage next.

Separate CPU0 visual run completed; all ten native captures inspected in order.
Street, team, enemies, HUD and effects visible; RT cycles through all four heroes.
Final Spider-Man position is near a building without an adjacent enemy, so this
is not proof of uninterrupted varied combat or continuous animation correctness.
No audible playback verification; audio crackling remains unresolved. See
numeric evidence and binary hashes in evidence/windows-20260930/GPU-DECODE-AND-AFFINITY.json.
No proprietary output committed. Three qualifying repeats and ten-minute varied
combat remain outstanding. Goal active/unmet; no new FPS improvement claimed.

## Extended hero combat and recurring stalls - 2026-10-01

Added tools/routes/xbox-timed-hero-combat.json: 106 timed holds totaling 47.46
guest seconds, extending the visible route with RT hero cycling, movement,
light/heavy attacks, powers and block inputs. Visual, separate timing/profile,
and formatter-shadow runs all exited 0. All ten visual captures inspected in
order: active control cycles Spider-Man -> Captain America -> Iron Man ->
Wolverine -> Spider-Man amid enemies, hits/projectiles, team, street and HUD.
Spider-Man's HUD is red in the Iron Man capture; do not claim every hero stayed
healthy. These snapshots do not validate every intermediate frame/animation.

Same JIT runner, one logical CPU, Vulkan, 3x EFB, Cubeb muted. Formatter ON
for visual/timing runs, shadow for comparison; default remains OFF. No runtime
changes or rebuild; prior supported Windows build passed 39/39 native tests.
No-capture runtime/audio-profile diagnostic after two seconds: 46.7131s,
29.9702 newly rendered FPS, guest speed 100.0005%, P99 40.9289ms, worst 67.8137ms,
five frames >50ms, lowest rolling second 29. Later hero cycles alone: 35.8325s,
29.9728 FPS, P99 39.2988ms, worst 53.2186ms, two frames >50ms. This is not acceptance.
One late 53.2186ms frame has no JIT compilation, 6.0611ms traced waits; another
51.8684ms frame has 17.0198ms compilation. A separate stall remains unexplained.
Next instrument CPU-side graphics FIFO decoding (RunGpuOnCpu), absent from the
current wait timers, before calling unclassified time guest CPU execution.

Shadow completed 1,845,049 comparisons, 710,670 floating calls, zero output/FPSCR/
preserved-register mismatches and abandoned calls; one pending at shutdown is
unverified. This broadens comparison coverage but does not prove full timing or
world-state equivalence. No audio listening/device verification; crackling is
unresolved. Three qualifying repeats and ten-minute varied combat still needed.
Evidence: evidence/windows-20260930/EXTENDED-HERO-COMBAT.json. Game captures,
states and logs remain private. Goal active/unmet; no new FPS gain claimed.

## JIT metadata allocation pool rejected - 2026-09-30

Tested a small-node pool shared by the 64 backpatch metadata hash tables.
The supported Windows candidate build passed 39/39 native tests in 5.60s,
including temporary value-lifetime, clear/reuse and allocation-release checks.
Tooling: 139 run, 138 passed, one skipped in 53.784s.
Two candidate diagnostics did not demonstrate better compilation or frame tails:

| Run | Compilations | Compile elapsed ms | Worst frame ms | Lowest rolling second |
| --- | --- | --- | --- | --- |
| Original prior | 7431 | 291.2170 | 75.1227 | 29 |
| Pooled first | 7428 | 328.2433 | 89.8683 | 28 |
| Pooled repeat | 7975 | 358.6358 | 94.9575 | 27 |
| Original restored | 9685 | 330.6934 | 78.3300 | 29 |

Same short guest-timed clear-combat route, full JIT, one logical CPU, Vulkan,
3x EFB, experimental formatter ON, Cubeb muted, runtime/audio profiling and
no captures. Runs are not deterministic; spans include preemption and nest.
These are diagnostic results, not a controlled proof of allocator causality.
No new visual or audible validation. No frame-rate gain is claimed.

Rejected the pool change; both edited source/test files restored exactly to HEAD.
Rebuilt the original implementation using Build.cmd --cpu jit --jobs 2: exit0,
39/39 native tests in 5.70s. No compiler warnings/errors found; existing CMake
deprecation, Wayland, long-path and lz4 IPO warnings remain. Local LTO ON,
source default OFF. Runtime/port/module-info/launcher binaries and hashes recorded
in evidence/windows-20260930/METADATA-POOL-REJECTED.json. Trial patch/binary and
proprietary test outputs retained privately in .local; user static work untouched.

Next investigate guarded precompilation and non-compilation stalls, not further
allocator tuning without new evidence. Formatter remains default OFF; solid-30
and audio unresolved, three qualifying repeats and ten-minute varied combat
still required. Goal active/unmet.

## Clear combat compilation-burst diagnosis - 2026-09-30

One additional diagnostic on the new guest-timed route exited 0: existing JIT
runner, one logical CPU, Vulkan, 3x EFB, experimental formatter ON, Cubeb muted,
no screenshots, runtime/audio profiling. No runtime changes or rebuild.
The 10.9009-second post-warmup window contained 7431 JIT compilations totaling
291.217ms elapsed. Worst frame 75.1227ms included 35.5118ms compilation and
1502 newly encountered blocks (34749 guest instructions); none of those PCs
had compiled earlier in this run. A 55.547ms frame compiled 410 new blocks,
23.6967ms total. Another 55.1219ms frame compiled only two blocks/0.5261ms;
compilation is not the sole remaining cause. GPU fence spans totaled 2.5173ms.

The jit_backpatch timer measures metadata insertion, not runtime fault handling.
Its 13.6976ms coverage in the second spike is nested inside emission/compilation,
not extra time to add to those spans. All spans include preemption; unclassified
time is not assumed to be CPU work. No new visual/audible quality validation.

Next investigate same-thread JIT prewarming during loading to move bulk cold
compilation out of combat, while separately diagnosing non-compilation stalls.
Direct arbitrary Jit calls are not safe: analysis failures can modify guest
npc/Exceptions and invoke CheckExceptions. Any warming entry point must fail
closed on invalid code/context, preserve translation and feature flags, retain
GQR/speculative-register guards, ordinary SMC invalidation and cache lifetime,
and account for loading cost. No warm-cache implementation or success claim yet.
Formatter remains default OFF. Solid-30 and audio goals remain unmet; three
qualifying repeats and ten-minute varied combat are still required.
Evidence: evidence/windows-20260930/CLEAR-COMBAT-COMPILE-BURSTS.json.

## Visible guest-timed combat and audio comparison - 2026-09-30

Added tools/routes/xbox-timed-clear-combat.json: 26 guest-clock holds totaling
12.26 seconds, covering approach, light/heavy attacks, powers, block and camera.
Five fresh frames before the first capture avoid the observed magenta restore
frame; open-street movement avoids the previous tree occlusion. This is a short
route improvement, not a general visual-readiness fix or ten-minute session.
Both visual runs and both separate no-screenshot timing runs exited 0. Every
hold completed; maximum release lateness was 72 guest cycles at 729MHz. All six
native captures per visual run inspected sequentially: visible living player,
combat poses/web effects, enemies/team, street background and HUD. Grab/use,
charge behavior, exact animation speed and world/interrupt equivalence remain
unproven. Combat outcomes differ; this is not a deterministic input movie.

Same existing Windows runner, JIT, one logical CPU, Vulkan, 3x EFB, Cubeb muted.
No runtime changes or rebuild this checkpoint. After two seconds warmup and
ending at the last input hold (before saving), no-screenshot diagnostics:

| Formatter | New FPS | Guest speed | P99 ms | Worst ms | >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Original/default | 26.1435 | 100.1500% | 59.6310 | 79.9998 | 35 | 24 |
| Experimental on | 29.9708 | 100.0025% | 55.1822 | 70.9986 | 5 | 28 |

Audio profiling remains enabled, so these are diagnostics, not acceptance runs.
Ten full interior audio seconds: original DMA/stream empty dequeues 1/0,
candidate 1/1; maximum callback gaps 17.4256/12.8155ms. Screenshot runs had
41/36 and 49/47 empty dequeues respectively and substantial capture stalls;
do not use those runs as unintrusive performance evidence. These counters do
not establish audible crackling or device underruns. No listening/device-output
verification; sound remains unresolved. Formatter remains default OFF and the
solid-30 goal unmet. Three qualifying repeats and ten-minute varied combat are
still required. Next investigate remaining stalls/audio starvation without
capture overhead and extend healthy varied combat/correctness coverage.
Evidence: evidence/windows-20260930/GUEST-TIMED-COMBAT.json. Private images,
states and logs stay outside Git; user static work remains untouched.

## Guest-clock input timing and movement comparison - 2026-09-30

Added process-local xbox_time commands: port, milliseconds (1..600000), input
fields, optional release, and a required path for the timing receipt. Input
release runs in CoreTiming rather than waiting for rendered frames or a host
sleep. Receipts record start/end ticks, requested duration, clock rate, lateness
and completion. The benchmark rejects inconsistent/incomplete receipts, reused
receipt paths and releases more than 1ms late. Controller polling retains its
usual guest cadence. Between-command gaps are not a deterministic input movie.

First live testing caught event-name reuse: RegisterEvent keeps the old callback,
so the second hold stalled. Original timed out; an initial candidate was started
too early and terminated. Neither attempt is accepted. Fixed one registered
event per core lifetime with a current hold, rebuilt, then ran the retries serially.
Final Windows build exited 0: 39/39 native tests in 7.46s; tooling 138 passed,
one skipped of 139 in 20.767s. No compiler warnings/errors; existing CMake
deprecation, Wayland, long-path and lz4 IPO warnings remain. Local LTO stays ON,
source default OFF. Runtime executables rebuilt; hashes are in the evidence.

Both fixed runs exited 0 using JIT, one logical processor, 3x EFB and Cubeb muted.
Two-second movement hold: original 9 guest cycles late, candidate 0. Half-second
neutral hold: original 50 cycles late, candidate 0 (729000000 ticks/second).
Movement window contained 56 new frames original versus 60 candidate; neutral
contained 12 versus 15. These are counts in guest-clock windows, not host FPS.
All six native captures inspected sequentially. Spider-Man moves from the street
to the same curbside area in both modes; scene, enemies, team, HUD and effects
remain present. This supports a bounded movement comparison, not numeric position,
full world-speed or interrupt equivalence. No audible/device-output verification.

Route: tools/routes/xbox-timed-movement.json, using the private prepared statue
state. Evidence: evidence/windows-20260930/GUEST-TIMED-INPUT.json. Next broaden
guest-timed movement/animation/combat comparisons and establish a healthy visible
varied route. Formatter remains default OFF; crackling and solid-30 combat remain
unresolved. Three qualifying repeats and ten-minute varied combat still required.

## Formatter contract guards and combat shadow validation - 2026-09-30

The default-off formatter now hashes the known caller epilogue as well as its
existing formatter region. It rejects overlapping format/output/caller-frame
storage, wrapped or misaligned frames, unavailable FPU and enabled FP exception
modes. Shadow validation explicitly fails on changed caller-preserved GPR/FPR/CR
state; the benchmark requires the new ABI mismatch counter. Volatile differences
remain diagnostic. These checks do not establish interrupt/timing equivalence or
full transitive helper-code identity; the experiment is still default OFF.

Supported Windows build exited 0: 39/39 native tests in 7.67s. Tooling: 138 run,
137 passed, one skipped in 20.896s. No compiler warnings/errors. Existing CMake
deprecation, Wayland, long-path and lz4 CMP0069 IPO warnings remain. Local LTO
cache remains ON, source default OFF. Produced runtime binaries include
moderngekko-run.exe, moderngekko-port.exe and moderngekko-module-info.exe.

Combat shadow: 469646 completed comparisons, 179931 floating calls, zero output,
FPSCR or preserved-register mismatches, zero pending/abandoned. All five shadow
and five replacement captures inspected sequentially: scene/HUD/enemies and hit
effects present. Replacement Spider-Man becomes downed behind the tree; this is
not a suitable sustained-combat route. All three runs exited 0. The separate
no-screenshot/no-PCM timing run measured 29.9599 FPS after two seconds, P99
46.3253ms, worst 47.4481ms, lowest rolling second 29, guest speed 99.9663%.
Cold restore maximum was 272.4009ms. This is one short diagnostic, not acceptance
or a new substantial performance gain. Cubeb enabled/muted; private pre-volume
PCM captured in the visual run, structurally checked but not listened to. Audio
crackling and device playback remain unverified.

Next fix the measurement limitation: xbox_frames holds for newly rendered frames,
so at 25 versus 30 FPS the input lasts different amounts of game time. Add
guest-clock input timing for movement/animation speed comparison, then broaden
correctness and varied combat validation. Goal active/unmet; three qualifying
repeats and ten-minute varied combat remain required. Evidence and binary hashes:
evidence/windows-20260930/FORMATTER-CONTRACT-GUARDS.json. Proprietary outputs stay
private; unfinished static work remains untouched.

## Original formatter combat cadence - 2026-09-30

The earlier near-30 statue/LTO measurements below used the opt-in formatter
replacement. That experiment remains default OFF and has not passed complete
correctness/timing acceptance. It is not the validated production baseline.

Two new runs with the original formatter, the same LTO runner and statue route,
one logical CPU, Vulkan and 3x EFB gave the following after a two-second warmup:

| Run | Newly rendered FPS | Guest speed | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- | --- |
| Original first | 25.4255 | 99.9960% | 53.4737 | 61.6665 | 53 | 23 |
| Original repeat | 25.2343 | 100.0035% | 60.2698 | 64.8466 | 64 | 23 |

Guest intervals include 104/110 frames at 50.050ms and 187/183 at 33.367ms.
These short restored runs are not qualifying repeats or proof of original
console performance. They show that host JIT speed alone is insufficient to
remove delays already present in this measured guest timeline. Intrusive block
profiling separately attributes about 20% of resident guest cycles to the decimal
conversion page; this is not a host CPU time percentage. Next investigate a
behavior-preserving game-library optimization with explicit ABI, interrupt and
world-speed validation, alongside host stalls and improved combat routing.

All four investigation runs exited 0. All five native visual captures were
inspected sequentially: street/statue, team/HUD, enemies and hit effects present;
player partly tree-occluded and low health, statue intact. This is sampled visual
evidence, not full-frame inspection. Cubeb was active but muted. The repeat's
12 interior profile seconds recorded one DMA and three streaming empty dequeues,
maximum callback gap 14.4052ms. Those counters are not device underruns or audible
click counts, and their interval differs from the FPS window. Crackling remains
unresolved; no audible verification. No runtime/source default changed and no
new build was required for these measurements. The preceding 39/39 Windows
build result remains scoped to that build. Goal active/unmet: three qualifying
repeats and ten-minute varied combat still required. Numeric evidence and private
artifact hashes: evidence/windows-20260930/ORIGINAL-FORMATTER-CADENCE.json.

## Windows optional LTO trial - 2026-09-30

The existing ENABLE_LTO option failed with MSVC C2220/C5049 because the shared
PCH embedded absolute build paths under deterministic compilation. A conditional
PUBLIC /pathmap on build_pch maps the build root to a relative prefix; consumers
inherit the same mapping. Deterministic mode and warning-as-error checks remain.
The supported Build.cmd --cpu jit --jobs 2 path then completed: 39/39 native tests
in 17.07s. Existing CMake warnings, D9025 (/W3 overridden by /W0), and D9002
(ignored -fexceptions in a dependency) remain. No final build errors.

Short statue-route comparisons after the same two-second warmup:

| Build/run | Average FPS | P99 ms | Worst ms | Frames >50ms | Lowest rolling second |
| --- | --- | --- | --- | --- | --- |
| Baseline earlier | 29.8588 | 46.4992 | 60.6316 | 2 | 29 |
| Baseline repeat | 29.9659 | 48.5620 | 55.5134 | 2 | 29 |
| LTO first | 29.9710 | 42.9138 | 46.8131 | 0 | 29 |
| LTO repeat | 30.0332 | 46.7293 | 51.0239 | 2 | 29 |

These approximately ten-second segments are not deterministic movies or qualifying
acceptance repeats. Full restored-route maxima were 270.1/293.8ms baseline and
304.7/446.1ms LTO; cold restoration is reported separately, not mislabeled as a
steady combat stall. LTO is not established as a major FPS improvement or a fix
for pacing. No default was changed: ENABLE_LTO remains OFF in source; the local
experimental CMake cache is ON and the current runner is the LTO candidate.
Original runner retained at .local/build/windows-x64/runtime/moderngekko-run-pre-lto.exe.
A renamed-baseline launch timed out before restoration; using its original name
completed. Cause unproven; future comparisons should use the original filename.

All five candidate native captures inspected sequentially: statue/street, enemies,
team, HUD and hit effects present. Tree obscures the player, health becomes low,
and statue remains intact. This route needs clearer visibility and progression.
Cubeb enabled/muted; first LTO timing run had eight full interior seconds with no
DMA/streaming empty dequeues and 10.7509ms maximum callback gap. Not a listening
or device-output check; crackling unresolved. JIT, one logical CPU, normal clocks,
3x EFB and opt-in formatter retained. No proprietary captures/saves tracked.
Evidence: evidence/windows-20260930/WINDOWS-LTO-TRIAL.json. Goal active/unmet;
three qualifying repeats and ten-minute varied combat still required. Next pursue
substantial runtime/compilation cost reductions and improve the varied route.

## Xbox fusion aiming and tutorial validation - 2026-09-30

Generated Xbox profile v2 adds right-stick aiming while LB+RB are held. This
reaches tutorial/profile icons between the existing face-button corner targets.
Press A while aiming to confirm; stick movement suppresses the fusion-request
shake and camera input. Neutral stick retains existing corner shortcuts. Existing
custom profiles are preserved, so this does not silently migrate user settings.

The final Windows build passed 39/39 native tests (7.78s); tooling passed 137
of 138 tests with one skip (19.824s). No compiler errors/warnings; existing CMake
deprecation, Wayland and object-path warnings remain. The exact final runner was
used in the successful native replay. Tests cover fractional aim, preset override,
dead-zone fallback, all directions, delayed confirmation and action isolation.

All five successful replay captures were inspected sequentially: ready prompt,
correct icon aim, dismissed dialog, Spider-Man/Captain America fusion with 738
damage, then Hero Training: Fusion (COMPLETE). Street, heroes, HUD, effects and
debris were visible. Route tools/routes/xbox-fusion-ready.json starts from the
private prepared ready-prompt state, not normal boot. An earlier pickup-inclusive
replay exited 0 but failed visually to advance the tutorial; pickup repeatability
is still open. The successful follow-up state is private under
.local/automation/xbox-fusion-ready-stick-20260930/fusion-followup.sav.

This clears a test-flow obstacle, not the performance goal. Cubeb was active but
muted; no listening/device-output verification or new audio/FPS improvement is
claimed. Physical controller testing, other fusion partners and revive remain
open. One logical CPU, Vulkan, 3x EFB and normal speed were retained; formatter
optimization was opt-in. Next establish varied encounters from the completed
fusion state and address cold compilation/audio stalls. Three qualifying repeats
and ten-minute varied combat remain required. Evidence:
evidence/windows-20260930/XBOX-FUSION-AIM.json. Goal active and unmet.

## JIT reverse-link index sharding - 2026-09-30

Finalization phase traces isolated a reverse-link hash-table growth pause:
32768->65536 buckets cost 6.4294 ms. Split the destination-address index into
64 standard maps with stable value references. Linking/unlinking feature checks
and branch patching are unchanged; empty destination entries are still erased.
No extra threads or large upfront reserve. Opt-in phase spans retain >=100us
entry-map/range/link work; slow index logging requires runtime profiling.

Worst linking phase fell from 6.515 ms to 1.300/0.3149 ms in short/full candidate
probes. Whole-finalization maxima fell from 6.5169 to 1.3015/1.0299 ms. Total
compilation cost remains substantial and routes are not deterministic. The full
candidate averaged 29.9701 FPS, P99 39.3357 ms, maximum 94.2242 ms, lowest rolling
second 28, six frames over 50 ms, guest speed 100.0002%. After five seconds of
warmup, maximum remained 51.2082 ms with two frames over 50 ms. This is a targeted
stall reduction, not the requested sustained-30 result or major overall gain.

All five native probes exited 0. All seven candidate native captures inspected
sequentially: combat, scenery/HUD, effects, damage, knockback, broken prop and
hero changes present. Fusion tutorial persists; completed fusion and varied
encounters unverified. Cubeb active/muted; 63 complete interior seconds had two
DMA and one streaming empty dequeues. Not click counts. Crackling unresolved;
no listening/device-output verification. One baseline frame also had a 50.05 ms
guest interval, so not every slow host interval is host-only delay.

Final Windows build exit0, 39/39 tests in5.87s; tooling138 ran,137passed,1skip
in21.645s. Native container test checks stable references and 200000 operations
against the old container. No compiler warnings/errors; existing CMake warnings
remain. Final comment/line-ending rebuild changed artifact hashes after probes;
exact tested and final hashes are recorded separately. Goal active/unmet;
three qualifying repeats/ten-minute varied combat remain open. Next reduce total
first-use compilation and validate fusion/varied encounters, not just this fight.
Evidence: `evidence/windows-20260930/JIT-LINK-INDEX-SHARDING.json`.

## Continuous Xbox combat and queued replay - 2026-09-30

Added `tools/routes/xbox-continuous-combat.json`: attacks, powers, movement,
blocking, jumping, fusion-selection attempts and hero changes without a long
neutral tail. `--queue-route` publishes the complete command sequence before
waiting for receipts, removing per-action host round trips through the native
50 ms command poll. Default serial mode remains available. This is not a
deterministic input movie; advisory status can remain stale while the queue
drains. Timing runs use `--no-screenshots` and end at the route's `read_timing`
guest tick, excluding capture/shutdown overhead. This changes the test route,
not the engine's performance.

The serial continuous route reproduced a 17-FPS rolling second and 156.135 ms
worst frame despite 29.8483 FPS average. Two overlapping one-second audio buckets
contained 11 DMA and 11 streaming empty dequeues. Those counts are not audible
clicks; temporal correlation does not prove the stall's cause. Later replays did
not reproduce that full burst. External CPU sampling cannot explain a burst
that did not recur in the sampled run.

Queued timing/profile runs averaged 29.9702/29.9705 FPS at normal guest speed,
with worst frames 92.225/90.7416 ms, rolling-second minima 28/28, and six frames
above 50 ms each. The profile's worst frame included 55.2543 ms JIT compilation:
32.8388 ms emission, 15.6272 ms finalization, 4.4357 ms analysis (nested spans).
A later 59.4009 ms frame had no JIT/shader/pipeline compilation and 8.4644 ms of
measured waits; remaining elapsed time is not proof of CPU execution. Prior
trace analysis found 1572/1844/1221 first-seen blocks in three cold bursts, with
no repeat addresses in those bursts. Both cold compilation and later stalls
need work; no audio fix or sustained-solid-30 claim is made.

All 14 native captures across two visual routes were inspected sequentially:
street, HUD, heroes, enemies, combat effects and hero changes present. Fusion
tutorial persists; completed fusion and varied encounters remain unverified.
Audio ran through muted Cubeb with private pre-volume PCM capture; no listening
or device-output verification. Crackling remains unresolved. These are short
one-encounter diagnostics, not qualifying repeats or ten-minute acceptance.

All seven runs exited 0 and completed their routes. Tooling: 138 tests ran,
137 passed, 1 skipped in 23.598s. Native source/binaries unchanged from bd9736ff
(prior Windows build exit0, 39/39 runtime tests). No new native rebuild needed
for this Python/route checkpoint. One logical CPU, 3x EFB, normal clocks and
formatter opt-in ON remain unchanged. Goal active/unmet. Next profile bulk JIT
finalization and correlate later stalls with CPU service in the same run.
Evidence: `evidence/windows-20260930/CONTINUOUS-COMBAT-QUEUE.json`.

## JIT backpatch rehash spike reduction — 2026-09-30

Thread CPU/cycle diagnostics isolated a 54.6 ms emission of only 29 guest
instructions (181.7 million thread cycles). Slow-instruction tracing located
a floating-point load; memory metadata tracing then measured a 51.0 ms map
insertion. Growth thresholds were 65537, 131073 and 262145 entries: whole-table
rehashing was a concrete source of the pause. Windows CPU time is quantized;
cycles are not converted to wall time.

Replaced the JIT backpatch table with 64 independent pointer hash tables.
Value references remain stable through insertion; missing lookup and clear/reuse
semantics are preserved. No extra threads or large up-front reserve is used.
A native test covers 300000 entries, stable references through growth,
replacement, missing keys, clearing/reuse and page-spaced keys. Fault lookup
uses the returned value pointer. Game operations/protections are unchanged.

Two candidate runs reduced maximum measured insertion from 51.0003 ms to
3.5261/1.6944 ms and longest single compilation from 51.1061 ms to
6.5887/6.9024 ms. The first candidate's affected frame12676 changed from
85.4468 to 34.7919 ms. Overall P99 stayed about 42.2 ms; worst frames were
97.1453/84.2682 ms, average FPS 29.9770/29.9711. Total compilation time did
not improve. Remaining cold bursts compile many blocks in one frame; this
is targeted spike reduction, not sustained solid-30 combat acceptance.

Opt-in diagnostics add emission CPU ns/cycles/address/block-size fields;
-1 means unavailable counters. --jit-emission-address selects one block,
or 0xffffffff for all. Profiling must be enabled; instruction/backpatch
spans below 100us are omitted. Nested spans overlap. Profiling adds overhead.

Final Windows build exit0: 39/39 tests in 7.74s. Tooling137 ran:136 passed,
1 skipped; final span6/6 passed. No compiler warnings/errors found; existing
CMake configuration warnings remain. All seven endpoint captures inspected:
scenery/HUD/characters present, varying attacks/downed heroes, some endpoints
away from the crowd. Neutral route tails do not prove sustained varied combat.
Cubeb was active but muted; private mixer recordings do not verify audible
quality. Crackling unresolved. No qualifying repeats or ten-minute acceptance.

Next address bulk-compilation bursts and improve the varied-combat route.
Goal remains active. Evidence and binary hashes:
evidence/windows-20260930/JIT-BACKPATCH-SHARDING.json.

## Audio capture and JIT compilation stalls — 2026-09-30

Added explicit --capture-audio with --profile-audio --audio Cubeb: retain at most
60 seconds of final stereo 16-bit mixer PCM in a preallocated ring, write WAV
only after Cubeb callbacks stop. Volume/mute is applied downstream by Cubeb, so
this records a non-silent signal while host playback stays muted. Ordinary
benchmarks clear inherited capture settings. Non-stereo capture is unsupported
and validation fails if no valid stereo file is produced. Recordings contain
proprietary audio and stay under ignored .local, never Git or source backups.
This captures neither Windows/device playback nor missing hardware callbacks.

Extended opt-in runtime spans to JIT compilation, its analysis/code-generation/
finalization phases, and Vulkan shader/pipeline compilation. Spans include
preemption and nested spans overlap; do not add parent and child durations.
The analyzer's unclassified field means non-wait elapsed time and includes
recorded compilation; it is not CPU utilization or additional work to sum.

Default-size diagnostic: 29.9701 FPS, P99 38.9769 ms, maximum 92.3095 ms,
minimum rolling second 29. All 1230 guest intervals were 33.367 ms. The worst
frame contained 54.4101 ms in JIT compilation; pipeline work was negligible.
A 1024-instruction block-limit experiment did not fix it (max101.8018 ms,
P99 42.5999 ms) and was removed from source and tooling. Final detailed trace:
29.9768 FPS, P99 40.6840 ms, maximum89.6308 ms, minimumrolling28. Its worst
frame included 56.0475 ms in JIT emission, 0.202 ms analysis and 0.3239 ms
finalization. Next separate actual compiler CPU consumption from preemption
inside emission and inspect the expensive code-generation path. No JIT limit,
clock adjustment, rendering reduction or default formatter promotion remains.

Three private PCM recordings contain 48.532/48.772/48.572 seconds of 48-kHz
stereo audio. Peaks23415/28551/25122, no full-scale samples. These signal
statistics DO NOT verify audible quality or absence of crackling. First run's
40 aligned seconds: 4000 callbacks, maximum gap13.3603 ms; DMA/streaming empty
queue counts2/2 (not audible-click counts). The user's sound report remains
unresolved; there has been no listening or device-output verification.

All three native endpoint captures were inspected sequentially. Street scenery,
HUD and heroes are present; baseline ends largely away from the crowd, while
later endpoints show enemies/combat and incapacitated heroes. This does not
establish sustained varied combat. All probes: headless JIT/Vulkan, one logical
CPU, 3x EFB, normal clocks, formatter opt-in ON, Cubeb volume0. No qualifying
three-repeat or ten-minute acceptance. Goal active/unmet; manual test deferred.

Final Windows build exit0, 38/38 runtime tests7.20s. Full tooling suite136 tests:
135passed,1skip22.132s; final targeted runner12/12 and span5/5 passed. No compiler
warnings/errors found; existing configure warnings remain. Exact artifacts,
failed experiment and measurements: evidence/windows-20260930/AUDIO-JIT-STALLS.json.

## Mixed formatter rewrite experiment — 2026-09-30

A bounded 64-address unsupported-format census found frequent mixed string,
signed-integer and float calls. The census run counted 399572 unsupported calls,
including 11332 overflow events beyond its first 64 addresses; it is not an
exhaustive ranking. Logs contain addresses/counts, not proprietary format text.

Extended the default-off formatter experiment with transactional bare %s, %d,
%f and %% support. It reads separately bounded GPR/FPR save-area cursors,
retains the original code/hash/caller/direct-RAM guards, rejects string/output
and string/va_list aliasing, null or unterminated strings, spills beyond saved
register arguments, unsupported formats and unsupported floats. Rejections
leave guest output and argument cursors untouched. Supported calls still create
the complete text and advance the caller's va_list; no logs are suppressed.

Shadow compared 769742 completed eligible calls with zero output/va_list/return/
FPSCR mismatches and zero abandoned calls. One of 769743 eligible calls remained
pending at shutdown and is explicitly unverified. Unsupported calls were 56158;
this different live run is not an exact causal reduction measurement. Observed
register-change masks remain GPR=00001ff9, PS0=00000003, PS1=00000000, CR=23.
The census-only run observed wider volatile floating clobbers (PS0 mask1f,PS1
mask03); register preservation is not architectural equivalence. Timing and
broader alias/caller behavior remain unproven; keep the rewrite disabled by
default. It is a library replacement, not instruction-timing equivalence.

The benchmark now requires an actual matching formatter summary, nonzero
completed comparisons/replacements, zero mismatch/abandoned counters and
reconciled eligible/compared/pending counts. Pending original calls at shutdown
are bounded and reported, never counted as verified comparisons.

The first expanded candidate measured 29.9796 FPS at 100.0319% guest speed,
P99 39.0198 ms, maximum 84.8094 ms and minimum rolling one-second FPS 28.
All 1230 guest intervals were 33.367 ms. Four host intervals exceeded 50 ms;
they occurred early (frames 12630,12662,12676,12753) with negligible GPU fence
wait and 68–79 ms unclassified elapsed time. Profile JIT/shader compilation and
other host work next; unclassified time alone does not identify the cause.
This single headless diagnostic still fails solid-30 delivery. Three native
endpoint captures were inspected sequentially: actual combat, effects, scenery
and HUD are present, with incapacitated heroes in the candidate endpoint. The
route's neutral tail is not proof of continuous varied combat or ten-minute
acceptance. Source/frame pacing and audio still need broader validation.

Windows build passed: 37/37 runtime tests in 7.19 s, including mixed-format
integer extremes, negative zero, literal percent and transactional rejection.
Python suite: 134 run, 133 passed, one skipped (22.789 s); after pending-count
handling changed, all 11 combat-runner tests passed (0.065 s). No compiler
warnings/errors found; existing CMake warnings remain. Exact binaries and
native diagnostic results: evidence/windows-20260930/MIXED-FORMATTER.json.
Goal active/unmet; sound crackling remains unresolved. All runs remain headless
Vulkan/Cubeb volume=0, JIT, one logical CPU, 3x EFB and normal clocks. No audible
quality claim, qualifying repeat or ten-minute combat acceptance is made.

## Formatter caller-state audit and remaining hotspots — 2026-09-30

Extended the default-off formatter shadow diagnostic to aggregate changed
register numbers (GPR, both paired-single lanes, and condition-register fields).
No raw register contents are written. Original code still runs in shadow mode;
replacement behavior and the default-off setting are unchanged.

A headless combat shadow run compared 398399 supported calls, including 150262
floating calls: zero output/va_list/return/FPSCR mismatches, abandoned or pending
samples. Original code changed GPR 0 and 3–12, floating PS0 registers 0–1, and
CR fields 0, 1, 5. No PS1 changes were observed. Other inspected registers stayed
unchanged. This strengthens the observed caller-state audit; it is not complete
architectural equivalence, interrupt timing, alias coverage or whole-game proof.
Register masks describe original execution, not errors; in replacement mode
there are no shadow register samples and zero masks are not validation.

An intrusive block profile with replacement enabled completed after the headless
submission fix. It recorded 36,504 resident executed blocks, 31.991 billion
cycles versus 32.541 billion guest ticks and zero skipped idle ticks. Region
803c1000 still accounts for 12.463% of recorded cycles; 8036f000 for 12.194%,
80368000 for 7.400%. Inspection identifies remaining decimal conversion and
indirect per-element/paired-single processing loops. The narrow replacement
handled 928630 calls but 918823 were unsupported; investigate those unsupported
formats and live callers before another rewrite. These whole-run formatter
counts and gated resident-block counts have different scopes. Profile timings
are heavily perturbed and must not be cited as release FPS.

A separate candidate run without block profiling averaged 29.8246 FPS at
100.0001% guest speed, P99 51.2428 ms, maximum 84.9062 ms and minimum rolling
one-second FPS 28 (frames 12620–13850). Guest intervals: 1074 at 33.367 ms,
84 at 50.05 ms, 72 at 16.683 ms. It still fails sustained-30 pacing. Runtime/audio
span profiling remained enabled; this is a diagnostic, not visible acceptance.
All three native endpoint captures were inspected in order. Heroes, Doombots,
street scenery, HUD and combat effects were present; the candidate endpoint
shows a fusion result and incapacitated heroes. Not sustained varied combat.

Windows build passed, 37/37 runtime tests in 7.57 s. No compiler warnings/errors
found; existing CMake deprecation, Wayland and object-path warnings remain.
Exact build artifacts and candidate timing are in
`evidence/windows-20260930/FORMATTER-STATE-AUDIT.json`. All new runs are headless,
JIT, Vulkan, 3x EFB, one host CPU, normal clocks and muted Cubeb. The user's
visible-performance and sound failures remain unresolved. Goal active/unmet;
no manual retest requested and no experimental replacement promoted.

## Headless GPU submission diagnosis — 2026-09-30

The user's visible 17–18 FPS and poor-audio report still fails acceptance.
Added opt-in bounded runtime spans (`--profile-runtime`, environment
`OPENMUA2_RUNTIME_SPANS`) for throttle, GPU pacing, worker, Vulkan fence/submit/
present and Presenter elapsed time. `tools/analyze_runtime_spans.py` merges
nested spans and isolates CPU-thread waits; unclassified time is not CPU time.
Traces are emitted after shutdown, capped at 262144 spans, and rejected when
samples are dropped. Ordinary benchmark runs clear inherited span profiling.

Found and fixed a headless-specific scheduling defect: Presenter returned before
submitting GPU work, batching many frames until resources filled. Headless
frames now flush pending vertices and submit through the existing nonblocking
backend Flush path. Visible presentation is unchanged. Do not extrapolate this
fix to the user's visible playtest or treat old headless stalls as its diagnosis.

Matched route/profile diagnostics (one host CPU, JIT, Vulkan, 3x EFB, normal
clocks, formatter off, Cubeb volume=0) reduced accumulated fence waits from
5.041 s to 0.00694 s over frames 12620–13850. P99 fell from 93.493 to 50.777 ms;
average FPS rose from 27.669 to 28.581, minimum rolling 1 s FPS from 21 to 24.
Repeat: 27.679 FPS, P99 51.746 ms, total fence waits 0.00658 s.
Scene progression differs; these are bounded diagnostics, not a controlled
visible-game FPS gain. Guest cadence still includes 50.05 ms frames. The goal
remains active/unmet, and sound is unresolved. Native endpoint captures were
inspected; they show street scenery, heroes/Doombots, combat and HUD. This does
not establish continuous varied combat, normal boot, or audible quality.

Windows build: `Build.cmd --cpu jit --jobs 2`, exit 0; 37/37 CTest tests passed
in 7.21 s. Python suite: 133 run, 132 passed, 1 skipped, 23.292 s. Configuration
warns about vendored deprecations, missing Wayland and long object paths; no compiler errors/warnings
found. Exact binaries, run hashes, repeat results and limits are recorded in
`evidence/windows-20260930/HEADLESS-SUBMISSION-DIAGNOSIS.json`.
Next: profile remaining guest frame-production stalls and the visible rendering
path without host UI automation; do not request another manual test yet.

## Audio and frame-cadence diagnosis — 2026-09-30

Added an offline synthetic mixer benchmark and opt-in numeric audio profiling.
Use --profile-audio --audio Cubeb with tools/run_combat_benchmark.py; explicit
profiling permits a headless Cubeb diagnostic, while ordinary headless runs
remain silent. Inherited profiling is cleared for ordinary benchmarks. Counters
are bounded, written after callbacks stop, and contain no audio samples.
Channel IDs: 0 DMA, 1 streaming, 2–5 remote speakers, 6 portal, 7–10 GBA,
11 Cubeb callback. Producer counts cover only 0/1; other producer fields are
unavailable. Final traces include steady-clock anchors for frame correlation.

The synthetic full mixer used about 1% of one CPU, so removing auxiliary mixers
is not a credible tens-of-FPS solution. Live traces found subnormal fades in
auxiliary channels, but all mixing occupied only 1.21523 s across 44 aligned
seconds (2.762% elapsed time, including preemption). DMA/streaming queues emptied
2/7 times in that interval; those counts are not audible-glitch counts.

All new gameplay probes were headless Vulkan/Cubeb with isolated volume=0,
one logical CPU, 3x EFB and formatter off. They are diagnostics, not visible
playtest or audible-quality acceptance. With mid-route screenshots the worst
interval was 533.65 ms; without captures it fell to 100.83 ms. Scene progression
varied, so do not label this a controlled gameplay optimization.

The no-capture run averaged 27.117 FPS at 100.073% guest speed. The last instrumented run
measured 27.180 FPS over frames 12620–13850, P99 93.91 ms, maximum 112.59 ms and
minimum rolling 1 s FPS 20. It still fails. Its guest cadence included 237
50.05-ms intervals among 1,230 intervals. A five-second process sample used
66.62% of the single allowed CPU; that is limited headless evidence, not proof
about the visible 17–18 FPS manual test. Investigate frame production and waits
before investing in small mixer optimizations.

All four initial native captures were inspected sequentially; the last instrumented run's
one endpoint capture was inspected too. Street scenery, heroes, Doombots,
damage/effects and HUD were present. The final scene has Wolverine active and
Spider-Man down. The neutral-input tail and incapacitation do not establish
ten-minute varied combat. The final screenshot is outside the measured interval.
Audio was muted; the user's poor-audio report remains unresolved.

Windows build/tests passed; exact timings, binaries, hashes and diagnostic
limits are in evidence/windows-20260930/AUDIO-FRAME-DIAGNOSIS.json. No FPS gain,
audio fix or acceptance is claimed. Next separate throttle sleep, CPU execution,
GPU submission/waits and irregular guest-frame production. Keep all normal-speed
and visual requirements, the default-off formatter, and the whole-process
one-core constraint. Goal remains active/unmet. User testing stays deferred;
future manual launches start normally. User edits and proprietary assets remain
preserved outside this checkpoint.

## Xbox controls and failed user playtest — 2026-09-30

The physical Xbox One controller was detected, but the first manual launch used
the old generic mapping and a combat savestate. The user reported wrong controls,
roughly 17–18 FPS and poor audio, and deferred further playtesting. That FPS is
user-observed, not an instrumented interval. Earlier short 27–29 FPS results do
not establish playability. Sustained 30 FPS and acceptable audio remain unmet.

New profiles map actions: A attack/confirm, B heavy/back, X grab, Y jump, LB
block/use, Start pause, RB+face powers and right-stick camera rotation. Power
chords suppress ordinary face actions. LT/RT cycle heroes temporarily; D-pad
still navigates menus/activates direct powers. Direct D-pad hero selection and
contextual X use remain open. Physical Wiimotes are not a supported port target;
physical motion and IR pass-through bindings were removed. Existing custom
profiles are preserved, and fresh CLI profiles select a connected SDL gamepad.

Fusion now has a gamepad-only candidate: LB+RB generates the internal request;
face buttons provide preset aiming and delayed confirmation. A captured
Spider-Man/Captain America sequence produced 723 damage, 6 KOs and completed
Hero Training: Fusion. The final-binary repeat produced 738 damage and 6 KOs.
Fusion was underway before the face-selection command,
so this does not establish every partner choice or revive behavior. Wider
validation remains required. Original UI prompts have not been rewritten.

The new process-local Xbox test device exercises the actual generated profile
and normal controller reports. It creates no OS controller or desktop input.
Tests cover action/chord isolation, axes, release, delayed confirmation and bad
automation values. Native captures confirm pause/resume, hero cycling and camera
rotation. Crowded captures do not individually prove jump, grab or every power.
All captures are reviewed sequentially; this is sampled evidence, not every frame.
Cubeb was active in the action probe; audible quality failed per the user report.
The fusion probe was headless/silent and provides no FPS or audio acceptance.

Windows build: Build.cmd --cpu jit --jobs 2, exit 0; 37/37 runtime tests (6.89 s),
29 profile cases, 127 tooling tests (126 passed, 1 skipped; 24.124 s).
No final compiler errors/warnings found; existing CMake deprecation, Wayland and
object-path warnings remain. An intermediate diagnostic compile error was fixed.
Artifact hashes and scoped results: evidence/windows-20260930/XBOX-CONTROLS.json.

Future manual launches must use normal boot/title, without --load-state. The
requested relaunch was deferred after the user's FPS/audio feedback. No game is
left running. Normal startup with the new profile is not newly visually verified.
Before another manual test, select the new generated Xbox profile explicitly;
existing old profiles were preserved and are not silently migrated.
One logical processor, Vulkan, 3x EFB, widescreen and formatter default-off remain.

Resolution correction: 1920x1080 names the 3x EFB preset, not a fixed output size.
Earlier measured EFB: 1920x1584; raw game frame: 1920x1410; window image excluding
borders: 1536x866. The 2501x1410 PNGs are aspect-corrected native captures.

Next: correlate sustained frame times/guest speed with audio starvation and CPU
costs under the whole-process one-core limit, then close remaining control gaps.
Private assets, saves, captures and logs stay in .local. User StaticRecomp edits,
Build-With-Log.cmd and the unfinished static experiment stash remain preserved.

## Whole-process one-core correction — 2026-09-30

The user's requirement is to run the entire game on one CPU core. The earlier
CPUThread=False setting only serialized CPU/GPU runtime work; helper threads
could still use other host cores. Earlier results do not demonstrate the required
whole-process limit and are excluded from acceptance under that requirement.

The Windows runner now selects one already-allowed logical processor before
RunMain, applies process affinity and reads it back. It fails startup if the
restriction cannot be confirmed. All game threads share this processor, including
later audio/I/O/shader workers. It replaces the optional largest-cache mask and
does not change any other process or global Windows setting. The benchmark now
requires both this one-bit host affinity and serial CPU/GPU confirmation.
This limits the game to one hardware thread, including on SMT processors.

The existing JIT backend still translates Wii PowerPC instructions at runtime
using Dolphin components. Affinity controls CPU placement; it does not change
that architecture into a native original Xbox port. The future Xbox port remains
an optimization/design constraint, not a demonstrated implementation.

Windows build exited 0: 36/36 runtime tests in 7.15 s and 8/8 tooling tests.
The OS test checked affinity and 1,000 CPU samples in each of four new workers.
Independent process reads confirmed mask 0x1 at startup and during actual combat
(the latter process had 52 threads, all confined by its process mask).
Current runner: 15,722,496 bytes, SHA256
82e4896d898baf8f828a69e63a4220f46823c0962247b218870b77663bfc63fc.
No compiler warnings/errors found; existing CMake deprecation, Wayland and long
object-path warnings remain. Build log: .local/logs/jit-one-host-core-build.log.
Exact hashes of all four produced executables are in the evidence.

Two short captured combat runs at 1920x1080 preset/3xEFB, Vulkan/Cubeb:

| Formatter | Average FPS | P99 ms | Maximum ms | Lowest rolling 1 s FPS | Guest speed |
| --- | ---: | ---: | ---: | ---: | ---: |
| off | 27.3288 | 60.63 | 488.54 | 16 | 97.471% |
| on | 29.1423 | 58.33 | 461.83 | 18 | 97.370% |

Frames 12620-12990, 370 intervals each; both routes completed and exited 0. Neither
passes solid 30 or normal-speed acceptance. These one-off runs include a midpoint
screenshot and varying combat progression. Severe stalls need a capture-overhead
comparison before attributing them to gameplay. The live end-of-route counter
was lower than the analyzed-window average; it is not the combat-window FPS.

All six native captures were inspected sequentially for actual fights, effects,
HUD and scenery. Actual captures are 2501x1410; sampled visuals and Cubeb activation
do not establish every-frame correctness or audible quality/synchronization.
The formatter remains default-off with its documented timing/correctness limits.
Goal active/unmet; qualifying repeats, ten-minute varied combat and full Xbox
controls remain open. Private assets, outputs, user edits and stash are preserved.

Evidence: evidence/windows-20260930/JIT-ONE-HOST-CORE.json.

## Guarded formatter rewrite experiment — 2026-09-30

Single-core CPU/GPU emulation remains enforced. The new default-off
`--simple-format shadow|on` experiment replaces a narrow game-library formatter
operation with portable host formatting: literal text and bare `%f` for bounded,
finite binary32 values. Live code hashing, caller/buffer/ABI and direct-RAM guards
restrict eligibility; unsupported cases execute the original instructions. The
caller still rotates its four buffers and the logging sink still filters/emits.
No messages, effects or game assets are removed or patched.

This is a high-level game-library rewrite, **not instruction-timing equivalence**.
It does not retain the original formatter loop's latency or internal interrupt
points. Clock rates and existing JIT return/downcount handling are unchanged,
but less guest work executes. Normal emulation speed alone cannot validate that
timing change. Keep the experiment off pending broader correctness/timing review;
these results do not satisfy the goal's correctness or sustained-FPS acceptance.

Shadow mode executed the original and matched 284,398 eligible calls, including
104,852 calls with floating conversions. It compared all 1,024 output bytes (untouched tail
included), return count, 12-byte va_list and full FPSCR: zero mismatches, abandoned
or pending samples. This is bounded observed-state evidence, not all architectural
state, exhaustive guest rounding/alias/guard coverage or full-game equivalence.
Host component tests compare supported random floats with snprintf; unsupported
formats and values retain the original path.

Three short runs per mode used the same runner, save, profile and input sequence
at 1920x1080 preset / 3x EFB, Vulkan/Cubeb, normal clocks and single-core mode.
Each measures frames 12620–12990, 370 newly rendered frame intervals:

| Mode / run | Average FPS | P99 ms | Maximum ms | Lowest rolling 1 s FPS |
| --- | ---: | ---: | ---: | ---: |
| Off 1 | 26.4021 | 51.37 | 73.57 | 22 |
| Off 2 | 28.3425 | 50.47 | 77.03 | 26 |
| Off 3 | 26.1663 | 66.73 | 83.41 | 23 |
| On 1 | 29.6495 | 51.06 | 67.49 | 27 |
| On 2 | 29.8070 | 61.08 | 67.54 | 28 |
| On 3 | 29.4533 | 51.39 | 73.44 | 26 |

Mean of run averages: 26.9703 off versus 29.6366 on
(+2.6662 FPS,
+9.89%). Input receipt
timing and combat progression vary; this is provisional evidence, not a precise
causal gain. All six runs fail solid-30 pacing. The earlier literal-only pair
measured 25.9815 versus 26.3175 FPS and was insufficient by itself.

All 18 native captures from these six short runs were inspected sequentially:
heroes/Doombots, attacks, beam/impact effects, damage, street/building/tree scenery,
HUD and fusion tutorial were visible. Aspect-corrected native captures are 2501x1410 (not window dimensions).
This is sampled visual evidence, not every frame. Mid-route capture can add
stalls. Cubeb activation is confirmed; audible quality/synchronization is unverified.

**Route correction:** seven captures from the full original-code shadow replay
show attacks carrying Spider-Man out of the crowd and into an empty alley late
in the sequence. Earlier full-route averages remain diagnostic and must not be
described as continuous crowded combat. An uncaptured original-code shadow run
also produced an unexplained 56.4917 FPS outlier; no replacement was active.
Retain it as unresolved evidence, excluded from FPS-gain or frame-cap claims.

Windows `Build.cmd --cpu jit --jobs 2` exited 0: 36/36 runtime tests in 4.97 s,
8/8 tooling tests. Current runner: 15,722,496 bytes, SHA256
4a0ad7d58d5d9123ca9b78c697ba6a13f32dec8075a660197767e3b009c242aa.
ModernGekko.exe, moderngekko-port.exe and moderngekko-module-info.exe were also
produced; artifact hashes are in the evidence. No proprietary game DLL is needed.
Final compiler diagnostics: no warnings/errors found. Existing CMake deprecation,
missing Wayland and object-path warnings remain. An initial compile failed on a
missing Memmap include and reported two size-conversion warnings; both were fixed
before the successful builds. Log: .local/logs/jit-simple-format-build.log.

Next expand guest correctness/guard/timing validation, repair the combat route
to stay engaged, then profile remaining slow frames. The three qualifying repeats,
ten-minute varied combat session and full Xbox controls remain outstanding. The
Xbox control table stays at the bottom of the to-do file. Goal remains active.
Proprietary data/raw logs/captures stay ignored; user StaticRecomp edits,
Build-With-Log.cmd and the unfinished static experiment stash are preserved.

Evidence: evidence/windows-20260930/JIT-SIMPLE-FORMAT.json.

## Single-core requirement and active JIT callers — 2026-09-30

The user requires single-core execution and intends an eventual original Xbox
port. CPU/GPU emulation now has a per-run CPUThread=False override, including
imported per-game settings. The benchmark writes the same setting into its
isolated profile and rejects missing/dual-core effective runtime confirmation.
Threaded comparisons below are historical and excluded from promotion. Favor
portable algorithms, less work and lower memory use. This Windows x64 backend
is not an Xbox port or proof of whole-process performance on one Xbox CPU;
Windows audio/I/O/shader workers remain possible.

Added opt-in --jit-profile-callers with --jit-block-profile: up to 32 selected
blocks, 64 distinct LRs per block, first argument/raw-stack examples and explicit
overflow counts. Ordinary runs disable inherited profiling. This is intrusive,
resident-block-only evidence, not release FPS or a complete call-stack trace.

The crowded caller probe completed with exit 0 in 70.266 s. Three selected
blocks recorded 11,107, 319,639 and 222,615 entries; counts reconcile with the
block dump, with zero overflow. Sampled formatting serves animation diagnostics.
Five 32-byte active code samples match the original DOL. The live logger has
one stream with mask 7: observed channels 0x20 and 0x1000 are discarded after
formatting; channel zero remains enabled. Next prove filter and temporary-buffer
side effects before guarded lazy formatting. No logging was disabled, original
game data changed, clock/cycle charges altered or FPS gain claimed.

Windows Build.cmd --cpu jit --jobs 2 exited 0. Runtime tests: 35/35 in 6.93 s;
tooling tests: 8/8. Runner: 15,593,984 bytes, SHA256
af4b30488e8a4293bb0f6a10614a643b434c94a38fe6bf7701c8d672f1708164.
Build log: .local/logs/jit-caller-single-core-final-build.log. No compiler
warnings/errors found. CMake deprecation, missing Wayland and object-path
warnings remain. ModernGekko.exe, moderngekko-port.exe and module-info were also
linked; exact artifact hashes are in the evidence. No generated game DLL needed.

An actual Cubeb combat replay with conflicting imported per-game CPUThread=True
completed with exit 0 in 27.219 s and confirmed dual_core=0, 3x EFB and normal
clocks. All three native captures were inspected sequentially: crowded heroes/
Doombots, attacks, beam/impact effects, damage, scenery, HUD and fusion tutorial.
The preset is 1920x1080; actual window captures are 2501x1410. This short captured
route validates settings and sampled content, not FPS acceptance or every frame.
Audio quality/synchronization remains unverified. Sustained 30 FPS, three-repeat
and ten-minute varied combat acceptance, and full Xbox controls remain open.

Evidence: evidence/windows-20260930/JIT-CALLERS-SINGLE-CORE.json.
Private data/captures remain ignored. User static edits, Build-With-Log.cmd and
the unfinished static experiment stash are preserved. Goal remains active.


## JIT block profile and crowded control comparison — 2026-09-30

Full JIT remains primary; sustained 30 FPS is unmet. Added opt-in
`--jit-block-profile` to the process-local combat harness, with reset/dump
commands and an explicit disabled-profile error. It changes only the copied
benchmark profile. Ordinary runs explicitly disable inherited JIT profiling
and debugging. This profiler is intrusive and reports resident blocks only;
invalidated blocks are absent. Its timings are not release-FPS evidence.

The crowded profile completed with exit 0: 35,597 resident blocks,
24.387 billion recorded guest cycles versus 24.780 billion elapsed guest ticks,
and zero skipped idle ticks. Read-only inspection identifies decimal conversion
and 64-bit division among hot loops. Region 803c1000 contributes 19.05% of
recorded cycles. Trace active callers, including indirect calls, before choosing
an optimization. Debug configuration files alone do not prove active logging;
no original game code/configuration or cycle charges were changed.

Sequential uninstrumented same-runner controls at 1920x1080 preset / 3x EFB,
Vulkan/Cubeb and normal clocks measured 27.524 FPS single-core and 26.318 FPS
threaded over frames 12620–13690. P99: 50.66 / 52.63 ms; maximum: 83.40 /
84.41 ms; rolling one-second minimum: 22 FPS both. Input receipt delays can
change exact guest timing. These one-off results do not establish a gain or
acceptance; threaded execution and the presentation queue remain unpromoted.

Windows `Build.cmd --cpu jit --jobs 2` exited 0; 34/34 runtime tests (5.18 s)
and 7/7 tooling tests passed. Runner: 15,589,376 bytes, SHA256
bbc951840accbe8ee3fddad92e506d91c92aa56aa6306975bbf556f93e3fc740.
CMake deprecation/Wayland/path-length warnings remain; final compiler errors
and warnings: none found. Two earlier attempts failed on misplaced parser
tests; corrected before the successful build. Log:
.local/logs/jit-block-profile-build-final.log.

An intentional negative run inherited an enabled diagnostic profile, disabled
it, rendered combat, then correctly rejected the dump command without creating
a misleading file. Harness exit 1 was expected; runner cleanup exited 0.
All three native combat captures were inspected sequentially: heroes/Doombots,
attacks, effects, damage indicator, HUD and scenery. The earlier portrait/camera
probe also exited 0; all four captures were inspected. The game confirmed the
previous fusion with 724 damage and six KOs. The active hero did not change;
camera motion overlaps fusion completion and needs an independent check.
Full Xbox mapping remains pending. Cubeb activation is confirmed; audible
quality/synchronization and every-frame visuals remain unverified.

Evidence: `evidence/windows-20260930/JIT-BLOCK-PROFILE.json`.
Three-repeat and ten-minute varied combat acceptance remain outstanding.
Private assets, raw profiles, disassembly and captures remain outside Git.
User static edits, Build-With-Log.cmd and the static experiment stash are preserved.

## Presentation experiment and native fusion input — 2026-09-30

Full JIT remains primary. Goal active/unmet. A clean heavier fight measured
26.0101 FPS over 41.14 seconds at the same 1920x1080 preset / 3x EFB and
100.008% emulation speed: P99 66.43 ms, maximum 107.19 ms, rolling one-second
minimum 22 FPS. Guest copy intervals also slow (median 37.69 ms, P99 62.81 ms).
Next profile guest frame production; presentation smoothing alone cannot fix it.

The default-off `MODERNGEKKO_PRESENT_QUEUE=1` / `--present-queue` experiment
copies XFBs into independent GPU textures and presents a bounded FIFO at half
refresh. It never advances the game or invents frames. One short single-core
test retained 29.969 FPS and reduced P99 to 36.18 ms / maximum 41.24 ms.
This is not a solid-30 pass and adds latency. Rush+smoothing regressed to
23.10 FPS with 35 queue underflows and is rejected. The earlier cache-lock
prototype falsely counted duplicates at ~60 FPS and is explicitly rejected.
The final snapshot design preserves cache identity and duplicate detection.

Added process-local Wii/Nunchuk acceleration and native Nunchuk shake inputs.
`pad_frames` accepts `release=0` to retain a chord across captures; its default
still releases. Updates use the controller state lock. The new input test checks
actual serialized Nunchuk acceleration/buttons, clearing, and shake waveform.
Native X-axis shake with Z opened the fusion tutorial. Subsequent pointer-ready
confirmation and Captain America portrait selection consumed stars and showed
attack effects. All four entry, four intermediate and five corrected-selection
captures were inspected sequentially. The tutorial banner remained. This proves
the tested input path, not completed Xbox controls or every-frame correctness.

The PS2-derived Xbox target table is at the bottom of docs/PERFORMANCE-GOAL.md.
Full mapping, camera and direct hero selection remain pending. No UI changes.
The user does not need to test yet. Fusion now has a usable process-local route
for extending combat: .local/automation/jit-fusion-ready-20260930/fusion-ready.sav.

An earlier crowded replay failed command 128; its exact cause was lost by the
old status-file handling. Failed commands now retain a per-command reason before
publishing their receipt, and the harness saves exceptions in result.json.
An intentional pause/pad_frames negative run confirmed correct rejection and
preserved detail; a later full crowded replay completed. The old failure is not
reclassified as a pass or claimed fixed without a diagnosis.

Windows `Build.cmd --cpu jit --jobs 2` exited 0; 34/34 runtime tests and 6/6
combat tooling tests passed. Runner: 15581184 bytes, SHA256
05841f688879dd9e707823630641f82b7beedebe00a1a4db17fde39efd82fc25.
CMake deprecation/Wayland/object-path warnings remain; no compiler warning/error
diagnostics found. Private build log: .local/logs/jit-native-shake-input-build.log.
Cubeb activation verified; audible quality/sync unverified. Three-repeat and
ten-minute sustained combat acceptance remain outstanding. Evidence:
evidence/windows-20260930/JIT-QUEUE-AND-INPUT.json.
All proprietary data stays private. User static diagnostics and Build-With-Log.cmd
remain uncommitted; the unfinished static experiment remains stashed.

## JIT pacing diagnostics and profile-directory fix — 2026-09-30

The JIT primary path is committed at 0642cd3e. Additional opt-in
`MODERNGEKKO_PRESENT_TIMES` telemetry records XFB-copy, before-present and
after-present events, intended/queued timestamps and effective runtime settings.
`--trace-presentation` exposes it in the combat harness. Buffers report overflow;
copy counts are not FPS, queued timestamps are not display scanout. Copy guest
ticks are recorded only in single-core mode to avoid reading the CPU clock from
a separate video thread. Guest/VI timestamps do not prove CPU-only causality.

Fixed a startup omission: create the profile directories before UICommon Init.
The previous fresh profiles lacked Config and Cache parents, preventing config
and shader-cache persistence. Actual fresh-profile validation now found six
shader-cache files and saved Dolphin.ini. This is not a claimed combat FPS gain.

Windows `Build.cmd --cpu jit --jobs 2` exited 0; runtime tests 32/32, including
bounded presentation-event serialization. Combat tooling tests 5/5 passed.
Runner: 15572480 bytes, SHA256
f7bc2cf579f763e8667345323e7506605ac0913026575114b05c6511213305e9.
Same CMake deprecation/Wayland/path-length warnings; no compiler warning/error
diagnostics. Private build log: .local/logs/jit-presentation-cache-build.log.

At the same 1920x1080 preset / 3x EFB, one-off instrumented short runs measured:
default max 52.89 ms; immediate 47.40 ms; immediate+smoothing 50.07 ms;
dual-core+immediate 43.77 ms; dual-core+immediate+smoothing 49.98 ms.
These private setting experiments were not promoted. Immediate mode needs
further visual/composition validation and is not unique-frame acceptance proof.
The final cache-fix runner's default run measured 29.9700 FPS, P99 51.29 ms,
maximum 55.77 ms, 99.9998% emulation speed. No solid-30 pass.

Default before-to-after presentation is usually below one millisecond. Copy
production already varies: guest copy intervals in the final single-core run
had median 33.2944 ms, P99 39.2912 ms and maximum 45.0064 ms. VI quantization
adds 16.683/33.367/50.050 ms presentation intervals. More host CPU throughput
alone cannot be assumed to remove this cadence. Next profile production and
extend real continuous combat. Ten-minute acceptance remains unverified.

Evidence: evidence/windows-20260930/JIT-PRESENTATION-DIAGNOSTIC.json.
The final runner also completed a longer mixed visual route with Cubeb and exit
0. All four native captures were inspected sequentially: Spider-Man selected,
destructible scenery, an enemy-introduction cutscene, and crowded combat effects
with a fusion tutorial. This is not continuous-combat FPS evidence. Native game
Options confirmed A attack, B smash, D-pad powers, Z block/use, C jump, +/- hero
switching, 1 camera, 2 pause, and Z plus Nunchuk motion for fusion. The existing
harness exposes buttons/sticks, but not Nunchuk motion. Private route/save:
.local/automation/jit-varied-visual-20260930/varied-combat.sav.
All original data, generated output, caches, native captures and raw logs stay
private. User static diagnostics and Build-With-Log.cmd remain uncommitted.

## JIT primary path built and repeated — 2026-09-30

The user explicitly selected full JIT. Build.cmd and Run.cmd now default to JIT;
--cpu staticrecomp retains the native path and --native-rel still implies it.
The runner accepts --cpu jit|staticrecomp, overriding legacy environment selection.
JIT validates the original local game and runs without a generated module DLL.
Backend-specific build receipts retain binary verification. The combat runner
requires the selected CPU backend to be confirmed in runtime.log.

Build.cmd --cpu jit --jobs 2 exited 0: 32/32 Windows runtime tests passed.
Tooling: 123 run, 122 passed, one existing skip. After adding legacy receipt
preservation coverage, targeted workspace tests: 46 passed, one existing skip.
Actual runner negative checks
rejected invalid backend names and rejected a missing static module despite a
conflicting JIT environment setting. CMake reported deprecations, unavailable
Wayland and object-path-length warnings; no compiler error/warning diagnostics.
Runner: 15569920 bytes, SHA256
d8f0e926a4a827dfdc30a410d28f3c8e6f6412a25f8a8ef9f571f8df7e9a62c7.

Three module-free JIT/Cubeb combat repeats at the same 1920x1080 preset / 3x EFB
completed with clean receipts and exit 0: 29.9712, 29.9697, 30.0676 FPS over
frames 11472-11625, approximately 100% game speed. P99: 51.02, 51.23, 50.51 ms;
maximum: 52.03, 53.90, 54.06 ms. These short runs do NOT prove solid 30 FPS.
Guest VI/presentation intervals also alternate among 16.683/33.367/50.050 ms.
The analyzer now reports guest cadence and host-minus-guest interval residuals;
these are not a CPU-only profile or proof of game-simulation causality.

A rebuilt-runner extended visual replay completed with Cubeb and exit 0. All
five native captures were inspected in sequence: heroes/enemies, movement,
damage/effects, HUD, scenery and a downed hero were visible. This sampled review
does not establish every-frame correctness or audible audio quality/sync.
Tooling tests overlapped the visual run, which is excluded from FPS comparisons.

Goal active/unmet: next investigate presentation/producer timing and extend the
route for genuine ten-minute combat. No repeated-save-load substitute for that
acceptance. No proprietary assets, translations, captures or raw logs committed.
The unfinished direct-chunk patch is retained in the stash named "Preserve
unfinished direct chunk experiment before JIT priority". The user's three static
diagnostic edits are restored and remain uncommitted; Build-With-Log.cmd retained.
Evidence: evidence/windows-20260930/JIT-PRIMARY-CHECKPOINT.json.

## Aggregate native execution profile — 2026-09-30

The opt-in dispatch profiler now totals every PC before truncating its ranking.
It records the first/last dispatch timestamp without adding new clock reads.
This separates measured generated-call time from the enclosing wall span; the
remainder includes profiling, scheduling, fallback, synchronization and runtime
work and must not be described as dispatcher CPU time alone.

Runtime rebuilt with exit 0. All 32 scoped moderngekko tests passed. An initial
unrestricted ctest invocation exited 8 because third-party fullbench/fuzzer/
zstreamtest executables were unbuilt; that remains a scoped validation limit.
Two Clang combat profile runs completed with clean receipts and exit 0:
13.481/21.894 seconds generated/span, then 15.164/24.358 seconds. The first
run overlapped runtime tests; the repeat began after they ended. XEMU background
load remained. Both attribute about 62% of instrumented wall time to generated
calls. Neither run is release-FPS evidence. Aggregate consistency checks passed.

Evidence: `evidence/windows-20260930/AGGREGATE-DISPATCH-PROFILE.json`.
The 30 FPS goal remains unmet. Next investigate generated-code entry/return
frequency and helper overhead, not only the external dispatcher. Preserve
native eligibility/hash checks, exceptions and interrupt/timing boundaries.

## Compiler candidates completed; combat still below target — 2026-09-30

Both build sessions finished with exit 0. Clang/O2 produced a 170525696-byte
module, SHA256 47922fb187294b601b2b8667805c42bfda639ecfe9ffc014d36b7d1a0bcb6d51,
with all 524 chunk hashes verified. MSVC/O2/Ob2/c1024 produced a 407890432-byte
module, SHA256 bd4ea9876c137637a4d1947468dabd68aba4bde7c27ec881813a9e3ca18b6b00,
with all 2094 chunk hashes verified. Both use module ABI 3 / CPU ABI 4.
MSVC pipeline tests: recompiler 19/19, runtime 32/32. Clang recompiler tests:
19/19; a private probe of its actual module helpers passed 16/16 signed
single/double halfway-rounding cases across all four guest RN modes. This is
bounded validation, not complete floating-point or gameplay equivalence.

At the existing 1920x1080 preset / 3x EFB, sequential same-runner combat tests
measured baseline 6.1936 FPS, Clang 10.9880 FPS, MSVC c1024 9.4445 FPS over
frames 11472–11625. P99 frame times: 283.17, 167.95, 170.13 ms respectively.
All routes completed and exited 0 with command receipts validated. Compiler
load had ended; XEMU was using CPU in the background. These single comparisons
are diagnostic, not repeated or sustained acceptance. Goal remains unmet.
Clang is the stronger compiler candidate; the production default is unchanged.

A Clang windowed Cubeb run completed with exit 0. All four native captures were
inspected sequentially: heroes/Doombots fighting, damage indicators, hit effects,
HUD and scenery were visible. These are sampled images, not every frame.
Audible quality/synchronization remains unverified. A gated Clang dispatch
profile completed: leading entries include 803682c4 and 8035ba00; frequent
scheduler/context entries remain. Next measure generated execution versus
runtime dispatcher/continuation costs before another expensive compiler sweep.

Exact metrics, hashes, warnings and profile samples are in
`evidence/windows-20260930/COMPILER-COMBAT-COMPARISON.json`. Clang reported one
nested-comment warning and unused CMake variables. MSVC reported generator
SMC warnings and CMake deprecation/Wayland/path/unused-variable warnings; no
compiler error diagnostics. Private assets, generated C, images and raw logs
remain under .local. Earlier statements below about live builds are historical.

## Idle-cycle hypothesis ruled out for this fight — 2026-09-29

Added read_timing to the process-local automation protocol. It snapshots
CoreTiming ticks and idle_ticks under a CPU-thread guard, then releases the
guard before writing the requested private file. It never generates host input
or changes CPU timing policy. Missing-path parsing is rejected. Runtime rebuilt
with exit 0 and no reported warning/error; all 32 Windows runtime tests passed.

Completed native and JIT combat probes both had zero idle-tick delta:
native over 3651052829 ticks; JIT over 4385648130 ticks. Both exited 0 with
clean command receipts. The differing command-route intervals are not paired
FPS windows. Compiler load was active; these timing-only runs had no captures
or audible audio. IDLE-CYCLE-PROBE.json records the measurements. Do not pursue
idle skipping as the explanation for this fight's native/JIT speed gap.

Two module builds remain active: session 99026 for the isolated c1024/Ob2
MSVC experiment, and session 66634 for an isolated Clang/O2 module with the
original 4096-instruction chunks. Clang 22.1.2 already exists at
C:/Program Files/LLVM/bin/clang.exe and targets x86_64-pc-windows-msvc.
No toolchain was installed. The private .local/scratch/build-clang-module.cmd
uses workspace CMake helpers and the existing Windows SDK; its output is
.local/build/windows-x64/module-mg01-rel-clang-o2/gRMSE52_recomp.dll.
Clang uses O2, fp-contract=off, no-fast-math, IPO off. Its script audits with
the existing verifier and writes .local/receipts/clang-module-experiment.json
only on success. It does not replace the default build receipt. See
.local/ACTIVE-PERFORMANCE-BUILDS.json for active commands/logs. Neither module
has a completed audit or performance result yet. Goal remains active and unmet.

## Ob1 build completed; no combat speedup — 2026-09-29

The long-running session 60093 finished with exit 0. Windows native module:
.local/build/windows-x64/module-mg01-rel-o2-ob1/gRMSE52_recomp.dll,
591055360 bytes, SHA256
11b4d161ffc02f1ff9c49292973961651c999249e43ac0adf3b0a38dbc24f631.
Audit passed all 524 chunk hashes, module ABI 3 / CPU ABI 4. Recompiler tests
19/19; runtime tests 32/32. Full build warning counts: D9025 276 (275 W3/W0,
one expected O2/Od), C4711 4364, C5045 93, D9002 1 (ignored -fexceptions).
No compiler error diagnostics. Exact results: INLINE-EXPERIMENT.json.

After this project's compile/backup ended, the same-runner 3x timing pair
measured baseline 6.4243 FPS versus Ob1 5.8556 FPS over the same 153 intervals.
Both completed with exit 0 and clean receipts. This rejects Ob1 as a performance
win; it is not a repeated acceptance result. The DLL also grew from 314896384
to 591055360 bytes. Production default remains Ob0. A separate windowed Cubeb
run completed; all four native captures were inspected in order and showed
combat, damage/effects, HUD and scenery. Audible quality/sync remains unverified.

Next experiment: Build.cmd --native-rel --c-chunk-instructions 1024
--module-msvc-inline 2 --jobs 4. The new explicit chunk-size option isolates
DOL/REL generation keys and module output, records configuration and overrides
inherited generator settings. Generation verifies the DOL section-derived chunk
count instead of assuming 325 chunks. Default remains 4096. Tooling tests:
118 run, 117 passed, one skipped. This experiment is running in session 99026,
log .local/logs/combat-c1024-ob2-build.log. Do not restart a live build.
Smaller functions may permit stronger optimization but add dispatch boundaries;
no performance gain is assumed. Goal remains active and unmet.

## Combat-only dispatch profile — 2026-09-29

The earlier per-dispatch profile included boot/restoration. Added optional
STATICRECOMP_PROFILE_GATE_FILE: native profiling waits for a private marker,
checked at 65536-dispatch intervals until it opens. The benchmark publishes
that marker only after the restored frame threshold. One latched decision
covers both timestamps, preventing a gate transition between start/end.
Normal runs clear this variable. The profile includes initial neutral frames;
it is not exactly the narrower frame-analysis window.

The rebuilt Windows runner completed the gated combat route with exit 0,
clean command receipts and one gate-open event. The ranking corroborates the
old profile: 8035ba00 (1233.5 ms), 803682c4 (1165.9 ms), context/scheduler and
other frequent short entries remain expensive. These intrusive timings under
compiler load are for ranking only, not release FPS. No captures or audio were
collected in this timing-only run. See COMBAT-GATED-PROFILE.json for exact data.

Runtime build: exit 0, no reported warning/error. Tests: 32/32 Windows runtime
and 5/5 benchmark tooling passed. Runner SHA256:
cd561d89b1018078272a429d82f50822cafb77851ac790abc3bed8639e518457.
Use the same current runner for both native DLL comparisons.
The Ob1 build is still live in session 60093, last seen 443/534. No candidate
module result yet; goal remains active and unmet.

Source inspection found the direct cross-chunk call emitter's table setter
has no callers in the current DolRecomp source. Do not simply enable it:
its direct calls would need native eligibility/hash/host-call and REL checks,
bounded cycle handling and regression coverage before a safe game experiment.
This is a future design lead, not an implemented optimization.

## Selective JIT profiling checkpoint — 2026-09-29

Added opt-in STATICRECOMP_FALLBACK_USE_JIT=1 for forced fallback ranges.
Default execution remains unchanged; without the flag (or without a JIT),
forced ranges still use the interpreter. The benchmark accepts --jit-ranges,
validates address ranges, rejects combination with --jit-diagnostic, clears
inherited diagnostic variables, and records the selected ranges in metadata.
Native eligibility/hash checks, host-call handling and timing remain active.

The runtime rebuilt through the existing MSVC environment with exit 0 and no
reported warning/error in its incremental build log. All 32 Windows runtime
tests passed. Tooling: 116 tests run, 115 passed, one skipped. An existing
source-pattern test needed its expected condition updated for the new opt-in
JIT gate; the empty-range short-circuit remains covered.

Concurrent-build diagnostics at the unchanged 3x preset: fresh native control
5.7365 FPS; scheduler-region JIT 6.1493; broader math/library-region JIT 6.7784;
80000000-80500000 region JIT 9.1802. These are not clean acceptance benchmarks
or a production speedup. All four scheduler-run native captures were inspected
in order and showed ongoing combat, damage/effects, HUD and scenery; other runs
were timing-only. No sustained-30, every-frame or audible-quality pass.
A complementary upper-code-region run measured 5.8206 FPS over 151 intervals
(two fewer than the other 153-interval diagnostic windows).
See evidence/windows-20260929/SELECTIVE-JIT-DIAGNOSTICS.json. Raw data is private.

The isolated native Ob1 module build remains live in session 60093, last seen
at 432/534; no candidate DLL/audit/FPS result yet. Do not restart it merely for
a stationary log. New runner SHA256:
9fd119537f37407911838cdb447e511b6bdb296d90084329f43eb993f4f51807.
Compare both baseline and Ob1 using this same runner after the build finishes.
Goal remains active and unmet; production defaults have not been changed.

## Command-publication race fixed; extended combat route — 2026-09-29

The extended route exposed a Windows sharing failure while renaming a command
from .txt.tmp to .txt. Root cause: ListCommandFiles consumes every regular
file in the watched directory, including temporary files. The runner now
stages in a separate sibling directory, publishes a closed file atomically,
and retries only bounded Windows sharing/lock violations. Cleanup stop uses
the same publication path. Completion now requires exactly the expected
processed command names and zero failed command files. Four focused tests
passed; full tooling suite: 115 run, 114 passed, one skipped.

Audited prior completed combat command receipts and exact payloads. The
3x baseline, 3x JIT, native profile, trace-off retry and both runtime/audio
checks passed. The old 1x run has failed/000003.txt.tmp and is EXCLUDED from
controlled resolution comparisons; COMBAT-BASELINE.json marks it invalid.
The aborted trace-off attempt and first extended-route attempt remain failed.
See evidence/windows-20260929/COMMAND-RECEIPT-AUDIT.json. The valid same-3x
native-versus-JIT comparison still supports a CPU-path bottleneck.

The corrected extended native route completed with exit 0 and clean receipts.
All five native captures were inspected in order: movement away from the
lamppost, active melee, damage/effects, reduced enemy health and defeated
Doombots. This is sampled visual evidence, not every-frame validation. The
power attempt did not establish activation; a separate three-capture ability
probe also completed but did not clearly demonstrate a shield throw. Do not
claim a power-effect pass. Both runs used Null audio and concurrent compiler
load, so neither is an optimization or sustained-FPS result.

Private extended route: .local/scratch/combat-route-extended.json; completed
run .local/automation/combat-extended-control-build-load-retry contains
combat-extended.sav (frame around 11832). Ability probe lives in
.local/automation/combat-power-control-build-load. Preserve the original
short route for direct baseline/candidate comparisons.

The Ob1 build remains live in session 60093, still compiling large generated
functions with no reported failure. No candidate DLL/audit/FPS result yet.
Continue the live build, then benchmark with the corrected publisher. Goal
remains active; no production speedup or sustained-30 claim is made.

## Rebuilt-runtime combat and audio-path checks — 2026-09-29

The Ob1 module build is still live in terminal session 60093; it has passed
chunk0165 and chunk0167 with no reported compile failure. Large generated
functions can take many minutes and several GB of compiler memory. Do not
restart this build merely because its log is temporarily unchanged.

While it compiles, the rebuilt runtime completed two baseline-module combat
replays, both exit 0 and route_completed=true. All four native screenshots
from each replay were inspected in chronological order: heroes, Doombots,
attacks, damage numbers/flashes, HUD, scenery and destructible props appeared.
These are sampled captures, not inspection of every frame. The headless run
used No Audio Output; the windowed run reported Cubeb. Audio-enabled execution
is confirmed, but audible quality/synchronization is not. No host input or
screen capture was used; windowed replay used process-local automation.

Added --windowed and --audio to tools/run_combat_benchmark.py. The tool rejects
non-silent audio requests in headless mode because the runtime forces silent
audio there. CLI rejection was checked (exit 2 before output creation), Python
compilation passed, and the windowed Cubeb replay completed. Defaults preserve
silent headless timing runs.

Common frames 11472..11625 under concurrent compiler load measured 5.335 FPS
headless and 6.320 FPS windowed/Cubeb. These are functional checks under load,
not a controlled compiler comparison or a performance improvement. Private
runs: .local/automation/combat-ob1-control-build-load and
.local/automation/combat-ob1-control-audio-build-load. Both use the unchanged
baseline Ob0 module. The Ob1 module has not linked/audited or been benchmarked.
Next: finish the live build, audit, compare baseline/candidate without compiler
load, and continue toward the unchanged sustained-30 combat goal.

## MSVC helper-inline experiment in progress — 2026-09-29

The sustained-30 goal remains active. Added opt-in `--module-msvc-inline 1`
with separate module output and build-receipt metadata; default remains Ob0.
Existing strict floating point, IPO-off setting and chunk0201 Od workaround
are unchanged. Windows Build.cmd --native-rel --module-msvc-inline 1
--module-suffix o2-ob1 --jobs 2 is still running in the local workspace.
Runtime compilation/link succeeded and ModernGekko tests passed 32/32;
recompiler tests passed 19/19. Tooling suite ran 110 tests, 109 passed and one
skipped; after adding output-isolation coverage, workspace suite ran 42 tests,
41 passed and one skipped. No complete candidate module or FPS result yet.

A separate MSVC O2/Ob1/fp:strict compile of the historically troublesome
chunk0165 succeeded (769562-byte private object). Full module compilation
has started (534 build steps). Nonfatal dependency/CMake warnings and D9025
were observed. Private log: .local/logs/combat-ob1-build.log. Keep the build
running; inspect completion before restarting anything. Its terminal session
is 60093. Probe session 83777 completed with exit 0.

The rebuilt runner SHA256 is
9c5af2645a5544bdea5e87dba76e27ada82d70b3539176423740349def6fffba.
The build receipt remains stale until the full Build.cmd finishes its module
audit. Baseline DLL remains .local/build/windows-x64/module-mg01-rel-o2/
gRMSE52_recomp.dll. Candidate output is module-mg01-rel-o2-ob1 in the same
build root. Existing user diagnostic edits were stashed during runtime build,
then restored; they are not in this runner or this checkpoint commit. The
preservation stash remains available. Build-With-Log.cmd is untouched.

Next: finish module build/audit, replay the same combat save, and repeat the
baseline beside the candidate. Another unrelated project was compiling;
record contention and do not stop it. Inspect native captures before claiming
correctness. No production speedup, audio pass, or sustained-30 pass is claimed.

## Combat timing and CPU comparison — 2026-09-29

Goal remains active; no production speedup or sustained-30 pass is claimed.
Added opt-in bounded unique-frame timing at after_present, independent of the
status-file loop; no file I/O on the video thread. Added the replay runner and
analyzer documented in PERFORMANCE-GOAL.md. Timestamp tests cover duplicate
presents, rewind epochs and overflow; analyzer rejects lost/discontinuous data
and reports frame tails, rolling FPS and normal-speed ratio. The optional
status read tolerates transient Windows sharing errors; an aborted attempt was
retained and a complete retry succeeded.

Windows/MSVC runtime rebuild exited 0, ModernGekko 32/32 tests passed. Tooling
suite 108 passed / 1 skipped (109 run); subsequent analyzer suite 5/5 passed.
Native DOL+REL audit remains 524/524 PASS. Compilation had nonfatal dependency
warnings (including D9025 and C4711), with no compile/link failure.

Replayed combat.sav with repeated A-button attacks. Native captures showed
Doombots, attacks, reduced health, hit effects and debris. All measured runs
exited 0 and completed the route. Common frames 11472..11625 (153 intervals):
- Native 3x with captures: 6.062 FPS, p99 302.36 ms, 20.23% normal speed.
- Native 1x with captures: 5.762 FPS, p99 274.39 ms, 19.16% normal speed.
- Native 3x without captures: 5.899 FPS, p99 264.21 ms, 19.62% normal speed.
- Diagnostic JIT 3x with captures: 30.069 FPS, p99 88.98 ms, 100.00% speed.
These short comparisons point strongly to the native CPU path, not resolution.
The JIT run is not a substitute for native optimization or sustained acceptance.
A trace-off replay took 23.907 s after warmup versus 25.750 s trace-on; one pair
cannot resolve tracing overhead from run variability. Per-dispatch profiling is
intrusive; use its cost ranking, never its FPS as release performance.

Next: pursue major generated-code/dispatch improvements, starting with the
Windows global /Ob0 workaround versus explicit helper inlining
and repeated short-block overhead. Keep all guards and timing. Time-heavy
blocks include 8035ba00, 803682c4, 803ee9e8 and 803eea68; fixed-stride dispatch
frequency alone was not a useful cost ranking.

Private runs under .local/automation/combat-* contain traces, copied saves,
metadata and captures. Public aggregate: evidence/windows-20260929/COMBAT-BASELINE.json.
Current runner SHA256 d349b67d272b526dc3875fa362a1201baf20696ef71f5f899ca3c5245791cab7.
The main build receipt still describes the prior runner; regenerate it with the
normal Build.cmd --native-rel before normal receipt-verified launch. This
checkpoint used explicit hashed runner/module paths. Original binaries are
preserved under .local/perf-baseline-789e77f9. Pre-existing diagnostic edits are
restored after the telemetry build and remain excluded from tested binaries.

## Active sustained-30-FPS goal — 2026-09-29

The user requested major frame-rate improvements and will not accept less than
solid 30 FPS in combat. The active goal and acceptance criteria are recorded in
`docs/PERFORMANCE-GOAL.md`; small improvements do not complete it.

Planning is complete and execution has begun. GitHub main was checked at
f14ea4a8. Existing runtime diagnostic edits remain untouched and excluded from
the running binaries. First enemy encounter reached via process-local inputs;
native captures showed multiple Doombots, enemy health UI, projectile/effect
activity and reduced player health. This is a checkpoint inspection, not full
combat correctness or sustained-30 validation. Private run:
`.local/automation/20260929T164700-combat-route`, clean exit 0 after 395.797 s.
`route-progress.sav` (first encounter) and `combat.sav` (enemies surrounding
Captain America) are private reusable checkpoints. Saves were created; replay
repeatability remains to be checked. No speedup implemented or claimed yet.

The diagnostic run recorded 3,291,406,422 native dispatches, 9,200,590 bursts,
985,217 JIT fallback entries, 514,671 native exceptions and zero failed chunks.
Hot dispatch samples include 803f2b60, 803f2b38, 803f680c and 80007448. These are
frequency samples, not time attribution, and fixed-stride sampling can alias
loops. Do not infer the bottleneck or a speedup from the counts alone.
Next: replay combat, implement low-overhead unique-frame timing and benchmark
analysis, establish uninstrumented combat baseline, then profile CPU/GPU and
short-block/OS scheduling costs. Preserve all timing and verification guards.

## Windows performance checkpoint — 2026-09-29

OpenMUA2 was actually run using the audited Windows O2/indexed native DOL+REL
binaries from the c54bde7 source checkpoint. Native application captures confirmed
the opening cinematic, Latveria level environment, four heroes, HUD/minimap,
player movement and party-following. Combat was not reached or measured.

- Ryzen 7 8745HS, Radeon 780M (driver 32.0.23033.1002), 27.8 GiB usable RAM.
- Configured 1920x1080 preset maps to 3x EFB scale (1920x1584 backing buffer).
  Native aspect-corrected captures were 2501x1410; this was an offscreen Vulkan
  run with Null audio, not a measured 1080p window presentation.
- Opening-level traversal/idle/obstacle-contact interval: **1,366 new frames /
  219.516 seconds = 6.223 FPS**, mean reported emulation speed **20.666%**.
- Counted unique XFB frames against monotonic wall time, excluding duplicate
  presentations. Headless fps/vps status fields stayed zero and were not used.
- No combat FPS, 1% lows, audio validation or full-game visual correctness claim.
  Captures were inspected at checkpoints, not every rendered frame.
- Both private runtime sessions exited 0. Source diagnostics restored after the
  build remain uncommitted and were not part of the tested binaries.
- Evidence: `evidence/windows-20260929/PERFORMANCE.json`. Raw logs, captures,
  copied saves and checkpoints remain private under `.local/automation/`;
  latest run `20260929T161554-action-fps` has `level-start.sav` and `traversal.sav`.
- Next performance work: reach an actual enemy encounter, measure a repeatable
  combat interval, then profile CPU/GPU before attributing the low frame rate.

## Windows continuation — 2026-09-29

- Confirmed `origin` is `https://github.com/GTTeancum/OpenMUA2.git` and pulled
  `main` with `--ff-only` to `5e5ffc8daf84ec10b59637ecebc9d6134af6a15c`.
- Preserved three pre-existing local runtime diagnostic edits in a named Git
  stash and `.local/pre-pull-diagnostics-20260929.patch` before pulling. They are
  excluded from the source fix commit and tested binaries, and restored after
  validation. The pre-existing `Build-With-Log.cmd` remains untracked.
- Used the existing `Build.cmd` / `tools/msvc-env.cmd` / `tools/workspace.py`
  Windows path: Visual Studio 2022 Community 17.14.29, MSVC 19.44.35225.0,
  CMake 4.3.1, Ninja, Python 3.12.10, x64 Release, two jobs per module build.
- Initial default build exited 1 because `gen_module_tables.py` could not read
  indexed coverage arrays. Added exact run-interval parsing, malformed-range
  rejection, and synthetic regressions for hash equivalence, coverage holes,
  and merged-table precedence. The latter prevents retained DOL arrays from
  adding overlapping ranges to a merged DOL+REL module.
- Fixed the nonfatal `fatal: bad revision '^master'` version-metadata diagnostic
  by supporting `main` and detached checkouts; regression fixtures cover
  `master`, `main`, and a checkout with neither branch.
- Added this handoff to the source snapshot/backup allowlist and documented
  current Windows O2/indexed defaults and `--native-rel` in the build guide.
- Final `Build.cmd --jobs 2 --native-rel` exited **0**. The runtime tree was
  incrementally rebuilt; both module output trees were newly configured and
  compiled. O2/indexed, IPO off, `/Ob0 /fp:strict`, and the existing `/Od`
  exception for `chunk_0201_text1_80326900.c` were retained.
- DOL-only module: **325/325** chunk hashes PASS, 2 code ranges.
- Native DOL+REL module: **524/524** chunk hashes PASS, 524 code ranges,
  one REL module, 8,571,904 covered bytes including 3,255,744 REL text bytes.
  Both audits passed module ABI 3 / CPU ABI 4 and uncovered-address rejection.
  Final rebuilds of both modules returned `ninja: no work to do.`
- Tooling: **104 passed, 1 skipped** (105 run; POSIX symlink fixture skipped).
  DolRecomp: **19/19**. ModernGekko: **31/31**. Broader runtime CTest:
  **50 passed**, `playTests` disabled; CI exclusions `fullbench|fuzzer|zstreamtest`
  retained. GXRuntime: **1/1**. Counts overlap between runtime suites.
  `moderngekko-run.exe --help` exited 0; port usage printed and exited its
  source-defined usage status 2.
- Remaining nonfatal warnings: MSVC C4996 in DolRecomp test fixtures, upstream
  CMake policy/minimum-version deprecations, seven object-path warnings,
  optional Wayland/PkgConfig detection, unused FetchContent options, and the
  generator's self-modifying-code advisory. No final compile/link/audit failure.
- Binary paths, sizes, SHA-256 values, and validation details:
  `evidence/windows-20260929/VALIDATION.json`. Main outputs under
  `.local/build/windows-x64/` are `runtime/moderngekko-run.exe`,
  `runtime/ModernGekko.exe`, `runtime/moderngekko-port.exe`,
  `runtime/moderngekko-module-info.exe`, `dolrecomp/dolrecomp.exe`,
  `native-audit/openmua2-verify-module.exe`, and `gRMSE52_recomp.dll` in
  `module-mg01-o2/` and `module-mg01-rel-o2/`.
- Final native build log: `.local/logs/windows-main-20260929-native-rel-final.log`.
  Build receipt: `.local/receipts/build.json` selects the native DOL+REL DLL.
  Other logs share the `windows-main-20260929-` prefix; failures were retained.
- Original WBFS SHA-256 was rechecked unchanged. Assets, generated translation,
  binaries, receipts, and raw logs stay outside Git. No proprietary output is
  included in the source commit.
- **RMSE52 was not launched. No gameplay, graphics, audio, controls, saves, or
  FPS validation is claimed.** A current actual game run remains the next
  performance-validation step, respecting the user's ban on desktop automation.

## Mandatory workflow

- GitHub `main` is authoritative; inspect it before editing.
- **Multiplatform performance is the active priority.**
- Preserve SMC/hash/audit/REL eligibility/verification protections.
- Use moderately short turns because resume-stream failures occur: one focused merge/implementation plus validation, then update/attach this handoff.
- At the end of every turn, update this file, commit it to `main`, and attach `MUA2-CANONICAL-HANDOFF.md` in chat.
- Never commit proprietary RMSE52 data, extracted files, generated proprietary translation output, saves, logs, screenshots, or RAM captures.
- RMSE52 game assets are now verified persistent in `/MUA2/RMSE52-Game-Files/Extracted`; restore working copies under `.local/game` from the 10 `RMSE52-extracted.tar.NNN` chunks. The original split uploads are no longer required for normal continuation.
- Never claim FPS/gameplay validation without an actual RMSE52 run.

## Important locations

- Static recomp runtime: `project/lib/ModernGekko/vendor/dolphin/Source/Core/Core/PowerPC/StaticRecomp/`
- DolRecomp: `project/lib/DolRecomp/`
- Vendored DolRecomp: `project/lib/ModernGekko/vendor/dolphin/DolRecomp/`
- Module template/export: `project/lib/ModernGekko/vendor/dolphin/module-template/`
- Tooling: `tools/`
- Tests: `tests/`
- CI: `.github/workflows/`
- Current status: `docs/CURRENT-STATUS.md`
- Work log: `docs/WORKLOG-2026-09-23.md`
- Recovery plan: `docs/RECOVERY-PLAN.md`
- Windows workspace target: `D:\\Programming\\GitHub\\OpenMUA2\\`
- Local proprietary/generated data: `.local/` only
- Persistent private game-file Library: `/MUA2/RMSE52-Game-Files/Extracted`

## Accepted runtime/performance state

Native DOL + REL execution is present. SMC/chunk-hash protection remains enabled.

Key accepted performance commits:

- `a6328f40bb0c98a58c8f50e52a30d4da55390b7e` — `O2 + indexed` default.
- `bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc` — page-index combined DOL+REL dispatch.
- `f4b7f991deb4a9e787b97c3bac2a475faadfc1f6` — empty/single-chunk merged-dispatch fast paths.
- `405b81de4136a7532e966218185a190f6eb9230d` — page-local ordinary indexed dispatch.
- `917a5d893087736a74566dbaf71de5f3989a6d90` — cached x86-64-v3 feature detection.
- `4e677ac8491bc3b5256998ba3688893f61d07695` — module-load baseline/v3 chassis binding.
- `5e6ad207c77affbf500bf5327ce6222e9e7fd7c1` — reuse module-load v3 choice for indirect dispatch.
- `2776fa0a3e80136495a32552b9d909e16dcfcd5e` — gate host-call probes by cached per-chunk coverage.
- `af7938bdb63d4530ea43a6ba445800fc4171a153` — reuse runtime→linked PC between eligibility and dispatch.
- `7b7e412f671039e1d29e2cdff7b2e51509bc046e` — same-section linked→runtime REL hint.
- `7a9cdc0165257ec19301972785150e4961f7f65e` — same-section runtime→linked continuation hint.
- `12a75b2db225678368be5e2b6340218ba2aafa2f` — skip empty forced-fallback range scans in native eligibility.
- `5b87d08b8f6e4daff2ca64bab75d67632ec28721` — read cached host-call chunk state directly in the burst path.
- `8f8c32a9c90e039c898d78eb8313c7d9d64f28c5` — chassis-only generated dispatch skips duplicate host-call dispatch.
- `dc43d362d425134222d00aa02dd4dbec99fcf222` — remove redundant explicit module-active check from the native burst back-edge.
- `20dd90851c3a2625ddb973f8850e2494bb33ca9f` — skip the forced-fallback helper in the interpreter/fallback branch when no forced-fallback ranges are configured.
- `de411096f5986980ac58e1f3a7373d8f5351dc67` — reuse generated linked results across eligible native burst continuations.
- `d3aeab80a3ae44c2d18265ad9f1e5868bf369e50` — check cheap burst termination conditions before continuation eligibility.
- `b78431f21625ad61b4f66855f5f94b859f04cf7a` — check fallback-slice termination before dispatchability/host-call probes.
- `7b11a1869c85aec8d5384c7f044779454e37a69f` — reuse the proven native-entry exact host-call result in fallback.

Important accepted correctness/runtime commits include absolute REL section-table support (`67d75dc6...`), native cache-control codegen (`770db108...`), scalar FMA repair (`7e0b4866...`), MEM2 lockstep journaling (`26334ad6...`), and merged DOL+REL eligibility guards (`46091e1a...`).

## Latest accepted work — PR #22

PR #22 `Remove redundant module-active burst check` is **MERGED**.

Merge commit:

- `dc43d362d425134222d00aa02dd4dbec99fcf222`

Behavior:

- Removes the explicit `m_module_active &&` from the native burst back-edge.
- `fast_native_continue()` already rejects inactive modules on both paths:
  - REL/forced-fallback path: `FastDispatchableAt() -> ChunkIndexOf()` rejects `!m_module_active`.
  - direct lookup path: rejects `!m_module_active || m_chunk_lookup_table.empty()`.
- Host-call, downcount, CPU-state, exception, timing, and dispatchability behavior remain unchanged.

Validation:

- OpenMUA2 tooling run `36047001104`: PASS on Ubuntu + Windows.
- ModernGekko run `36047001119`: standalone + full build/test PASS on Ubuntu + Windows, including MSVC/Ninja.

Status doc:

- `039782520c25933f7c6653797332bd7e8d0715cd` — `Record merged module-active burst cleanup`

## Latest accepted work — PR #23

PR #23 `Skip empty forced-fallback scan in interpreter path` is **MERGED**.

Merge commit:

- `20dd90851c3a2625ddb973f8850e2494bb33ca9f`

Behavior:

- Extends PR #19's empty forced-fallback-range short circuit to the interpreter/fallback branch in `Run()`.
- When `m_forced_fallback_ranges` is empty, the fallback path no longer calls `IsForcedFallbackAddress(ppc.pc)`.
- Configured forced-fallback ranges retain the exact prior behavior.
- Native dispatch, REL, SMC, host-call, timing, and exception semantics are unchanged.

Validation:

- OpenMUA2 tooling run `36055136834`: PASS on Ubuntu + Windows.
- ModernGekko run `36055136817`: standalone + full build/test PASS on Ubuntu + Windows, including MSVC/Ninja.

Status doc:

- `2de1dee811932d3d010dc606abf02cb7ab749769` — `Record merged interpreter fallback fast path`

## Historical RMSE52 performance boundary

Historical accepted native-REL run, predating recent optimizations:

- reported FPS ~`10.319`
- guest-frame FPS ~`2.062`
- speed ~`0.1404`
- native dispatches `174,476,293`
- native exceptions `5,959`
- hook fallback `1,299,055`
- hook fast cache `1,295,291`
- hook slow `3,764`
- JIT fallback runs `11,023`

This proves native REL progression, **not current performance**.

## Latest accepted work — PR #24

PR #24 `Reuse generated linked result on burst continuation` is **MERGED**.

Merge commit:

- `de411096f5986980ac58e1f3a7373d8f5351dc67`

Behavior:

- Preserves the linked PC returned by generated dispatch while keeping `m_guest.pc` in runtime form for host-side semantics.
- `TranslateRelAddress()` reports the active REL section selected during linked→runtime translation.
- `FastDispatchableLinkedAt()` uses a preserved linked result for the next verified chunk lookup when its invariants hold, avoiding the normal runtime→linked conversion.
- Forced-fallback and exact host-call checks remain on the runtime address.
- REL chunks must still belong to the resolved active REL section; DOL chunks require the non-REL sentinel.
- Lockstep-checked blocks and native exception returns disable preserved-result reuse.
- Any direct-linked eligibility miss falls back to `FastDispatchableAt() -> ResolveNativeAddress() -> RefreshRelSections()`.

Validation:

- OpenMUA2 tooling run `36071914753`: **PASS on Ubuntu + Windows**.
- ModernGekko run `36071914841`: **PASS all four jobs**:
  - standalone Windows: PASS;
  - standalone Ubuntu: PASS;
  - full Windows build/test: PASS;
  - full Ubuntu build/test: PASS.
- No RMSE52 game-side FPS claim is made from CI.

Status doc:

- `1256b9ff7e59dd48750fdc93ebcfec63a900ab22` — `Record merged linked continuation fast path`

## Latest accepted work — PR #25

PR #25 `Check cheap burst termination before continuation lookup` is **MERGED**.

Merge commit:

- `d3aeab80a3ae44c2d18265ad9f1e5868bf369e50`

Behavior:

- Reorders the native burst back-edge to check `ppc.downcount > 0` and CPU running state before `fast_native_continue()`.
- When the burst must already terminate, it skips one chunk/REL/host-call continuation eligibility probe.
- When the burst can continue, the existing `fast_native_continue()` path is unchanged.
- Synchronous/external exception breaks remain before the back-edge condition.
- Skipped REL refresh and lazy host-call-cache population are eligibility/cache work only and are re-established on the next eligible entry.

Validation:

- OpenMUA2 tooling run `36074380247`: **PASS on Ubuntu + Windows**.
- ModernGekko run `36074380288`: **PASS all four jobs**:
  - standalone Windows: PASS;
  - standalone Ubuntu: PASS;
  - full Windows build/test: PASS;
  - full Ubuntu build/test: PASS.
- No RMSE52 game-side FPS claim is made from CI.

Status doc:

- `3417a27ee6a0d3b032650020724c9758b2191973` — `Record merged cheap burst termination gate`

## Latest accepted work — PR #27

PR #27 `Check fallback termination before dispatch probes` is **MERGED**.

Merge commit:

- `b78431f21625ad61b4f66855f5f94b859f04cf7a`

Behavior:

- Reorders the interpreter-only fallback loop so `ppc.downcount > 0` and CPU running state are checked before `DispatchableAt(ppc.pc)` and `IsHostCallAddress(ppc.pc)`.
- An exhausted/stopped fallback slice no longer performs chunk verification, REL eligibility refresh, or an exact host-call lookup only to discover that it cannot continue.
- If the slice can continue, the same dispatchability and host-call stop conditions still run before another interpreted instruction.
- `IsHostCallAddress()` is read-only.
- Deferred `DispatchableAt()` work is eligibility preparation only; after a downcount exit the next outer-loop entry runs `core_timing.Advance()` and checks dispatchability before any native execution. After CPU stop there is no next execution to prepare.
- Forced-fallback, exception delivery, timing, SMC/hash protection, lockstep, and native re-entry semantics otherwise remain unchanged.

Validation:

- OpenMUA2 tooling run `36130494685`: **PASS on Ubuntu + Windows**.
- ModernGekko run `36130494695`: **PASS all four jobs**:
  - standalone Windows: PASS;
  - standalone Ubuntu: PASS;
  - full Windows build/test: PASS;
  - full Ubuntu build/test: PASS.
- No RMSE52 game-side FPS claim is made from CI.

Status doc:

- `c1ee8eba08ba06d2a08b687e26a8615f5c63ebfc` — `Record merged fallback termination ordering`

## Latest accepted work — PR #28

PR #28 `Reuse native entry host-call result` is **MERGED**.

Merge commit:

- `7b11a1869c85aec8d5384c7f044779454e37a69f`

Behavior:

- Splits native-entry eligibility into `entry_dispatchable` and `entry_host_call`.
- `entry_host_call` is evaluated only after `DispatchableAt()` succeeds and retains the same candidate-chunk coverage plus exact-address `host_call_at()` semantics.
- If the otherwise-dispatchable entry is rejected because it is an exact host call, the fallback branch reuses `entry_host_call` rather than immediately calling `IsHostCallAddress(ppc.pc)` again for the same PC.
- If the module is inactive or `DispatchableAt()` fails, the fallback branch still performs the original direct `m_guest.host_call && IsHostCallAddress(ppc.pc)` check.
- Host-call handling, LR JIT invalidation, passthrough state, timing, forced-fallback, SMC/hash, lockstep, and exception behavior are unchanged.

Validation:

- Initial OpenMUA2 tooling run `36142577297`: failed only because an older host-call gate regression still expected inline `!host_call_at(ppc.pc, entry_chunk_index)`.
- Updated that stale source-string regression without changing runtime behavior; current PR head became `8aba565403be5607bbfb9fa902f3ea82ef2394fa`.
- Current-head OpenMUA2 tooling run `36142703033`: **PASS on Ubuntu + Windows**.
- Superseded ModernGekko run `36142577241` was cancelled by workflow concurrency after the test-only head update.
- Current-head ModernGekko run `36142703037`: **PASS all four jobs**:
  - standalone Windows: PASS;
  - standalone Ubuntu: PASS;
  - full Windows build/test: PASS;
  - full Ubuntu build/test: PASS.
- No RMSE52 game-side FPS claim is made from CI.

Status doc:

- `fa6dde0c484903e9e9bfabf024af22f0569991c3` — `Record merged native-entry host-call reuse`

## Historical Linux container blockers

1. RMSE52 assets are persistent and verified; **re-upload is not required**.
2. The current private build checkpoint is persisted at:
   `/MUA2/Build-Checkpoints/MUA2-BUILD-CHECKPOINT-52obj.tar.zst`.
3. That checkpoint contains:
   - merged/regenerated 524-chunk source;
   - current GCC O2/no-IPO CMake/Ninja build tree;
   - current live REL audit JSON;
   - **52 durable total objects / 45 generated chunk objects**.
4. Checkpoint archive SHA-256:
   `c0b6dd25fe34bc03ff0a0d98fa7d77ee39ef5614d0257273a4ff722965fc5869`.
5. GCC O2+IPO remains unsuitable in this container because of LTO memory/time behavior.
6. A fresh optimized current-main native-REL game-side baseline is still required before accepting another performance micro-optimization.

## Historical Linux container continuation

1. If local state is missing, restore:
   `/MUA2/Build-Checkpoints/MUA2-BUILD-CHECKPOINT-52obj.tar.zst`.
2. Continue GCC O2/no-IPO from **52 total / 45 generated chunk objects**.
3. Compile the next finite explicit batch of missing chunk objects, small enough for Ninja to exit normally.
4. Verify each new target returns `ninja: no work to do.` and `ninja -t deps` reports `(VALID)`.
5. Persist a newer private checkpoint after meaningful progress.
6. Once the optimized module links, run the native audit and require **524/524** chunk-hash PASS.
7. Gameplay comes only after the optimized module passes audit.

## Previous container update — 2026-09-29

What happened:

- User explicitly asked whether the project can be pushed to Git. Confirmed GitHub write access through the connected GitHub integration.
- The local container had reset, so the private **48-object** checkpoint was restored from Library storage.
- Restored state verified exactly:
  - **48 total objects**;
  - **41 generated chunk objects**;
  - checkpoint SHA-256 matched `cef43c1af70f39b3bca44b46ed3c42fe1b66467c75bc12f3d5f01816971d3704`.
- Selected the next four genuinely missing generated chunk targets:
  - `chunk_0020_text1_80052900.c.o`
  - `chunk_0021_rel1_80E9E164.c.o`
  - `chunk_0021_text1_80056900.c.o`
  - `chunk_0022_rel1_80EA2164.c.o`
- Compiled all four as one finite explicit `ninja -j4` batch.
- Ninja exited normally.
- Re-requested all four targets; every one returned:
  `ninja: no work to do.`
- Verified all four dependency records report `(VALID)`.
- Durable build state advanced:
  - **48 → 52 total objects**;
  - **41 → 45 generated chunk objects**.
- Created and uploaded a new persistent private checkpoint:
  `/MUA2/Build-Checkpoints/MUA2-BUILD-CHECKPOINT-52obj.tar.zst`.
- Archive size: ~47 MB.
- Archive SHA-256:
  `c0b6dd25fe34bc03ff0a0d98fa7d77ee39ef5614d0257273a4ff722965fc5869`.
- No optimized `gRMSE52_recomp.so` has linked yet.
- No source/runtime semantics changed.
- No gameplay or FPS test was attempted.
