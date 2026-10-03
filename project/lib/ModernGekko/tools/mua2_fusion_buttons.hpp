#pragma once
#include "mua2_action_bindings.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <optional>

namespace moderngekko::controls {
// Experimental profile protocol: minus carries LT; LT+face carries one power
// direction plus the native confirm button. The game still performs selection.
inline int FusionButtonSlot(std::span<const std::uint8_t> values) {
  if (values.size()!=124*4 || ReadBE(values,14*4)!=0x3f800000) return -1;
  constexpr std::array<unsigned,4> powers{29,31,30,32}; // A, B, X, Y
  int slot=-1;
  for (unsigned i=0;i<powers.size();++i) {
    const auto v=ReadBE(values,powers[i]*4);
    if (!v || v==0xbf800000) continue; // Opposite signed direction is inactive.
    if (v!=0x3f800000 || slot!=-1) return -1;
    slot=static_cast<int>(i);
  }
  return slot;
}
inline void ConsumeFusionMarkers(std::span<std::uint8_t> active,
                                 std::span<const std::uint8_t> values,
                                 bool request_context) {
  if (active.size()!=20 || values.size()!=124*4 ||
      ReadBE(values,14*4)!=0x3f800000) return;
  const auto clear=[&](unsigned id) {
    const auto offset=4*(id/32);
    const auto word=ReadBE(active,offset)&~(1u<<(id%32));
    for(unsigned i=0;i<4;++i) active[offset+i]=std::uint8_t(word>>(24-8*i));
  };
  // Clear only this protocol's marker/direction actions. Scalar samples remain
  // available to the native picker in the same input object, with no host latch.
  for(unsigned id:{14u,28u,29u,30u,31u,32u,34u,35u,36u,37u,
                   91u,92u,93u,94u,95u,96u,97u,98u,119u}) clear(id);
  if (!request_context || FusionButtonSlot(values)<0) clear(9);
}

// nullopt means this is not the guarded native request context. Zero suppresses
// an invalid selection inside that context; it must never fall back to a cursor.
// The native picker continues its own actor/type/eligibility/cost checks.
template<typename Reader>
std::optional<std::uint32_t> FusionCandidate(Reader read, std::uint32_t owner,
                                            std::uint32_t input,
                                            std::optional<int> direct_slot = std::nullopt) {
  const auto ram=[&](std::uint32_t a,std::size_t n) {
    const auto end=std::uint64_t(a)+n;
    if (!(a&3) && ((a>=0x80000000 && end<=0x81800000) ||
                  (a>=0x90000000 && end<=0x94000000))) return read(a,n);
    return std::span<const std::uint8_t>{};
  };
  auto globals=ram(0x80816a20,24);
  if(globals.size()!=24 || ReadBE(globals,0)!=1 ||
     ReadBE(globals,16)!=owner || !owner) return std::nullopt;
  auto actor=ram(owner,0xa38);
  if(actor.size()!=0xa38 || ReadBE(actor,0x9c)!=0x8052a748 ||
     (actor[0x4d0]&0x80)) return 0;
  const auto port=ReadBE(actor,0x45c);
  if(port>=4 || input!=0x81313274+port*0xbe00) return 0;
  const float health=std::bit_cast<float>(ReadBE(actor,0x2a0));
  if(!std::isfinite(health) || health<=0) return 0;
  auto device=ram(input,0xbe00);
  if(device.size()!=0xbe00 || ReadBE(device,0)!=0x811b4398) return 0;
  const auto slot=direct_slot ? *direct_slot : FusionButtonSlot(device.subspan(0xbb20,124*4));
  if(slot<0) return 0;
  auto team=ram(0x80629490,0x740), global=ram(0x80817368,4);
  if(team.size()!=0x740 || ReadBE(team,0x100)!=0x80534c90 || global.size()!=4)
    return 0;
  const auto count=ReadBE(team,0x73c);
  if(count==0 || count>4 || static_cast<unsigned>(slot)>=count) return 0;
  auto manager=ram(ReadBE(global,0),0x1400);
  if(manager.size()!=0x1400 || ReadBE(manager,0)!=0x81177d08 ||
     ReadBE(manager,0x13fc)!=0x1ff) return 0;
  unsigned owners=0;
  std::uint32_t candidate=0;
  std::array<std::uint32_t,4> handles{};
  for(unsigned i=0;i<count;++i) {
    const auto handle=ReadBE(team,0x72c+4*i), index=handle&0x1ff;
    if(!handle || index>=420 || ReadBE(manager,0xd6c+4*index)!=handle ||
       !(ReadBE(manager,0xd30+4*(index/32))&(1u<<(index%32)))) return 0;
    for(unsigned j=0;j<i;++j) if(handles[j]==handle) return 0;
    handles[i]=handle;
    const auto address=ReadBE(manager,4+4*index);
    auto member=ram(address,0xa38);
    if(member.size()!=0xa38 || ReadBE(member,0x48)!=handle ||
       ReadBE(member,0x9c)!=0x8052a748) return 0;
    const auto member_port=ReadBE(member,0x45c);
    const bool ai=(member[0x4d0]&0x80)!=0;
    if(!ai && member_port==port) {
      if(address!=owner) return 0;
      ++owners;
    }
    if(i==static_cast<unsigned>(slot)) {
      // Other human players and self are ineligible. Dead AI targets remain
      // subject to the game's existing revival and resource checks.
      const float target_health=std::bit_cast<float>(ReadBE(member,0x2a0));
      if(address==owner || !ai || !std::isfinite(target_health) || target_health<0)
        return 0;
      candidate=address;
    }
  }
  return owners==1 ? candidate : 0;
}
}
