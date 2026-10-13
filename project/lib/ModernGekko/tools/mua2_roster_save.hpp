#pragma once
#include "mua2_roster_layout.hpp"
#include <algorithm>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace moderngekko::roster::save {
// Native hero serialization is 24 + 544 + 48 + 88 + 1 bytes. It excludes
// vtables, resource pointers, and runtime-only fields. Keep its byte order.
inline constexpr std::size_t RecordBytes = 705, NameBytes = 32, HeaderBytes = 32;
inline constexpr std::array<std::uint8_t, 8> Magic{'O','M','U','A','2','R','4','0'};
inline constexpr std::uint32_t Version = 1;
inline constexpr std::size_t StreamCapacity = 0x38000;
using Record = std::array<std::uint8_t, RecordBytes>;
struct NamedRecord {
  std::string name;
  Record bytes{};
  bool operator==(const NamedRecord&) const = default;
};
struct Section {
  std::vector<NamedRecord> heroes, snapshots;
  std::vector<std::string> unlocked;
  bool snapshot_valid = false;
  bool operator==(const Section&) const = default;
};
struct Decoded { Section section; std::size_t consumed; };

inline bool ValidName(const std::string& name) {
  return !name.empty() && name.size() < NameBytes &&
         std::all_of(name.begin(), name.end(), [](unsigned char c) { return c >= 32 && c < 127; });
}
inline std::uint32_t Read32(std::span<const std::uint8_t> b, std::size_t at) {
  return (std::uint32_t(b[at]) << 24) | (std::uint32_t(b[at+1]) << 16) |
         (std::uint32_t(b[at+2]) << 8) | b[at+3];
}
inline void Write32(std::span<std::uint8_t> b, std::size_t at, std::uint32_t value) {
  for (unsigned i=0; i<4; ++i) b[at+i] = std::uint8_t(value >> (24-i*8));
}
inline bool Valid(const Section& s) {
  if (s.heroes.size() > Expanded.snapshots_per_bank ||
      s.snapshots.size() > Expanded.snapshots_per_bank || s.unlocked.size() > DefinitionCapacity) return false;
  for (const auto* records : {&s.heroes, &s.snapshots}) {
    std::set<std::string> names;
    for (const auto& r : *records) if (!ValidName(r.name) || !names.insert(r.name).second) return false;
  }
  std::set<std::string> names;
  for (const auto& name : s.unlocked) if (!ValidName(name) || !names.insert(name).second) return false;
  return true;
}
inline std::optional<std::vector<std::uint8_t>> Encode(const Section& s, std::size_t available) {
  if (!Valid(s)) return {};
  const auto size = HeaderBytes + (s.heroes.size()+s.snapshots.size())*(NameBytes+RecordBytes) +
                    s.unlocked.size()*NameBytes;
  if (size > available || size >= StreamCapacity) return {};
  std::vector<std::uint8_t> out(size);
  std::copy(Magic.begin(), Magic.end(), out.begin());
  Write32(out,8,Version); Write32(out,12,std::uint32_t(size));
  Write32(out,16,std::uint32_t(s.heroes.size())); Write32(out,20,std::uint32_t(s.snapshots.size()));
  Write32(out,24,std::uint32_t(s.unlocked.size())); Write32(out,28,s.snapshot_valid ? 1 : 0);
  std::size_t at = HeaderBytes;
  const auto name = [&](const std::string& n) {
    std::copy(n.begin(),n.end(),out.begin()+at); at += NameBytes;
  };
  for (const auto* records : {&s.heroes,&s.snapshots}) for (const auto& r : *records) {
    name(r.name); std::copy(r.bytes.begin(),r.bytes.end(),out.begin()+at); at += RecordBytes;
  }
  for (const auto& n : s.unlocked) name(n);
  return out;
}
inline std::optional<Decoded> Decode(std::span<const std::uint8_t> data) {
  if (data.size() < HeaderBytes || !std::equal(Magic.begin(),Magic.end(),data.begin()) ||
      Read32(data,8) != Version) return {};
  const auto size = Read32(data,12), heroes = Read32(data,16), snapshots = Read32(data,20),
             unlocked = Read32(data,24), flags = Read32(data,28);
  if (heroes > Expanded.snapshots_per_bank || snapshots > Expanded.snapshots_per_bank ||
      unlocked > DefinitionCapacity || flags > 1) return {};
  const std::size_t expected = HeaderBytes + (heroes+snapshots)*(NameBytes+RecordBytes) + unlocked*NameBytes;
  if (size != expected || size > data.size() || size >= StreamCapacity) return {};
  Decoded result{}; result.consumed = size; result.section.snapshot_valid = flags != 0;
  std::size_t at = HeaderBytes;
  const auto name = [&]() -> std::optional<std::string> {
    const auto bytes = data.subspan(at,NameBytes); at += NameBytes;
    const auto end = std::find(bytes.begin(),bytes.end(),0);
    if (end == bytes.end() || !std::all_of(end,bytes.end(),[](auto c) { return c == 0; })) return {};
    std::string n(bytes.begin(),end); if (!ValidName(n)) return {}; return n;
  };
  for (auto [records,count] : {std::pair{&result.section.heroes,heroes},
                             std::pair{&result.section.snapshots,snapshots}}) {
    records->reserve(count);
    for (std::uint32_t i=0;i<count;++i) {
      auto n=name(); if (!n) return {};
      NamedRecord r{*n}; std::copy_n(data.begin()+at,RecordBytes,r.bytes.begin()); at += RecordBytes;
      records->push_back(std::move(r));
    }
  }
  for (std::uint32_t i=0;i<unlocked;++i) { auto n=name(); if (!n) return {}; result.section.unlocked.push_back(*n); }
  if (!Valid(result.section)) return {};
  return result;
}

