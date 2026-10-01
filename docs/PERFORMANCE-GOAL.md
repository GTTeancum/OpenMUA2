# OpenMUA2 sustained combat performance goal

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

## Earlier serial-runtime constraint — superseded by whole-process limit above

Keep CPU/GPU emulation on one thread. Multicore emulation is excluded from
the performance plan and acceptance results, including imported profiles.
The runtime applies a per-run single-core override; the benchmark also writes
CPUThread=False into its isolated profile. Older threaded comparisons below
are historical diagnostics and are not candidates for promotion.

The eventual original Xbox port is now a design constraint: prioritize portable
algorithms, reduced work and memory use, and avoid requiring parallel CPU
execution for gains. The current Windows x64 JIT/graphics stack is not an Xbox
port; platform backend and memory work remain future tasks. Windows audio,
I/O and shader workers can still exist; this setting is not host CPU affinity
or proof that the whole application fits on one original Xbox CPU.

Keep the present Windows resolution, normal-speed and combat pacing acceptance
requirements. Do not lower them in anticipation of the future console port.

## JIT direction authorized 2026-09-30

The user explicitly selected full JIT as the primary performance path. The
30 FPS combat, pacing, resolution, correctness, three-repeat and ten-minute
acceptance requirements below remain in force. Full JIT results now qualify
as candidate acceptance evidence; earlier statements limiting JIT to diagnostics
describe the superseded static-only direction. Average FPS alone is insufficient.

1. Preserve the unfinished direct-chunk experiment and existing user edits.
2. Add explicit CPU selection and a Windows JIT build/launch path that does not
   require generating or compiling a proprietary native game DLL.
3. Build/test that path; verify actual JIT activation, original game validation,
   native captures and audio-enabled combat. Retain explicit static recompilation.
4. Measure repeated and longer fights with host and guest frame timestamps;
   separate game cadence from host stalls and profile the slow-frame causes.
5. Implement measured pacing/performance improvements without speed hacks,
   omitted work, duplicated frames, reduced resolution or weakened correctness.
6. Complete three repeats and ten minutes of actual combat, then publish exact
   results, limitations, source/handoff checkpoints and a verified source backup.

For JIT-only execution no generated module is loaded, so module ABI/hash audit
is not applicable to that path. Original game DOL/REL validation and JIT memory,
code-invalidation, exception and timing protections remain required. Native
module audits remain required for static-mode changes and comparisons.

Use `Build.cmd --cpu jit --jobs 2` and `Run.cmd --cpu jit` (both default to
JIT). The combat runner now defaults to `--cpu jit` without `--module`.
For historical static experiments specify `--cpu staticrecomp --module ...`.
`--jit-diagnostic` remains a compatibility alias; it no longer requires an
environment override. Every completed replay must confirm the selected backend
in runtime.log. Explicit CLI selection overrides the legacy environment value.

Requested 2026-09-29. Target: sustained nominal 30 newly rendered FPS during
actual combat on the Ryzen 7 8745HS / Radeon 780M Windows host, at normal game
speed and the existing 1920x1080 preset / 3x EFB scale. The existing 6.223 FPS
opening traversal result is not a combat baseline. Small gains are intermediate
work, not completion. No claim of feasibility or completion precedes measurement.

## Acceptance

- Entire game process on one host logical processor; verify Windows affinity.
  CPU/GPU runtime execution also remains serial.
- Actual repeatable enemy combat with movement, attacks, damage and effects.
- Per-frame timestamps, rolling FPS, frame-time percentiles, counts and longest
  duration of slow-frame episodes; exclude loading from combat statistics but
  report it separately. Count unique game frames, never duplicate presentations.
- Sustain the title's nominal 30 FPS cadence at normal emulation speed; report
  exact cadence if NTSC timing is fractional. Do not pass on average FPS alone.
- At least three repeat runs and a ten-minute sustained combat session with
  varied encounters/effects where available, at the acceptance resolution.
