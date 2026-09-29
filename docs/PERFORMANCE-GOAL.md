# OpenMUA2 sustained combat performance goal

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

## MSVC inline experiment

`Build.cmd --native-rel --module-msvc-inline 1` builds an isolated O2/Ob1
module through the existing Windows toolchain. The default remains Ob0;
strict floating-point semantics and the documented per-chunk Od workaround
are preserved. Record the exact build result and compare the same private
combat route against the baseline before changing production defaults.
The build receipt records the requested inline level. This option by itself
is not evidence of a performance improvement.
