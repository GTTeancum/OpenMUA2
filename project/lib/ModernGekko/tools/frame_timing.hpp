#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <vector>
#include "frame_cpu_counters.hpp"

namespace moderngekko::telemetry {
// Owned by the video thread while running; read only after Core::Shutdown joins it.
// No file I/O, locks, or allocation in Record. Frame IDs may rewind on state load.
class FrameTiming {
public:
  explicit FrameTiming(std::size_t capacity = 120000) : m_capacity(capacity) {
    m_samples.reserve(capacity);
  }
  void Record(std::uint64_t frame, std::uint64_t present,
              std::uint64_t guest_ticks, std::int64_t host_ns, bool duplicate,
              FrameCpuCounters counters = {}) {
    if (duplicate)
      return;
    if (m_started && frame <= m_previous_frame)
      ++m_epoch;
    m_started = true;
    m_previous_frame = frame;
    if (m_samples.size() == m_capacity) {
      ++m_dropped;
      return;
    }
    m_samples.push_back({m_epoch, frame, present, guest_ticks, host_ns, counters});
  }
  void Write(std::ostream& out) const {
    out << "# host_ns=steady_clock after_present callback; not display scanout\n"
        << "# dropped_samples=" << m_dropped << '\n'
        << "epoch,frame,present,guest_ticks,host_ns,thread_cpu_ns,process_cpu_ns,thread_cycles,thread_id\n";
    for (const auto& s : m_samples)
      out << s.epoch << ',' << s.frame << ',' << s.present << ','
          << s.guest_ticks << ',' << s.host_ns << ',' << s.counters.thread_ns << ','
          << s.counters.process_ns << ',' << s.counters.thread_cycles << ','
          << s.counters.thread_id << '\n';
  }
private:
  struct Sample {
    std::uint64_t epoch, frame, present, guest_ticks;
    std::int64_t host_ns;
    FrameCpuCounters counters;
  };
  std::vector<Sample> m_samples;
  std::size_t m_capacity;
  std::uint64_t m_epoch = 0, m_previous_frame = 0, m_dropped = 0;
  bool m_started = false;
};

// Diagnostic event order on the video thread. XFB copy events are not FPS:
// a game can copy more than once per frame. Zero timing fields mean unavailable.
class PresentationTiming {
public:
  explicit PresentationTiming(std::size_t capacity = 240000) : m_capacity(capacity) {
    m_samples.reserve(capacity);
  }
  void Record(const char* phase, std::uint64_t frame, std::uint64_t present,
              std::uint64_t guest_ticks, std::int64_t host_ns,
              std::int64_t intended_ns, std::int64_t actual_ns,
              int reason, int accuracy) {
    if (m_samples.size() == m_capacity) {
      ++m_dropped;
      return;
    }
    // phase must be a string literal with static lifetime.
    m_samples.push_back({phase, frame, present, guest_ticks, host_ns,
                         intended_ns, actual_ns, reason, accuracy});
  }
  void Write(std::ostream& out) const {
    out << "# diagnostic video events; copy count is not FPS; not display scanout\n"
        << "# dropped_samples=" << m_dropped << '\n'
        << "phase,frame,present,guest_ticks,host_ns,intended_ns,actual_ns,reason,accuracy\n";
    for (const auto& s : m_samples)
      out << s.phase << ',' << s.frame << ',' << s.present << ',' << s.guest_ticks
          << ',' << s.host_ns << ',' << s.intended_ns << ',' << s.actual_ns
          << ',' << s.reason << ',' << s.accuracy << '\n';
  }
private:
  struct Sample {
    const char* phase;
    std::uint64_t frame, present, guest_ticks;
    std::int64_t host_ns, intended_ns, actual_ns;
    int reason, accuracy;
  };
  std::vector<Sample> m_samples;
  std::size_t m_capacity;
  std::uint64_t m_dropped = 0;
};
} // namespace moderngekko::telemetry
