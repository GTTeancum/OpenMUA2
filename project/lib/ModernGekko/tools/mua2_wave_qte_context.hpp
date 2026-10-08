#pragma once
#include "mua2_action_bindings.hpp"
#include <bit>
#include <cmath>

namespace moderngekko::controls {
struct WaveQteActor {
  std::uint32_t actor = 0, handle = 0, node = 0, name_tag = 0;
  bool operator==(const WaveQteActor&) const = default;
};

// Resolve only a live, uniquely owned actor in one of the verified wave moves.
// The runtime must additionally validate the live tap-data object and code.
template <typename Reader>
bool FindWaveQteActor(Reader read, unsigned port, WaveQteActor* result = nullptr) {
  if (result) *result = {};
  if (port >= 4) return false;
  const auto ram = [&](std::uint32_t address, std::size_t size) {
    const auto end = std::uint64_t(address) + size;
    if (!(address & 3) && ((address >= 0x80000000 && end <= 0x81800000) ||
                          (address >= 0x90000000 && end <= 0x94000000)))
      return read(address, size);
    return std::span<const std::uint8_t>{};
  };
  const auto global = ram(0x80817368, 4);
  if (global.size() != 4) return false;
  const auto manager = ram(ReadBE(global, 0), 0x1400);
  if (manager.size() != 0x1400 || ReadBE(manager, 0) != 0x81177d08 ||
      ReadBE(manager, 0x13fc) != 0x1ff) return false;
  unsigned owners = 0;
  WaveQteActor found;
  for (unsigned i = 0; i < 420; ++i) {
    if (!(ReadBE(manager, 0xd30 + 4 * (i / 32)) & (1u << (i % 32)))) continue;
    const auto handle = ReadBE(manager, 0xd6c + 4 * i);
    const auto address = ReadBE(manager, 4 + 4 * i);
    if (!handle || (handle & 0x1ff) != i || address > 0xfffff5c7u) return false;
    const auto type = ram(address + 0x9c, 4);
    if (type.size() != 4) return false;
    if (ReadBE(type, 0) != 0x8052a748) continue;
    const auto actor = ram(address, 0xa38);
    if (actor.size() != 0xa38 || ReadBE(actor, 0x48) != handle) return false;
    if ((actor[0x4d0] & 0x80) || ReadBE(actor, 0x45c) != port) continue;
    if (++owners != 1) return false;
    const float health = std::bit_cast<float>(ReadBE(actor, 0x2a0));
    if (!std::isfinite(health) || health <= 0) continue;
    const auto node_address = ReadBE(actor, 0x3a8);
    const auto node = ram(node_address, 16);
    if (node.size() != 16 || ReadBE(node, 0) != 0x811b0620) continue;
    const auto strings = ram(0x805f8828, 0x4008);
    if (strings.size() != 0x4008) continue;
    const auto generation = ReadBE(strings, 0), tag = ReadBE(node, 12);
    const auto index = tag & 0xffffff;
    if (!generation || generation > 255 || tag >> 24 != generation || index >= 4096)
      continue;
    const auto name_address = std::uint64_t(0x805f8828) + 0x4008 +
                              ReadBE(strings, 4 + index * 4);
    for (std::string_view name : {"grab_struggle_attacker", "power_smash_challenge_victim"}) {
      const auto end = name_address + name.size() + 1;
      if (name_address < 0x80000000 || end > 0x81800000) continue;
      // Interned strings need not be word aligned.
      const auto text = read(static_cast<std::uint32_t>(name_address), name.size() + 1);
      if (text.size() == name.size() + 1 && text.back() == 0 &&
          std::equal(name.begin(), name.end(), text.begin()))
        found = {address, handle, node_address, tag};
    }
  }
  if (owners != 1 || !found.actor) return false;
  if (result) *result = found;
  return true;
}
}
