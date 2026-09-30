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

    def test_union_clips_and_merges_unsorted_overlap(self):
        self.assertEqual(mod.union_ns([(90, 120), (105, 170), (160, 210), (250, 300)], 100, 200), 100)

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
