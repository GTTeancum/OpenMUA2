#include "mua2_action_bindings.hpp"
#include <array>
#include <iostream>
using namespace moderngekko::controls;
using Table = std::array<std::uint8_t, DescriptorTableSize>;
void put(Table& b, std::size_t p, std::uint32_t v) {
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
  std::cout << "Transactional action binding checks passed\n";
}
