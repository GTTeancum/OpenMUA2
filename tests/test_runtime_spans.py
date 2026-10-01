import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('runtime_spans', Path(__file__).parents[1]/'tools/analyze_runtime_spans.py')
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)

FRAMES = '# dropped_samples=0\nepoch,frame,present,guest_ticks,host_ns\n0,1,1,0,100\n0,2,2,729,200\n0,3,3,1458,300\n'
SPANS = '# dropped_samples=0\nkind,begin_ns,end_ns,target_ns,thread\nthrottle,110,150,140,1\ngpu_worker,140,180,0,1\ngpu_fence,160,210,0,1\ngpu_submit,120,260,0,2\n'


class RuntimeSpansTest(unittest.TestCase):
    def test_nested_spans_do_not_double_count_or_include_other_thread(self):
        result = mod.analyze(FRAMES, SPANS)
        self.assertAlmostEqual(result['cpu_thread_wait_seconds'], 100e-9)
        self.assertAlmostEqual(result['cpu_thread_unclassified_seconds'], 100e-9)
        self.assertEqual(result['totals']['gpu_submit']['threads'], [2])

    def test_cpu_counters_preserved_without_clipping_or_time_conversion(self):
        spans = SPANS.replace('target_ns,thread', 'target_ns,thread,cpu_ns,cycles,address,instructions')
        spans = spans.replace(',1\n', ',1,-1,-1,0,0\n').replace(',2\n', ',2,-1,-1,0,0\n')
        spans += 'jit_emit,120,240,0,1,200,420,4096,7\n'
        spans += 'jit_emit,90,250,0,1,250,450,8192,8\n'
        measured = mod.analyze(FRAMES, spans)['longest_jit_emissions']
        self.assertEqual(len(measured), 1)
        self.assertEqual((measured[0]['cpu_ns'], measured[0]['cycles'], measured[0]['instructions']), (200, 420, 7))

    def test_union_clips_and_merges_unsorted_overlap(self):
        self.assertEqual(mod.union_ns([(90, 120), (105, 170), (160, 210), (250, 300)], 100, 200), 100)

    def test_compile_spans_are_not_counted_as_waits(self):
        result = mod.analyze(FRAMES, SPANS + 'jit_compile,120,240,0,1\nshader_compile,150,250,0,2\n')
        self.assertAlmostEqual(result['cpu_thread_wait_seconds'], 100e-9)
        self.assertAlmostEqual(result['totals']['jit_compile']['union_ms'], 120e-6)
        phases = ''.join(f'{kind},120,240,0,1\n' for kind in ('jit_entry_map', 'jit_ranges', 'jit_links'))
        detailed = mod.analyze(FRAMES, SPANS + phases)
        self.assertAlmostEqual(detailed['cpu_thread_wait_seconds'], 100e-9)
        for kind in ('jit_entry_map', 'jit_ranges', 'jit_links'):
            self.assertAlmostEqual(detailed['totals'][kind]['union_ms'], 120e-6)

    def test_decode_work_is_not_wait_time_and_nested_coverage_is_clipped(self):
        result = mod.analyze(FRAMES, SPANS + 'gpu_decode_slow,120,240,0,1\n')
        self.assertAlmostEqual(result['cpu_thread_wait_seconds'], 100e-9)
        self.assertAlmostEqual(result['totals']['gpu_decode_slow']['union_ms'], 120e-6)
        by_frame = {row['frame']: row for row in result['worst_frames']}
        self.assertAlmostEqual(by_frame[2]['cpu_thread_coverage_ms']['gpu_decode_slow'], 80e-6)
        self.assertAlmostEqual(by_frame[3]['cpu_thread_coverage_ms']['gpu_decode_slow'], 40e-6)
        self.assertIn('lower bound', result['gpu_decode_scope'])

    def test_missing_or_dropped_data_and_discontinuities_rejected(self):
        for f, s in ((FRAMES.replace('dropped_samples=0', 'dropped_samples=1'), SPANS),
                     (FRAMES, SPANS.replace('dropped_samples=0', 'dropped_samples=1')),
                     (FRAMES.replace('0,2,2', '1,2,2'), SPANS),
                     (FRAMES.replace('0,2,2', '0,4,2'), SPANS),
                     (FRAMES, SPANS.replace('110,150', '150,110')),
                     (FRAMES, SPANS.replace('throttle', 'unknown'))):
            with self.subTest(f=f, s=s), self.assertRaises(ValueError):
                mod.analyze(f, s)

    def test_ambiguous_cpu_thread_rejected(self):
        with self.assertRaises(ValueError):
            mod.analyze(FRAMES, SPANS + 'throttle,250,280,0,2\n')


if __name__ == '__main__':
    unittest.main()
