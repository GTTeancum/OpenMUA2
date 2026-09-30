# Current status — GitHub main

September 29, 2026. This file tracks the current reconstructed source on `main`; the older LOCAL01 recovery boundary is no longer an accurate description of the checked-in implementation.

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

## Windows build checkpoint — September 29, 2026

The existing Windows entry point completed successfully on the local x64 host:
`Build.cmd --jobs 2 --native-rel`, Release, MSVC 19.44.35225.0, Ninja, CMake 4.3.1.
Both O2/indexed modules linked with IPO off and passed the native audit: DOL-only
325/325 chunk hashes; native DOL+REL 524/524, including uncovered-address rejection.
The runtime was incrementally rebuilt; the two module trees were newly built.
The selected build receipt points to `.local/build/windows-x64/module-mg01-rel-o2/gRMSE52_recomp.dll`.

The initial DOL module failure exposed missing indexed-array support in the
module-table generator. Exact coverage parsing now handles those arrays while
preserving merged-table precedence and SMC/hash protection. Regression tests
cover holes, hashes, and malformed intervals. Version metadata no longer assumes
`master` exists, and source backups now accept the canonical handoff document.

Validation: tooling 104 passed / 1 skipped; DolRecomp 19/19; ModernGekko 31/31;
broader runtime CTest 50 passed with `playTests` disabled and the CI exclusions;
GXRuntime 1/1. These runtime suite counts overlap. Nonfatal upstream CMake and
test-fixture deprecation warnings remain. Detailed warnings, binary hashes and
sizes are in `../evidence/windows-20260929/VALIDATION.json`; the latest handoff
records private local log paths. Pre-existing uncommitted runtime diagnostic
edits are preserved separately from this build and source commit.

This is a verified Windows build/module audit, **not a game execution**. RMSE52
was not launched, and no current gameplay or FPS result is available.

## Current reconstructed source

The repository still originates from the recoverable MG01 + FPC01 workspace, but the following missing pieces have now been reimplemented from the retained source/evidence and committed as new work rather than presented as recovered MR01 bytes:

- Native DOL + REL packaging and runtime eligibility are present. RMSE52's absolute linked REL section-table pointer is accepted with guest-RAM bounds checks, and native REL code remains protected by the existing chunk-hash / SMC verification path. Different or modified code is not made native merely to advance boot.
- REL metadata generation now binds the tracked live audit to the exact REL bytes it validated. Commit `2a86452d2aafe5156f9c4015ad7b2a3ec64b65de` requires audit status `LIVE_TEXT_MATCH` and an exact SHA-256 match against `source_rel_sha256` before replaying relocations or emitting native REL metadata, so a stale audit cannot silently authorize changed REL bytes.
- REL relocation replay is now bound to the retained live-text audit result as well. Commit `eb48bfa68c20ea2d9e9c8731a13cf311d72b3771` requires the zero-adjust comparison to be an exact match, requires well-formed and equal expected/observed text SHA-256 values, and verifies the replayed executable text bytes against that hash before native REL metadata is emitted. Same-size but incorrectly replayed text is therefore rejected.
- DolRecomp emits `dcbf`, `dcbst`, `dcbi` and `icbi` through the dedicated cache-control hook instead of the generic instruction-fallback callback. The Dolphin cache invalidation semantics and generated cycle accounting remain intact.
- The generated scalar FMA helper has been repaired to use the same instruction-shaped arithmetic/rounding path as the runtime floating-point implementation. Targeted regression coverage compares result bits, FPSCR state and write/no-write behavior across rounding/NI/VE/FI/FR cases and edge values.
- The lockstep checker contains local-loop boundary alignment: when native execution yields back at an inlined loop header, the interpreter shadow continues until it has performed the native charged work rather than comparing different loop iterations at the same PC.
- The runtime floating compare path preserves the fifth FPRF classification bit while replacing only FPCC, with a targeted runtime regression.
- MEM2 is now included in lockstep memory journaling/restoration. MEM1 and MEM2 use disjoint physical journal keys; native and interpreter MMU writes feed the same diagnostic journal, the shadow receives the pre-block image, and the native post-image is restored afterward.
- Merged DOL+REL dispatch keeps native eligibility bounded to generated code chunks. Alignment and overlap guards are covered, and commit `67db5f86875f6e40dca4f70b65fce63996a251ad` adds a regression proving an uncovered guest-address hole between generated ranges is not bridged into native eligibility. Commit `b777ddbe0d2c556fc00ee632bd273d0c1ee13621` additionally rejects unaligned generated chunk boundaries before emitting the merged table, and `5e7fc98e09f53c33f6c383682973af8824889d13` pins that PowerPC instruction-alignment invariant with a regression.
- Multiplatform performance is now the active development priority. Commit `a6328f40bb0c98a58c8f50e52a30d4da55390b7e` makes DolRecomp's existing page-indexed native dispatch the default and changes the OpenMUA2 workspace native-module optimization default from O0 to O2 on both Windows and Linux. `--dispatch-lookup linear` remains available for controlled A/B comparison, and generation cache keys/receipts record the dispatch mode so measurements cannot accidentally reuse output from the other mode.

