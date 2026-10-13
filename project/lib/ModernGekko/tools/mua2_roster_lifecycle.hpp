#pragma once
#include "mua2_roster_patches.hpp"

namespace moderngekko::roster {
struct ManagerMemberSite {
  std::uint32_t address, original, old_bias, new_bias, width;
};
// Allocation and teardown address pairs are reviewed separately from the
// internal pool constructors. No change here touches the 350-definition map,
// the 13-NPC pool's own layout, or the active team size.
inline constexpr std::array ManagerLifecycleMembers{
  ManagerMemberSite{0x800a04f4u, 0x38632d60u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x800a0500u, 0x386353acu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x800a0518u, 0x38637780u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x800a0524u, 0x38637790u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x800a0530u, 0x386345e0u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x800a053cu, 0x38635500u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x800a0548u, 0x38635cb0u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x800a0554u, 0x38635f70u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x800a0564u, 0x980345d0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x800a0568u, 0x980345d1u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009fb20u, 0x38635f70u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009fb30u, 0x38635cb0u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009fb40u, 0x38635500u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009fb50u, 0x386345e0u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009fb60u, 0x38637790u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009fb70u, 0x386353acu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009fb80u, 0x38632d60u, 65536u, 131072u, 4u}
};
inline std::optional<std::vector<InstructionPatch>> LifecyclePatches() {
  std::vector<InstructionPatch> patches;
  for (const auto& site : ManagerLifecycleMembers) {
    const auto old_member = site.old_bias + std::int16_t(site.original & 0xffff);
    const auto member = RelocateMember(Members, old_member, site.width);
    if (!member) return {};
    const auto word = SignedImmediate(site.original, std::int64_t(*member) - site.new_bias);
    if (!word) return {};
    patches.push_back({site.address, site.original, *word});
  }
  patches.push_back({0x8009fb18u, 0x3c7f0002u, *SignedImmediate(0x3c7f0002u, 4)});
  patches.push_back({0x8009fb28u, 0x3c7f0002u, *SignedImmediate(0x3c7f0002u, 4)});
  patches.push_back({0x8009fb38u, 0x3c7f0002u, *SignedImmediate(0x3c7f0002u, 4)});
  patches.push_back({0x8009fb48u, 0x3c7f0002u, *SignedImmediate(0x3c7f0002u, 4)});
  patches.push_back({0x8009fb58u, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x8009fb68u, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x8009fb78u, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x8009fc10u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009fc18u, 0x3ba0002cu, *SignedImmediate(0x3ba0002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009fc78u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009fc7cu, 0x3ba0002cu, *SignedImmediate(0x3ba0002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009fca4u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009fcacu, 0x3ba0002cu, *SignedImmediate(0x3ba0002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009fcd4u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009fcdcu, 0x3ba0002cu, *SignedImmediate(0x3ba0002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009fd3cu, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009fd40u, 0x3ba0002cu, *SignedImmediate(0x3ba0002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009fd4cu, 0x2c1d002cu, *SignedImmediate(0x2c1d002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009feecu, 0x38a06720u, 0x3ca00001u});
  patches.push_back({0x8009ff5cu, 0x38c00021u, *SignedImmediate(0x38c00021u, Expanded.snapshots_per_bank)});
  patches.push_back({0x800a00fcu, 0x2c040021u, *SignedImmediate(0x2c040021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a0104u, 0x3ba00021u, *SignedImmediate(0x3ba00021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a0164u, 0x2c040021u, *SignedImmediate(0x2c040021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a0168u, 0x3ba00021u, *SignedImmediate(0x3ba00021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a0174u, 0x2c1d0021u, *SignedImmediate(0x2c1d0021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a01b8u, 0x2c040021u, *SignedImmediate(0x2c040021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a01c0u, 0x3ba00021u, *SignedImmediate(0x3ba00021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a01e8u, 0x2c040021u, *SignedImmediate(0x2c040021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a01f0u, 0x3ba00021u, *SignedImmediate(0x3ba00021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a0250u, 0x2c040021u, *SignedImmediate(0x2c040021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a0254u, 0x3ba00021u, *SignedImmediate(0x3ba00021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a0260u, 0x2c1d0021u, *SignedImmediate(0x2c1d0021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a04a8u, 0x3c600002u, *SignedImmediate(0x3c600002u, (Expanded.size + 0x8000u) >> 16)});
  patches.push_back({0x800a04b8u, 0x38636230u, *SignedImmediate(0x38636230u, std::int16_t(Expanded.size & 0xffffu))});
  patches.push_back({0x800a04f0u, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x800a04fcu, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x800a0508u, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x800a0520u, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x800a052cu, 0x3c7f0002u, *SignedImmediate(0x3c7f0002u, 4)});
  patches.push_back({0x800a0538u, 0x3c7f0002u, *SignedImmediate(0x3c7f0002u, 4)});
  patches.push_back({0x800a0544u, 0x3c7f0002u, *SignedImmediate(0x3c7f0002u, 4)});
  patches.push_back({0x800a0550u, 0x3c7f0002u, *SignedImmediate(0x3c7f0002u, 4)});
  patches.push_back({0x800a055cu, 0x3c7f0002u, *SignedImmediate(0x3c7f0002u, 4)});
  patches.push_back({0x800a0764u, 0x38c06720u, 0x3cc00001u});
  patches.push_back({0x800a07c0u, 0x38e00021u, *SignedImmediate(0x38e00021u, Expanded.snapshots_per_bank)});
  std::sort(patches.begin(), patches.end(), [](const auto& a, const auto& b) {
    return a.address < b.address;
  });
  return patches;
}
} // namespace moderngekko::roster