// The retail stream contains no roster identities. Only the verified stock
// order can be migrated; a mod's unversioned order cannot be inferred safely.
inline constexpr std::array<std::string_view,29> RetailNames{
  "default","edit","Blade","CaptainAmerica","Daredevil","LukeCage","Ironman","Psylocke",
  "Spiderman","NickFury","Iceman","Songbird","HumanTorch","Thor","Deadpool","Penance",
  "GreenGoblin","Venom","Hulk","InvisibleWoman","MrFantastic","Storm","Thing","Warbird",
  "Wolverine","Gambit","Cyclops","Jean_Grey","team_character"};
inline constexpr std::size_t MarkerBytes=258;
inline bool RetailMarker(std::span<const std::uint8_t> data,std::size_t at,std::string_view label) {
  return at+MarkerBytes<=data.size() && data[at]=='\n' && data[at+257]=='\n' &&
         std::equal(label.begin(),label.end(),data.begin()+at+1) && data[at+1+label.size()]==0;
}
inline std::optional<Decoded> DecodeRetail(std::span<const std::uint8_t> data,
                                          std::span<const std::string> npc_names = {}) {
  constexpr auto unlocked_at=RetailNames.size()*RecordBytes+MarkerBytes;
  constexpr auto restore_at=unlocked_at+704;
  constexpr auto flag_at=restore_at+MarkerBytes;
  constexpr auto bytes=flag_at+1+33*RecordBytes;
  if(data.size()<bytes || !RetailMarker(data,unlocked_at-MarkerBytes,"[CHARACTERSUNLOCKED]") ||
     !RetailMarker(data,restore_at,"[CHARACTERSRESTORE]") || data[flag_at]>1) return {};
  Decoded out{};out.consumed=bytes;out.section.snapshot_valid=data[flag_at]!=0;
  for(std::size_t i=0;i<RetailNames.size();++i) {
    NamedRecord hero{std::string(RetailNames[i])},snapshot{hero.name};
    std::copy_n(data.begin()+i*RecordBytes,RecordBytes,hero.bytes.begin());
    // Snapshot zero is the native null definition; the first hero is index one.
    std::copy_n(data.begin()+flag_at+1+(i+1)*RecordBytes,RecordBytes,snapshot.bytes.begin());
    out.section.heroes.push_back(std::move(hero));out.section.snapshots.push_back(std::move(snapshot));
  }
  const auto count=Read32(data,unlocked_at+700);
  if(count>DefinitionCapacity || npc_names.size()+RetailNames.size()>=DefinitionCapacity)return {};
  for(std::size_t i=0;i<count;++i) {
    const auto at=unlocked_at+i*2;
    const unsigned id=(unsigned(data[at])<<8)|data[at+1];
    if(!id || id>RetailNames.size()+npc_names.size())return {};
    if(id<=RetailNames.size())out.section.unlocked.emplace_back(RetailNames[id-1]);
    else out.section.unlocked.push_back(npc_names[id-RetailNames.size()-1]);
  }
  if(!Valid(out.section))return {};
  return out;
}
} // namespace moderngekko::roster::save