## Active development priority — multiplatform performance

Performance on both Windows and Linux is the primary focus. Existing correctness, SMC, audit, and eligibility guards remain enabled, but expanding lockstep/correctness coverage is deferred unless a performance change produces a concrete failure that blocks measurement or execution.

Current performance baseline on `main`:

- native module optimization default: `O2`
- generated dispatch lookup default: `indexed`
- explicit A/B control: `--dispatch-lookup indexed|linear`
- build receipts record `module_opt` and `dispatch_lookup`
- DOL and REL generation cache identities include the dispatch mode
- combined DOL+REL dispatch now uses a 4 KiB guest-page index to narrow each lookup before binary search (`bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc`), with empty/single-chunk page fast paths from `f4b7f991deb4a9e787b97c3bac2a475faadfc1f6`
- ordinary DolRecomp indexed dispatch now emits an exact page-local run window; empty pages return immediately, single-run pages skip the scan, and multi-run scans cannot walk beyond the current 4 KiB page (`405b81de4136a7532e966218185a190f6eb9230d`)
- x86-64-v3 capability probing is cached (`917a5d893087736a74566dbaf71de5f3989a6d90`), normal chassis dispatch is bound to the selected baseline/v3 function at module load (`4e677ac8491bc3b5256998ba3688893f61d07695`), and generated indirect dispatch reuses the same module-load choice (`5e6ad207c77affbf500bf5327ce6222e9e7fd7c1`)
- native burst host-call checks now reuse the verified chunk index and consult cached per-chunk host-call coverage before invoking exact-address lookup (`2776fa0a3e80136495a32552b9d909e16dcfcd5e`); chunks known clean skip the per-block host-call probe while candidate chunks retain the exact check
- native eligibility now skips the out-of-line forced-fallback range scan entirely when no forced-fallback ranges are configured (`12a75b2db225678368be5e2b6340218ba2aafa2f`); configured ranges retain the existing exact range check
- the interpreter/fallback branch now applies the same empty-range short circuit (`20dd90851c3a2625ddb973f8850e2494bb33ca9f`), avoiding `IsForcedFallbackAddress()` when no forced-fallback ranges exist while preserving configured-range behavior
- native host-call eligibility now reads the cached per-chunk host-call state directly and only calls `ChunkContainsHostCall()` for unknown state (`5b87d08b8f6e4daff2ca64bab75d67632ec28721`); exact address checks remain limited to candidate chunks
- chassis-entered generated dispatch now uses dedicated host-call-free helpers (`8f8c32a9c90e039c898d78eb8313c7d9d64f28c5`), avoiding a second generated `ppc_host_call()` probe after StaticRecomp has already rejected host-call PCs; replacement dispatch and physical MEM1 alias fallback remain intact, while generic/indirect generated dispatch stays host-call-aware
- native dispatchability now returns the runtime→linked PC it already resolved and the burst loop reuses that value for the immediately following dispatch (`af7938bdb63d4530ea43a6ba445800fc4171a153`); continuation eligibility similarly carries the next linked PC, removing the duplicate `ResolveNativeAddress()` immediately before every native dispatch while preserving linked→runtime translation afterward
- post-dispatch REL return translation now carries the active REL section index and fast-paths returns that remain in the same section (`7b7e412f671039e1d29e2cdff7b2e51509bc046e`); cross-section and DOL returns retain the existing full `ResolveRuntimeAddress()` scan
- continuation-side runtime→linked resolution now reuses the previous active REL section as a first lookup hint (`7a9cdc0165257ec19301972785150e4961f7f65e`); same-section continuations avoid the linear active-section scan while hint misses preserve the old full scan, direct-DOL lookup, and REL refresh fallback
- native burst continuation now preserves the linked result returned by generated dispatch and, when its invariants remain valid, verifies the next chunk directly through `FastDispatchableLinkedAt()` (`de411096f5986980ac58e1f3a7373d8f5351dc67`). Forced-fallback and exact host-call checks still use the runtime address; REL chunks must still belong to the resolved active section; lockstep-checked blocks, native exception returns, and any direct-linked miss fall back to the existing runtime→linked resolver/REL-refresh path
- the native burst back-edge now checks `ppc.downcount > 0` and CPU running state before continuation eligibility (`d3aeab80a3ae44c2d18265ad9f1e5868bf369e50`), avoiding a chunk/REL/host-call probe when the burst must already terminate while leaving the existing continuation path unchanged for eligible bursts
- the interpreter-only fallback loop now checks `ppc.downcount > 0` and CPU running state before `DispatchableAt(ppc.pc)` and `IsHostCallAddress(ppc.pc)` (`b78431f21625ad61b4f66855f5f94b859f04cf7a`), avoiding chunk verification/REL refresh/host-call probes when the fallback slice must already end while preserving the same checks before any eligible re-entry
- native-entry host-call rejection now preserves the exact `host_call_at()` result and reuses it in the fallback branch (`7b11a1869c85aec8d5384c7f044779454e37a69f`), avoiding an immediate duplicate `IsHostCallAddress(ppc.pc)` lookup while retaining the direct lookup whenever module activity/dispatchability was not proven
- no current-main game-side speedup is claimed until the proprietary RMSE52 route is measured

