#include "VideoCommon/PresentationQueueClock.h"

#include <deque>
#include <vector>

int main()
{
  VideoCommon::PresentationQueueClock clock;
  // Real Wii clock values must be multiplied at 64-bit width.
  if (clock.FieldPeriod(729000000, 1458000000, 24324300) != 12162150 ||
      clock.FieldPeriod(729000000, 0, 24324300) != 0)
    return 6;
  std::deque<int> pending;
  std::vector<int> shown;
  std::vector<int> output_ticks;
  // Source cadence includes compensating 3/1-field gaps around nominal 2 fields.
  const std::vector<int> arrivals{0, 2, 5, 6, 8, 10, 13, 14, 16, 18};
  std::size_t next = 0;
  for (int tick = 0; tick <= 20; ++tick)
  {
    if (next < arrivals.size() && tick == arrivals[next])
      pending.push_back(static_cast<int>(next++));
    if (clock.Due(tick, 1, pending.size()))
    {
      shown.push_back(pending.front());
      pending.pop_front();
      output_ticks.push_back(tick);
    }
  }
  if (shown.size() != arrivals.size() || !pending.empty() || clock.Underflows() != 0)
    return 1;
  for (std::size_t i = 0; i < shown.size(); ++i)
    if (shown[i] != static_cast<int>(i) || output_ticks[i] != 2 + 2 * static_cast<int>(i))
      return 2;
  // A producer stall must be observable, with no invented output frame.
  if (clock.Due(22, 1, 0) || clock.Underflows() != 1 || clock.Due(23, 1, 1))
    return 3;
  if (!clock.Due(24, 1, 2) || clock.Due(25, 1, 1) || !clock.Due(26, 1, 1))
    return 4;
  // A state load can rewind guest time; the display queue must be explicitly reset.
  clock.Reset();
  if (clock.Due(0, 0, 2) || clock.Due(0, 1, 1) || !clock.Due(1, 1, 2))
    return 5;
  return 0;
}