- Check native captures for expected geometry, characters, animation, effects,
  HUD and behavior. Check audio-enabled execution; explicitly identify anything
  requiring user observation under the ban on host input and desktop capture.
- Windows build/tests and native module audit pass. Preserve SMC/hash protection,
  REL eligibility, exceptions, timing and saves. No skipping game work, speed
  hacks, disabled effects or silent resolution reduction to meet the target.

## Execution plan

1. Preserve existing binaries, source changes and private saves. Confirm remote
   main. Build a repeatable combat route using process-local automation only;
   create private combat checkpoints and visual evidence.
2. Add low-overhead, opt-in frame-time capture and a reproducible benchmark
   runner with run metadata, warmup, stage labels and result analysis. Validate
   metrics independently and measure instrumentation overhead.
3. Profile the current fight: native versus fallback execution, costly PCs,
   dispatch/burst counts, memory access, CPU/GPU time and synchronization.
   Use resolution and backend comparisons as diagnostics, not acceptance runs.
4. Rank bottlenecks by measured share and achievable total speedup. Implement
   substantial changes to the dominant path, potentially generated dispatch/
   block chaining, fallback coverage, memory access or synchronization. Avoid
   spending the effort budget on unmeasured low-impact micro-optimizations.
5. Rebuild through the existing Windows/MSVC path, run relevant regression tests
   and audit, then replay the identical fight. Compare correctness and frame-time
   distributions. Keep measured wins; retain raw failed experiment evidence
   privately. Re-profile after major changes rather than assuming the next cost.
6. Run acceptance tests with profiling disabled, repeat/soak and audio-enabled
   execution. Record remaining limits. Update canonical handoff and evidence,
   commit/push reviewed source changes to main and make a verified source backup.

## Constraints and checkpoints

No OS/UI input or Codex capture. Native process-local inputs/captures only.
Proprietary data, generated translations, saves, screenshots, raw logs and RAM
remain outside Git under .local. Preserve existing uncommitted diagnostic edits.
Use existing toolchains; no installers or global configuration changes.
Keep the goal active while meaningful work remains. A 1-2 FPS improvement is
not an acceptable final result. Document inability to prove a criterion rather
than quietly weakening it. This plan may change with profiling evidence while
the acceptance target remains fixed.

## Benchmark tooling added at the first measurement checkpoint

Set `MODERNGEKKO_FRAME_TIMES` to a private CSV path to capture unique-frame
steady-clock timestamps at the renderer's after-present callback. Records are
buffered (120,000 maximum) and written after the video thread stops. The parent
folder must exist. Zero lost samples are required; partial/crashed runs are not
usable. These timestamps measure completed callback cadence, not monitor
scanout. Duplicate presents are omitted and frame-counter rewinds start an epoch.

`tools/run_combat_benchmark.py` accepts explicit --runner, --module, --game,
--user, --state, --route and --output paths. Use an ignored .local output folder;
it copies user data and saves there. A private JSON route has restored_frame_min
and commands using the native automation protocol. The restore threshold must
match the chosen save. Frame-bounded attacks are repeated but the file-command
polling adds small wall-time gaps, so this is not a deterministic input movie.
The result records route completion separately from process exit. Input remains
inside the target process; it never generates host input.

Use --no-screenshots for timing runs, with separate captured replays for visual
checks. --no-trace permits overhead comparison. --resolution 640x528 and
--jit-diagnostic are diagnostic comparisons only; they do not satisfy the native
3x target. --profile-dispatch enables intrusive per-dispatch timing; never treat
its FPS as release performance. Profiling variables are cleared for normal runs.

Analyze with `tools/analyze_frame_times.py TRACE --start-frame FRAME --warmup 5
--guest-ticks-per-second 729000000 --output PRIVATE_JSON` for this Wii build.
The caller must select a contiguous post-restore sequence. The analyzer rejects
missing/overflowed traces, counter gaps/rewinds and reversed timestamps rather
than silently removing them. It reports average and rolling FPS, frame-time
tails, 1% lows, long-frame episodes and emulated/host elapsed time. No automatic
pass flag substitutes for combat visuals, normal speed or sustained validation.

