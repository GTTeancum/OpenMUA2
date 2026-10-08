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

// Identify menus where Start means continue/accept, not cancel. Use the active
// menu, never a retained/closed allocation: title, profile-name keyboard, and
// the joined-player profile screen whose Ready prompt starts the game.
// Caller already guards the exact game executable and evaluator instructions.
template<typename Reader>
inline std::uint32_t ActiveMenuType(Reader read) {
  const auto ram=[&](std::uint32_t address,std::size_t size) {
    const auto end=std::uint64_t(address)+size;
    if (!(address&3) && ((address>=0x80000000 && end<=0x81800000) ||
                        (address>=0x90000000 && end<=0x94000000)))
      return read(address,size);
    return std::span<const std::uint8_t>{};
  };
  const auto global=ram(0x8081736c,4);
  if(global.size()!=4) return false;
  const auto manager=ram(ReadBE(global,0),25864);
  if(manager.size()!=25864 || ReadBE(manager,0)!=0x81198830) return false;
  const auto menu=ram(ReadBE(manager,25860),10408);
  if (menu.size()!=10408) return false;
  const auto type=ReadBE(menu,10404);
  return type;
}
template<typename Reader>
inline bool IsStartAcceptScreen(Reader read) {
  const auto type=ActiveMenuType(read);
  // CW_Results_Hack uses MENU_OK (105) for Continue Game.
  return type==0x8118bae8 || type==0x811942f0 || type==0x8118f008 ||
    type==0x81194460;
}

// Start emits native continue/accept on the title and profile screens. Elsewhere
// retain pause/back so it can close the PDA again. Neither raw Wii source is
// synthesized: + also switches heroes and B also triggers smash/grab/use.
inline bool MapStartButton(std::span<std::uint8_t> active,
                           std::span<std::uint8_t> values, bool accept_screen) {
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
  const unsigned target=accept_screen ? 105 : 90;
  put(active,target/32*4,ReadBE(active,target/32*4)|(1u<<(target%32)));
  put(values,target*4,0x3f800000);
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