Historical profiling showed very high native-dispatch counts and concentrated time in tiny runtime/cross-chunk entries. The accepted dispatch work now reduces lookup cost in both the ordinary DolRecomp path and the merged native DOL+REL path, removes repeated host-feature selection from normal/indirect dispatch, skips host-call address probes in chunks already known clean, reuses runtime→linked REL resolution across eligibility and dispatch, fast-paths same-section linked→runtime and runtime→linked transitions, and reuses verified generated linked results across eligible burst continuations. The next optimization work should focus on remaining cross-section/cross-chunk transfer overhead and other chassis checks that still execute on every native block before revisiting lower-volume correctness work.

## Fresh validation of current work

For the MEM2 repair, a focused Linux validation workflow first applied only the six intended source-file changes, configured/built GXRuntime, ran its tests, configured/built the ModernGekko Linux runtime and ran the ModernGekko tests. Every step completed successfully before the source repair was integrated into `main` as commit `26334ad651567bf098401f781d1189e5f75c296c` (`Journal MEM2 in lockstep verification`).

After that integration, the normal GXRuntime matrix completed successfully on both `ubuntu-latest` and `windows-latest`. This is useful Windows portability coverage for the shared runtime change, but it is **not** a full Windows game execution test.

The cross-platform ModernGekko workflow introduced by `c1eeb1a2fb8d9ea31954c818fe21d9269b7edc25` completed successfully. Its standalone tests and full configure/build/test jobs all passed on both Ubuntu and Windows, including the MSVC/Ninja Windows path. This remains source/runtime validation rather than proprietary RMSE52 execution.

