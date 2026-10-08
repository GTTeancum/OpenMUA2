#include "mua2_fusion_buttons.hpp"
#include "mua2_fusion_entry.hpp"
#include <iostream>
#include <map>
#include <tuple>
#include <vector>
using namespace moderngekko::controls;
void put(std::span<std::uint8_t> b,std::size_t p,std::uint32_t v) {
  for(unsigned i=0;i<4;++i) b[p+i]=std::uint8_t(v>>(24-8*i));
}
struct Fixture {
  std::map<std::uint32_t,std::vector<std::uint8_t>> memory;
  static constexpr std::uint32_t owner=0x90012000, input=0x8131f074;
  Fixture() {
    auto& globals=memory[0x80816a20];globals.resize(24);put(globals,0,1);put(globals,16,owner);put(globals,20,1);
    auto& global=memory[0x80817368];global.resize(4);put(global,0,0x90001000);
    auto& manager=memory[0x90001000];manager.resize(0x1400);put(manager,0,0x81177d08);put(manager,0x13fc,511);put(manager,0xd30,30);
    auto& team=memory[0x80629490];team.resize(0x740);put(team,0x100,0x80534c90);put(team,0x73c,4);
    for(unsigned i=0;i<4;++i) {
      const unsigned handle=0x200+i+1, address=0x90010000+i*0x1000;
      put(team,0x72c+i*4,handle);put(manager,4+(i+1)*4,address);put(manager,0xd6c+(i+1)*4,handle);
      auto& actor=memory[address];actor.resize(0xa38);put(actor,0x48,handle);put(actor,0x9c,0x8052a748);
      put(actor,0x45c,1);put(actor,0x2a0,0x42480000);actor[0x4d0]=i==2?0:128;
    }
    auto& device=memory[input];device.resize(0xbe00);put(device,0,0x811b4398);
    put(device,0xbb20+14*4,0x3f800000);put(device,0xbb20+29*4,0x3f800000);put(device,0xbb20+32*4,0xbf800000);
  }
  std::span<const std::uint8_t> read(std::uint32_t a,std::size_t n) const {
    auto it=memory.upper_bound(a);if(it==memory.begin()) return {};--it;
    const auto off=std::uint64_t(a)-it->first;if(off+n>it->second.size()) return {};
    return std::span<const std::uint8_t>(it->second).subspan(off,n);
  }
  auto candidate(std::uint32_t who=owner,std::uint32_t pad=input) const {
    return FusionCandidate([&](auto a,auto n){return read(a,n);},who,pad);
  }
};
int main() {
  Fixture f;
  if(f.candidate()!=0x90010000 || f.candidate(Fixture::owner,0x81313274)!=0) return 1;
  for(auto [a,o,v]:{
      std::tuple{0x80629490u,0x73cu,5u},
      std::tuple{0x80629490u,0x72cu,0x202u}, // duplicate roster handle
      std::tuple{0x90001000u,0xd70u,0x401u}, // stale target handle
      std::tuple{0x90001000u,0xd30u,28u}, // target removed
      std::tuple{0x90010000u,0x48u,0x401u},
      std::tuple{0x90010000u,0x9cu,0u},
      std::tuple{0x90010000u,0x4d0u,0u}, // second human on owner port
      std::tuple{0x90012000u,0x2a0u,0u},
      std::tuple{0x90012000u,0x4d0u,0x80000000u},
      std::tuple{Fixture::input,0xbb20u+14*4,0u},
      std::tuple{Fixture::input,0xbb20u+31*4,0x3f800000u}}) {
    auto bad=f;put(bad.memory[a],o,v);if(bad.candidate()!=0) return 2;
  }
  auto outside=f;put(outside.memory[0x80816a20],0,0);if(outside.candidate()) return 3;
  auto dead=f;put(dead.memory[0x90010000],0x2a0,0);if(dead.candidate()!=0x90010000) return 4;
  for(unsigned slot=0;slot<4;++slot) {
    auto selected=f;auto values=std::span<std::uint8_t>(selected.memory[Fixture::input]).subspan(0xbb20,496);
    for(unsigned id:{29u,30u,31u,32u}) put(values,id*4,0);
    constexpr unsigned powers[]{29,31,30,32}, opposite[]{32,30,31,29};
    put(values,powers[slot]*4,0x3f800000);put(values,opposite[slot]*4,0xbf800000);
    if(FusionButtonSlot(values)!=static_cast<int>(slot) ||
       selected.candidate()!=(slot==2?0u:0x90010000+slot*0x1000)) return 5;
  }
  std::array<std::uint8_t,20> active{};std::array<std::uint8_t,496> values{};
  put(active,0,(1u<<9)|(1u<<14)|(1u<<29));put(active,4,1u<<24);auto original=active;
  ConsumeFusionMarkers(active,values,true);if(active!=original) return 6;
  put(values,14*4,0x3f800000);put(values,29*4,0x3f800000);auto original_values=values;
  ConsumeFusionMarkers(active,values,true);
  if(ReadBE(active,0)!=(1u<<9) || ReadBE(active,4)!=(1u<<24) || values!=original_values) return 7;
  ConsumeFusionMarkers(active,values,false);if(ReadBE(active,0)!=0) return 8;
  // Direct selection does not depend on retained Wii marker scalar values.
  auto direct=f;std::fill(direct.memory[Fixture::input].begin()+0xbb20,direct.memory[Fixture::input].end(),0);
  const auto read=[&](auto a,auto n){return direct.read(a,n);};
  if(FusionCandidate(read,Fixture::owner,Fixture::input,1)!=0x90011000 ||
     FusionCandidate(read,Fixture::owner,Fixture::input,-1)!=0 ||
     FusionCandidate(read,Fixture::owner,Fixture::input,2)!=0 ||
     FusionCandidate(read,Fixture::owner,Fixture::input,4)!=0) return 9;
  // Four distinct human owners can select each other, never themselves.
  auto humans=f;
  for(unsigned i=0;i<4;++i) {
    auto& actor=humans.memory[0x90010000+i*0x1000];actor[0x4d0]=0;put(actor,0x45c,i);
    auto& device=humans.memory[0x81313274+i*0xbe00];device.resize(0xbe00);put(device,0,0x811b4398);
  }
  for(unsigned owner=0;owner<4;++owner) {
    put(humans.memory[0x80816a20],16,0x90010000+owner*0x1000);
    for(unsigned partner=0;partner<4;++partner) {
      const auto target=FusionCandidate([&](auto a,auto n){return humans.read(a,n);},
          0x90010000+owner*0x1000,0x81313274+owner*0xbe00,int(partner));
      if(target!=(owner==partner?0u:0x90010000+partner*0x1000))return 10;
    }
  }
  // The activation edge must arrive after block entry, once per LT hold.
  std::array<FusionEntryState,4> entry;
  GamepadSample lt;lt.connected=true;lt.inputs[unsigned(GamepadInput::LT)]=1;
  for(unsigned port=0;port<4;++port) {
    const auto owner=0x90010000+port*0x1000;
    auto& actor=humans.memory[owner];put(actor,0x3a8,0x90020000+port*0x100);
    auto& node=humans.memory[0x90020000+port*0x100];node.resize(16);put(node,0,0x811b0620);put(node,8,0x90021000+port*4);
    auto& h=humans.memory[0x90021000+port*4];h.resize(4);put(h,0,0x811accd0);
    const auto reader=[&](auto a,auto n){return humans.read(a,n);};
    auto who=ReadFusionActor(reader,port);if(who.actor!=owner || who.handler!=0x811accd0)return 20;
    auto a=BuildGamepadActions(lt,false,false);entry[port].Apply(a,who);
    if(!(a.active[2]&16) || (a.active[7]&2) || ReadBE(a.values,33*4))return 21;
    put(h,0,0x811a6e48);who=ReadFusionActor(reader,port);
    a=BuildGamepadActions(lt,false,false);entry[port].Apply(a,who);
    if(!(a.active[7]&2) || ReadBE(a.values,33*4)!=0x3f800000)return 22;
    a=BuildGamepadActions(lt,false,false);entry[port].Apply(a,who);
    if(!(a.active[7]&2))return 23;
    a=BuildGamepadActions(lt,false,false);entry[port].Apply(a,{owner,0x811a45e8});if(a.active[7]&2)return 30;
    a=BuildGamepadActions(lt,false,false);entry[port].Apply(a,who);if(a.active[7]&2)return 31;
    GamepadActions neutral;entry[port].Apply(neutral,who);
    a=BuildGamepadActions(lt,false,false);entry[port].Apply(a,who);if(!(a.active[7]&2))return 24;
    entry[port]={};a=BuildGamepadActions(lt,false,false);entry[port].Apply(a,{owner,0x811accd0});
    a=BuildGamepadActions(lt,false,false);entry[port].Apply(a,{owner+4,0x811a6e48});if(a.active[7]&2)return 25;
    a=BuildGamepadActions(lt,false,false);entry[port].Apply(a,who);if(a.active[7]&2)return 26;
    entry[port]={};a=BuildGamepadActions(lt,false,false);entry[port].Apply(a,{});
    a=BuildGamepadActions(lt,false,false);entry[port].Apply(a,who);if(a.active[7]&2)return 27;
    actor[0x4d0]=128;if(ReadFusionActor(reader,port).actor)return 28;actor[0x4d0]=0;
    put(actor,0x2a0,0);if(ReadFusionActor(reader,port).actor)return 29;put(actor,0x2a0,0x42480000);
  }
  std::cout<<"Fusion roster, ownership, stale-handle, modifier and native-context guards passed\n";
}
