# OpenMUA2 sustained combat performance goal

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
