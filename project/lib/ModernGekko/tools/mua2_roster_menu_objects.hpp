#pragma once
#include "mua2_roster_layout.hpp"
namespace moderngekko::roster {
inline constexpr std::uint32_t MenuObjectCapacity=320, MenuObjectStride=1312;
inline constexpr std::uint32_t MenuObjectStart=48, MenuObjectQueue=419888;
inline constexpr std::uint32_t MenuObjectHead=421168, MenuObjectTail=421172;
inline constexpr std::uint32_t MenuObjectFree=421176, MenuObjectBitmap=421180;
inline constexpr std::uint32_t MenuObjectCount=421220, MenuObjectBytes=421224;
static_assert(MenuObjectStart+MenuObjectCapacity*MenuObjectStride==MenuObjectQueue);
static_assert(MenuObjectQueue+MenuObjectCapacity*4==MenuObjectHead);
static_assert(MenuObjectBitmap+MenuObjectCapacity/8==MenuObjectCount);
inline constexpr std::array MenuObjectPatches{
    InstructionPatch{0x80fe6e7c, 0x2c0400d2, 0x2c040140}, // object iteration limit
    InstructionPatch{0x80fe6e84, 0x3ba000d2, 0x3ba00140}, // object iteration limit
    InstructionPatch{0x80fe6ee4, 0x2c0400d2, 0x2c040140}, // object iteration limit
    InstructionPatch{0x80fe6ee8, 0x3ba000d2, 0x3ba00140}, // object iteration limit
    InstructionPatch{0x80fe6ef4, 0x2c1d00d2, 0x2c1d0140}, // object iteration limit
    InstructionPatch{0x80fe6f0c, 0x38630020, 0x38630030}, // object array after expanded bitmap
    InstructionPatch{0x80fe6f24, 0x2c0400d2, 0x2c040140}, // object iteration limit
    InstructionPatch{0x80fe6f2c, 0x3ba000d2, 0x3ba00140}, // object iteration limit
    InstructionPatch{0x80fe6f54, 0x2c0400d2, 0x2c040140}, // object iteration limit
    InstructionPatch{0x80fe6f5c, 0x3ba000d2, 0x3ba00140}, // object iteration limit
    InstructionPatch{0x80fe6fbc, 0x2c0400d2, 0x2c040140}, // object iteration limit
    InstructionPatch{0x80fe6fc0, 0x3ba000d2, 0x3ba00140}, // object iteration limit
    InstructionPatch{0x80fe6fcc, 0x2c1d00d2, 0x2c1d0140}, // object iteration limit
    InstructionPatch{0x80fe6fdc, 0x38a0001c, 0x38a00028}, // clear 320 allocation bits
    InstructionPatch{0x80fe703c, 0x3c630004, 0x3c630006}, // pool metadata upper offset
    InstructionPatch{0x80fe7044, 0x38633460, 0x38636830}, // pool metadata offset
    InstructionPatch{0x80fe7094, 0x38030020, 0x38030030}, // object array after expanded bitmap
    InstructionPatch{0x80fe70c0, 0x3c7f0004, 0x3c7f0006}, // pool metadata upper offset
    InstructionPatch{0x80fe70c8, 0x386337b4, 0x38636d3c}, // pool metadata offset
    InstructionPatch{0x80fe70d0, 0x3c7f0004, 0x3c7f0006}, // pool metadata upper offset
    InstructionPatch{0x80fe70d8, 0x38633460, 0x38636830}, // pool metadata offset
    InstructionPatch{0x80fe70e0, 0x3c9f0004, 0x3c9f0006}, // pool metadata upper offset
    InstructionPatch{0x80fe70e4, 0x806437d0, 0x80646d64}, // pool metadata offset
    InstructionPatch{0x80fe70ec, 0x900437d0, 0x90046d64}, // pool metadata offset
    InstructionPatch{0x80fe7104, 0x80c30348, 0x80c30500}, // free queue/capacity
    InstructionPatch{0x80fe7108, 0x80a30350, 0x80a30508}, // free queue/capacity
    InstructionPatch{0x80fe7110, 0x90c30348, 0x90c30500}, // free queue/capacity
    InstructionPatch{0x80fe7114, 0x2c0600d2, 0x2c060140}, // free queue/capacity
    InstructionPatch{0x80fe711c, 0x90030350, 0x90030508}, // free queue/capacity
    InstructionPatch{0x80fe7128, 0x90030348, 0x90030500}, // free queue/capacity
    InstructionPatch{0x80fe712c, 0x380000d1, 0x3800013f}, // free queue/capacity
    InstructionPatch{0x80fe7194, 0x38630020, 0x38630030}, // object array after expanded bitmap
    InstructionPatch{0x80fe7204, 0x3ce30004, 0x3ce30006}, // pool metadata upper offset
    InstructionPatch{0x80fe720c, 0x800737d0, 0x80076d64}, // pool metadata offset
    InstructionPatch{0x80fe721c, 0x2c0000d2, 0x2c000140}, // free queue/capacity
    InstructionPatch{0x80fe7238, 0x800737ac, 0x80076d34}, // pool metadata offset
    InstructionPatch{0x80fe7248, 0x81243460, 0x81246830}, // pool metadata offset
    InstructionPatch{0x80fe7260, 0x808537b4, 0x80856d3c}, // pool metadata offset
    InstructionPatch{0x80fe7268, 0x908537b4, 0x90856d3c}, // pool metadata offset
    InstructionPatch{0x80fe726c, 0x808737ac, 0x80876d34}, // pool metadata offset
    InstructionPatch{0x80fe7274, 0x908737ac, 0x90876d34}, // pool metadata offset
    InstructionPatch{0x80fe7278, 0x2c0400d2, 0x2c040140}, // free queue/capacity
    InstructionPatch{0x80fe7284, 0x908737ac, 0x90876d34}, // pool metadata offset
    InstructionPatch{0x80fe7288, 0x3d030004, 0x3d030006}, // pool metadata upper offset
    InstructionPatch{0x80fe7290, 0x80e837b0, 0x80e86d38}, // pool metadata offset
    InstructionPatch{0x80fe7298, 0x80c837d0, 0x80c86d64}, // pool metadata offset
    InstructionPatch{0x80fe72a0, 0x90e837b0, 0x90e86d38}, // pool metadata offset
    InstructionPatch{0x80fe72ac, 0x90c837d0, 0x90c86d64}, // pool metadata offset
    InstructionPatch{0x80fe72b0, 0x3b650020, 0x3b650030}, // object array after expanded bitmap
    InstructionPatch{0x80fe864c, 0x381a0020, 0x381a0030}, // object array after expanded bitmap
    InstructionPatch{0x80fe8678, 0x3c7a0004, 0x3c7a0006}, // pool metadata upper offset
    InstructionPatch{0x80fe8680, 0x386337b4, 0x38636d3c}, // pool metadata offset
    InstructionPatch{0x80fe8688, 0x3c7a0004, 0x3c7a0006}, // pool metadata upper offset
    InstructionPatch{0x80fe8690, 0x38633460, 0x38636830}, // pool metadata offset
    InstructionPatch{0x80fe8698, 0x3c9a0004, 0x3c9a0006}, // pool metadata upper offset
    InstructionPatch{0x80fe869c, 0x806437d0, 0x80646d64}, // pool metadata offset
    InstructionPatch{0x80fe86a4, 0x900437d0, 0x90046d64}, // pool metadata offset
    InstructionPatch{0x8100a7bc, 0x38636c50, 0x80636c50}, // shared object pool pointer
    InstructionPatch{0x8100a7c4, 0x38636c50, 0x80636c50}, // shared object pool pointer
    InstructionPatch{0x8100d128, 0x387f6c50, 0x807f6c50}, // shared object pool pointer
};
}
