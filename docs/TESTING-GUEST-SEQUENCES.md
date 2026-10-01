# Guest-clock input sequences

`tools/run_combat_benchmark.py --guest-sequence-start TICKS` preloads a route into
one process-local Xbox input sequence. `TICKS` is an absolute guest clock value;
zero starts at the current guest time. A nonzero start must still be in the
future when the native command is applied, within 60 guest seconds. A missed
start fails instead of silently shifting the test. No game clocks are changed.

Use a fixed absolute start with the same private restored state for comparisons.
This controls input deadlines; it does not guarantee deterministic game state.
Compare numeric observations as well as timing. Keep the existing benchmark
CPU, resolution and backend checks. User restriction: tutorial plaza only,
`--no-screenshots`, no desktop input or capture.

Sequence routes permit only:

- `xbox_time`: existing port, milliseconds, button/axis values, release and receipt
  path fields. Every hold starts where the previous hold was scheduled to end.
- `read_memory`: existing address, size and output path. Guest memory is copied
  into a preallocated buffer at that boundary, without writing guest memory.
- `read_timing`: records guest and idle ticks at that boundary.
- `check_memory`: address and hexadecimal `data`; compares live bytes. A mismatch
  cancels remaining sequence steps and clears synthetic input immediately.

All timed holds use the same controller port. At most 2048 commands, ten minutes
of requested holds and 64 MiB of memory snapshots are accepted. Each output path
must be distinct. Screenshots, state loads/saves, frame-based input, nested
sequences and guest memory writes are rejected. Once execution starts, a sequence clears its
synthetic controller when it ends, fails or is interrupted, including when its
last hold specified `release=0`.

The scheduler advances input and takes snapshots on the guest CPU thread. File
parsing, buffer allocation and controller setup happen beforehand. Snapshot
files and timing receipts are written after execution. A boundary may run late
by a guest instruction/block, but that lateness is not added to later deadlines.
Missing an entire hold interval fails the sequence. The benchmark validates
completed receipts and rejects callback lateness exceeding 1 ms. Snapshot copies
and ordinary event scheduling still have overhead; this is a diagnostic harness.

Native protocol users submit `command=xbox_sequence`, `path=<directory>` and
optional `start_ticks=<decimal ticks>`. The directory contains command files in
lexicographic order. Relative output paths resolve against that directory;
place outputs elsewhere. The Python benchmark prepares that directory itself
from the original route JSON, using absolute output paths under its fresh run
folder. Do not modify a sequence after submitting it.

`check_memory` is also supported as an ordinary command, but an ordinary failed
command does **not** cancel later queued commands. Use the sequence form when a
guard must prevent subsequent input. A sequence failure preserves completed
receipts/probes and publishes a failed top-level command receipt; the benchmark
then performs its normal process-local stop cleanup.

Use `scheduled_start_ticks` / `scheduled_end_ticks` for the intended timeline,
and `start_ticks` / `end_ticks` for observed boundaries. Analyze rendered frames
between the first input boundary and the final hold end; exclude startup and
post-sequence file output. Use a guest-time cutoff for warmup when comparing
identical game intervals; wall-time warmup selects different frames during slow
runs. Record the cutoff explicitly. Probe generation,
hero/opponent counts, positions and health are numerical evidence only. They do
not establish visible effects, correct rendering, audible sound, or continuous
combat between probes. Proprietary contracts, probes and restored states stay
under `.local`, outside Git.
