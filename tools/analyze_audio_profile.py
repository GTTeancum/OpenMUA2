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
    state_columns = ('empty_running', 'empty_not_running')
    has_state = all(k in reader.fieldnames for k in state_columns)
    if any(k in reader.fieldnames for k in state_columns) and not has_state:
        raise ValueError('incomplete core-state counters')
    if has_state:
        required += state_columns
    trim_columns = ('queue_trim_events', 'queue_trimmed_granules',
                    'queue_trim_running', 'queue_trim_not_running')
    has_trim = all(k in reader.fieldnames for k in trim_columns)
    if any(k in reader.fieldnames for k in trim_columns) and not has_trim:
        raise ValueError('incomplete queue-trim counters')
    if has_trim:
        required += trim_columns
    rows = {}
    origin = None
    for source in reader:
        try:
            r = {k: int(source[k]) for k in required}
        except (ValueError, TypeError):
            raise ValueError('invalid audio profile counter') from None
        if any(v < 0 for v in r.values()) or r['channel'] >= 12 or r['second'] > 1200:
            raise ValueError('audio profile counter outside bounds')
        if has_state and r['empty_running'] + r['empty_not_running'] != r['empty_dequeues']:
            raise ValueError('core-state counters do not sum to empty reads')
        if has_trim:
            if r['queue_trim_running'] + r['queue_trim_not_running'] != r['queue_trim_events']:
                raise ValueError('queue-trim state counters do not sum to events')
            if r['queue_trimmed_granules'] < r['queue_trim_events'] or (
                    r['queue_trim_events'] == 0 and r['queue_trimmed_granules'] != 0):
                raise ValueError('inconsistent queue-trim granules')
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
        if has_state:
            channels[name]['empty_while_core_running'] = sum(r['empty_running'] for r in selected)
            channels[name]['empty_while_core_not_running'] = sum(r['empty_not_running'] for r in selected)
        if has_trim:
            channels[name]['queue_trims'] = {
                'events': sum(r['queue_trim_events'] for r in selected),
                'discarded_granules': sum(r['queue_trimmed_granules'] for r in selected),
                'events_running': sum(r['queue_trim_running'] for r in selected),
                'events_not_running': sum(r['queue_trim_not_running'] for r in selected),
                'nonzero_buckets': [{k: r[k] for k in ('second',) + trim_columns}
                                    for r in selected if r['queue_trim_events']],
            }
    prefix = 0
    # The final bucket may be partial; the overflow bucket spans arbitrary time.
    while prefix < min(last, 1200) and all(
            rows[prefix, c]['empty_dequeues'] == 0 for c in (0, 1)):
        prefix += 1
    return {
        'schema': 1,
        'scope': 'Mixer empty reads and queue trims; not audible dropouts, combat boundaries or device playback.',
        'covered_buckets': last + 1,
        'initial_complete_buckets_without_main_empty_reads': prefix,
        'final_bucket_second': last,
        'overflow_bucket_present': last == 1200,
        'core_state_counters_available': has_state,
        'queue_trim_counters_available': has_trim,
        'channels': channels,
        'max_callback_gap_ms': max(rows[s, 11]['max_callback_gap_ns'] for s in range(last + 1)) / 1e6,
        'limitations': [
            'Final bucket is retained, not discarded or classified as shutdown.',
            'One empty dequeue is a granule read, not one audible dropout.',
            'Queue trims count discarded granules; they do not prove an audible defect.',
            'Zero empty reads do not establish clean audio, speed, visuals or combat coverage.',
            'Bucket 1200, if present, combines all later observations.',
            'Core Running includes in-game menus; not-running can include host pause or shutdown.',
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
