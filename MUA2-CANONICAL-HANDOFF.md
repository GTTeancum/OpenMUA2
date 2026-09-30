# MUA2 CANONICAL HANDOFF — Wii Marvel: Ultimate Alliance 2 native PC recompilation

**Project:** Wii Marvel: Ultimate Alliance 2 USA (RMSE52) native-PC recompilation  
**Repository:** `GTTeancum/OpenMUA2`  
**Canonical branch:** `main`  
**Date:** 2026-09-30

**Latest local checkpoint:** Windows build and audit completed below. The older
Linux container's 52-object checkpoint remains historical and is not a blocker
for the Windows workspace.

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
