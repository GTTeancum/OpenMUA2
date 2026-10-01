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
// The evaluator sums both digital sources even for a partially held chord.
// Preserve its active bits, but give use the ordinary digital 0/1 magnitude.
// Reject unexpected evaluator output without changing any value.
inline bool NormalizeChordUse(std::span<const std::uint8_t> active,
                              std::span<std::uint8_t> values) {
  if (active.size()!=20 || values.size()!=124*4) return false;
  const auto bits=ReadBE(active,0);
  const bool use=(bits & (1u<<21))!=0, grab=(bits & (1u<<11))!=0;
  const auto value=ReadBE(values,21*4);
  if (use!=grab || (use ? value!=0x40000000 :
      (value!=0 && value!=0x3f800000))) return false;
  const std::uint32_t normalized=use ? 0x3f800000 : 0;
  for (unsigned i=0;i<4;++i)
    values[21*4+i]=std::uint8_t(normalized>>(24-8*i));
  return true;
}

// Experimental Xbox Start carries native pause and menu-back together. Do not
// emulate the Wiimote B source: it also means smash/grab/context lift.
inline bool MapPauseBack(std::span<std::uint8_t> active,
                         std::span<std::uint8_t> values) {
  if (active.size()!=20 || values.size()!=124*4 ||
      !(ReadBE(active,4)&(1u<<7)) || ReadBE(values,39*4)!=0x3f800000)
    return false;
  const auto put=[](std::span<std::uint8_t> b, unsigned p, std::uint32_t v) {
    for(unsigned i=0;i<4;++i) b[p+i]=std::uint8_t(v>>(24-8*i));
  };
  for (unsigned id : {9u,10u,11u,21u,56u,58u,103u,123u}) {
    put(active,(id/32)*4,ReadBE(active,(id/32)*4)&~(1u<<(id%32)));
    put(values,id*4,0);
  }
  put(active,8,ReadBE(active,8)|(1u<<26));
  put(values,90*4,0x3f800000);
  return true;
}

// The experimental right-stick profile uses the native camera-enable source.
// That source also backs three unrelated menu commands. Keep camera/turn values
// intact, including negative CameraX; consume only these shared menu aliases.
inline bool ConsumeCameraMenuAliases(std::span<std::uint8_t> active,
                                    std::span<std::uint8_t> values) {
  if (active.size()!=20 || values.size()!=124*4 ||
      !(ReadBE(active,0)&(1u<<7)) || ReadBE(values,7*4)!=0x3f800000)
    return false;
  for (unsigned id : {99u,104u,122u}) {
    const auto offset=4*(id/32), word=ReadBE(active,offset)&~(1u<<(id%32));
    for(unsigned i=0;i<4;++i) {
      active[offset+i]=std::uint8_t(word>>(24-8*i));
      values[id*4+i]=0;
    }
  }
  return true;
}

// Experimental Back is the otherwise-disjoint camera+pause source chord.
// The matching profile excludes Start from camera-enable, so Start+right-stick
// cannot accidentally request Hero Management. Consume both marker sources.
inline bool MapHeroManagement(std::span<std::uint8_t> active,
                              std::span<std::uint8_t> values) {
  if (active.size()!=20 || values.size()!=496 ||
      !(ReadBE(active,0)&(1u<<7)) || !(ReadBE(active,4)&(1u<<7)) ||
      ReadBE(values,7*4)!=0x3f800000 || ReadBE(values,39*4)!=0x3f800000)
    return false;
  const auto put=[](std::span<std::uint8_t> b,unsigned p,std::uint32_t v) {
    for(unsigned i=0;i<4;++i) b[p+i]=std::uint8_t(v>>(24-8*i));
  };
  for (unsigned id : {7u,39u,99u,103u,104u,122u,123u}) {
    put(active,id/32*4,ReadBE(active,id/32*4)&~(1u<<(id%32)));
    put(values,id*4,0);
  }
  put(active,4,ReadBE(active,4)|(1u<<8));
  put(values,40*4,0x3f800000);
  return true;
}

}