## Presentation diagnostics added 2026-09-30

Use `--trace-presentation` for opt-in buffered XFB-copy/before/after events.
Effective CPU/GPU threading, presentation, resolution and speed settings appear
in runtime.log. `actual_ns` is the renderer's queue-submission timestamp, not
scanout. Copy guest ticks are available only in single-core mode. Immediate XFB
mode counts copies and needs composition/visual verification before any FPS
acceptance claim; one copy is not universally one visible game frame. No tested
presentation-setting experiment has passed the pacing target or become a default.

The runtime now creates Config/Cache parent directories before initialization.
Fresh-profile config and shader-cache writes were observed in an actual run.
This fixes persistence; no combat FPS improvement is attributed to it.

## MSVC inline experiment (historical)

`Build.cmd --native-rel --module-msvc-inline 1` builds an isolated O2/Ob1
module through the existing Windows toolchain. The default remains Ob0;
strict floating-point semantics and the documented per-chunk Od workaround
are preserved. Record the exact build result and compare the same private
combat route against the baseline before changing production defaults.
The build receipt records the requested inline level. This option by itself
is not evidence of a performance improvement.

For audio-enabled execution, use `--windowed --audio Cubeb`. This launches
the game window but all replay input remains inside the target process. The
headless runtime forces silent audio, so the runner rejects audible-backend
requests without --windowed. Check the actual backend in runtime.log; a
requested backend in run.json alone is not proof of activation. Successful
audio-enabled execution does not establish audible quality or synchronization.

Command files must be staged outside the runtime's watched commands directory:
it consumes all regular files, including .tmp. The replay runner publishes
closed files from a sibling staging folder and requires an exact successful
receipt set before reporting completion. Earlier 1x evidence is excluded by
the command-receipt audit; use clean reruns for resolution comparisons.

## Selective CPU-region diagnostics

Use --jit-ranges START-END[,START-END] to route chosen hexadecimal runtime
address ranges through the existing fallback JIT. This is a profiling tool,
not a production default or evidence of correctness. Native/JIT crossings
can dominate these results; they are not an additive cost decomposition.
Cannot combine with full --jit-diagnostic. Re-run the native control with
the same runner and separate loaded diagnostics from acceptance benchmarks.

Dispatch profiling now starts after the restored combat frame threshold via
STATICRECOMP_PROFILE_GATE_FILE. It includes initial neutral route frames and
is still intrusive. Keep full-run counters separate from gated timings; do
not label shutdown counters as combat-only counts.

## Smaller generated-function experiment

The full 4096-instruction Ob1 build completed and passed audit, but measured
5.86 FPS against 6.42 FPS for the baseline. Do not promote it. Next test uses
Build.cmd --native-rel --c-chunk-instructions 1024 --module-msvc-inline 2
--jobs 4. This changes generated function boundaries and compiler optimization,
not game resolution, cycle charges or effects. Generation keys, receipts and
module output isolate the experiment. Re-audit and compare the same fight;
smaller functions can also increase dispatch cost, so discard losses.

The explicit process-local command read_timing with path=<private file> writes
ticks and idle_ticks. Compare two snapshots after restoring the same save;
absolute idle counters may include prior saved execution. Initial native/JIT
combat probes both skipped zero idle ticks, so idle skipping is not a supported
optimization lead for this route. Snapshot pauses are diagnostic only.

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

## Guest producer investigation — 2026-09-30

Opt-in `--jit-block-profile` now resets counters after restored-state readiness
and dumps resident-block statistics. It is intrusive, excludes invalidated
blocks and must never be used for release-FPS acceptance. Ordinary benchmark
runs explicitly disable inherited profiling/debugging in their isolated copy.

