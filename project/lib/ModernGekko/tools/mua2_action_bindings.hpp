#pragma once
#include <algorithm>
#include <cstdint>
#include <span>
#include <string_view>

namespace moderngekko::controls {
enum class RebindResult { Rejected, AlreadyApplied, Applied };
inline constexpr std::size_t DescriptorSize = 0x98;
inline constexpr std::size_t DescriptorTableSize = 160 * DescriptorSize;
inline std::uint32_t ReadBE(std::span<const std::uint8_t> b, std::size_t p) {
  return (std::uint32_t(b[p]) << 24) | (std::uint32_t(b[p+1]) << 16) |
         (std::uint32_t(b[p+2]) << 8) | b[p+3];
}
// Only accepts the inspected stock bindings (or our exact result). Validate
// everything before writing; unknown/custom descriptors remain byte-identical.
// No action bits, player ownership, timing, or context are synthesized here.
inline RebindResult RebindContextUse(std::span<std::uint8_t> table) {
  if (table.size() != DescriptorTableSize) return RebindResult::Rejected;
  const auto descriptor = [&](unsigned id, std::string_view name) {
    auto d = table.subspan(id * DescriptorSize, DescriptorSize);
    if (ReadBE(d, 0) != id || !std::equal(name.begin(), name.end(), d.begin()+4) ||
        d[4+name.size()] != 0) return std::span<std::uint8_t>{};
    return d;
  };
  auto grab = descriptor(11, "Grab"), block = descriptor(12, "Block"),
       use = descriptor(21, "Action");
  if (grab.empty() || block.empty() || use.empty()) return RebindResult::Rejected;
  const auto binding = [](std::span<const std::uint8_t> d, unsigned n,
                          unsigned selector, std::uint32_t threshold) {
    auto p = 0x48 + n * 16;
    return ReadBE(d,p)==selector && d[p+4]==0 && ReadBE(d,p+8)==0 &&
           ReadBE(d,p+12)==threshold;
  };
  if (ReadBE(grab,0x44)!=2 || !binding(grab,0,3,0x3cf5c28f) ||
      !binding(grab,1,4,0x3cf5c28f) || ReadBE(block,0x44)!=1 ||
      !binding(block,0,133,0x3e19999a)) return RebindResult::Rejected;
  if (ReadBE(use,0x44)==2 &&
      std::equal(grab.begin()+0x48,grab.begin()+0x68,use.begin()+0x48))
    return RebindResult::AlreadyApplied;
  if (ReadBE(use,0x44)!=1 || !binding(use,0,133,0x3e19999a))
    return RebindResult::Rejected;
  std::copy_n(grab.begin()+0x48,32,use.begin()+0x48);
  use[0x47]=2;
  return RebindResult::Applied;
}
}
