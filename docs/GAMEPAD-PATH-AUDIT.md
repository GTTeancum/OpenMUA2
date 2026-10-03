# OpenMUA2 gamepad path audit — 2026-10-03

**A complete alternate gamepad path has not been verified in the Wii executable.**
The inspected native device factory and decoder implement KPad input. Reusable
logical actions and button-challenge code survive, but they do not constitute a
complete platform backend. The development direct-input layer is also incomplete.
Do not present either as an existing switch that removes every Wii interaction.

## Evidence by layer

| Layer | Code evidence | Finding / remaining work |
| --- | --- | --- |
| Device creation | 0x80293cf4; probe at 0x80293d34, type guard at 0x80293d44, constructor call at 0x80293d58 | Four ports are probed; only types 0 and 1 create devices in this factory. No alternate gamepad creation branch was found here. |
| Physical sample decoding | 0x802930f0; type dispatch 0x802932cc–0x802933d8 | KPad status, remote and extension handling. Unsupported types take the inactive path. A search of both executable files found KPad concrete input-device/system/manager names and generic input base classes; names alone are supporting evidence, not proof that all dormant code is absent. |
| Logical actions | 0x810f7e80; 124 action descriptors | Movement, combat, menus, cursor, gestures and button challenges share an action layer. An action name or unbound descriptor does not prove a working gamepad consumer. |
| Direct backend coverage hole | 0x810f7eac–0x810f7f1c versus hook 0x810f8544 | A zero device field at +20 takes a path that branches directly to 0x810f8654, bypassing the current direct-input hook. A complete backend must cover device lifecycle and all evaluator exits, not only the successful KPad evaluation path. This static hole is proven; its role in the failed modal run is not yet dynamically established. |
| Menu navigation | 0x81009d74; direction table 0x811698f8 | Previous contained menu checks passed. That evidence does not cover every menu or establish that the direct hook runs in gameplay/modal contexts. Internal widget names can differ from visible labels. |
| Additional menu operations | Current tools/mua2_gamepad_actions.hpp compared with action descriptors | No direct assignments for View Boosts (99), menu next/previous hero (103/104), next/previous profile (116/117), add/remove skill point (118/119), reallocate (122), or cycle heroes (123). They require consumer-specific verification and intentional mappings; do not blanket-enable actions. |
| Movement/combat | Current direct model assigns movement, attack, grab/use, block, jump and powers | Each assignment needs actual gameplay acceptance, including axes, transitions and ownership. Throw/directional throw (22–26), alternate power-selection actions (34–37) and other unassigned actions need consumer auditing; some may be unused, but absence of a binding does not settle that. |
| Hero switching | Native candidate hook 0x80052fac and direct hero-slot model | Reuses native selection checks. Full gameplay and menu hero-selection coverage is not established. |
| Fusion | Request action 33; native pickers 0x81068a60 / 0x810692c4 | Gamepad partner slots feed native eligibility/cost checks. First-use readiness, complete partner selection, revival and cancel still require an end-to-end pass. Retain dynamic revival-cost formatting. |
| Shared tutorial readiness | Setup 0x8024cba4; update 0x8024cdf8 | Five tutorial kinds share this class. Six traced call sites include first-use fusion and a script wrapper. Readiness queries action 9 and then performs pointer hit-testing at 0x8024d1fc or 0x8024d2f4. These are real pointer dependencies, not merely obsolete words in a texture. |
| Pointer warning | Show 0x8024c82c; separate warning class | Removing the message cannot substitute for replacing the consumers that need pointer data. Mixed/custom profiles and genuine disconnects must not be mislabeled. |
| Button challenges | Wrappers 0x80eb69a4 / 0x80eb6aa8; initializer 0x80100474; update 0x800f7a70 | Real button challenge code exists: actions 43–46, deadlines, player selection, success/failure callbacks. This is reusable code, not proof of a complete PS2/PSP/Xbox input backend. |
| Sequence/motion QTEs | Parser 0x80096ab0; button token parser 0x80098330 | Parser prefers revbuttonseq; type 8 follows a separate path. Repeated X must replace motion semantics and retain native success/failure/cleanup. Generic button QTE support does not prove every motion family is covered. |
| Connection, idle and saves | Runtime still uses managed v5 device assignment/native polling lifecycle | Direct XInput Connected sampling exists, but whole-path reconnect, held-state cleanup, idle and save/load acceptance remain open. The native KPad-dependent early path is particularly relevant here. |
| UI prompts | Mixed generic action prompts, tutorial strings and context-specific banners | Prompt selection must follow actual interaction semantics. Statue/co-op = repeated X; fusion = LT plus A/B/X/Y. No global replacement of one shared motion picture with one action is valid. Controls-menu redesign remains deferred. |

## Latest runtime result: failed readiness acceptance

The saved genuine fusion-readiness block was loaded into an isolated, headless
single-core JIT run. Neutral, wrong-port A, X, LT+A, and plain A were exercised
inside that process. **Plain A did not close the readiness screen.** All ready
slots remained zero. The native renderer capture still showed the pointer
instructions and pointer warning. The deferred readiness-installation message
never appeared, so this run does not demonstrate that the new input observer
was reached. Do not infer success from its startup “installed” log.

An earlier candidate stalled during startup after attempting to inspect memory
before asynchronous boot completed; it was terminated. The revised candidate
exited normally after the failed acceptance check. Both builds passed 49 tests;
the final Windows build completed with exit 0 and 49/49 in 10.41 seconds. These
tests do not override the failed actual-game result. No FPS/audio or physical
controller acceptance is claimed. Captures came from the native renderer, with
no desktop capture or host input.

## Required implementation direction

1. Establish a gamepad input/update path independent of KPad availability, with
   observable invocation counters and raw connection/button/axis evidence.
2. Audit and map the remaining gameplay/menu consumers, keeping unused/debug
   actions disabled. Track every family above as passed, failed, or unverified.
3. Replace the shared pointer/readiness and motion families at their semantic
   boundary; preserve player ownership, costs, deadlines and callbacks.
4. Validate full flows, including release/hold, wrong ports, reconnect, idle,
   save/load and genuine first-use tutorials. Inspect actual rendered content.
5. Stage only a build that passes those checks. Current installed EXE, game data
   and real saves remain unchanged. Development readiness/HLE edits are pending
   and must not be packaged as an accepted fix.

This is an architecture and coverage audit, not an exhaustive proof that every
unreachable instruction in the executable lacks a dormant platform path, nor a
full-game playthrough. The available evidence is sufficient to reject the claim
that a complete gamepad backend has already been found or delivered.