Crowded normal-speed controls still measure 26.32–27.52 FPS, with 22 FPS rolling
one-second lows. Threading alone did not solve production delays. The guest
profile points to decimal conversion/division; a region containing decimal
conversion accounts for 19.05% of recorded cycles. Next trace its active callers
and determine whether equivalent work can be eliminated or implemented faster.
Do not disable diagnostics from a filename assumption, remove gameplay work,
change clock/cycle charges or count duplicate frames as a performance fix.
Full Xbox mapping, repeated acceptance and ten-minute varied combat remain open.
Evidence: `evidence/windows-20260930/JIT-BLOCK-PROFILE.json`.

## Active formatter callers resolved — 2026-09-30

The bounded, intrusive caller probe identifies animation diagnostics among the
hot formatting work. All three selected resident-block counts reconcile with
their captured callers and zero overflow. Active code samples match the original
DOL. A live logger snapshot has one stream with mask 7: observed channels 0x20
and 0x1000 are discarded after formatting, while channel zero remains enabled.
This is evidence for investigating lazy formatting, not permission to remove all
logging or claim that every hot conversion is unnecessary.

Next prove the live filter conditions and the temporary ring-buffer lifetime
effects. Implement a guarded, portable path only where the output is unused;
retain active diagnostics and original behavior when guards fail. Do not change
clock/cycle charges, remove game work or weaken code-identity protections.
Then compare uninstrumented single-core combat and inspect native output before
repeat/soak acceptance. No formatter optimization or new FPS gain is delivered
at this checkpoint. Evidence: evidence/windows-20260930/JIT-CALLERS-SINGLE-CORE.json.

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

## Xbox / XInput controls — quick reference

**PS2 target layout below; not the current complete implementation.** User testing
is deferred after poor frame rate/audio. New profiles use A attack, B heavy/back,
X grab, Y jump, LB block/use, Start pause, right stick camera and RB+face powers.
RB+A/B/X/Y selects down/right/left/up power slots. LT/RT currently cycle heroes;
D-pad still navigates menus/direct powers. Contextual X use, direct D-pad hero
selection and full fusion/revive behavior remain open. Fusion has a candidate
LB+RB request with delayed face-button partner aiming, tested only in a prepared
Spider-Man/Captain America sequence. No physical motion or manual pointer is an
intended requirement. Xbox gamepads are the supported target; Wii prompts remain.

| Xbox control | Action | PS2 equivalent |
| --- | --- | --- |
| Left stick | Move | Left analog stick |
| Right stick | Rotate camera | Right analog stick |
| A | Light attack; confirm in menus | Cross |
| B | Heavy attack; hold to charge | Circle |
| X | Grab / use / interact | Square |
| Y | Jump; flight where supported | Triangle |
| LB (hold) | Block | L1 |
| RB (hold) + A / B / X / Y | Use the corresponding power | R1 + face button |
| LB + RB (hold), then a face button | Fusion attack / revive teammate | L1 + R1, then face button |
| D-pad | Select hero | Directional buttons |
| Start / Menu | Pause | Start |

Power-slot order, direct hero selection, camera behavior and fusion/revive still
need validation against the Wii game. The PS2 manual lists Triangle for menu
back/cancel, which translates to Y in this target layout.

Sources: [Activision PS2 manual](https://www.gamesdatabase.org/Media/SYSTEM/Sony_Playstation_2/Manual/formated/Marvel_Ultimate_Alliance_2_-_2009_-_Activision.pdf),
printed pages 4–5. Its gameplay table repeats Cross for both attack and jump;
[the PS2 review](https://www.cheatcc.com/articles/marvel-ultimate-alliance-2-review-for-playstation-2-ps2-psx2/)
corroborates Triangle for jump/flight.

Current generated profile v2 fusion aiming: **hold LB+RB, aim with the right
stick, then press A to confirm**. This also reaches the tutorial ready icon.
With the stick neutral, face-button corner shortcuts remain available. This is
gamepad-only aiming; no physical Wiimote or mouse is required. Verified scope is
the prepared Spider-Man/Captain America tutorial sequence, not all partner/revive
flows. Existing controller profiles are preserved and may still use older bindings.
