#pragma once
#include <algorithm>
#include <cstdint>
#include <array>
#include <charconv>
#include <optional>
#include <filesystem>
#include <fstream>
#include <set>
#include <vector>
#include <string>
#include <string_view>

namespace moderngekko::controls {
// Herostat slot order, not physical model number order. The native saved byte
// remains the selected zero-based slot. Extra definitions live outside the
// six-entry retail object, so skin_07..10 cannot overwrite adjacent fields.
struct CostumeSet {
  std::array<std::string,10> skins{};
  static std::optional<unsigned> Slot(std::string_view key) {
    if(key=="skin")return 0;
    if(key.size()!=7 || key.substr(0,5)!="skin_")return {};
    unsigned slot=0;auto [end,ec]=std::from_chars(key.data()+5,key.data()+7,slot);
    if(ec!=std::errc{} || end!=key.data()+7 || slot<2 || slot>10)return {};
    return slot-1;
  }
  bool Set(std::string_view key,std::string_view value) {
    auto slot=Slot(key);if(!slot)return false;
    // Native actor/package identifiers are numeric. Preserve leading zeroes.
    if(value.empty() || value.size()>5 || value.find_first_not_of("0123456789")!=value.npos) {
      skins[*slot].clear();return true;
    }
    unsigned number=0;std::from_chars(value.data(),value.data()+value.size(),number);
    skins[*slot]=number?std::string(value):std::string{};return true;
  }
  std::string Model(unsigned slot) const {
    if(slot>=skins.size() || skins[slot].empty())return {};
    if(slot && skins[slot].size()<=2) {
      const auto base=Model(0);
      if(base.size()>=4)
        return base.substr(0,base.size()-2)+(skins[slot].size()==1?"0":"")+skins[slot];
    }
    if(slot==0 && skins[0].size()<=3)
      return (skins[0].size()==1?"0":"")+skins[0]+"01";
    return skins[slot];
  }
  template<class Available> unsigned Next(unsigned current,Available available) const {
    for(unsigned i=1;i<=skins.size();++i) {
      const unsigned slot=((current<skins.size()?current:0)+i)%skins.size();
      const auto model=Model(slot);
      if(!model.empty() && (slot==0 || available(model)))return slot;
    }
    return 0;
  }
};
// Read archive names only; never extract or rewrite installed game data.
inline void ReadCostumeArchiveNames(const std::filesystem::path& path,std::set<std::string>& names) {
  std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f)return;
  const auto length=f.tellg();if(length<22)return;
  const auto count=static_cast<std::size_t>(std::min<std::streamoff>(length,65557));
  std::vector<unsigned char> tail(count);f.seekg(length-static_cast<std::streamoff>(count));
  if(!f.read(reinterpret_cast<char*>(tail.data()),count))return;
  const auto word=[](const unsigned char* p) {return unsigned(p[0])|(unsigned(p[1])<<8);};
  const auto dword=[&](const unsigned char* p) {return word(p)|(word(p+2)<<16);};
  for(std::size_t pos=count-22;;--pos) {
    if(dword(tail.data()+pos)==0x06054b50 && pos+22+word(tail.data()+pos+20)==count) {
      const auto central=dword(tail.data()+pos+16),size=dword(tail.data()+pos+12);
      if(std::uint64_t(central)+size>std::uint64_t(length) || size>32*1024*1024)return;
      f.seekg(central);std::vector<unsigned char> data(size);
      if(!f.read(reinterpret_cast<char*>(data.data()),size))return;
      for(std::size_t at=0;at+46<=data.size();) {
        const auto* p=data.data()+at;if(dword(p)!=0x02014b50)return;
        const auto n=word(p+28),extra=word(p+30),comment=word(p+32);
        if(at+46+n+extra+comment>data.size())return;
        names.emplace(reinterpret_cast<const char*>(p+46),n);at+=46+n+extra+comment;
      }
      return;
    }
    if(pos==0)return;
  }
}
} // namespace moderngekko::controls
