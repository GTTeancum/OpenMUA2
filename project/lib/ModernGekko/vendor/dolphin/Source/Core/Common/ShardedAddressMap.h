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
// Single-owner address lookup with stable value references and smaller rehash
// batches. No full-table iteration is needed by the JIT reverse-link index.
template <typename Value, std::size_t Shards = 64>
class ShardedAddressMap
{
  static_assert(std::has_single_bit(Shards));
  using Map = std::unordered_map<std::uint32_t, Value>;
  static std::size_t Index(std::uint32_t key)
  {
    return ((key >> 2) ^ (key >> 16)) & (Shards - 1);
  }
public:
  Value& operator[](std::uint32_t key) { return m_maps[Index(key)][key]; }
  Value* Find(std::uint32_t key)
  {
    auto& map = m_maps[Index(key)];
    const auto found = map.find(key);
    return found == map.end() ? nullptr : &found->second;
  }
  std::size_t erase(std::uint32_t key) { return m_maps[Index(key)].erase(key); }
  void clear() { for (auto& map : m_maps) map.clear(); }
  std::size_t size() const
  {
    std::size_t count = 0;
    for (const auto& map : m_maps) count += map.size();
    return count;
  }
  std::size_t bucket_count() const
  {
    std::size_t count = 0;
    for (const auto& map : m_maps) count += map.bucket_count();
    return count;
  }
private:
  std::array<Map, Shards> m_maps;
};
}  // namespace Common
