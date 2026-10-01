// Copyright 2026 OpenMUA2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <charconv>
#include <cstddef>
#include <optional>
#include <string_view>

namespace JitCommon
{
// Explicit opt-in capacity only. Bound host allocations; 1 retains the initial
// experiment's 128K-entry meaning, while 0 selects the ordinary growth policy.
inline std::optional<std::size_t> ParseBackpatchReserve(std::string_view text)
{
  if (text == "1") return 131072;
  std::size_t value = 0;
  const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
  if (text.empty() || result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
      value > 1048576) return {};
  return value;
}
}  // namespace JitCommon
