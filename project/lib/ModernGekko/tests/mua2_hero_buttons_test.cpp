#include "mua2_hero_buttons.hpp"
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
  auto candidate(std::uint32_t who=owner,std::uint32_t port=1,std::uint32_t pad=input) const {
    return HeroButtonIndex([&](auto a,auto n){return read(a,n);},who,port,pad,0x80800000);
  }
};
int main() {
  Fixture f;
  auto& array=f.memory[0x80800000];array.resize(16);
  for(unsigned i=0;i<4;++i) put(array,4*i,0x90010000+i*0x1000);
  auto& device=f.memory[Fixture::input];
  for(unsigned id:{14u,29u,30u,31u,32u}) put(device,0xbb20+4*id,0);
  put(device,0xbb20+13*4,0x3f800000);
  constexpr unsigned directions[]{32,31,29,30}, opposite[]{29,30,32,31};
  for(unsigned i=0;i<4;++i) {
    auto sample=f;auto& pad=sample.memory[Fixture::input];
    put(pad,0xbb20+4*directions[i],0x3f800000);
    put(pad,0xbb20+4*opposite[i],0xbf800000);
    if(sample.candidate()!=i) return 1;
  }
  put(device,0xbb20+32*4,0x3f800000);
  if(f.candidate()!=0 || f.candidate(Fixture::owner,0) ||
      f.candidate(Fixture::owner,1,0x81313274)) return 2;
  for(auto [a,o,v]:{
      std::tuple{0x80800000u,0u,0x90011000u}, // reordered transient array
      std::tuple{0x80629490u,0x73cu,5u},
      std::tuple{0x80629490u,0x72cu,0x202u},
      std::tuple{0x90001000u,0xd70u,0x401u},
      std::tuple{0x90001000u,0xd30u,28u},
      std::tuple{0x90010000u,0x48u,0x401u},
      std::tuple{0x90010000u,0x9cu,0u},
      std::tuple{0x90010000u,0x4d0u,0u}, // duplicate human owner
      std::tuple{Fixture::input,0xbb20u+14*4,0x3f800000u},
      std::tuple{Fixture::input,0xbb20u+31*4,0x3f800000u}}) {
    auto bad=f;put(bad.memory[a],o,v);if(bad.candidate()!=4) return 3;
  }
  auto dead=f;put(dead.memory[0x90010000],0x2a0,0);
  if(dead.candidate()!=0) return 4; // native helper must reject; no fallback index
  auto missing=f;missing.memory.erase(0x80800000);if(missing.candidate()!=4) return 5;
  auto released=f;put(released.memory[Fixture::input],0xbb20+13*4,0);
  if(released.candidate()) return 6;
  std::array<std::uint8_t,20> active{};
  put(active,0,(1u<<13)|(1u<<9)|(1u<<29));put(active,4,1u<<2);put(active,8,1u<<27);put(active,12,(1u<<9)|(1u<<22));
  const auto values=std::span<const std::uint8_t>(device).subspan(0xbb20,496);
  ConsumeHeroMarkers(active,values);
  if(ReadBE(active,0)!=((1u<<13)|(1u<<9)) || ReadBE(active,4)!=0 ||
      ReadBE(active,8)!=(1u<<27) || ReadBE(active,12)!=0) return 7; // menu up and attacks survive
  std::cout << "Direct hero index, ownership, stale-array and marker checks passed\n";
}
