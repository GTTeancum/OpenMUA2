#include <array>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <span>
#include <string_view>

#include "Core/PowerPC/JitCommon/SimpleFormat.h"
#include "Core/PowerPC/JitCommon/SimpleFormatContract.h"

int main()
{
  for (const auto text : {"", "a", "short text\n", "\x80\xff"})
  {
    std::array<u8, 64> actual, expected;
    actual.fill(0xa5);
    expected.fill(0xa5);
    const auto length = std::strlen(text);
    const int reference = std::snprintf(reinterpret_cast<char*>(expected.data()), expected.size(), "%s", text);
    const auto result = JitCommon::TryLiteralFormat(
        {reinterpret_cast<const u8*>(text), length + 1}, actual);
    if (!result || *result != reference || actual != expected)
      return 1;
  }
  const std::array<u8, 4> unterminated{'a','b','c','d'};
  std::array<u8, 4> output{1,2,3,4};
  const auto original = output;
  if (JitCommon::TryLiteralFormat(unterminated, output) || output != original)
    return 2;
  for (const auto text : {"%", "%f", "%%", "1234"})
  {
    if (JitCommon::TryLiteralFormat({reinterpret_cast<const u8*>(text), std::strlen(text)+1}, output) ||
        output != original)
      return 3;
  }
  std::array<u8, 6> overlap{'a','b','c',0,9,9};
  const auto before = overlap;
  if (JitCommon::TryLiteralFormat(overlap, {overlap.data()+1, 5}) || overlap != before)
    return 4;
  const std::array<u8, 4> exact{'a','b','c',0};
  if (JitCommon::TryLiteralFormat(exact, output) != 3 || output != exact)
    return 5;
  const auto pattern = std::string_view("[%f] and %f");
  const auto format = std::span(reinterpret_cast<const u8*>(pattern.data()), pattern.size()+1);
  u32 random = 0x12345678;
  for (int i = 0; i < 20000; ++i)
  {
    random = random * 1664525 + 1013904223;
    const float value = std::bit_cast<float>(random);
    if (!std::isfinite(value) || std::abs(value) > 1000000.0f ||
        (value != 0.0f && std::abs(value) < 1e-30))
      continue;
    std::array<u8, 128> actual, expected;
    actual.fill(0xa5); expected.fill(0xa5);
    const std::array<double, 2> values{value, -0.0};
    const int reference = std::snprintf(reinterpret_cast<char*>(expected.data()), expected.size(),
                                      "[%f] and %f", values[0], values[1]);
    const auto result = JitCommon::TryFixedFloatFormat(format, values, actual);
    if (!result || *result != reference || actual != expected)
      return 6;
  }
  const std::array<double, 2> unsupported{0.1, 0.0}; // Not an exact binary32 value.
  output = original;
  if (JitCommon::TryFixedFloatFormat(format, unsupported, output) || output != original ||
      JitCommon::CountFixedFloatFields({reinterpret_cast<const u8*>("%.2f"), 5}))
    return 7;
  const std::array<u8, 12> mixed{'%', 's', ':', '%', 'd', ':', '%', 'f', ':', '%', '%', 0};
  for (u32 bits : {0u, 1u, 0xffffffffu, 0x80000000u, 0x7fffffffu})
  {
    std::array<u8, 128> actual, expected;
    actual.fill(0xa5); expected.fill(0xa5);
    const s32 integer = std::bit_cast<s32>(bits);
    const std::array<double, 1> values{-0.0};
    const auto result = JitCommon::TryBasicFormat(mixed, actual,
        [&](u8 type, std::span<u8> target) -> std::optional<u32> {
      if (type == 's') { std::memcpy(target.data(), "test", 4); return 4; }
      if (type == 'f')
      {
        const std::array<u8, 3> f{'%', 'f', 0};
        return JitCommon::TryFixedFloatFormat(f, values, target);
      }
      auto* first = reinterpret_cast<char*>(target.data());
      const auto converted = std::to_chars(first, first + target.size(), integer);
      if (converted.ec != std::errc{}) return {};
      return static_cast<u32>(converted.ptr - first);
    });
    const int reference = std::snprintf(reinterpret_cast<char*>(expected.data()), expected.size(),
                                      "%s:%d:%f:%%", "test", integer, values[0]);
    if (!result || *result != reference || actual != expected) return 8;
  }
  for (const auto bad : {"prefix %08x", "prefix %n", "prefix %", "long literal"})
  {
    output = original;
    if (JitCommon::TryBasicFormat({reinterpret_cast<const u8*>(bad), std::strlen(bad) + 1}, output,
        [](u8, std::span<u8>) -> std::optional<u32> { return {}; }) || output != original) return 9;
  }
  std::array<u8, 128> rejected;
  rejected.fill(0xa5);
  const auto untouched = rejected;
  if (JitCommon::TryBasicFormat(mixed, rejected,
      [](u8, std::span<u8> target) -> std::optional<u32> {
        target[0] = 'x'; return {}; // Failed reader cannot partially publish output.
      }) || rejected != untouched) return 10;
  // Realistic disjoint stack/output/format, then argument aliasing, boundary
  // overlap, misalignment and 32-bit wrap cases that must retain original code.
  constexpr u32 sp = 0x817fe000, destination = 0x806b3af0, text = 0x80500000;
  if (!JitCommon::SimpleFormatLayoutSafe(sp, destination, text, sp + 0x68)) return 11;
  for (const u32 alias : {sp, sp + 8, sp + 0x68, sp + 0x97, sp - 511})
    if (JitCommon::SimpleFormatLayoutSafe(sp, destination, alias, sp + 0x68)) return 12;
  for (const u32 alias : {sp, sp + 8, sp + 0x97, sp - 1023})
    if (JitCommon::SimpleFormatLayoutSafe(sp, alias, text, sp + 0x68)) return 13;
  if (JitCommon::SimpleFormatLayoutSafe(sp + 8, destination, text, sp + 0x70) ||
      JitCommon::SimpleFormatLayoutSafe(0xfffffff0, destination, text, 0x58) ||
      JitCommon::SimpleFormatLayoutSafe(sp, destination, destination + 1023, sp + 0x68) ||
      !JitCommon::SimpleFormatRangesDisjoint(0x1000, 16, 0x1010, 16) ||
      JitCommon::SimpleFormatRangesDisjoint(0xfffffff0, 32, 0x1000, 16)) return 14;
  if (JitCommon::SimpleFormatPreservedStateChanged(0x1ff9, 3, 3, 0xe3)) return 15;
  for (u32 i = 0; i < 32; ++i)
  {
    const bool preserved_gpr = i == 1 || i == 2 || i >= 13;
    const bool preserved_fpr = i >= 14;
    if (JitCommon::SimpleFormatPreservedStateChanged(u32{1} << i, 0, 0, 0) != preserved_gpr ||
        JitCommon::SimpleFormatPreservedStateChanged(0, u32{1} << i, 0, 0) != preserved_fpr ||
        JitCommon::SimpleFormatPreservedStateChanged(0, 0, u32{1} << i, 0) != preserved_fpr) return 16;
  }
  for (u32 i = 0; i < 8; ++i)
    if (JitCommon::SimpleFormatPreservedStateChanged(0, 0, 0, u32{1} << i) !=
        (i >= 2 && i <= 4)) return 17;
  // Compare hexadecimal case/padding and unsigned boundaries with libc.
  for (const auto pattern : {"%x", "%X", "%08x", "%08X", "[%08X]"})
  {
    for (u32 value : {0u, 1u, 0xabcdefu, 0x80000000u, 0xffffffffu})
    {
      std::array<u8, 32> actual, expected;
      actual.fill(0xa5); expected.fill(0xa5);
      const auto result = JitCommon::TryBasicFormat(
          {reinterpret_cast<const u8*>(pattern), std::strlen(pattern) + 1}, actual,
          [&](u8 type, std::span<u8> target) -> std::optional<u32> {
            auto* first = reinterpret_cast<char*>(target.data());
            const auto converted = std::to_chars(first, first + target.size(), value, 16);
            if (converted.ec != std::errc{}) return {};
            if (type == 'X')
              for (auto* digit = first; digit != converted.ptr; ++digit)
                if (*digit >= 'a' && *digit <= 'f') *digit -= 'a' - 'A';
            return static_cast<u32>(converted.ptr - first);
          });
      const int reference = std::snprintf(reinterpret_cast<char*>(expected.data()),
                                         expected.size(), pattern, value);
      if (!result || *result != reference || actual != expected) return 18;
    }
  }
  // Padding must fit transactionally even when the unpadded value would fit.
  for (const auto pattern : {"%08x", "%08X", "%0", "%08", "%04x", "%08d"})
  {
    output = original;
    if (JitCommon::TryBasicFormat(
        {reinterpret_cast<const u8*>(pattern), std::strlen(pattern) + 1}, output,
        [](u8, std::span<u8> target) -> std::optional<u32> {
          target[0] = '1'; return 1;
        }) || output != original) return 19;
  }
  return 0;
}