The project tooling workflow for `67db5f86875f6e40dca4f70b65fce63996a251ad` completed successfully on both Ubuntu and Windows. The later tooling run for `5e7fc98e09f53c33f6c383682973af8824889d13` also passed on both `ubuntu-latest` and `windows-latest`; that regression verifies unaligned generated chunk metadata is rejected instead of entering the merged native dispatch table.

Tooling Actions run `35938476536` for `2a86452d2aafe5156f9c4015ad7b2a3ec64b65de` also passed on both `ubuntu-latest` and `windows-latest`. Its new regression verifies that exact audited REL bytes are accepted, changed REL bytes are rejected by SHA-256, and a non-matching live-audit status is rejected.

Pull-request tooling Actions run `35939460805` for the replayed-text guard passed on both `ubuntu-latest` and `windows-latest` before merge to `main` as `eb48bfa68c20ea2d9e9c8731a13cf311d72b3771`. The regression accepts correctly replayed text, rejects same-size replayed bytes whose SHA-256 differs from the live audit, rejects inconsistent expected/observed audit hashes, and rejects a text-section mismatch count.

Performance PR #2 (`a6328f40bb0c98a58c8f50e52a30d4da55390b7e`) was validated before merge by OpenMUA2 tooling run `35940341273` and DolRecomp run `35940341449`: both Ubuntu and Windows jobs passed. ModernGekko run `35940341332` had both standalone Windows and Ubuntu tests passing at merge time; its two larger full build/test jobs were still compiling and are not counted here as completed results.

Performance PR #3 (`bf2ecd76eb6050ceedba2c9e8d4d21619f06c8dc`) page-indexes the combined native DOL+REL dispatcher. OpenMUA2 tooling run `35941158719` passed on both Ubuntu and Windows before merge. Its tests cover page-index emission, empty uncovered pages, alignment, overlap rejection, and the existing dispatch-hole invariant.

Performance PR #9 was merged as `405b81de4136a7532e966218185a190f6eb9230d`. It adds an exact page-local run end to the ordinary DolRecomp indexed dispatcher, fast-paths empty and single-run pages, and bounds multi-run scans to the current page. DolRecomp Actions run `36000036463` passed configure/build/test on both Ubuntu and Windows. ModernGekko Actions run `36000035131` subsequently completed successfully: standalone and full build/test jobs all passed on Ubuntu and Windows.

Performance PR #13 was merged as `2776fa0a3e80136495a32552b9d909e16dcfcd5e`. It returns the already-resolved verified chunk index from dispatchability checks and gates `IsHostCallAddress()` behind cached `ChunkContainsHostCall()` coverage in native bursts, preserving exact host-call checks only for candidate chunks. OpenMUA2 tooling run `36003571111` passed on Ubuntu and Windows. ModernGekko run `36003570930` passed all four jobs: standalone and full build/test on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #14 was merged as `af7938bdb63d4530ea43a6ba445800fc4171a153`. It carries the runtime→linked PC already produced by `DispatchableAt`/`FastDispatchableAt` into the immediately following module dispatch and through native-burst continuation, removing the duplicate `ResolveNativeAddress()` call before dispatch while keeping post-dispatch linked→runtime translation intact. OpenMUA2 tooling run `36008961778` passed on Ubuntu and Windows. ModernGekko run `36008961830` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #17 was merged as `7b7e412f671039e1d29e2cdff7b2e51509bc046e`. It carries the active REL section index alongside the linked PC and uses that section as a hint for post-dispatch linked→runtime translation. Same-section returns now translate directly; cross-section and DOL returns still use the full `ResolveRuntimeAddress()` scan. OpenMUA2 tooling run `36026447896` passed on Ubuntu and Windows. ModernGekko run `36026447968` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #18 was merged as `7a9cdc0165257ec19301972785150e4961f7f65e`. It adds the reverse same-section hint to continuation-side runtime→linked resolution: the previous active REL section is checked first, and a miss falls back to the original full section scan, direct-DOL lookup, and `RefreshRelSections()` path. OpenMUA2 tooling run `36030032034` passed on Ubuntu and Windows. ModernGekko run `36030032243` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #19 was merged as `12a75b2db225678368be5e2b6340218ba2aafa2f`. It avoids calling `IsForcedFallbackAddress()` from `FastDispatchableAt()` / `DispatchableAt()` when the configured fallback-range vector is empty, preserving the exact existing range behavior when ranges are present. OpenMUA2 tooling run `36035472440` passed on Ubuntu and Windows. ModernGekko run `36035472344` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #20 was merged as `5b87d08b8f6e4daff2ca64bab75d67632ec28721`. It reads `m_chunk_host_call_state` directly in the native burst path so cached clean/candidate chunks avoid the out-of-line `ChunkContainsHostCall()` call; unknown chunks retain lazy discovery and candidate chunks retain exact `IsHostCallAddress()` checks. OpenMUA2 tooling run `36038408893` passed on Ubuntu and Windows. ModernGekko run `36038408875` passed standalone and full build/test jobs on Ubuntu and Windows, including the MSVC/Ninja full build.

