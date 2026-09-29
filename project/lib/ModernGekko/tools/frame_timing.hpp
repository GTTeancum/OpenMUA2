#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <vector>

namespace moderngekko::telemetry {
// Owned by the video thread while running; read only after Core::Shutdown joins it.
// No file I/O, locks, or allocation in Record. Frame IDs may rewind on state load.
class FrameTiming {
public:
  explicit FrameTiming(std::size_t capacity = 120000) : m_capacity(capacity) {
    m_samples.reserve(capacity);
  }
  void Record(std::uint64_t frame, std::uint64_t present,
              std::uint64_t guest_ticks, std::int64_t host_ns, bool duplicate) {
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
    m_samples.push_back({m_epoch, frame, present, guest_ticks, host_ns});
  }
  void Write(std::ostream& out) const {
    out << "# host_ns=steady_clock after_present callback; not display scanout\n"
        << "# dropped_samples=" << m_dropped << '\n'
        << "epoch,frame,present,guest_ticks,host_ns\n";
    for (const auto& s : m_samples)
      out << s.epoch << ',' << s.frame << ',' << s.present << ','
          << s.guest_ticks << ',' << s.host_ns << '\n';
  }
private:
  struct Sample {
    std::uint64_t epoch, frame, present, guest_ticks;
    std::int64_t host_ns;
  };
  std::vector<Sample> m_samples;
  std::size_t m_capacity;
  std::uint64_t m_epoch = 0, m_previous_frame = 0, m_dropped = 0;
  bool m_started = false;
};
} // namespace moderngekko::telemetry
