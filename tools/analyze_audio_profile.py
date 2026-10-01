"""Summarize mixer counters without equating empty reads with audible dropouts."""
import argparse
import csv
import io
import json
from pathlib import Path


def analyze(text):
    required = ('second', 'bucket_start_host_ns', 'channel', 'calls', 'samples',
                'empty_dequeues', 'produced_frames', 'max_callback_gap_ns')
    reader = csv.DictReader(io.StringIO(text))
    if not reader.fieldnames or not set(required) <= set(reader.fieldnames):
        raise ValueError('missing audio profile columns')
    rows = {}
    origin = None
    for source in reader:
        try:
            r = {k: int(source[k]) for k in required}
        except (ValueError, TypeError):
            raise ValueError('invalid audio profile counter') from None
        if any(v < 0 for v in r.values()) or r['channel'] >= 12 or r['second'] > 1200:
            raise ValueError('audio profile counter outside bounds')
        start = r['bucket_start_host_ns'] - r['second'] * 1_000_000_000
        if origin is not None and start != origin:
            raise ValueError('inconsistent audio profile clock origin')
        origin = start
        key = (r['second'], r['channel'])
        if key in rows:
            raise ValueError('duplicate audio profile bucket/channel')
        rows[key] = r
    if not rows:
        raise ValueError('empty audio profile')
    last = max(second for second, _ in rows)
    # Missing rows mean unknown coverage, never an inferred zero count.
    for second in range(last + 1):
        for channel in (0, 1, 11):
            if (second, channel) not in rows or not rows[second, channel]['calls']:
                raise ValueError('missing main mixer/callback coverage')
    channels = {}
    for channel, name in ((0, 'dma'), (1, 'music')):
        selected = [rows[s, channel] for s in range(last + 1)]
        channels[name] = {
            'total_empty_dequeues': sum(r['empty_dequeues'] for r in selected),
            'before_final_bucket_empty_dequeues': sum(r['empty_dequeues'] for r in selected[:-1]),
            'final_bucket_empty_dequeues': selected[-1]['empty_dequeues'],
            'nonzero_buckets': [{'second': r['second'], 'empty_dequeues': r['empty_dequeues']}
                                for r in selected if r['empty_dequeues']],
        }
    prefix = 0
    # The final bucket may be partial; the overflow bucket spans arbitrary time.
    while prefix < min(last, 1200) and all(
            rows[prefix, c]['empty_dequeues'] == 0 for c in (0, 1)):
        prefix += 1
    return {
        'schema': 1,
        'scope': 'Mixer empty-read counters; not audible dropouts, combat boundaries or device playback.',
        'covered_buckets': last + 1,
        'initial_complete_buckets_without_main_empty_reads': prefix,
        'final_bucket_second': last,
        'overflow_bucket_present': last == 1200,
        'channels': channels,
        'max_callback_gap_ms': max(rows[s, 11]['max_callback_gap_ns'] for s in range(last + 1)) / 1e6,
        'limitations': [
            'Final bucket is retained, not discarded or classified as shutdown.',
            'One empty dequeue is a granule read, not one audible dropout.',
            'Zero empty reads do not establish clean audio, speed, visuals or combat coverage.',
            'Bucket 1200, if present, combines all later observations.',
        ],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('profile', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    result = json.dumps(analyze(args.profile.read_text()), indent=2) + '\n'
    if args.output:
        args.output.write_text(result)
    else:
        print(result, end='')


if __name__ == '__main__':
    main()