Performance PR #21 was merged as `8f8c32a9c90e039c898d78eb8313c7d9d64f28c5`. It adds dedicated generated chassis dispatch helpers that skip the duplicate `ppc_host_call()` probe already handled by StaticRecomp, while preserving replacement dispatch and physical MEM1 alias fallback; normal/indirect generated dispatch remains host-call-aware. OpenMUA2 tooling run `36042200541`, DolRecomp run `36042200581`, and ModernGekko run `36042200621` all passed on Ubuntu and Windows, including full ModernGekko MSVC/Ninja integration.

Performance PR #22 was merged as `dc43d362d425134222d00aa02dd4dbec99fcf222`. It removes the redundant explicit `m_module_active` check from the native burst back-edge because `fast_native_continue()` already rejects inactive modules on both the REL/forced-fallback and direct lookup paths. OpenMUA2 tooling run `36047001104` and ModernGekko run `36047001119` passed on Ubuntu and Windows, including full MSVC/Ninja integration.

Performance PR #23 was merged as `20dd90851c3a2625ddb973f8850e2494bb33ca9f`. It extends the empty forced-fallback-range short circuit to the interpreter/fallback branch in `Run()`, preserving configured forced-fallback behavior and all native dispatch, REL, SMC, timing, and exception semantics. OpenMUA2 tooling run `36055136834` passed on Ubuntu and Windows. ModernGekko run `36055136817` passed standalone and full build/test jobs on Ubuntu and Windows, including full MSVC/Ninja integration.

Performance PR #24 was merged as `de411096f5986980ac58e1f3a7373d8f5351dc67`. It preserves the generated linked result across eligible native burst continuations so the next verified chunk can be checked without the normal runtime→linked resolution round trip, while retaining runtime forced-fallback/host-call checks, active-REL-section validation, lockstep/native-exception guards, and the existing resolver/REL-refresh fallback on any invariant miss. OpenMUA2 tooling run `36071914753` passed on Ubuntu and Windows. ModernGekko run `36071914841` completed successfully: standalone and full build/test jobs all passed on Ubuntu and Windows, including full MSVC/Ninja integration.

Performance PR #25 was merged as `d3aeab80a3ae44c2d18265ad9f1e5868bf369e50`. It checks cycle-budget exhaustion and CPU running state before `fast_native_continue()` at the native burst back-edge, so a burst that must already stop no longer performs another continuation chunk/REL/host-call eligibility probe. Exception breaks and the continuation behavior of still-eligible bursts are unchanged. OpenMUA2 tooling run `36074380247` passed on Ubuntu and Windows. ModernGekko run `36074380288` passed all four jobs: standalone and full build/test on Ubuntu and Windows, including full MSVC/Ninja integration.

