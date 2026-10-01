import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location(
    'audio_profile', Path(__file__).resolve().parents[1] / 'tools/analyze_audio_profile.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


def profile(events=None, omit=None):
    events = events or {}
    lines = ['second,bucket_start_host_ns,channel,calls,samples,empty_dequeues,produced_frames,max_callback_gap_ns']
    for second in range(4):
        for channel in (0, 1, 11):
            if (second, channel) == omit:
                continue
            lines.append(f'{second},{1000 + second * 1000000000},{channel},10,4800,'
                         f'{events.get((second, channel), 0)},3200,10000000')
    return '\n'.join(lines) + '\n'


class AudioProfileAnalysisTests(unittest.TestCase):
    def test_legacy_profile_does_not_invent_core_state(self):
        result = module.analyze(profile())
        self.assertFalse(result['core_state_counters_available'])
        self.assertNotIn('empty_while_core_running', result['channels']['dma'])

    def test_state_split_preserves_all_empty_reads(self):
        text = profile({(2, 0): 1, (3, 0): 61}).splitlines()
        text[0] += ',empty_running,empty_not_running'
        for i in range(1, len(text)):
            second, _, channel, *_ = text[i].split(',')
            text[i] += ',1,0' if (second, channel) == ('2', '0') else (
                ',0,61' if (second, channel) == ('3', '0') else ',0,0')
        result = module.analyze('\n'.join(text))
        self.assertTrue(result['core_state_counters_available'])
        self.assertEqual(result['channels']['dma']['empty_while_core_running'], 1)
        self.assertEqual(result['channels']['dma']['empty_while_core_not_running'], 61)
        with self.assertRaises(ValueError):
            module.analyze('\n'.join(text).replace(',0,61', ',0,60'))

    def test_queue_trims_are_optional_and_retain_state_and_granules(self):
        old = module.analyze(profile())
        self.assertFalse(old['queue_trim_counters_available'])
        self.assertNotIn('queue_trims', old['channels']['dma'])
        lines = profile().splitlines()
        lines[0] += ',queue_trim_events,queue_trimmed_granules,queue_trim_running,queue_trim_not_running'
        for i in range(1, len(lines)):
            second, _, channel, *_ = lines[i].split(',')
            lines[i] += ',2,17,1,1' if (second, channel) == ('2', '0') else ',0,0,0,0'
        text = '\n'.join(lines)
        result = module.analyze(text)
        trims = result['channels']['dma']['queue_trims']
        self.assertEqual((trims['events'], trims['discarded_granules'],
                          trims['events_running'], trims['events_not_running']), (2, 17, 1, 1))
        self.assertEqual(trims['nonzero_buckets'][0]['second'], 2)
        self.assertEqual(result['channels']['music']['queue_trims']['events'], 0)
        for invalid in (',2,17,1,0', ',2,1,1,1', ',0,17,0,0', ',2,-1,1,1'):
            with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                module.analyze(text.replace(',2,17,1,1', invalid))

    def test_partial_queue_trim_schema_rejected(self):
        text = profile().replace('max_callback_gap_ns', 'max_callback_gap_ns,queue_trim_events')
        with self.assertRaises(ValueError):
            module.analyze(text)

    def test_partial_state_schema_rejected(self):
        text = profile().replace('max_callback_gap_ns', 'max_callback_gap_ns,empty_running')
        with self.assertRaises(ValueError):
            module.analyze(text)

    def test_retains_final_burst_separately(self):
        result = module.analyze(profile({(2, 0): 1, (3, 0): 61, (3, 1): 92}))
        self.assertEqual(result['initial_complete_buckets_without_main_empty_reads'], 2)
        self.assertEqual(result['channels']['dma']['total_empty_dequeues'], 62)
        self.assertEqual(result['channels']['dma']['before_final_bucket_empty_dequeues'], 1)
        self.assertEqual(result['channels']['music']['final_bucket_empty_dequeues'], 92)
        self.assertEqual(result['max_callback_gap_ms'], 10)

    def test_clean_prefix_excludes_potentially_partial_last_bucket(self):
        self.assertEqual(module.analyze(profile())['initial_complete_buckets_without_main_empty_reads'], 3)

    def test_missing_coverage_is_not_zero(self):
        for missing in ((1, 0), (2, 1), (3, 11)):
            with self.subTest(missing=missing), self.assertRaises(ValueError):
                module.analyze(profile(omit=missing))

    def test_duplicate_rejected(self):
        text = profile()
        with self.assertRaises(ValueError):
            module.analyze(text + text.splitlines()[1] + '\n')

    def test_mixed_clock_origins_rejected(self):
        with self.assertRaises(ValueError):
            module.analyze(profile().replace('1000001000', '1000001001', 1))

    def test_bad_counters_rejected(self):
        for value in ('-1', 'nan', ''):
            with self.subTest(value=value), self.assertRaises(ValueError):
                module.analyze(profile().replace(',10,4800,', f',{value},4800,', 1))

    def test_empty_or_wrong_schema_rejected(self):
        for text in ('', 'hello\n1\n', profile().splitlines()[0]):
            with self.subTest(text=text), self.assertRaises(ValueError):
                module.analyze(text)


if __name__ == '__main__':
    unittest.main()
