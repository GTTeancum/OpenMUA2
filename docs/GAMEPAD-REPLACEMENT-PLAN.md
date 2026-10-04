## Shared four-port gamepad provider - 2026-10-03

## Shared gamepad capabilities and QTE completion - 2026-10-03

Found another shared Wii dependency: CInputManager vtable +124 reports device
connection and complete-controls capability independently of IsConnected/IsReady.
With empty Wii bindings and Extension=None, gameplay stopped updating while raw
input still polled. Merely changing the diagnostic profile to Nunchuk let the QTE
finish. The opt-in gamepad provider now replaces this shared physical-port query
with actual Xbox connection state for both outputs; disconnected pads stay false.
No extension identity, pointer, motion input or completion event is synthesized.

Moved QTE edge counting to input publication so animation callback cadence cannot
miss presses/releases. A recent native QTE clock is required, preventing progress
while gameplay is suspended. QTE and optional Xbox prompt hooks no longer require
managed Wii profile bindings when the shared provider owns input.

Final runtime test used empty Device/buttons and Extension=None on all four ports.
Wrong-port X counted zero; holding X counted once; twelve separate presses completed
the original stage/callback path, removed the interaction entity, cleared the actor
and restored movement. Native Start/B pause/resume passed: three X presses while
paused left progress at1, fresh X after resume made2. Each of four physical ports
independently reported connected/complete 1 -> 0 -> 1. Saved fixture and process-local
input only; no physical-controller or rendered acceptance claimed.

A prior-checkpoint five-minute main-menu idle test also passed A/B navigation and
reconnection, with timestamps past305 seconds. It is included as prior-build evidence,
not a new full regression of that flow. Final Windows Release runner build passed,
focused tests2/2 in1.94s; runtime/codegen suite50/50 in8.11s. Native run exited0.
No compiler warnings/errors in the targeted incremental build; existing unsupported
formatter diagnostics remain. Null audio; no FPS/audio claim.

Evidence: evidence/windows-20261003/GAMEPAD-CAPABILITIES-QTE.json. Installed game,
real saves and proprietary assets are unchanged. Provider remains opt-in. Next:
full per-player fusion (current selector excludes human partners), other QTE and
UI/targeting consumers, then whole-flow and physical/rendered acceptance. Preserve
unrelated readiness/wave/StaticRecomp experiments; do not resume prompt-only patches.


## Four joined gamepads and chooser ownership - 2026-10-03

The opt-in shared gamepad provider now maps horizontal D-pad/left-stick input to
HUDPreviousItem/HUDNextItem (63/64), consumed by the existing profile/hero join
chooser. Start opens joining; A confirms the profile and then an available hero.
The engine retains profile creation, occupied/KO checks and actor assignment.

A mapping-only candidate failed: unjoined chooser input also emitted gameplay
hero selection and changed player 0's hero. The corrected provider uses the
engine's joined-player flags and logical-to-physical mapping to restrict unjoined
pads to chooser/menu actions. Gameplay actions become available after native join.

A real headless plaza fixture now joined all four heroes through process-local
Start/A/direction input: actor slots 0..3 each have AI=false and owner ports 0..3.
Simultaneous movement actions were [0.25,0.375,0.5,0.625]; releasing port 2 changed
only its value to zero; all released values became zero. Selecting a hero already
owned by another player left all four owners unchanged. This is native state
verification, not physical-controller or rendered gameplay acceptance.

Cold boot reached the title and readiness-based Start/main/A/Story/B/main passed.
Earlier fixed-time assertions failed during the opening movie and menu transition;
those failures remain in diagnostics. No load-state was used for the cold check.
Windows Release runner build passed; focused tests 2/2 (1.97s), runtime suite plus
codegen_compile 50/50 (10.70s). Existing formatter diagnostics remain; Null audio,
no FPS/audio claims. Evidence: evidence/windows-20261003/FOUR-PLAYER-JOIN.json.

The installed game and real saves are unchanged. Full per-player fusion/QTE,
remaining UI/targeting consumers, cold campaign, idle/reconnect and visual/physical
acceptance still remain; provider is opt-in. Preserve unrelated local experiments.


Development flag OPENMUA2_GAMEPAD_PROVIDER=1 now publishes raw Xbox actions at
CInputManager's shared update boundary, independently of KPad availability gates.
All four ports own connection, neutral-on-connect, held/released input and action
state. Existing game-disabled/ownership checks and action queue bookkeeping are
preserved. Default remains the supported compatibility path; no installation changed.

Fixed a release-build HLE dispatcher defect: its empty observer/branch marker
functions could share an address after linker folding. Dispatch now uses explicit
hook identity. The identical saved-state run then executed all four update callbacks.
The immediate-query replacement is implemented but has not been exercised by these
runs; do not claim full caller coverage from update counters alone.

Actual process-local tests with all 124 legacy binding counts disabled on all four
input objects passed simultaneous distinct axes, per-port release/disconnect,
held reconnect suppression, fresh input after neutral and held-state clearing on
load. Synthetic pads were not assigned to Wii profiles. Main-menu A/B navigation
passed; port 1 A did not operate port 0's menu. Native Story-menu capture was inspected.
This does not prove four joined actors, physical pads, connection warnings, combat,
fusion, QTEs or complete removal of pointer interactions. Full goal remains active.

