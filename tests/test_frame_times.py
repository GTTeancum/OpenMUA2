import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location("frame_times", Path(__file__).parents[1]/"tools/analyze_frame_times.py")
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


def trace(times):
    return "# dropped_samples=0\nepoch,frame,present,guest_ticks,host_ns\n" + "".join(
        f"0,{i},{i*2},0,{round(t*1e9)}\n" for i,t in enumerate(times))

class FrameTimesTest(unittest.TestCase):
    def test_thirty(self):
        r = mod.analyze(trace([i/30 for i in range(301)]))
        self.assertAlmostEqual(r["average_fps"],30)
        self.assertAlmostEqual(r["frame_ms"]["p99"],1000/30,places=5)
        self.assertEqual(r["frames_over_50ms"],0)
        self.assertEqual(r["intervals"],300)
    def test_stall_is_not_hidden(self):
        r=mod.analyze(trace([0,.03,.06,.56,.59]))
        self.assertEqual(r["frames_over_50ms"],1)
        self.assertAlmostEqual(r["frame_ms"]["max"],500)
        self.assertAlmostEqual(r["longest_consecutive_over_50ms_seconds"],.5)
    def test_loss_and_reset_rejected(self):
        t=trace([0,.03,.06])
        for invalid in [t.replace("samples=0","samples=1"),
                        t.replace("0,2,4", "1,2,4"),
                        t.replace("0,2,4", "0,3,4"),
                        trace([0,.03,.02])]:
            with self.assertRaises(ValueError): mod.analyze(invalid)
    def test_frame_bounds(self):
        r=mod.analyze(trace([0,1,2,3,4]),start_frame=1,end_frame=3)
        self.assertEqual(r["intervals"],2)
        self.assertEqual(r["last_frame"],3)
    def test_warmup(self):
        r=mod.analyze(trace([0,1,2,3,4]),warmup=2)
        self.assertEqual(r["first_frame"],2)
        self.assertEqual(r["seconds"],2)

if __name__ == "__main__": unittest.main()
