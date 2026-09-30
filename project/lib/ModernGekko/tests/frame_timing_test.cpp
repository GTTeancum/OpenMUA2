#include "frame_timing.hpp"
#include <sstream>
#include <string>

int main() {
  moderngekko::telemetry::FrameTiming trace(3);
  trace.Record(100, 200, 10, 1000, false);
  trace.Record(100, 201, 11, 1010, true);
  trace.Record(101, 202, 12, 1020, false);
  trace.Record(50, 203, 13, 1030, false); // restored state
  trace.Record(51, 204, 14, 1040, false); // bounded buffer reports loss
  std::ostringstream out;
  trace.Write(out);
  const std::string expected =
      "# host_ns=steady_clock after_present callback; not display scanout\n"
      "# dropped_samples=1\n"
      "epoch,frame,present,guest_ticks,host_ns\n"
      "0,100,200,10,1000\n0,101,202,12,1020\n1,50,203,13,1030\n";
  if (out.str() != expected)
    return 1;
  moderngekko::telemetry::PresentationTiming phases(3);
  phases.Record("copy", 0, 0, 0, 1000, 0, 0, -1, -1);
  phases.Record("before", 101, 202, 12, 1020, 1040, 0, 1, 3);
  phases.Record("after", 101, 202, 12, 1050, 1040, 1045, 1, 2);
  phases.Record("copy", 0, 0, 0, 1060, 0, 0, -1, -1);
  std::ostringstream phase_out;
  phases.Write(phase_out);
  const std::string phase_expected =
      "# diagnostic video events; copy count is not FPS; not display scanout\n"
      "# dropped_samples=1\n"
      "phase,frame,present,guest_ticks,host_ns,intended_ns,actual_ns,reason,accuracy\n"
      "copy,0,0,0,1000,0,0,-1,-1\n"
      "before,101,202,12,1020,1040,0,1,3\n"
      "after,101,202,12,1050,1040,1045,1,2\n";
  return phase_out.str() == phase_expected ? 0 : 2;
}
