// Copyright 2026 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstring>
#include <optional>
#include <span>

#include "Common/CommonTypes.h"

namespace JitCommon
{
// Deliberately narrow: no conversions, truncation, unterminated input or overlap.
// A rejection leaves the destination untouched so the original code can run.
inline std::optional<u32> TryLiteralFormat(std::span<const u8> format, std::span<u8> output)
{
  const auto end = std::find(format.begin(), format.end(), 0);
  if (end == format.end() || std::find(format.begin(), end, '%') != end)
    return {};
  const auto length = static_cast<std::size_t>(end - format.begin());
  if (length >= output.size())
    return {};
  const auto source = reinterpret_cast<std::uintptr_t>(format.data());
  const auto destination = reinterpret_cast<std::uintptr_t>(output.data());
  if ((destination >= source && destination - source <= length) ||
      (source >= destination && source - destination <= length))
    return {};
  std::memcpy(output.data(), format.data(), length + 1);
  return static_cast<u32>(length);
}
inline std::optional<u32> CountFixedFloatFields(std::span<const u8> format)
{
  u32 fields = 0;
  for (std::size_t i = 0; i < format.size(); ++i)
  {
    if (!format[i])
      return fields;
    if (format[i] == '%' && (++i == format.size() || format[i] != 'f' || ++fields > 8))
      return {};
  }
  return {};
}

// Only bare %f with its standard six fractional digits. Locale-independent;
// finite binary32 values in a bounded normal range, plus signed zero. Other
// formats/values take the original formatter, with no partially written output.
inline std::optional<u32> TryFixedFloatFormat(std::span<const u8> format,
                                             std::span<const double> values,
                                             std::span<u8> output)
{
  const auto fields = CountFixedFloatFields(format);
  if (!fields || !*fields || *fields != values.size())
    return {};
  std::array<u8, 1024> temporary{};
  std::size_t position = 0;
  std::size_t argument = 0;
  for (std::size_t i = 0; format[i]; ++i)
  {
    if (format[i] != '%')
    {
      if (position + 1 >= temporary.size() || position + 1 >= output.size())
        return {};
      temporary[position++] = format[i];
      continue;
    }
    ++i;
    const double value = values[argument++];
    if (!std::isfinite(value) || std::abs(value) > 1000000.0 ||
        (value != 0.0 && std::abs(value) < 1e-30) ||
        static_cast<double>(static_cast<float>(value)) != value)
      return {};
    std::array<char, 64> number;
    const auto converted = std::to_chars(number.data(), number.data() + number.size(), value,
                                         std::chars_format::fixed, 6);
    if (converted.ec != std::errc{})
      return {};
    const auto length = static_cast<std::size_t>(converted.ptr - number.data());
    if (position + length >= temporary.size() || position + length >= output.size())
      return {};
    std::memcpy(temporary.data() + position, number.data(), length);
    position += length;
  }
  std::memcpy(output.data(), temporary.data(), position + 1);
  return static_cast<u32>(position);
}
// Transactional output for bare %s, %d, %f, %x/%X, %08x/%08X and %%. The reader owns argument
// validation and speculative cursor state; it must not mutate guest memory.
template <typename Reader>
std::optional<u32> TryBasicFormat(std::span<const u8> format, std::span<u8> output,
                                 Reader&& reader)
{
  std::array<u8, 1024> temporary{};
  std::size_t position = 0;
  for (std::size_t i = 0; i < format.size(); ++i)
  {
    if (!format[i])
    {
      if (position >= output.size()) return {};
      std::memcpy(output.data(), temporary.data(), position + 1);
      return static_cast<u32>(position);
    }
    if (position + 1 >= temporary.size() || position + 1 >= output.size()) return {};
    if (format[i] != '%')
    {
      temporary[position++] = format[i];
      continue;
    }
    if (++i == format.size()) return {};
    bool zero_pad_hex = false;
    if (format[i] == '0')
    {
      if (i + 2 >= format.size() || format[i + 1] != '8' ||
          (format[i + 2] != 'x' && format[i + 2] != 'X')) return {};
      zero_pad_hex = true;
      i += 2;
    }
    const u8 type = format[i];
    if (type == '%') { temporary[position++] = '%'; continue; }
    if (type != 's' && type != 'd' && type != 'f' && type != 'x' && type != 'X') return {};
    const auto capacity = std::min(temporary.size(), output.size()) - position - 1;
    const auto length = reader(type, std::span<u8>(temporary.data() + position, capacity));
    if (!length || *length > capacity) return {};
    if (zero_pad_hex)
    {
      if (*length > 8 || capacity < 8) return {};
      std::memmove(temporary.data() + position + 8 - *length,
                   temporary.data() + position, *length);
      std::memset(temporary.data() + position, '0', 8 - *length);
      position += 8;
    }
    else
      position += *length;
  }
  return {};
}
}  // namespace JitCommon
