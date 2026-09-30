#include <array>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <span>
#include <string_view>

#include "Core/PowerPC/JitCommon/SimpleFormat.h"

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
  return 0;
}
