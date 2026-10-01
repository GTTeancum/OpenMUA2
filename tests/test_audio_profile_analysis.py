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
