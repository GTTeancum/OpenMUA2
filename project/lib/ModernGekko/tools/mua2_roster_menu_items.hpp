#pragma once
#include "mua2_roster_layout.hpp"
#include "mua2_roster_menu_objects.hpp"

namespace moderngekko::roster {
// Separate guest-heap storage preserves every derived CMenu object offset.
// Nodes and values remain in the native red-black tree representation.
inline constexpr std::uint32_t MenuItems = 320;
inline constexpr std::uint32_t MenuMapPointer = 5308;
inline constexpr std::uint32_t MenuMapQueue = 5132, MenuMapHead = 6412;
inline constexpr std::uint32_t MenuMapTail = 6416, MenuMapFree = 6420;
inline constexpr std::uint32_t MenuMapBitmap = 6424, MenuMapCount = 6464;
inline constexpr std::uint32_t MenuMapValues = 6468, MenuMapBytes = 7748;
static_assert(12 + MenuItems * 16 == MenuMapQueue);
static_assert(MenuMapQueue + MenuItems * 4 == MenuMapHead);
static_assert(MenuMapBitmap + (MenuItems / 32) * 4 == MenuMapCount);
static_assert(MenuMapValues + MenuItems * 4 == MenuMapBytes);
inline constexpr std::array MenuItemPatches{
    InstructionPatch{0x80f60488, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80f60498, 0x389e14bc, 0x809e14bc}, // menu item map pointer
    InstructionPatch{0x80f604a0, 0x3bfe14bc, 0x83fe14bc}, // menu item map pointer
    InstructionPatch{0x80f604b8, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80f755e8, 0x387d14bc, 0x807d14bc}, // menu item map pointer
    InstructionPatch{0x80f755f8, 0x389d14bc, 0x809d14bc}, // menu item map pointer
    InstructionPatch{0x80f75600, 0x3bbd14bc, 0x83bd14bc}, // menu item map pointer
    InstructionPatch{0x80f75614, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80f89940, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80f89950, 0x389f14bc, 0x809f14bc}, // menu item map pointer
    InstructionPatch{0x80f8995c, 0x3bbf14bc, 0x83bf14bc}, // menu item map pointer
    InstructionPatch{0x80f89988, 0x808410a0, 0x80841944}, // item pointer array
    InstructionPatch{0x80f899e0, 0x387f14bc, 0x807f14bc}, // menu item map pointer
    InstructionPatch{0x80f899e8, 0x387f255c, 0x807f14bc}, // empty value destructor
    InstructionPatch{0x80f89ad4, 0x38a0001c, 0x38a00028}, // 320-bit allocation bitmap
    InstructionPatch{0x80f89af8, 0x93e3109c, 0x93e31940}, // map pool metadata
    InstructionPatch{0x80f89afc, 0x38631080, 0x38631918}, // map pool metadata
    InstructionPatch{0x80f89b04, 0x93fe1074, 0x93fe190c}, // map pool metadata
    InstructionPatch{0x80f89b0c, 0x93fe1078, 0x93fe1910}, // map pool metadata
    InstructionPatch{0x80f89b10, 0x93fe107c, 0x93fe1914}, // map pool metadata
    InstructionPatch{0x80f89b1c, 0x809e1074, 0x809e190c}, // map pool metadata
    InstructionPatch{0x80f89b20, 0x807e107c, 0x807e1914}, // map pool metadata
    InstructionPatch{0x80f89b28, 0x909e1074, 0x909e190c}, // map pool metadata
    InstructionPatch{0x80f89b2c, 0x2c0400d2, 0x2c040140}, // map pool metadata
    InstructionPatch{0x80f89b34, 0x901e107c, 0x901e1914}, // map pool metadata
    InstructionPatch{0x80f89b3c, 0x93fe1074, 0x93fe190c}, // map pool metadata
    InstructionPatch{0x80f89b40, 0x380000d1, 0x3800013f}, // map pool metadata
    InstructionPatch{0x80f89b5c, 0x38630d2c, 0x3863140c}, // map pool metadata
    InstructionPatch{0x80f89b70, 0x2c0000d2, 0x2c000140}, // map pool metadata
    InstructionPatch{0x80f8fb4c, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80f8fb5c, 0x389f14bc, 0x809f14bc}, // menu item map pointer
    InstructionPatch{0x80f8fb68, 0x3bbf14bc, 0x83bf14bc}, // menu item map pointer
    InstructionPatch{0x80f8fb94, 0x808410a0, 0x80841944}, // item pointer array
    InstructionPatch{0x80f8fbec, 0x387f14bc, 0x807f14bc}, // menu item map pointer
    InstructionPatch{0x80f8fbf4, 0x387f255c, 0x807f14bc}, // empty value destructor
    InstructionPatch{0x80f95174, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80f95184, 0x389f14bc, 0x809f14bc}, // menu item map pointer
    InstructionPatch{0x80f95190, 0x3bbf14bc, 0x83bf14bc}, // menu item map pointer
    InstructionPatch{0x80f951bc, 0x808410a0, 0x80841944}, // item pointer array
    InstructionPatch{0x80f95214, 0x387f14bc, 0x807f14bc}, // menu item map pointer
    InstructionPatch{0x80f9521c, 0x387f255c, 0x807f14bc}, // empty value destructor
    InstructionPatch{0x80f9ce3c, 0x80a61080, 0x80a61918}, // map pool metadata
    InstructionPatch{0x80f9ce44, 0x90061080, 0x90061918}, // map pool metadata
    InstructionPatch{0x80f9ce48, 0x80a31074, 0x80a3190c}, // map pool metadata
    InstructionPatch{0x80f9ce4c, 0x8083107c, 0x80831914}, // map pool metadata
    InstructionPatch{0x80f9ce54, 0x90a31074, 0x90a3190c}, // map pool metadata
    InstructionPatch{0x80f9ce58, 0x2c0500d2, 0x2c050140}, // map pool metadata
    InstructionPatch{0x80f9ce60, 0x9003107c, 0x90031914}, // map pool metadata
    InstructionPatch{0x80f9ce6c, 0x90031074, 0x9003190c}, // map pool metadata
    InstructionPatch{0x80f9ce70, 0x380000d1, 0x3800013f}, // map pool metadata
    InstructionPatch{0x80f9ce8c, 0x38630d2c, 0x3863140c}, // map pool metadata
    InstructionPatch{0x80f9ce94, 0x807f109c, 0x807f1940}, // map pool metadata
    InstructionPatch{0x80f9ce9c, 0x901f109c, 0x901f1940}, // map pool metadata
    InstructionPatch{0x80fa26d4, 0x387b14bc, 0x807b14bc}, // menu item map pointer
    InstructionPatch{0x80fa26e4, 0x389b14bc, 0x809b14bc}, // menu item map pointer
    InstructionPatch{0x80fa26ec, 0x3bbb14bc, 0x83bb14bc}, // menu item map pointer
    InstructionPatch{0x80fa2708, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fa275c, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fa277c, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fa279c, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fc4ec0, 0x387914bc, 0x807914bc}, // menu item map pointer
    InstructionPatch{0x80fc4ed0, 0x389914bc, 0x809914bc}, // menu item map pointer
    InstructionPatch{0x80fc4ed8, 0x3b9914bc, 0x839914bc}, // menu item map pointer
    InstructionPatch{0x80fc4ef8, 0x80a510a0, 0x80a51944}, // item pointer array
    InstructionPatch{0x80fc4f10, 0x80a510a0, 0x80a51944}, // item pointer array
    InstructionPatch{0x80fc7b8c, 0x387d14bc, 0x807d14bc}, // menu item map pointer
    InstructionPatch{0x80fc7b9c, 0x389d14bc, 0x809d14bc}, // menu item map pointer
    InstructionPatch{0x80fc7ba4, 0x3b9d14bc, 0x839d14bc}, // menu item map pointer
    InstructionPatch{0x80fc7bbc, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fc80ac, 0x387e14bc, 0x807e14bc}, // menu item map pointer
    InstructionPatch{0x80fc80bc, 0x389e14bc, 0x809e14bc}, // menu item map pointer
    InstructionPatch{0x80fc80c4, 0x3bfe14bc, 0x83fe14bc}, // menu item map pointer
    InstructionPatch{0x80fc80d8, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fc8acc, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80fc8af4, 0x389f14bc, 0x809f14bc}, // menu item map pointer
    InstructionPatch{0x80fc8b00, 0x3bbf14bc, 0x83bf14bc}, // menu item map pointer
    InstructionPatch{0x80fc8b20, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fc8b40, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fc8d1c, 0x387f14bc, 0x807f14bc}, // menu item map pointer
    InstructionPatch{0x80fc8d2c, 0x389f14bc, 0x809f14bc}, // menu item map pointer
    InstructionPatch{0x80fc8d34, 0x3b9f14bc, 0x839f14bc}, // menu item map pointer
    InstructionPatch{0x80fc8d48, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fc8d74, 0x807e10a0, 0x807e1944}, // item pointer array
    InstructionPatch{0x80fc8d90, 0x809e10a0, 0x809e1944}, // item pointer array
    InstructionPatch{0x80fc8df4, 0x387f14bc, 0x807f14bc}, // menu item map pointer
    InstructionPatch{0x80fc8e04, 0x389f14bc, 0x809f14bc}, // menu item map pointer
    InstructionPatch{0x80fc8e0c, 0x3b9f14bc, 0x839f14bc}, // menu item map pointer
    InstructionPatch{0x80fc8e20, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fca4f0, 0x3b9714bc, 0x839714bc}, // menu item map pointer
    InstructionPatch{0x80fca4fc, 0x387714bc, 0x807714bc}, // menu item map pointer
    InstructionPatch{0x80fca50c, 0x389714bc, 0x809714bc}, // menu item map pointer
    InstructionPatch{0x80fca524, 0x82c310a0, 0x82c31944}, // item pointer array
    InstructionPatch{0x80fcabc0, 0x387f14bc, 0x807f14bc}, // menu item map pointer
    InstructionPatch{0x80fcabd4, 0x389f14bc, 0x809f14bc}, // menu item map pointer
    InstructionPatch{0x80fcabe0, 0x3bdf14bc, 0x83df14bc}, // menu item map pointer
    InstructionPatch{0x80fcac00, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fcad04, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80fcad1c, 0x389d14bc, 0x809d14bc}, // menu item map pointer
    InstructionPatch{0x80fcad28, 0x3bfd14bc, 0x83fd14bc}, // menu item map pointer
    InstructionPatch{0x80fcad48, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fcaec4, 0x387e14bc, 0x807e14bc}, // menu item map pointer
    InstructionPatch{0x80fcaed4, 0x389e14bc, 0x809e14bc}, // menu item map pointer
    InstructionPatch{0x80fcaee0, 0x3bfe14bc, 0x83fe14bc}, // menu item map pointer
    InstructionPatch{0x80fcaf00, 0x808310a0, 0x80831944}, // item pointer array
    InstructionPatch{0x80fcafb8, 0x387e14bc, 0x807e14bc}, // menu item map pointer
    InstructionPatch{0x80fcafc8, 0x389e14bc, 0x809e14bc}, // menu item map pointer
    InstructionPatch{0x80fcafd4, 0x3bfe14bc, 0x83fe14bc}, // menu item map pointer
    InstructionPatch{0x80fcaff4, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fcb02c, 0x808310a0, 0x80831944}, // item pointer array
    InstructionPatch{0x80fcb204, 0x801b2558, 0x60000000}, // sidecar count observer
    InstructionPatch{0x80fcb208, 0x2c0000d2, 0x2c000140}, // menu item capacity
    InstructionPatch{0x80fcb480, 0x387b14bc, 0x807b14bc}, // menu item map pointer
    InstructionPatch{0x80fcb494, 0x389b14bc, 0x809b14bc}, // menu item map pointer
    InstructionPatch{0x80fcb4a0, 0x38bb14bc, 0x80bb14bc}, // menu item map pointer
    InstructionPatch{0x80fcb6f0, 0x801b2534, 0x801b2dcc}, // inlined map insertion metadata
    InstructionPatch{0x80fcb700, 0x838421e8, 0x838428c8}, // inlined map insertion metadata
    InstructionPatch{0x80fcb718, 0x8064253c, 0x80642dd4}, // inlined map insertion metadata
    InstructionPatch{0x80fcb720, 0x9004253c, 0x90042dd4}, // inlined map insertion metadata
    InstructionPatch{0x80fcb724, 0x807b2534, 0x807b2dcc}, // inlined map insertion metadata
    InstructionPatch{0x80fcb72c, 0x901b2534, 0x901b2dcc}, // inlined map insertion metadata
    InstructionPatch{0x80fcb730, 0x2c0000d2, 0x2c000140}, // insertion free queue wrap
    InstructionPatch{0x80fcb73c, 0x901b2534, 0x901b2dcc}, // inlined map insertion metadata
    InstructionPatch{0x80fcb740, 0x80bb2538, 0x80bb2dd0}, // inlined map insertion metadata
    InstructionPatch{0x80fcb748, 0x809b2558, 0x809b2dfc}, // inlined map insertion metadata
    InstructionPatch{0x80fcb754, 0x901b2538, 0x901b2dd0}, // inlined map insertion metadata
    InstructionPatch{0x80fcb760, 0x901b2558, 0x901b2dfc}, // inlined map insertion metadata
    InstructionPatch{0x80fcb7f0, 0x3863255c, 0x38632e00}, // inlined map value array
    InstructionPatch{0x80fcc4b4, 0x387714bc, 0x807714bc}, // menu item map pointer
    InstructionPatch{0x80fcc4c8, 0x389714bc, 0x809714bc}, // menu item map pointer
    InstructionPatch{0x80fcc4d4, 0x38d714bc, 0x80d714bc}, // menu item map pointer
    InstructionPatch{0x80fcc508, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fcc760, 0x387e14bc, 0x807e14bc}, // menu item map pointer
    InstructionPatch{0x80fcc770, 0x389e14bc, 0x809e14bc}, // menu item map pointer
    InstructionPatch{0x80fcc778, 0x3b9e14bc, 0x839e14bc}, // menu item map pointer
    InstructionPatch{0x80fcc78c, 0x83a310a0, 0x83a31944}, // item pointer array
    InstructionPatch{0x80fcc8d8, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80fcc8f0, 0x389f14bc, 0x809f14bc}, // menu item map pointer
    InstructionPatch{0x80fcc8fc, 0x3bbf14bc, 0x83bf14bc}, // menu item map pointer
    InstructionPatch{0x80fcc944, 0x808410a0, 0x80841944}, // item pointer array
    InstructionPatch{0x80fcc980, 0x387f14bc, 0x807f14bc}, // menu item map pointer
    InstructionPatch{0x80fcc988, 0x387f255c, 0x807f14bc}, // empty value destructor
    InstructionPatch{0x80fcf6e4, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80fcf6f4, 0x389c14bc, 0x809c14bc}, // menu item map pointer
    InstructionPatch{0x80fcf720, 0x3b7c14bc, 0x837c14bc}, // menu item map pointer
    InstructionPatch{0x80fcf734, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fcf7bc, 0x387c14bc, 0x807c14bc}, // menu item map pointer
    InstructionPatch{0x80fcf7d0, 0x389c14bc, 0x809c14bc}, // menu item map pointer
    InstructionPatch{0x80fcf7dc, 0x3b7c14bc, 0x837c14bc}, // menu item map pointer
    InstructionPatch{0x80fcf7fc, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fd00cc, 0x387b14bc, 0x807b14bc}, // menu item map pointer
    InstructionPatch{0x80fd00e0, 0x389b14bc, 0x809b14bc}, // menu item map pointer
    InstructionPatch{0x80fd00ec, 0x38bb14bc, 0x80bb14bc}, // menu item map pointer
    InstructionPatch{0x80fd0128, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fd0164, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80fd017c, 0x389e14bc, 0x809e14bc}, // menu item map pointer
    InstructionPatch{0x80fd018c, 0x801e2558, 0x60000000}, // sidecar count observer
    InstructionPatch{0x80fd01a0, 0x3bfe14bc, 0x83fe14bc}, // menu item map pointer
    InstructionPatch{0x80fd0214, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fd027c, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80fd028c, 0x389f14bc, 0x809f14bc}, // menu item map pointer
    InstructionPatch{0x80fd0298, 0x3bff14bc, 0x83ff14bc}, // menu item map pointer
    InstructionPatch{0x80fd02b8, 0x800310a0, 0x80031944}, // item pointer array
    InstructionPatch{0x80fd0b20, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80fd0b30, 0x389f14bc, 0x809f14bc}, // menu item map pointer
    InstructionPatch{0x80fd0b38, 0x3bff14bc, 0x83ff14bc}, // menu item map pointer
    InstructionPatch{0x80fd0b4c, 0x806310a0, 0x80631944}, // item pointer array
    InstructionPatch{0x80fd0bd4, 0x386314bc, 0x806314bc}, // menu item map pointer
    InstructionPatch{0x80fd0be4, 0x389f14bc, 0x809f14bc}, // menu item map pointer
    InstructionPatch{0x80fd0bec, 0x3bff14bc, 0x83ff14bc}, // menu item map pointer
    InstructionPatch{0x80fd0c00, 0x806310a0, 0x80631944}, // item pointer array
};
} // namespace moderngekko::roster
