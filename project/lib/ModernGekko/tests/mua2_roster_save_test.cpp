#include "mua2_roster_save.hpp"
#include <cstdlib>
#include <iostream>

namespace s = moderngekko::roster::save;
static void Require(bool ok, const char* message) {
  if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
int main() {
  s::Section fixture;
  for (unsigned i=0;i<42;++i) {
    s::NamedRecord r{"hero_"+std::to_string(i)};
    for (std::size_t b=0;b<r.bytes.size();++b) r.bytes[b]=std::uint8_t(i+b*7);
    fixture.heroes.push_back(r); fixture.snapshots.push_back(r); fixture.unlocked.push_back(r.name);
  }
  fixture.snapshot_valid=true;
  auto encoded=s::Encode(fixture,s::StreamCapacity);
  Require(bool(encoded),"40 heroes plus native entries must fit");
  const auto original=*encoded;
  auto decoded=s::Decode(original);
  Require(decoded && decoded->section==fixture && decoded->consumed==original.size(),"record identity/state round trip");
  Require(!s::Encode(fixture,original.size()-1),"remaining outer-stream space enforced before writing");
  for (std::size_t length=0;length<original.size();++length)
    Require(!s::Decode(std::span(original).first(length)),"truncated section accepted");
  encoded->insert(encoded->end(),123,0xaa);
  decoded=s::Decode(*encoded);
  Require(decoded && decoded->consumed==original.size() && decoded->section==fixture,
          "following save sections must not be consumed");
  for (auto [offset,value] : {std::pair{8u,2u}, {12u,0xffffffffu}, {16u,49u},
                            {20u,49u}, {24u,351u}, {28u,2u}}) {
    auto corrupt=original; s::Write32(corrupt,offset,value);
    Require(!s::Decode(corrupt),"unsupported version/count/flags accepted");
  }
  auto corrupt=original;
  std::fill_n(corrupt.begin()+s::HeaderBytes,s::NameBytes,'x');
  Require(!s::Decode(corrupt),"unterminated name accepted");
  corrupt=original;
  std::copy_n(corrupt.begin()+s::HeaderBytes,s::NameBytes,
              corrupt.begin()+s::HeaderBytes+s::NameBytes+s::RecordBytes);
  Require(!s::Decode(corrupt),"duplicate hero identity accepted");
  fixture.heroes.back().name=fixture.heroes.front().name;
  Require(!s::Encode(fixture,s::StreamCapacity),"duplicate identity written");
  fixture.heroes.back().name=std::string(32,'x');
  Require(!s::Encode(fixture,s::StreamCapacity),"oversized identity written");
  const auto first_marker=s::RetailNames.size()*s::RecordBytes;
  const auto unlocked=first_marker+s::MarkerBytes, second_marker=unlocked+704;
  const auto flag=second_marker+s::MarkerBytes;
  std::vector<std::uint8_t> retail(flag+1+33*s::RecordBytes);
  for(auto [at,label]:{std::pair{first_marker,std::string_view("[CHARACTERSUNLOCKED]")},
                      {second_marker,std::string_view("[CHARACTERSRESTORE]")}}) {
    retail[at]=retail[at+257]='\n';
    std::copy(label.begin(),label.end(),retail.begin()+at+1);
  }
  retail[0]=81;retail[flag+1+s::RecordBytes]=72;
  retail[unlocked+1]=4;s::Write32(retail,unlocked+700,1);
  auto migrated=s::DecodeRetail(retail);
  Require(migrated && migrated->consumed==retail.size() &&
          migrated->section.heroes.front().name=="default" &&
          migrated->section.heroes.front().bytes[0]==81 &&
          migrated->section.snapshots.front().bytes[0]==72 &&
          migrated->section.unlocked==std::vector<std::string>{"CaptainAmerica"},
          "retail identity and snapshot index migration");
  for(std::size_t n=0;n<retail.size();++n)
    Require(!s::DecodeRetail(std::span(retail).first(n)),"truncated retail section accepted");
  retail[unlocked+1]=30;
  Require(!s::DecodeRetail(retail),"unknown retail unlock identity accepted");
  const std::vector<std::string> npc_names{"mission_npc"};
  migrated=s::DecodeRetail(retail,npc_names);
  Require(migrated && migrated->section.unlocked==std::vector<std::string>{"mission_npc"},
          "mission NPC unlocks must migrate separately from hero-state records");
  retail[unlocked+1]=4;retail[first_marker+1]='?';
  Require(!s::DecodeRetail(retail),"unrecognized unversioned roster accepted");
  std::cout << "roster save framing, 40-hero state, bounds and corrupt-input checks passed\n";
}
