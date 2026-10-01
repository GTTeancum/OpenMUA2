#include "managed_xbox_profile.hpp"
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
  for (auto camera_x : {0xbf800000u,0u,0x3f800000u}) {
    active.fill(0xff); values.fill(0xa5);
    put(values,7*4,0x3f800000); put(values,2*4,camera_x);
    auto original_active=active; auto original_values=values;
    if (!ConsumeCameraMenuAliases(active,values)) return 17;
    for (unsigned id=0;id<124;++id) {
      const bool alias=id==99 || id==104 || id==122;
      const bool bit=(ReadBE(active,id/32*4)&(1u<<(id%32)))!=0;
      if (bit==alias || ReadBE(values,id*4)!=(alias?0:ReadBE(original_values,id*4))) return 18;
    }
    active=original_active;values=original_values;put(values,7*4,0);
    auto rejected=values;
    if (ConsumeCameraMenuAliases(active,values) || active!=original_active || values!=rejected) return 19;
    put(values,7*4,0x3f800000);put(active,0,ReadBE(active,0)&~(1u<<7));
    original_active=active;
    if (ConsumeCameraMenuAliases(active,values) || active!=original_active || values!=original_values) return 20;
  }
  if (ConsumeCameraMenuAliases(std::span(active).first(19),values) ||
      ConsumeCameraMenuAliases(active,std::span(values).first(495))) return 21;
  active.fill(0);values.fill(0);
  for (unsigned id:{7u,39u,99u,103u,104u,122u,123u}) {
    put(active,id/32*4,ReadBE(active,id/32*4)|(1u<<(id%32)));
    put(values,id*4,0x3f800000);
  }
  auto back_active=active;auto back_values=values;
  for (unsigned missing:{7u,39u}) {
    active=back_active;values=back_values;
    put(active,missing/32*4,ReadBE(active,missing/32*4)&~(1u<<(missing%32)));
    auto unchanged=active;
    if(MapHeroManagement(active,values) || active!=unchanged || values!=back_values) return 22;
  }
  active=back_active;values=back_values;
  if(!MapHeroManagement(active,values) || ReadBE(active,0)!=0 ||
      ReadBE(active,4)!=(1u<<8) || ReadBE(active,12)!=0 ||
      ReadBE(values,40*4)!=0x3f800000) return 23;
  for(unsigned id:{7u,39u,99u,103u,104u,122u,123u})
    if(ReadBE(values,id*4)) return 24;
  if(MapPauseBack(active,values) || ConsumeCameraMenuAliases(active,values)) return 25;
  if(MapHeroManagement(std::span(active).first(19),values) ||
     MapHeroManagement(active,std::span(values).first(495))) return 26;
  const std::string managed="[Wiimote1]\nDevice = SDL/0/Test\n"+std::string(XboxBodyV5);
  if(ManagedXboxPorts(managed)!=1 || ManagedXboxPorts(managed+"Buttons/A = Other\n")!=0) return 27;
  auto changed=managed;auto at=changed.find("Dead Zone = 15.0");
  changed.replace(at,16,"Dead Zone = 22.0");
  if(ManagedXboxPorts(changed)!=0) return 28;
  const std::string second="[Wiimote2]\nDevice = SDL/1/Test\n"+std::string(XboxBodyV5);
  if(ManagedXboxPorts(changed+second)!=2 || ManagedXboxPorts(managed+second)!=3) return 29;
  if(!XboxPortEnabled(2,0x81313274+0xbe00) || XboxPortEnabled(2,0x81313274) ||
     XboxPortEnabled(15,0x81313275) || XboxPortEnabled(15,0x81313274+4*0xbe00)) return 30;
  std::cout << "Transactional action binding checks passed\n";
}
