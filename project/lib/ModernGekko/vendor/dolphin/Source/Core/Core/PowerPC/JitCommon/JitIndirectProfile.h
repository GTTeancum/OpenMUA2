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

// Explicit, immutable experiment; predictions are guarded against runtime CTR.
// No guest addresses are baked into the runtime's source/default configuration.
struct IndirectHints
{
  u32 origin{};
  std::array<u32, 3> targets{};
  std::size_t size{};
};
inline std::optional<IndirectHints> ParseIndirectHints(std::string_view text)
{
  const auto separator = text.find(':');
  if (separator == std::string_view::npos) return {};
  const auto origin = ParseIndirectProfileAddress(text.substr(0, separator));
  if (!origin) return {};
  IndirectHints hints;
  hints.origin = *origin;
  text.remove_prefix(separator + 1);
  while (!text.empty())
  {
    const auto comma = text.find(',');
    const auto target = ParseIndirectProfileAddress(text.substr(0, comma));
    if (!target || hints.size == hints.targets.size()) return {};
    for (std::size_t i = 0; i < hints.size; ++i)
      if (hints.targets[i] == *target) return {};
    hints.targets[hints.size++] = *target;
    if (comma == std::string_view::npos) return hints;
    text.remove_prefix(comma + 1);
  }
  return {};  // Missing target or trailing comma.
}
inline const std::optional<IndirectHints>& GetIndirectHints()
{
  static const auto hints = []() -> std::optional<IndirectHints> {
    const char* raw = std::getenv("OPENMUA2_INDIRECT_HINTS");
    if (!raw) return {};
    const auto parsed = ParseIndirectHints(raw);
    if (!parsed)
      std::fprintf(stderr, "Invalid OPENMUA2_INDIRECT_HINTS; disabled\n");
    else
      std::fprintf(stderr, "Indirect hints: pc=%08x targets=%zu guarded=1 experimental=1\n",
                   parsed->origin, parsed->size);
    return parsed;
  }();
  return hints;
}

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
