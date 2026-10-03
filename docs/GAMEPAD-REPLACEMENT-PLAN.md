# Gamepad input replacement plan

Status: architecture audit, implementation incomplete. The installed release has
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
