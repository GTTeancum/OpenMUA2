#pragma once
#include "mua2_roster_patches.hpp"

namespace moderngekko::roster {
static_assert(CharacterInfoBytes == 488 && CharacterInfoCount == 484);
// Replace the old static-object getter with a native-heap singleton. Only eight
// bytes of the original object's reserved storage become metadata (tag/pointer).
// The new object remains in guest memory, so native saves of process state retain
// its ownership. It lives for the title's lifetime, like the original singleton.
// No neighboring BSS is borrowed, and no host-only allocation/pointer is used.
inline constexpr std::uint32_t CharacterInfoGetter = 0x800a0e74;
inline constexpr std::array CharacterInfoGetterCode{
  0x9421fff0u /* stwu r1,-16(r1) */,
  0x7c0802a6u /* mflr r0 */,
  0x90010014u /* stw r0,20(r1) */,
  0x93e1000cu /* stw r31,12(r1) */,
  0x3fe08062u /* lis r31,old static metadata high */,
  0x3bff7020u /* addi r31,r31,old static metadata low */,
  0x801f0000u /* lwz r0,0(r31) */,
  0x2c004d32u /* cmpwi r0,metadata tag */,
  0x40820010u /* bne allocate */,
  0x807f0004u /* lwz r3,4(r31) */,
  0x2c030000u /* cmpwi r3,0 */,
  0x40820054u /* bne return */,
  0x386001e8u /* li r3,488 */,
  0x38800003u /* li r4,3 */,
  0x3ca08053u /* lis r5,class-name high */,
  0x38a50380u /* addi r5,r5,class-name low */,
  0x3cc08053u /* lis r6,source-name high */,
  0x38c6f0f8u /* addi r6,r6,source-name low */,
  0x38e00000u /* li r7,0 */,
  0x481504c1u /* bl native allocator */,
  0x2c030000u /* cmpwi r3,0 */,
  0x4182002cu /* beq return (allocation failure) */,
  0x907f0004u /* stw r3,4(r31) */,
  0x38800000u /* li r4,0 */,
  0x38a001e8u /* li r5,488 */,
  0x4bf656e5u /* bl native memset */,
  0x807f0004u /* lwz r3,4(r31) */,
  0x3c808053u /* lis r4,vtable high */,
  0x38840360u /* addi r4,r4,vtable low */,
  0x90830000u /* stw r4,0(r3) */,
  0x38004d32u /* li r0,metadata tag */,
  0x901f0000u /* stw r0,0(r31) */,
  0x83e1000cu /* lwz r31,12(r1) */,
  0x80010014u /* lwz r0,20(r1) */,
  0x7c0803a6u /* mtlr r0 */,
  0x38210010u /* addi r1,r1,16 */,
  0x4e800020u /* blr */
};
inline constexpr std::array CharacterInfoGetterOriginal{
  0x9421fff0u,
  0x7c0802a6u,
  0x90010014u,
  0x880da8b0u,
  0x7c000775u,
  0x408201b8u,
  0x3c808062u,
  0x3c608053u,
  0x38c47020u,
  0x38000000u,
  0x38a60010u,
  0x38630360u,
  0x392601b4u,
  0x90647020u,
  0x7c854840u,
  0x90060004u,
  0x90060008u,
  0x9006000cu,
  0x40840158u,
  0x7d054850u,
  0x3c602aabu,
  0x3948000bu,
  0x38e60154u,
  0x3803aaabu,
  0x7c005096u,
  0x7c000e70u,
  0x54030ffeu,
  0x7d601a14u,
  0x398b0001u,
  0x2c0c0008u,
  0x408100f4u,
  0x38600000u,
  0x38800000u,
  0x41850028u,
  0x55000001u,
  0x39000001u,
  0x40820010u
};
inline std::optional<std::vector<InstructionPatch>> CharacterInfoPatches() {
  std::vector<InstructionPatch> patches;
  for (std::size_t i = 0; i < CharacterInfoGetterCode.size(); ++i) {
    if (CharacterInfoGetterCode[i] != CharacterInfoGetterOriginal[i])
      patches.push_back({CharacterInfoGetter + std::uint32_t(i * 4),
                         CharacterInfoGetterOriginal[i], CharacterInfoGetterCode[i]});
  }
  patches.push_back({0x800b2ff4u, 0x801601b4u, *SignedImmediate(0x801601b4u, CharacterInfoCount)});
  patches.push_back({0x800b3000u, 0x807601b4u, *SignedImmediate(0x807601b4u, CharacterInfoCount)});
  patches.push_back({0x800b300cu, 0x901601b4u, *SignedImmediate(0x901601b4u, CharacterInfoCount)});
  patches.push_back({0x800b3014u, 0x807601b4u, *SignedImmediate(0x807601b4u, CharacterInfoCount)});
  patches.push_back({0x800b3964u, 0x801e01b4u, *SignedImmediate(0x801e01b4u, CharacterInfoCount)});
  patches.push_back({0x800b4200u, 0x801601b4u, *SignedImmediate(0x801601b4u, CharacterInfoCount)});
  patches.push_back({0x800b42f0u, 0x801d01b4u, *SignedImmediate(0x801d01b4u, CharacterInfoCount)});
  patches.push_back({0x800b2ff8u, 0x2c000024u, *SignedImmediate(0x2c000024u, PlayableHeroes)});
  std::sort(patches.begin(), patches.end(), [](const auto& a, const auto& b) {
    return a.address < b.address;
  });
  return patches;
}
} // namespace moderngekko::roster
