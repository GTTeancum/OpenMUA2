// Copyright 2026 OpenMUA2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <array>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string_view>

#include "Common/CommonTypes.h"

namespace JitCommon
{
inline std::optional<u32> ParseIndirectProfileAddress(std::string_view text)
{
  if (text.empty() || text.size() > 8) return {};
  u32 address = 0;
  const auto parsed = std::from_chars(text.data(), text.data() + text.size(), address, 16);
  if (parsed.ec != std::errc{} ||
      parsed.ptr != text.data() + text.size() || address == 0 || (address & 3))
    return {};
  return address;
}

// CPU-thread-only diagnostic. Fixed storage; overflow is explicitly counted.
// Feature flags distinguish the same target in different translation contexts.
struct IndirectTargetCounts
{
  struct Entry { u32 target{}, flags{}; u64 count{}; };
  std::array<Entry, 64> entries{};
  std::size_t size{};
  u64 total{}, overflow{};
  void Record(u32 target, u32 flags)
  {
    ++total;
    for (std::size_t i = 0; i < size; ++i)
      if (entries[i].target == target && entries[i].flags == flags)
      {
        ++entries[i].count;
        return;
      }
    if (size == entries.size()) { ++overflow; return; }
    entries[size++] = {target, flags, 1};
  }
};

struct IndirectProfile
{
  IndirectProfile()
  {
    if (const char* raw = std::getenv("OPENMUA2_INDIRECT_PROFILE_PC"))
    {
      address = ParseIndirectProfileAddress(raw);
      if (!address) std::fprintf(stderr, "Invalid OPENMUA2_INDIRECT_PROFILE_PC; disabled\n");
    }
  }
  std::optional<u32> address;
  IndirectTargetCounts counts;
  void Flush() const
  {
    if (!address) return;
    std::fprintf(stderr, "Indirect target profile: pc=%08x total=%llu targets=%zu overflow=%llu scope=whole-process-intrusive\n",
                 *address, static_cast<unsigned long long>(counts.total), counts.size,
                 static_cast<unsigned long long>(counts.overflow));
    for (std::size_t i = 0; i < counts.size; ++i)
    {
      const auto& e = counts.entries[i];
      std::fprintf(stderr, "Indirect target: address=%08x flags=%u count=%llu\n",
                   e.target, e.flags, static_cast<unsigned long long>(e.count));
    }
  }
};
inline IndirectProfile& GetIndirectProfile()
{
  // Process lifetime, flushed explicitly after CPU shutdown.
  static auto* profile = new IndirectProfile;
  return *profile;
}
inline void RecordIndirectTarget(u32 target, u32 flags)
{
  GetIndirectProfile().counts.Record(target, flags);
}
}  // namespace JitCommon
