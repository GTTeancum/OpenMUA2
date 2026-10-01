#include "mua2_action_bindings.hpp"
#include <array>
#include <iostream>
using namespace moderngekko::controls;
using Table = std::array<std::uint8_t, DescriptorTableSize>;
void put(std::span<std::uint8_t> b, std::size_t p, std::uint32_t v) {
  for(int i=0;i<4;++i) b[p+i]=std::uint8_t(v>>(24-i*8));
}
Table stock() {
  Table b{};
  for(auto [id,name] : {std::pair{11,"Grab"}, {12,"Block"}, {21,"Action"}}) {
    auto p=id*DescriptorSize; put(b,p,id);
    std::copy_n(name,std::string_view(name).size(),b.begin()+p+4);
    put(b,p+0x44,id==11?2:1);
    put(b,p+0x48,id==11?3:133); put(b,p+0x54,id==11?0x3cf5c28f:0x3e19999a);
    if(id==11) {put(b,p+0x58,4);put(b,p+0x64,0x3cf5c28f);}
  }
  return b;
}
int main() {
  auto b=stock(),before=b;
  if(RebindContextUse(b)!=RebindResult::Applied) return 1;
  const auto p=21*DescriptorSize;
  for(std::size_t i=0;i<b.size();++i)
    if(i!=p+0x47 && !(i>=p+0x48 && i<p+0x68) && b[i]!=before[i]) return 2;
  if(ReadBE(b,p+0x44)!=2 || ReadBE(b,p+0x48)!=3 || ReadBE(b,p+0x58)!=4) return 3;
  auto after=b;
  if(RebindContextUse(b)!=RebindResult::AlreadyApplied || b!=after) return 4;
  for(auto offset : {11*DescriptorSize,12*DescriptorSize+4,21*DescriptorSize+4,
      11*DescriptorSize+0x44,11*DescriptorSize+0x48,11*DescriptorSize+0x4c,
      11*DescriptorSize+0x50,11*DescriptorSize+0x54,12*DescriptorSize+0x48,
      21*DescriptorSize+0x44,21*DescriptorSize+0x48}) {
    auto bad=stock();bad[offset]^=1;auto copy=bad;
    if(RebindContextUse(bad)!=RebindResult::Rejected || bad!=copy) return 5;
  }
  if(RebindContextUse(std::span(b).first(b.size()-1))!=RebindResult::Rejected) return 6;
  std::array<std::uint8_t,20> active{};
  std::array<std::uint8_t,124*4> values{};
  for (auto held : {false,true}) {
    for (auto partial : {false,true}) {
      values.fill(0xa5);
      put(active,0,held ? (1u<<11)|(1u<<21) : 0);
      put(values,21*4,held ? 0x40000000 : partial ? 0x3f800000 : 0);
      auto original=values;
      if (!NormalizeChordUse(active,values) ||
          ReadBE(values,21*4)!=(held ? 0x3f800000u : 0u)) return 7;
      for (std::size_t j=0;j<values.size();++j)
        if ((j<21*4 || j>=22*4) && values[j]!=original[j]) return 8;
    }
  }
  for (auto invalid : {0x7fc00000u,0x40400000u,0xbf800000u}) {
    put(values,21*4,invalid);auto original=values;
    if (NormalizeChordUse(active,values) || values!=original) return 9;
  }
  put(active,0,1u<<21);put(values,21*4,0x40000000);auto original=values;
  if (NormalizeChordUse(active,values) || values!=original) return 10;
  if (NormalizeChordUse(std::span(active).first(19),values) ||
      NormalizeChordUse(active,std::span(values).first(values.size()-1))) return 11;
  active.fill(0); values.fill(0);
  auto idle_active=active; auto idle_values=values;
  if (MapPauseBack(active,values) || active!=idle_active || values!=idle_values) return 12;
  put(active,4,1u<<7); put(values,39*4,0x3f800000);
  for (unsigned id : {9u,10u,11u,21u,56u,58u,103u,123u}) {
    put(active,id/32*4,ReadBE(active,id/32*4)|(1u<<(id%32)));
    put(values,id*4,0x3f800000);
  }
  if (!MapPauseBack(active,values) || ReadBE(values,90*4)!=0x3f800000 ||
      !(ReadBE(active,8)&(1u<<26)) || ReadBE(values,39*4)!=0x3f800000) return 13;
  for (unsigned id : {9u,10u,11u,21u,56u,58u,103u,123u})
    if ((ReadBE(active,id/32*4)&(1u<<(id%32))) || ReadBE(values,id*4)) return 14;
  auto mapped_active=active; auto mapped_values=values;
  if (!MapPauseBack(active,values) || active!=mapped_active || values!=mapped_values) return 15;
  if (MapPauseBack(std::span(active).first(19),values) ||
      MapPauseBack(active,std::span(values).first(495))) return 16;
  std::cout << "Transactional action binding checks passed\n";
}
