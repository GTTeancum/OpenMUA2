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
  return out.str() == expected ? 0 : 1;
}
