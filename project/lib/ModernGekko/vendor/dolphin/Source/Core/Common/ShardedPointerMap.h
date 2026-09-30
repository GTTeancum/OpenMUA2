// Copyright 2026 OpenMUA2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace Common
{
// Single-threaded ownership, stable value addresses, and smaller rehash batches.
// Dense generated-code pointers are spread by address bits, not pointer alignment.
template <typename Value, std::size_t Shards = 64>
class ShardedPointerMap
{
  static_assert(std::has_single_bit(Shards));
  using Map = std::unordered_map<const void*, Value>;
  static std::size_t Index(const void* key)
  {
    const auto bits = reinterpret_cast<std::uintptr_t>(key);
    return ((bits >> 4) ^ (bits >> 16)) & (Shards - 1);
  }
public:
  Value& operator[](const void* key) { return m_maps[Index(key)][key]; }
  Value* Find(const void* key)
  {
    auto& map = m_maps[Index(key)];
    const auto found = map.find(key);
    return found == map.end() ? nullptr : &found->second;
  }
  void clear() { for (auto& map : m_maps) map.clear(); }
  std::size_t size() const
  {
    std::size_t count = 0;
    for (const auto& map : m_maps) count += map.size();
    return count;
  }
  std::size_t bucket_count(const void* key) const { return m_maps[Index(key)].bucket_count(); }
private:
  std::array<Map, Shards> m_maps;
};
}  // namespace Common
