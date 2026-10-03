#pragma once
#include "mua2_action_bindings.hpp"
#include <array>
#include <optional>

namespace moderngekko::controls {
// Experimental profile: plus marks an unmodified D-pad press. Keep menu
// directions available while consuming the corresponding gameplay powers and
// synthetic plus-button menu exit/skill-point actions.
inline void ConsumeHeroMarkers(std::span<std::uint8_t> active,
                               std::span<const std::uint8_t> values) {
  if (active.size()!=20 || values.size()!=496 ||
      ReadBE(values,13*4)!=0x3f800000 || ReadBE(values,14*4)==0x3f800000) return;
  for (unsigned id : {29u,30u,31u,32u,34u,35u,36u,37u,105u,118u}) {
    const auto offset=4*(id/32), word=ReadBE(active,offset)&~(1u<<(id%32));
    for(unsigned i=0;i<4;++i) active[offset+i]=std::uint8_t(word>>(24-8*i));
  }
}

// Called only at the guarded next-hero helper call, after the game's mode and
// player gates. Four means reject through the native helper's index check.
// Never retain the caller's temporary actor array beyond this invocation.
template<typename Reader>
std::optional<std::uint32_t> HeroButtonIndex(Reader read, std::uint32_t owner,
    std::uint32_t port, std::uint32_t input, std::uint32_t actor_array,
    std::optional<unsigned> direct_slot = std::nullopt) {
  const auto ram=[&](std::uint32_t a,std::size_t n) {
    const auto end=std::uint64_t(a)+n;
    if (!(a&3) && ((a>=0x80000000 && end<=0x81800000) ||
                  (a>=0x90000000 && end<=0x94000000))) return read(a,n);
    return std::span<const std::uint8_t>{};
  };
  if(port>=4 || input!=0x81313274+port*0xbe00) return std::nullopt;
  const auto device=ram(input,0xbe00);
  if(device.size()!=0xbe00 || ReadBE(device,0)!=0x811b4398) return std::nullopt;
  const auto values=device.subspan(0xbb20,496);
  unsigned selected=4;
  if(direct_slot) selected=*direct_slot;
  else {
  if(ReadBE(values,13*4)!=0x3f800000) return std::nullopt;
  if(ReadBE(values,14*4)==0x3f800000) return 4;
  constexpr std::array<unsigned,4> directions{32,31,29,30}; // Up, Right, Down, Left
  for(unsigned i=0;i<4;++i) {
    const auto v=ReadBE(values,4*directions[i]);
    if(v==0 || v==0xbf800000) continue;
    if(v!=0x3f800000 || selected!=4) return 4;
    selected=i;
  }
  }
  if(selected>=4) return 4;
  const auto team=ram(0x80629490,0x740), global=ram(0x80817368,4);
  const auto actors=ram(actor_array,16);
  if(team.size()!=0x740 || ReadBE(team,0x100)!=0x80534c90 ||
      global.size()!=4 || actors.size()!=16) return 4;
  const auto count=ReadBE(team,0x73c);
  if(count==0 || count>4 || selected>=count) return 4;
  const auto manager=ram(ReadBE(global,0),0x1400);
  if(manager.size()!=0x1400 || ReadBE(manager,0)!=0x81177d08 ||
      ReadBE(manager,0x13fc)!=511) return 4;
  unsigned owners=0;
  std::array<std::uint32_t,4> handles{};
  for(unsigned i=0;i<count;++i) {
    const auto handle=ReadBE(team,0x72c+4*i), index=handle&511;
    if(!handle || index>=420 || ReadBE(manager,0xd6c+4*index)!=handle ||
        !(ReadBE(manager,0xd30+4*(index/32))&(1u<<(index%32)))) return 4;
    for(unsigned j=0;j<i;++j) if(handles[j]==handle) return 4;
    handles[i]=handle;
    const auto address=ReadBE(manager,4+4*index);
    const auto member=ram(address,0xa38);
    if(member.size()!=0xa38 || ReadBE(member,0x48)!=handle ||
        ReadBE(member,0x9c)!=0x8052a748 || ReadBE(actors,4*i)!=address) return 4;
    if(!(member[0x4d0]&128) && ReadBE(member,0x45c)==port) {
      if(address!=owner) return 4;
      ++owners;
    }
  }
  for(unsigned i=count;i<4;++i) if(ReadBE(actors,4*i)) return 4;
  // The original helper checks self, AI ownership, health, and interaction
  // eligibility, then performs the complete native player-control handoff.
  return owners==1 ? selected : 4;
}
}
