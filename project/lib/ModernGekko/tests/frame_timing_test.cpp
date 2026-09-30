#include "frame_timing.hpp"
#include "../vendor/dolphin/Source/Core/Common/RuntimeTiming.h"
#include <sstream>
#include <array>
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
  if (phase_out.str() != phase_expected) return 2;
  Common::RuntimeTiming::Trace spans(5);
  std::array<std::thread, 4> workers;
  for (int i = 0; i < 4; ++i)
    workers[i] = std::thread([&, i] {
      for (int j = 0; j < 100; ++j)
        spans.Record(Common::RuntimeTiming::Kind::GpuFence, 10, 20, 0, i + 1);
    });
  for (auto& thread : workers) thread.join();
  std::ostringstream span_out;
  spans.Write(span_out);
  const auto text = span_out.str();
  if (text.find("# dropped_samples=395\n") == std::string::npos) return 3;
  std::size_t at = 0;
  int count = 0;
  while ((at = text.find("gpu_fence,10,20,0,", at)) != std::string::npos) {
    ++count;
    ++at;
  }
  return count == 5 ? 0 : 4;
}
