#include "mua2_interaction_context.hpp"
#include "mua2_wave_qte_context.hpp"
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
  explicit Fixture(std::string_view name="generic_sequence") {
    memory[0x80817368].resize(4);put(memory[0x80817368],0,0x90001000);
    auto& m=memory[0x90001000];m.resize(0x1400);put(m,0,0x81177d08);
    put(m,0x13fc,0x1ff);put(m,0xd30,6);put(m,8,0x90010000);
    put(m,12,0x90020000);put(m,0xd70,1);put(m,0xd74,2);
    auto& a=memory[0x90010000];a.resize(0xa38);put(a,0x48,1);
    put(a,0x9c,0x8052a748);put(a,0x2a0,0x42480000);
    put(a,0x45c,1);put(a,0x3a8,0x90030000);put(a,0x6e8,2);
    auto& t=memory[0x90020000];t.resize(0xa0);put(t,0x48,2);put(t,0x9c,0x811769e0);
    auto& n=memory[0x90030000];n.resize(16);put(n,0,0x811b0620);put(n,12,0x0d000001);
    auto& s=memory[0x805f8828];s.resize(0x4040);put(s,0,13);
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
int CheckFixture(Fixture f) {
  if(!f.eligible() || f.eligible(0) || f.eligible(4)) return 1;
  CoopInteraction resolved;
  if (!HasCoopInteraction([&](auto a, auto n) { return f.read(a, n); }, 1, &resolved) ||
      resolved != CoopInteraction{0x90010000, 1, 0x90020000, 2}) return 20;
  if (HasCoopInteraction([&](auto a, auto n) { return f.read(a, n); }, 4, &resolved) ||
      resolved != CoopInteraction{}) return 21;

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
  for (unsigned input : {0u, 9u, 10u, 11u}) {
    std::array<std::uint8_t,20> active{};
    std::array<std::uint8_t,496> values{};
    if (input) { put(active,0,1u<<input); put(values,input*4,0x3f800000); }
    put(active,4,(1u<<24)|(1u<<26)); // Existing motion bits must be removed.
    put(values,56*4,0x3f800000); put(values,58*4,0x3f800000);
    auto down=ConsumeButtonQteInput(active,values);
    if (!down || *down!=(input==11)) return 4;
    if (ReadBE(active,4) || ReadBE(values,56*4) || ReadBE(values,58*4)) return 5;
    if (ReadBE(active,0)!=(input && input!=11 ? 1u<<input : 0)) return 6;
    if (ReadBE(values,11*4)) return 7;
    if (input==9 || input==10)
      if (ReadBE(values,input*4)!=0x3f800000) return 8;
  }
  std::array<std::uint8_t,20> active{};
  std::array<std::uint8_t,496> values{};
  put(active,0,(1u<<11)|(1u<<21));
  put(values,11*4,0x3f800000); put(values,21*4,0x3f800000);
  if (ConsumeButtonQteInput(active,values)!=true || ReadBE(active,0) ||
      ReadBE(values,21*4)) return 9;
  put(active,0,1u<<11); // Chord magnitudes must not suppress the digital edge.
  put(values,11*4,0x40000000);
  if (ConsumeButtonQteInput(active,values)!=true || ReadBE(active,0) ||
      ReadBE(values,11*4)) return 10;
  if (ConsumeButtonQteInput(std::span(active).first(19),values) ||
      ConsumeButtonQteInput(active,std::span(values).first(495))) return 11;
  std::cout<<"Interaction ownership, handles and direct X consumption passed\n";
  return 0;
}
int CheckWaveActors() {
  for (auto name : {"grab_struggle_attacker", "power_smash_challenge_victim"}) {
    Fixture f{name};
    const auto eligible = [](const Fixture& fixture, unsigned port = 1) {
      return FindWaveQteActor([&](auto a, auto n) { return fixture.read(a, n); }, port);
    };
    WaveQteActor actor;
    if (!FindWaveQteActor([&](auto a, auto n) { return f.read(a, n); }, 1, &actor) ||
        actor != WaveQteActor{0x90010000, 1, 0x90030000, 0x0d000001} ||
        eligible(f, 0) || eligible(f, 4) || f.eligible()) return 30;
    for (auto [address, offset, value] : {
        std::tuple{0x90001000u, 0xd70u, 0x201u},
        std::tuple{0x90001000u, 0xd30u, 4u},
        std::tuple{0x90010000u, 0x4d0u, 0x80000000u},
        std::tuple{0x90010000u, 0x2a0u, 0u},
        std::tuple{0x90010000u, 0x2a0u, 0x7fc00000u},
        std::tuple{0x90010000u, 0x3a8u, 0xfffffffcu},
        std::tuple{0x90030000u, 12u, 0x0c000001u},
        std::tuple{0x805f8828u, 8u, 0xffffffffu}}) {
      auto changed = f; put(changed.memory[address], offset, value);
      if (eligible(changed)) return 31;
    }
    auto duplicate = f; duplicate.memory[0x90040000] = f.memory[0x90010000];
    put(duplicate.memory[0x90040000], 0x48, 3);
    auto& m = duplicate.memory[0x90001000];
    put(m, 0xd30, 14); put(m, 16, 0x90040000); put(m, 0xd78, 3);
    if (eligible(duplicate)) return 32;
    // Wave challenges do not require a CCoopEntity target.
    put(f.memory[0x90010000], 0x6e8, 0);
    if (!eligible(f)) return 33;
    if (FindWaveQteActor([&](auto a, auto n) { return f.read(a, n); }, 4, &actor) ||
        actor != WaveQteActor{}) return 34;
  }
  for (auto name : {"generic_sequence", "grab_struggle_attacker_extra",
                    "power_smash_challenge_victi", "idle"}) {
    Fixture f{name};
    if (FindWaveQteActor([&](auto a, auto n) { return f.read(a, n); }, 1)) return 35;
  }
  return 0;
}
int main() {
  if (const int result = CheckWaveActors()) return result;
  for (auto name : {"generic_sequence", "electro_sequence"}) {
    const int result = CheckFixture(Fixture{name});
    if (result) { std::cerr << name << ": " << result << '\n'; return result; }
  }
  for (auto name : {"generic_sequenc", "electro_sequenc", "electro_sequence_extra",
                    "Electro_sequence", "other_sequence"})
    if (Fixture{name}.eligible()) return 22;
  return 0;
}
