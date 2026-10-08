#pragma once
#include "mua2_gamepad_actions.hpp"
#include "mua2_action_bindings.hpp"

namespace moderngekko::controls {
struct FusionActorState {
  std::uint32_t actor=0, handler=0;
};
// Resolve one live, human-controlled roster member for this physical port.
// Never cache addresses across ownership changes or save-state restoration.
template<typename Reader>
FusionActorState ReadFusionActor(Reader read, unsigned port) {
  const auto ram=[&](std::uint32_t a,std::size_t n) {
    const auto end=std::uint64_t(a)+n;
    if(!(a&3) && ((a>=0x80000000 && end<=0x81800000) ||
                    (a>=0x90000000 && end<=0x94000000)))return read(a,n);
    return std::span<const std::uint8_t>{};
  };
  if(port>=4)return {};
  const auto team=ram(0x80629490,0x740),global=ram(0x80817368,4);
  if(team.size()!=0x740 || ReadBE(team,0x100)!=0x80534c90 || global.size()!=4)return {};
  const auto count=ReadBE(team,0x73c),manager=ReadBE(global,0);
  const auto entities=ram(manager,0x1400);
  if(count==0 || count>4 || entities.size()!=0x1400 || ReadBE(entities,0)!=0x81177d08 ||
     ReadBE(entities,0x13fc)!=511)return {};
  FusionActorState result;
  std::array<std::uint32_t,4> handles{};
  for(unsigned i=0;i<count;++i) {
    const auto handle=ReadBE(team,0x72c+i*4),index=handle&511;
    if(!handle || index>=420 || ReadBE(entities,0xd6c+4*index)!=handle ||
       !(ReadBE(entities,0xd30+4*(index/32))&(1u<<(index%32))))return {};
    for(unsigned j=0;j<i;++j)if(handles[j]==handle)return {};
    handles[i]=handle;
    const auto address=ReadBE(entities,4+4*index);
    const auto actor=ram(address,0xa38);
    if(actor.size()!=0xa38 || ReadBE(actor,0x48)!=handle || ReadBE(actor,0x9c)!=0x8052a748)return {};
    if((actor[0x4d0]&128) || ReadBE(actor,0x45c)!=port)continue;
    const float health=std::bit_cast<float>(ReadBE(actor,0x2a0));
    if(result.actor || !std::isfinite(health) || health<=0)return {};
    const auto node=ram(ReadBE(actor,0x3a8),16);
    if(node.size()!=16 || ReadBE(node,0)!=0x811b0620)return {};
    const auto handler=ram(ReadBE(node,8),4);
    if(handler.size()!=4)return {};
    result={address,ReadBE(handler,0)};
  }
  return result;
}
struct FusionEntryState {
  std::uint32_t owner=0;
  bool held=false, fired=false, issued=false;
  void Apply(GamepadActions& actions, FusionActorState actor) {
    const bool requested=(actions.active[7]&2)!=0; // Logical action33 before sequencing.
    if(!requested) { *this={};return; }
    actions.active[7]&=std::uint8_t(~2u);
    for(unsigned i=0;i<4;++i)actions.values[33*4+i]=0;
    if(!held) {held=true;owner=actor.actor;}
    if(!owner || owner!=actor.actor) {fired=true;return;}
    // A held request cannot acquire a new actor or retrigger after completion.
    if(actor.handler==0x811a45e8 || (issued && actor.handler!=0x811a6e48))fired=true;
    actions.Set(12); // Shared native preparatory action, not a Wii button.
    if(!fired && actor.handler==0x811a6e48) {
      actions.Set(33);issued=true; // Retain the level until the native handler consumes the edge.
    }
  }
};
} // namespace moderngekko::controls
