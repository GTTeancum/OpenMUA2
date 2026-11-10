// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Guest-RAM footprint census for the original-Xbox feasibility gate (P0.1 in
// docs/ORIGINAL-XBOX-PORT-PLAN.md). It answers: how many 4 KiB pages of MEM1 and
// MEM2 does the game actually use, as an upper bound and as a working set?
//
// Per page, per sample:
//   * nonzero: the page currently contains a non-zero byte.
//   * ever_nonzero: the page was non-zero in any sample. Upper bound on the
//     footprint, since Dolphin zero-fills RAM at boot.
//   * changed: the page's content hash differs from the previous sample. This is
//     the working set over one sampling interval.
//   * ever_changed: the page differed from the previous sample at least once
//     after the first sample. This is the mutable footprint.
// Read-only pages that never change after load are code, constant data and
// resident assets. They count toward ever_nonzero, but not toward ever_changed.
//
// Sampling reads guest RAM while the emulated CPU may be writing it. The numbers
// are statistical, not a coherent snapshot. No allocation or I/O happens in
// Sample() after the first call.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ostream>
#include <span>
#include <string_view>
#include <vector>

namespace moderngekko::telemetry {

class MemoryCensus {
public:
  static constexpr std::size_t kPageSize = 4096;
  static constexpr std::size_t kRegionCount = 2;  // MEM1, MEM2

  explicit MemoryCensus(std::size_t capacity = 100000) : m_capacity(capacity) {
    m_samples.reserve(capacity);
  }

  // `frame` is informational, and `host_ms` is milliseconds since the census
  // started. A region span may be empty (for example, MEM2 on GameCube).
  void Sample(std::uint64_t frame, std::uint64_t host_ms,
              std::span<const std::uint8_t> mem1, std::span<const std::uint8_t> mem2) {
    const std::span<const std::uint8_t> regions[kRegionCount] = {mem1, mem2};
    Record record{frame, host_ms, {}};
    for (std::size_t r = 0; r < kRegionCount; ++r)
      record.counts[r] = m_regions[r].Sample(regions[r], m_sample_count == 0);
    ++m_sample_count;
    if (m_samples.size() == m_capacity) {
      ++m_dropped;
      return;
    }
    m_samples.push_back(record);
  }

  std::uint64_t SampleCount() const { return m_sample_count; }

  struct Counts {
    std::uint32_t pages = 0;
    std::uint32_t nonzero = 0;
    std::uint32_t ever_nonzero = 0;
    std::uint32_t changed = 0;
    std::uint32_t ever_changed = 0;
  };

  Counts Latest(std::size_t region) const {
    return m_samples.empty() ? Counts{} : m_samples.back().counts[region];
  }

  // Peak over all samples, field by field.
  Counts Peak(std::size_t region) const {
    Counts peak;
    for (const auto& s : m_samples) {
      const auto& c = s.counts[region];
      peak.pages = std::max(peak.pages, c.pages);
      peak.nonzero = std::max(peak.nonzero, c.nonzero);
      peak.ever_nonzero = std::max(peak.ever_nonzero, c.ever_nonzero);
      peak.changed = std::max(peak.changed, c.changed);
      peak.ever_changed = std::max(peak.ever_changed, c.ever_changed);
    }
    return peak;
  }

