#pragma once
#include "mua2_roster_layout.hpp"

namespace moderngekko::roster {
// The native playable list includes its default entry. Menu locations are a
// separate byte-valued namespace and may be sparse; never use the hero count
// as the location-array size or as the stage-item clearing bound.
inline constexpr std::uint32_t MenuLocationCount = 256;
inline constexpr std::uint8_t EmptyMenuLocation = 255;
inline constexpr std::array MenuPatches{
    InstructionPatch{0x80f74940, 0x2c150020, 0x2c150100}, // hide pad00_focus..pad255_focus
    InstructionPatch{0x80f76798, 0x8b832ad0, 0x8b832bc0},
    InstructionPatch{0x80f783ec, 0x8b632ad0, 0x8b632bc0},
    InstructionPatch{0x80f7cfa4, 0x8ae32ad0, 0x8ae32bc0},
    InstructionPatch{0x80f7d23c, 0x8ae32ad0, 0x8ae32bc0},
    InstructionPatch{0x80f80b68, 0x38000020, 0x38000029}, // 40 heroes + default
    InstructionPatch{0x80f80cc8, 0x9ae52ad0, 0x9ae52bc0}};
static_assert(MenuLocationFlags == 11200 && PlayableHeroes + 1 == 41);
static_assert(MenuLocationFlags + MenuLocationCount <= MenuSlotBytes);
} // namespace moderngekko::roster
