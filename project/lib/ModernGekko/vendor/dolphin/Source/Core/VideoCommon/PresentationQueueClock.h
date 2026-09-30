// Copyright 2026 OpenMUA2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <cstddef>
#include <cstdint>

namespace VideoCommon
{
// Experimental half-refresh presentation clock. It consumes existing frames;
// it never advances emulation, duplicates a frame, or tells a producer to skip.
class PresentationQueueClock
{
public:
  static std::uint64_t FieldPeriod(std::uint32_t ticks_per_second,
                                   std::uint32_t refresh_numerator,
                                   std::uint32_t refresh_denominator)
  {
    return refresh_numerator == 0 ? 0 :
        std::uint64_t{ticks_per_second} * refresh_denominator / refresh_numerator;
  }

  bool Due(std::uint64_t tick, std::uint64_t field_period, std::size_t available)
  {
    if (!m_started)
    {
      if (available < 2 || field_period == 0)
        return false;
      m_started = true;
      m_next_tick = tick;
    }
    if (tick < m_next_tick)
      return false;
    if (available == 0)
    {
      ++m_underflows;
      m_started = false;
      return false;
    }
    m_next_tick = tick + 2 * field_period;
    return true;
  }

  void Reset() { m_started = false; }
  std::uint64_t Underflows() const { return m_underflows; }

private:
  bool m_started = false;
  std::uint64_t m_next_tick = 0;
  std::uint64_t m_underflows = 0;
};
}  // namespace VideoCommon
