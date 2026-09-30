"""Correlate bounded runtime wait spans with unique-frame timestamps.

Spans measure elapsed time, not CPU execution. Nested/parallel spans are merged
before reporting coverage; unclassified time is not assumed to be CPU work.
"""
import argparse
import collections
import csv
import io
import json
from pathlib import Path


KINDS = {'throttle', 'gpu_pacing', 'gpu_worker', 'gpu_fence', 'gpu_submit', 'gpu_present', 'present',
         'jit_compile', 'shader_compile', 'pipeline_compile', 'jit_analyze', 'jit_emit', 'jit_finalize', 'jit_instruction', 'jit_backpatch', 'jit_entry_map', 'jit_ranges', 'jit_links'}
WAITS = {'throttle', 'gpu_pacing', 'gpu_worker', 'gpu_fence'}


def rows(text):
    lines = text.splitlines()
    if [x for x in lines if x.startswith('# dropped_samples=')] != ['# dropped_samples=0']:
        raise ValueError('missing or nonzero dropped_samples')
    return list(csv.DictReader(io.StringIO('\n'.join(x for x in lines if not x.startswith('#')))))


def union_ns(intervals, begin, end):
    merged_end = begin
    total = 0
    for a, b in sorted(intervals):
        a, b = max(a, begin, merged_end), min(b, end)
        if b > a:
            total += b - a
            merged_end = b
    return total


def analyze(frame_text, span_text, start_frame=0, end_frame=None, ticks_per_second=729000000):
    if ticks_per_second <= 0:
        raise ValueError('ticks_per_second must be positive')
    frames = [{k: int(v) for k, v in r.items()} for r in rows(frame_text)
              if int(r['frame']) >= start_frame and
              (end_frame is None or int(r['frame']) <= end_frame)]
    if len(frames) < 2:
        raise ValueError('not enough frames')
    for a, b in zip(frames, frames[1:]):
        if a['epoch'] != b['epoch'] or b['frame'] != a['frame'] + 1 or b['host_ns'] <= a['host_ns']:
            raise ValueError('frame discontinuity or invalid timestamp')
    begin, end = frames[0]['host_ns'], frames[-1]['host_ns']
    events = []
    for r in rows(span_text):
        event = {k: (v if k == 'kind' else int(v)) for k, v in r.items()}
        if event['kind'] not in KINDS or event['end_ns'] < event['begin_ns']:
            raise ValueError('invalid span')
        if event['begin_ns'] < end and event['end_ns'] > begin:
            events.append(event)
    if not events:
        raise ValueError('no overlapping runtime spans')
    throttle_threads = {r['thread'] for r in events if r['kind'] == 'throttle'}
    if len(throttle_threads) != 1:
        raise ValueError('expected one identifiable CPU throttle thread')
    cpu_thread = throttle_threads.pop()
    grouped = collections.defaultdict(list)
    for r in events:
        grouped[r['kind']].append(r)
    totals = {kind: {
        'calls': len(group),
        'union_ms': union_ns([(r['begin_ns'], r['end_ns']) for r in group], begin, end) / 1e6,
        'max_span_ms': max(r['end_ns'] - r['begin_ns'] for r in group) / 1e6,
        'threads': sorted({r['thread'] for r in group})}
        for kind, group in grouped.items()}
    events.sort(key=lambda r: r['begin_ns'])
    cursor = 0
    active = []
    intervals = []
    for a, b in zip(frames, frames[1:]):
        low, high = a['host_ns'], b['host_ns']
        while cursor < len(events) and events[cursor]['begin_ns'] < high:
            active.append(events[cursor])
            cursor += 1
        active = [r for r in active if r['end_ns'] > low]
        covered = {kind: union_ns([(r['begin_ns'], r['end_ns']) for r in active
                                  if r['kind'] == kind and r['thread'] == cpu_thread], low, high) / 1e6
                   for kind in KINDS}
        waits = union_ns([(r['begin_ns'], r['end_ns']) for r in active
                          if r['kind'] in WAITS and r['thread'] == cpu_thread], low, high)
        intervals.append({'frame': b['frame'], 'host_ms': (high - low) / 1e6,
                          'guest_ms': (b['guest_ticks'] - a['guest_ticks']) * 1000 / ticks_per_second,
                          'cpu_thread_wait_union_ms': waits / 1e6,
                          'cpu_thread_unclassified_ms': (high - low - waits) / 1e6,
                          'cpu_thread_coverage_ms': covered})
    return {'scope': 'elapsed spans, not CPU utilization or visual correctness; coverage is merged, not summed',
            'first_frame': frames[0]['frame'], 'last_frame': frames[-1]['frame'],
            'seconds': (end - begin) / 1e9, 'cpu_thread': cpu_thread,
            'totals': totals,
            'jit_instruction_spans': [r for r in events if r['kind'] == 'jit_instruction'],
            'jit_emission_counters_scope': 'Whole spans fully inside window; CPU ns are quantized, -1 unavailable; cycles are not wall time',
            'longest_jit_emissions': sorted(
                [r for r in events if r['kind'] == 'jit_emit' and
                 r['begin_ns'] >= begin and r['end_ns'] <= end],
                key=lambda r: r['end_ns'] - r['begin_ns'], reverse=True)[:20],
            'cpu_thread_wait_seconds': sum(r['cpu_thread_wait_union_ms'] for r in intervals) / 1000,
            'cpu_thread_unclassified_seconds': sum(r['cpu_thread_unclassified_ms'] for r in intervals) / 1000,
            'worst_frames': sorted(intervals, key=lambda r: r['host_ms'], reverse=True)[:20]}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('frames', type=Path)
    p.add_argument('spans', type=Path)
    p.add_argument('--start-frame', type=int, default=0)
    p.add_argument('--end-frame', type=int)
    p.add_argument('--output', type=Path)
    args = p.parse_args()
    result = analyze(args.frames.read_text(), args.spans.read_text(), args.start_frame, args.end_frame)
    text = json.dumps(result, indent=2) + '\n'
    if args.output: args.output.write_text(text)
    else: print(text, end='')


if __name__ == '__main__':
    main()
