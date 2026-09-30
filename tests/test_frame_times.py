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
    def test_guest_cadence_is_separate_from_host_stall(self):
        text = ("# dropped_samples=0\nepoch,frame,present,guest_ticks,host_ns\n"
                "0,0,0,0,0\n0,1,2,33,33000000\n"
                "0,2,5,83,133000000\n0,3,6,100,150000000\n")
        result = mod.analyze(text, guest_ticks_per_second=1000)
        self.assertEqual(result["guest_frame_ms"]["max"], 50)
        self.assertEqual(result["guest_interval_counts_ms"],
                         {"33.000": 1, "50.000": 1, "17.000": 1})
        self.assertAlmostEqual(result["host_minus_guest_interval_ms"]["max"], 50)
        self.assertAlmostEqual(result["emulation_speed_percent"], 100*.1/.15)
    def test_invalid_guest_clock_rejected_when_analyzing_cadence(self):
        with self.assertRaisesRegex(ValueError, 'non-increasing guest'):
            mod.analyze(trace([0,.03,.06]), guest_ticks_per_second=1000)

if __name__ == "__main__": unittest.main()
