"""Analyze opt-in OpenMUA2 frame timestamps; never infers visual correctness."""
import argparse
import bisect
import csv
import io
import json
import math
from pathlib import Path
import statistics


def analyze(text, start_frame=0, warmup=0.0, end_frame=None):
    lines = text.splitlines()
    dropped = [int(x.split("=", 1)[1]) for x in lines
               if x.startswith("# dropped_samples=")]
    if dropped != [0]:
        raise ValueError("missing or nonzero dropped_samples metadata")
    rows = [{k: int(v) for k, v in row.items()} for row in
            csv.DictReader(io.StringIO("\n".join(x for x in lines if not x.startswith("#"))))]
    rows = [r for r in rows if r["frame"] >= start_frame and
            (end_frame is None or r["frame"] <= end_frame)]
    if len(rows) < 2:
        raise ValueError("not enough frames")
    # Caller must choose a single contiguous loaded-state segment explicitly.
    # Never silently discard resets, skipped samples, or long frame intervals.
    for a, b in zip(rows, rows[1:]):
        if b["epoch"] != a["epoch"] or b["frame"] != a["frame"] + 1:
            raise ValueError("frame discontinuity; select a contiguous run with --start-frame")
        if b["host_ns"] <= a["host_ns"]:
            raise ValueError("non-increasing host timestamp")
    cutoff = rows[0]["host_ns"] + warmup * 1e9
    rows = [r for r in rows if r["host_ns"] >= cutoff]
    if len(rows) < 2:
        raise ValueError("warmup excludes measured frames")
    times = [(r["host_ns"] - rows[0]["host_ns"]) / 1e9 for r in rows]
    dt = [b-a for a, b in zip(times, times[1:])]
    ordered = sorted(dt)
    def percentile(p):
        return ordered[max(0, math.ceil(len(ordered)*p)-1)] * 1000
    worst = ordered[-max(1, math.ceil(len(ordered)*.01)):]
    windows = []
    for n in range(max(0, math.floor((times[-1]-1)*10)+1)):
        end = 1+n*.1
        windows.append(bisect.bisect_right(times, end) - bisect.bisect_right(times, end-1))
    longest = current = 0.0
    for delta in dt:
        current = current + delta if delta > .05 else 0.0
        longest = max(longest, current)
    return {"scope": "new-frame callback timing, not display scanout or correctness proof",
            "first_frame": rows[0]["frame"], "last_frame": rows[-1]["frame"],
            "emulated_ticks": rows[-1]["guest_ticks"]-rows[0]["guest_ticks"],
            "intervals": len(dt), "seconds": times[-1], "average_fps": len(dt)/times[-1],
            "frame_ms": {"median": statistics.median(dt)*1000,
                         "p95": percentile(.95), "p99": percentile(.99), "max": max(dt)*1000},
            "one_percent_low_fps": 1/statistics.mean(worst),
            "rolling_1s_fps_min": min(windows) if windows else None,
            "rolling_1s_fps_median": statistics.median(windows) if windows else None,
            "frames_over_33_334ms": sum(x > .033334 for x in dt),
            "frames_over_50ms": sum(x > .05 for x in dt),
            "longest_consecutive_over_50ms_seconds": longest,
            "warmup_seconds": warmup, "dropped_samples": 0}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--start-frame", type=int, default=0)
    parser.add_argument("--warmup", type=float, default=0)
    parser.add_argument("--end-frame", type=int)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--guest-ticks-per-second", type=int)
    args = parser.parse_args()
    if args.warmup < 0:
        parser.error("warmup must be nonnegative")
    result = analyze(args.trace.read_text(), args.start_frame, args.warmup, args.end_frame)
    if args.guest_ticks_per_second is not None:
        if args.guest_ticks_per_second <= 0:
            parser.error("guest ticks per second must be positive")
        result["guest_ticks_per_second"] = args.guest_ticks_per_second
        result["emulated_seconds"] = result["emulated_ticks"] / args.guest_ticks_per_second
        result["emulation_speed_percent"] = 100 * result["emulated_seconds"] / result["seconds"]
    output = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.write_text(output)
    print(output, end="")

if __name__ == "__main__":
    main()
