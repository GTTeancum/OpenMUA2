// Copyright 2026 OpenMUA2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>

namespace AudioCommon::Performance
{
using Clock = std::chrono::steady_clock;
inline thread_local std::size_t current_channel = 0;

struct Counters
{
  // Producer counts are currently instrumented for DMA/streaming (0/1) only.
  // Zero produced_frames on another channel means unavailable, not silence.
  std::atomic<std::uint64_t> calls{}, samples{}, wall_ns{}, max_wall_ns{};
  std::atomic<std::uint64_t> subnormal_fades{}, empty_dequeues{}, produced_frames{};
  std::atomic<std::uint64_t> max_callback_gap_ns{}, mxcsr{};
};

// Bounded numeric diagnostics only: no samples, disk I/O or allocations on the
// callback after initialization. Dump only after Cubeb has stopped callbacks.
class Profile
{
public:
  Profile() : m_start(Clock::now())
  {
    if (const char* path = std::getenv("OPENMUA2_AUDIO_PROFILE"))
      m_path = path;
    if (!m_path.empty()) m_rows = std::make_unique<Rows>();
  }
  bool Enabled() const { return !m_path.empty(); }
  Counters& At(std::size_t channel)
  {
    const auto second = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - m_start).count();
    // Last bucket records overflow rather than indexing beyond the bound.
    return (*m_rows)[second < 1200 ? static_cast<std::size_t>(second) : 1200][channel];
  }
  static void Maximum(std::atomic<std::uint64_t>& target, std::uint64_t value)
  {
    auto old = target.load(std::memory_order_relaxed);
    while (old < value && !target.compare_exchange_weak(old, value, std::memory_order_relaxed)) {}
  }
  void Flush()
  {
    if (!Enabled() || m_flushed.exchange(true)) return;
    std::ofstream out(m_path);
    out << "second,bucket_start_host_ns,channel,calls,samples,wall_ns,max_wall_ns,subnormal_fade_calls,empty_dequeues,produced_frames,max_callback_gap_ns,mxcsr\n";
    const auto start_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(m_start.time_since_epoch()).count();
    for (std::size_t second = 0; second < m_rows->size(); ++second)
      for (std::size_t channel = 0; channel < 12; ++channel)
      {
        const auto& r = (*m_rows)[second][channel];
        if (!r.calls && !r.produced_frames && !r.empty_dequeues) continue;
        out << second << ',' << start_ns + second * 1000000000ULL << ','
            << channel << ',' << r.calls << ',' << r.samples << ','
            << r.wall_ns << ',' << r.max_wall_ns << ',' << r.subnormal_fades << ','
            << r.empty_dequeues << ',' << r.produced_frames << ','
            << r.max_callback_gap_ns << ',' << r.mxcsr << '\n';
      }
  }
private:
  std::string m_path;
  Clock::time_point m_start;
  using Rows = std::array<std::array<Counters, 12>, 1201>;
  std::unique_ptr<Rows> m_rows;
  std::atomic<bool> m_flushed{};
};

inline Profile& Get()
{
  // Intentionally process-lifetime storage: no static destruction race with
  // audio/core shutdown. The fixed-size allocation happens once.
  static Profile* profile = new Profile;
  return *profile;
}

class ChannelScope
{
public:
  explicit ChannelScope(std::size_t channel) : m_old(current_channel)
  {
    if (Get().Enabled()) current_channel = channel;
  }
  ~ChannelScope() { if (Get().Enabled()) current_channel = m_old; }
private:
  std::size_t m_old;
};

class MixScope
{
public:
  explicit MixScope(std::size_t samples) : m_samples(samples)
  {
    if (Get().Enabled()) { m_row = &Get().At(current_channel); m_start = Clock::now(); }
  }
  void Finish(float fade)
  {
    if (!m_row) return;
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - m_start).count();
    m_row->calls.fetch_add(1, std::memory_order_relaxed);
    m_row->samples.fetch_add(m_samples, std::memory_order_relaxed);
    m_row->wall_ns.fetch_add(ns, std::memory_order_relaxed);
    Profile::Maximum(m_row->max_wall_ns, ns);
    if (std::fpclassify(fade) == FP_SUBNORMAL)
      m_row->subnormal_fades.fetch_add(1, std::memory_order_relaxed);
  }
private:
  Counters* m_row = nullptr;
  Clock::time_point m_start;
  std::size_t m_samples;
};

inline void Produced(std::size_t channel, std::size_t frames)
{
  if (Get().Enabled()) Get().At(channel).produced_frames.fetch_add(frames, std::memory_order_relaxed);
}
inline void EmptyDequeue()
{
  if (Get().Enabled()) Get().At(current_channel).empty_dequeues.fetch_add(1, std::memory_order_relaxed);
}
inline void CallbackState(std::uint32_t mxcsr)
{
  if (!Get().Enabled()) return;
  static thread_local Clock::time_point previous{};
  const auto now = Clock::now();
  auto& row = Get().At(11);
  if (previous != Clock::time_point{})
    Profile::Maximum(row.max_callback_gap_ns,
                     std::chrono::duration_cast<std::chrono::nanoseconds>(now - previous).count());
  previous = now;
  row.mxcsr.store(mxcsr, std::memory_order_relaxed);
}
}  // namespace AudioCommon::Performance
