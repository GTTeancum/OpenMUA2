// Copyright 2026 OpenMUA2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <ostream>
#include <span>
#include <string>
#include <vector>

namespace AudioCommon::DiagnosticCapture
{
// Single callback producer. Allocate before stream start, read only after stop.
// Retain the tail rather than growing with play time. This captures pre-volume
// mixer output, not Windows/device playback or missing hardware callbacks.
class StereoRing
{
public:
  explicit StereoRing(std::size_t frames) : m_samples(frames * 2) {}
  void Append(std::span<const std::int16_t> samples)
  {
    if (m_samples.empty() || samples.size() % 2) return;
    const auto original = samples.size();
    if (samples.size() > m_samples.size()) samples = samples.last(m_samples.size());
    auto position = static_cast<std::size_t>((m_total + original - samples.size()) % m_samples.size());
    const auto first = std::min(samples.size(), m_samples.size() - position);
    std::copy_n(samples.data(), first, m_samples.data() + position);
    std::copy(samples.begin() + first, samples.end(), m_samples.begin());
    m_total += original;
  }
  std::uint64_t TotalFrames() const { return m_total / 2; }
  std::size_t RetainedFrames() const { return std::min<std::uint64_t>(m_total, m_samples.size()) / 2; }
  bool WriteWav(std::ostream& out, std::uint32_t rate) const
  {
    const auto count = RetainedFrames() * 2;
    if (!rate || count > (UINT32_MAX - 36) / 2) return false;
    const auto le = [&](std::uint32_t value, int bytes) {
      for (int i = 0; i < bytes; ++i) out.put(static_cast<char>((value >> (i * 8)) & 255));
    };
    out.write("RIFF", 4); le(static_cast<std::uint32_t>(36 + count * 2), 4);
    out.write("WAVEfmt ", 8); le(16, 4); le(1, 2); le(2, 2);
    le(rate, 4); le(rate * 4, 4); le(4, 2); le(16, 2);
    out.write("data", 4); le(static_cast<std::uint32_t>(count * 2), 4);
    if (count)
    {
      const auto start = static_cast<std::size_t>((m_total - count) % m_samples.size());
      const auto write = [&](std::size_t offset, std::size_t size) {
        if constexpr (std::endian::native == std::endian::little)
          out.write(reinterpret_cast<const char*>(m_samples.data() + offset), size * 2);
        else
          for (std::size_t i = 0; i < size; ++i) le(static_cast<std::uint16_t>(m_samples[offset + i]), 2);
      };
      const auto first = std::min(count, m_samples.size() - start);
      write(start, first); write(0, count - first);
    }
    return out.good();
  }
private:
  std::vector<std::int16_t> m_samples;
  std::uint64_t m_total = 0;
};

class Capture
{
public:
  void Init(std::uint32_t rate, bool stereo)
  {
    m_ring.reset(); m_path.clear();
    const char* path = std::getenv("OPENMUA2_AUDIO_CAPTURE");
    if (!path || !*path || !stereo || rate == 0 || rate > 192000) return;
    m_path = path; m_rate = rate;
    m_ring = std::make_unique<StereoRing>(static_cast<std::size_t>(rate) * 60);
  }
  void Append(const std::int16_t* samples, std::size_t frames)
  {
    if (m_ring) m_ring->Append({samples, frames * 2});
  }
  void Flush()
  {
    if (!m_ring) return;
    std::ofstream output(m_path, std::ios::binary);
    if (m_ring->WriteWav(output, m_rate))
    {
      std::ofstream info(m_path + ".txt");
      info << "scope=pre-volume stereo mixer output; not device playback\n"
           << "sample_rate=" << m_rate << "\ntotal_frames=" << m_ring->TotalFrames()
           << "\nretained_frames=" << m_ring->RetainedFrames() << '\n';
    }
    m_ring.reset();
  }
private:
  std::unique_ptr<StereoRing> m_ring;
  std::string m_path;
  std::uint32_t m_rate = 0;
};
inline Capture& Get() { static Capture* capture = new Capture; return *capture; }
}  // namespace AudioCommon::DiagnosticCapture
