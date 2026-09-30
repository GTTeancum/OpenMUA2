#include "../vendor/dolphin/Source/Core/Common/ShardedAddressMap.h"
#include <unordered_set>
#include "../vendor/dolphin/Source/Core/Common/ShardedPointerMap.h"
#include <cstdint>
#include <vector>

int main()
{
  Common::ShardedPointerMap<std::uint64_t> map;
  // Real addresses with dense and page-spaced keys; force repeated shard growth.
  std::vector<unsigned char> storage(300000 * 17);
  auto* first = &map[storage.data()];
  *first = 123;
  for (std::size_t i = 1; i < 300000; ++i) map[storage.data() + i * 17] = i * 7;
  if (map.size() != 300000 || first != map.Find(storage.data()) || *first != 123) return 1;
  for (std::size_t i = 1; i < 300000; ++i)
    if (!map.Find(storage.data() + i * 17) || *map.Find(storage.data() + i * 17) != i * 7) return 2;
  if (map.Find(storage.data() + 1)) return 3;
  map[storage.data() + 17] = 999;
  if (*map.Find(storage.data() + 17) != 999 || map.size() != 300000) return 4;
  map.clear();
  if (map.size() || map.Find(storage.data())) return 5;
  for (std::size_t i = 0; i < 1000; ++i) map[storage.data() + i * 4096] = i;
  for (std::size_t i = 0; i < 1000; ++i)
    if (*map.Find(storage.data() + i * 4096) != i) return 6;
  if (map.size() != 1000) return 7;
  Common::ShardedAddressMap<std::unordered_set<unsigned>> links;
  std::unordered_map<std::uint32_t, std::unordered_set<unsigned>> reference;
  auto* stable = &links[0x80000000];
  stable->insert(77);
  for (unsigned i = 1; i < 100000; ++i) links[0x80000000 + i * 4].insert(i);
  if (links.Find(0x80000000) != stable || !stable->contains(77)) return 8;
  for (unsigned i = 1; i < 100000; ++i)
    if (!links.Find(0x80000000 + i * 4)->contains(i)) return 9;
  links.clear();
  // Reverse-link insertion, duplicate exits, removal and empty-key erasure,
  // checked against the old standard container over dense and sparse keys.
  std::uint32_t seed = 42;
  for (unsigned i = 0; i < 200000; ++i)
  {
    seed = seed * 1664525u + 1013904223u;
    const std::uint32_t key = 0x80000000u + ((seed >> 8) & 2047u) * (i % 2 ? 4u : 65536u);
    const unsigned source = (seed >> 22) & 31u;
    if (i % 3)
    {
      links[key].insert(source);
      reference[key].insert(source);
    }
    else
    {
      if (auto* value = links.Find(key))
      {
        value->erase(source);
        if (value->empty()) links.erase(key);
      }
      auto found = reference.find(key);
      if (found != reference.end())
      {
        found->second.erase(source);
        if (found->second.empty()) reference.erase(found);
      }
    }
    const auto found = reference.find(key);
    const auto* value = links.Find(key);
    if ((found == reference.end()) != (value == nullptr)) return 10;
    if (value && *value != found->second) return 11;
  }
  if (links.size() != reference.size()) return 12;
  for (const auto& [key, value] : reference)
    if (!links.Find(key) || *links.Find(key) != value) return 13;
  links.clear();
  if (links.size() || links.Find(0x80000000) || links.erase(0x80000000)) return 14;
  links[0xffffffff].insert(99);
  return links.Find(0xffffffff)->contains(99) ? 0 : 15;
}
