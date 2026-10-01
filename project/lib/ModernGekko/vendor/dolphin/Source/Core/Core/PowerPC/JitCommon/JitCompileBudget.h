// Copyright 2026 OpenMUA2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <bitset>
#include <charconv>
#include <cstdint>
#include <limits>
#include <string_view>

// Host compilation work allowance. Guest clocks are only observed, never modified.
class JitCompileBudget
{
public:
  static std::uint64_t ParseNanoseconds(std::string_view value)
  {
    unsigned us = 0;
    const auto result = std::from_chars(value.data(), value.data() + value.size(), us);
    if (result.ec != std::errc{} || result.ptr != value.data() + value.size() ||
        us < 250 || us > 16000)
      return 0;
    return std::uint64_t{us} * 1000;
  }
  void Reset()
  {
    m_window = std::numeric_limits<std::uint64_t>::max();
    m_spent = 0;
  }
  void Observe(std::uint64_t ticks, std::uint32_t ticks_per_second)
  {
    const auto quantum = ticks_per_second / 30;
    const auto window = quantum ? ticks / quantum : 0;
    if (window != m_window)
    {
      m_window = window;
      m_spent = 0;
    }
  }
  bool Exhausted(std::uint64_t limit) const { return limit != 0 && m_spent >= limit; }
  void Charge(std::uint64_t elapsed)
  {
    const auto room = std::numeric_limits<std::uint64_t>::max() - m_spent;
    m_spent += elapsed < room ? elapsed : room;
  }
private:
  std::uint64_t m_window = std::numeric_limits<std::uint64_t>::max();
  std::uint64_t m_spent = 0;
};


// Bounded hotness hints only: a hit requests ordinary compilation of CURRENT
// guest code. No guest instructions, translations or execution results are cached.
class JitColdCodeVisits
{
public:
  void Clear() { m_valid.reset(); }
  bool Revisited(std::uint32_t pc, std::uint32_t flags)
  {
    const auto index = ((pc >> 2) ^ (pc >> 12)) & 4095;
    const auto key = (std::uint64_t{flags} << 32) | pc;
    const bool hit = m_valid[index] && m_keys[index] == key;
    m_keys[index] = key;
    m_valid.set(index);
    return hit;
  }
private:
  std::array<std::uint64_t, 4096> m_keys{};
  std::bitset<4096> m_valid;
};
