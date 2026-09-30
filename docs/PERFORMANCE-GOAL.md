# OpenMUA2 sustained combat performance goal

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
