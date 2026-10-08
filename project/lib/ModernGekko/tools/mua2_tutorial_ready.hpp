#pragma once
#include "mua2_action_bindings.hpp"
#include <optional>

namespace moderngekko::controls {
// Called only after the native action-9 rising-edge query succeeds. The native
// player/profile loop supplies the packed ready slot; it is not the port index.
inline std::optional<unsigned> TutorialReadySlot(
    std::span<const std::uint8_t> help, unsigned port, unsigned slot,
    bool single_icon) {
  if (help.size()!=80 || port>=4 ||
      ReadBE(help,24)!=0x80564248 || help[20]!=1 ||
      help[64]!=0 || help[65]!=1 || help[66]!=0 || help[67]>4 ||
      help[68]!=(single_icon?1:0)) return std::nullopt;
  const auto owner=ReadBE(help,76);
  if (owner!=0xffffffff && owner!=port) return std::nullopt;
  if (slot<32 || slot>56 || (slot-32)%8 || (single_icon && slot!=32))
    return std::nullopt;
  return slot;
}
// Hide a pointer-only warning only when every joined profile uses our gamepad
// mapping. Other warning classes and mixed/custom configurations stay intact.
inline bool GamepadOnlyProfiles(unsigned managed,std::span<const std::uint8_t> profiles) {
  if (profiles.size()!=16) return false;
  bool any=false;
  for(unsigned port=0;port<4;++port) {
    if (!ReadBE(profiles,port*4)) continue;
    if (!(managed & (1u<<port))) return false;
    any=true;
  }
  return any;
}
}
