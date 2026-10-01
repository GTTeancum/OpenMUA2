#include "mua2_interaction_context.hpp"
#include <array>
#include <map>
#include <vector>
#include <iostream>
using namespace moderngekko::controls;
void put(std::span<std::uint8_t> b,std::size_t p,std::uint32_t v) {
  for(unsigned i=0;i<4;++i) b[p+i]=std::uint8_t(v>>(24-i*8));
}
struct Fixture {
  std::map<std::uint32_t,std::vector<std::uint8_t>> memory;
  Fixture() {
    memory[0x80817368].resize(4);put(memory[0x80817368],0,0x90001000);
    auto& m=memory[0x90001000];m.resize(0x1400);put(m,0,0x81177d08);
    put(m,0x13fc,0x1ff);put(m,0xd30,6);put(m,8,0x90010000);
    put(m,12,0x90020000);put(m,0xd70,1);put(m,0xd74,2);
    auto& a=memory[0x90010000];a.resize(0xa38);put(a,0x48,1);
    put(a,0x9c,0x8052a748);put(a,0x2a0,0x42480000);
    put(a,0x45c,1);put(a,0x3a8,0x90030000);put(a,0x6e8,2);
    auto& t=memory[0x90020000];t.resize(0xa0);put(t,0x48,2);put(t,0x9c,0x811769e0);
    auto& n=memory[0x90030000];n.resize(16);put(n,0,0x811b0620);put(n,12,0x0d000001);
    auto& s=memory[0x805f8828];s.resize(0x4020);put(s,0,13);
    constexpr std::string_view name="generic_sequence";
    std::copy(name.begin(),name.end(),s.begin()+0x4008);
  }
  std::span<const std::uint8_t> read(std::uint32_t a,std::size_t n) const {
    auto it=memory.upper_bound(a);if(it==memory.begin()) return {};--it;
    const auto offset=std::uint64_t(a)-it->first;
    if(offset+n>it->second.size()) return {};
    return std::span<const std::uint8_t>(it->second).subspan(offset,n);
  }
  bool eligible(unsigned port=1) const {
    return HasCoopInteraction([&](auto a,auto n){return read(a,n);},port);
  }
};
int main() {
  Fixture f;
  if(!f.eligible() || f.eligible(0) || f.eligible(4)) return 1;
  for (auto [address,offset,value] : {
      std::tuple{0x90001000u,0xd74u,0x202u}, // stale handle
      std::tuple{0x90001000u,0xd30u,2u}, // removed target
      std::tuple{0x90010000u,0x6e8u,0u}, // exited interaction
      std::tuple{0x90010000u,0x4d0u,0x80000000u}, // AI
      std::tuple{0x90010000u,0x2a0u,0u}, // dead
      std::tuple{0x90010000u,0x3a8u,0xfffffffcu}, // invalid pointer
      std::tuple{0x90020000u,0x9cu,0x8052a748u}, // ordinary actor target
      std::tuple{0x90030000u,12u,0x0c000001u}, // stale name generation
      std::tuple{0x805f8828u,0x4008u,0u}}) { // other sequence
    auto changed=f;put(changed.memory[address],offset,value);
    if(changed.eligible()) return 2;
  }
  auto duplicate=f;duplicate.memory[0x90040000]=f.memory[0x90010000];
  put(duplicate.memory[0x90040000],0x48,3);
  auto& m=duplicate.memory[0x90001000];put(m,0xd30,14);put(m,16,0x90040000);put(m,0xd78,3);
  if(duplicate.eligible()) return 3;
  std::array<std::uint8_t,DescriptorTableSize> table{};
  for(auto [id,name,selector]:{std::tuple{56u,"ShakeGesture",256u},std::tuple{58u,"LiftGesture",259u}}) {
    const auto p=id*DescriptorSize;put(table,p,id);std::copy_n(name,std::string_view(name).size(),table.begin()+p+4);
    put(table,p+0x44,1);put(table,p+0x48,selector);put(table,p+0x54,0x3e19999a);
  }
  for(unsigned input:{0u,9u,10u,11u}) {
    std::array<std::uint8_t,20> active{};std::array<std::uint8_t,496> values{};
    put(active,0,input?1u<<input:0);if(input) put(values,input*4,0x3f800000);
    auto a=active;auto v=values;
    if(ApplyInteractionButtons(false,table,active,values) || active!=a || values!=v) return 4;
    if(!ApplyInteractionButtons(true,table,active,values)) return 5;
    const auto expected=input==9?1u<<24:input==10?1u<<26:0;
    if(ReadBE(active,4)!=expected || ReadBE(active,0)!=(input==11?1u<<11:0)) return 6;
    if(ReadBE(values,56*4)!=(input==9?0x3f800000u:0u) ||
       ReadBE(values,58*4)!=(input==10?0x3f800000u:0u)) return 7;
  }
  std::array<std::uint8_t,20> active{};std::array<std::uint8_t,496> values{};
  put(active,0,(1u<<9)|(1u<<10));auto before=active;
  if(ApplyInteractionButtons(true,table,active,values) || active!=before) return 8;
  std::cout<<"Interaction context, ownership, stale-handle and button checks passed\n";
}