  void Write(std::ostream& out) const {
    static constexpr std::string_view names[kRegionCount] = {"mem1", "mem2"};
    out << "# OpenMUA2 guest memory census v1; page_size=" << kPageSize
        << "; samples=" << m_sample_count << "; dropped_samples=" << m_dropped << '\n'
        << "# racy sampling: statistical, not a coherent snapshot\n";
    for (std::size_t r = 0; r < kRegionCount; ++r) {
      const Counts p = Peak(r);
      out << "# peak " << names[r] << ": pages=" << p.pages
          << " nonzero=" << p.nonzero << " (" << KiB(p.nonzero) << " KiB)"
          << " ever_nonzero=" << p.ever_nonzero << " (" << KiB(p.ever_nonzero) << " KiB)"
          << " changed=" << p.changed << " (" << KiB(p.changed) << " KiB)"
          << " ever_changed=" << p.ever_changed << " (" << KiB(p.ever_changed) << " KiB)\n";
    }
    // Per-MiB occupancy of ever_nonzero pages: shows *where* usage sits, which
    // matters for any compacted guest-memory layout.
    for (std::size_t r = 0; r < kRegionCount; ++r) {
      const auto& region = m_regions[r];
      if (region.flags.empty())
        continue;
      out << "# " << names[r] << "_mib_ever_nonzero_pages(of 256):";
      const std::size_t pages_per_mib = (1u << 20) / kPageSize;
      for (std::size_t base = 0; base < region.flags.size(); base += pages_per_mib) {
        unsigned used = 0;
        for (std::size_t i = base; i < std::min(region.flags.size(), base + pages_per_mib); ++i)
          used += (region.flags[i] & kEverNonzero) ? 1u : 0u;
        out << ' ' << used;
      }
      out << '\n';
    }
    out << "frame,host_ms";
    for (std::size_t r = 0; r < kRegionCount; ++r)
      out << ',' << names[r] << "_pages," << names[r] << "_nonzero," << names[r]
          << "_ever_nonzero," << names[r] << "_changed," << names[r] << "_ever_changed";
    out << '\n';
    for (const auto& s : m_samples) {
      out << s.frame << ',' << s.host_ms;
      for (const auto& c : s.counts)
        out << ',' << c.pages << ',' << c.nonzero << ',' << c.ever_nonzero << ','
            << c.changed << ',' << c.ever_changed;
      out << '\n';
    }
  }

private:
  static constexpr std::uint8_t kEverNonzero = 1;
  static constexpr std::uint8_t kEverChanged = 2;

  static std::uint64_t KiB(std::uint32_t pages) {
    return static_cast<std::uint64_t>(pages) * (kPageSize / 1024);
  }

  // Hashes one page and reports whether it contains a non-zero byte. Unaligned
  // memcpy loads keep this strict-aliasing clean. Compilers lower them to
  // plain moves.
  static std::uint64_t HashPage(const std::uint8_t* page, std::size_t size, bool& nonzero) {
    std::uint64_t hash = 0x9E3779B97F4A7C15ull;
    std::uint64_t any = 0;
    std::size_t i = 0;
    for (; i + 8 <= size; i += 8) {
      std::uint64_t word;
      std::memcpy(&word, page + i, sizeof(word));
      any |= word;
      hash = (hash ^ word) * 0x100000001B3ull;
      hash ^= hash >> 29;
    }
    for (; i < size; ++i) {
      any |= page[i];
      hash = (hash ^ page[i]) * 0x100000001B3ull;
    }
    nonzero = any != 0;
    return hash;
  }

  struct Region {
    std::vector<std::uint64_t> hashes;
    std::vector<std::uint8_t> flags;

    Counts Sample(std::span<const std::uint8_t> bytes, bool first) {
      const std::size_t pages = (bytes.size() + kPageSize - 1) / kPageSize;
      if (hashes.size() != pages) {
        // First sample or a resized region (e.g. a different game booted): restart.
        hashes.assign(pages, 0);
        flags.assign(pages, 0);
        first = true;
      }
      Counts c;
      c.pages = static_cast<std::uint32_t>(pages);
      for (std::size_t p = 0; p < pages; ++p) {
        const std::size_t offset = p * kPageSize;
        const std::size_t size = std::min(kPageSize, bytes.size() - offset);
        bool nonzero = false;
        const std::uint64_t hash = HashPage(bytes.data() + offset, size, nonzero);
        if (nonzero) {
          ++c.nonzero;
          flags[p] |= kEverNonzero;
        }
        if (!first && hash != hashes[p]) {
          ++c.changed;
          flags[p] |= kEverChanged;
        }
        hashes[p] = hash;
        c.ever_nonzero += (flags[p] & kEverNonzero) ? 1u : 0u;
        c.ever_changed += (flags[p] & kEverChanged) ? 1u : 0u;
      }
      return c;
    }
  };

  struct Record {
    std::uint64_t frame;
    std::uint64_t host_ms;
    Counts counts[kRegionCount];
  };

  Region m_regions[kRegionCount];
  std::vector<Record> m_samples;
  std::size_t m_capacity;
  std::uint64_t m_sample_count = 0;
  std::uint64_t m_dropped = 0;
};

}  // namespace moderngekko::telemetry