Performance PR #27 was merged as `b78431f21625ad61b4f66855f5f94b859f04cf7a`. It moves the interpreter-only fallback loop's cheap cycle-budget and CPU-running checks ahead of `DispatchableAt(ppc.pc)` and `IsHostCallAddress(ppc.pc)`, so an exhausted or stopped slice no longer verifies chunks, refreshes REL eligibility, or probes an exact host-call address solely to discover that the slice cannot continue. Those checks still run before any eligible re-entry. OpenMUA2 tooling run `36130494685` passed on Ubuntu and Windows. ModernGekko run `36130494695` passed standalone and full build/test jobs on Ubuntu and Windows, including full MSVC/Ninja integration.

Performance PR #28 was merged as `7b11a1869c85aec8d5384c7f044779454e37a69f`. It stores the exact host-call result from an otherwise-dispatchable native entry and reuses that result in the immediately following fallback branch, removing a duplicate exact-address host-call lookup only on the path where `host_call_at()` already proved the same PC. Non-dispatchable/module-inactive paths retain the direct lookup. The first tooling run (`36142577297`) exposed one stale source-string regression; that test was updated without changing runtime behavior. Current-head tooling run `36142703033` passed on Ubuntu and Windows. ModernGekko run `36142703037` passed standalone and full build/test jobs on Ubuntu and Windows, including full MSVC/Ninja integration.

The earlier current-source commits also include cross-platform DolRecomp CI for the cache-control generator change and cross-platform GXRuntime CI for the FMA/runtime changes.

## Validation boundary

No fresh RMSE52 WBFS boot/gameplay session has been run from GitHub CI after the latest cache-control, FMA and MEM2 changes because the proprietary game image is intentionally not in the repository. Historical MR01 gameplay and the 11,832-comparison differential pass remain useful evidence and reproduction targets, but they are **not** treated as fresh validation of current `main`.

The latest recorded live native-REL benchmark predates the direct cache-control generator change: it advanced game frames but remained far below the 30 FPS target. A speedup from the later source changes must be measured rather than inferred.

No new claim is made here for a complete level, long-session stability, multiplayer, audible audio, game-owned save/reload, physical-controller gameplay, or Windows game execution.

## Immediate priorities

1. Build the current `O2 + indexed` native module/runtime against the exact RMSE52 image and benchmark the same route on Windows and Linux wherever the local game workspace is available.
2. A/B `--dispatch-lookup indexed` against `--dispatch-lookup linear` with the same compiler, optimization level, route, warmup, graphics/audio settings, and sample window. Keep both raw results.
3. Profile native-dispatch/chassis overhead on the faster baseline: dispatch count, hottest dispatch PCs, burst length, host-call checks, REL address translation, native exceptions, and JIT fallback. Ordinary/merged dispatch lookup, host-feature selection, clean-chunk host-call probing, and duplicate runtime→linked resolution have now been reduced, so measure after these changes.
4. Optimize shared generated/native transfer paths that benefit MSVC and GCC/Clang together. Same-section REL translation is fast-pathed in both directions, eligible continuation reuses the generated linked result directly, empty forced-fallback scans are removed, cached host-call state is read directly, duplicate generated host-call chassis probes and native-entry exact lookups are removed, and both native and interpreter burst/fallback back-edges exit on exhausted downcount/CPU stop before expensive eligibility work. A concrete next shared target is per-slice game-ID gating: `SConfig::GetGameID()` acquires the metadata mutex and returns a `std::string` copy every timing slice before comparing it to the static module's `char game_id[8]`. Add a narrow locked predicate that compares the stored game ID in place (while still reading current metadata every slice) and use it for module activation, preserving dynamic metadata-change visibility without the repeated string copy.
5. Rebuild and re-measure on both platforms after each accepted performance change. Do not infer a speedup from source structure or CI.
6. Defer additional lockstep/correctness expansion until performance work reaches a useful plateau or a concrete failure blocks further performance measurement. Existing correctness/SMC/audit guards stay enabled.

The original extracted `sys/main.dol` remains the boot source. Any merged/generated DOL/REL reference is code-generation material only and must never replace the game's real boot DOL.
