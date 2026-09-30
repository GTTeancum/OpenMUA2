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
  return map.size() == 1000 ? 0 : 7;
}