Windows Release runner and two relevant test binaries rebuilt successfully;
focused tests 2/2 (2.18s), full existing regression suite 49/49 (7.28s). Initial full
Build.cmd completed before the manager changes (49/49, 14.41s); GUI/package was
not rebuilt or staged afterwards. No compiler/linker errors. Existing configure
warnings remain. A separate intrusive JIT-profiling run crashed before startup
(exit 3221225477); no profile was produced and its private config was restored.

PS2 executable reference is extracted only under ignored .local/ps2-reference.
Its shared tutorial accepts action 9 by scheduling normal close after 0.2 seconds;
Wii instead enters phase 1 pointer readiness. This is code evidence, not an asset
inference, and that semantic replacement is still pending. PS2 config initialization
loops through four slots using one method; retail multiplayer support is not inferred.

Next: finish shared connection/status and four-player ownership, then implement
PS2-style shared tutorial acceptance and audit remaining pointer/motion consumers.
Do not resume isolated prompt substitutions. Preserve real saves and single-core
execution. Prior readiness/wave/StaticRecomp experiments remain local and excluded
from this focused checkpoint. Evidence: SHARED-GAMEPAD-PROVIDER.json and
PS2-GAMEPAD-REFERENCE.json under evidence/windows-20261003.

# Gamepad input replacement plan

Latest audit: `GAMEPAD-PATH-AUDIT.md`. Shared readiness acceptance failed;
the direct backend has an early-exit coverage hole and unmapped menu operations.
Do not infer complete input coverage from prior menu checks.

Status: direct action backend implemented behind a development flag; menu checks
passed, shared pointer/motion replacement and gameplay validation incomplete. The installed release has
not changed. A complete dormant gamepad backend has NOT been verified.

## Code evidence

The igKPadInputDeviceManager device-enumeration function at 0x80293cf4 probes
four ports and creates a device only for reported types 0 or 1. Its sample
decoder at 0x802930f0 translates remote buttons and the type-1 extension into
engine signals. The inspected decoder has no alternative gamepad branch.
This is code evidence, not a conclusion drawn from glyphs or class names.
It does not prove no unused code exists elsewhere.

CInput evaluates 124 logical actions at 0x810f7e80. Current action bits,
previous bits and scalar values are distinct. Scalar values can survive button
release: the failed readiness candidate incorrectly used stale scalar B/X
values to reject a fresh A press. Button state must come from current bits or
the new raw gamepad sample. The existing Xbox v5 profile translates physical
buttons into Wii buttons, tilt, IR and Nunchuk shake before this evaluator.
That path is the compatibility layer to replace.

The surviving button-challenge parser and callbacks are reusable gameplay code,
not proof of a complete platform input backend. Names of unbound actions alone
also do not establish working consumers.

## Replacement boundary and execution order

1. Introduce a per-port gamepad sample owned by the input layer, with connection,
   buttons, triggers, sticks and edges. Hardware and process-local tests feed the
   same interface. Clear held state on disconnect/load/reassignment; preserve
   single-core execution and existing timing. Do not derive Xbox state from Wii
   chords, tilt, cursor position or gesture recognizers.
2. Feed the native logical-action layer directly. Establish a single explicit
   mapping for gameplay, menus, hero selection and powers, preserving the agreed
   layout until each consumer is verified. Keep native action history/queues and
   player ownership. Remove the synthetic marker protocol and stock Wii alias
   leakage. Do not enable debug/network actions merely because descriptors exist.
3. Replace semantic consumers by interaction family: readiness uses A/B and joined
   player readiness; fusion uses LT plus face-button partner selection with native
   eligibility/cost/revival rules; motion QTEs use fresh repeated X presses with
   native success/failure/cleanup. Aim/target interactions need explicit gamepad
   semantics, not an always-valid fake pointer. Enumerate remaining callers of
   cursor and gesture actions plus direct device accesses before declaring coverage.
4. Make prompts use the same interaction/action context as input. Statue/co-op QTE
   shows repeated X; fusion shows LT + A/B/X/Y. Remove pointer instructions only
   with their working replacement. Controls-menu redesign remains deferred.
5. Validate full flows in isolated profiles using process-local input and native
   renderer captures: cold start/menus, actual statue, genuine first-use fusion
   readiness and selection, QTE families, cancel/release/held buttons, wrong ports,
   idle/reconnect, save/load and death/retry. Inspect content, not frame counters.
   Fixture tests must be labeled; physical-controller acceptance remains separate.
6. Only then package/stage the root EXE and matching assets, preserve real saves,
   and report Windows build/tests and actual installed hashes. Never commit
   proprietary extracted or generated data.

## Candidate disposition

The unvalidated tutorial-ready and pointer-warning candidate is parked under
workspace work/parked-tutorial-ready, with its patch/header/source. It is not in
active source and was never installed. Preexisting experimental wave-QTE and
StaticRecomp work is preserved. The local compiled runner is still the older
failed candidate: rebuild before any packaging. No new build or gameplay pass
is claimed by this audit.

Evidence: evidence/windows-20261003/INPUT-ARCHITECTURE-AUDIT.json.
