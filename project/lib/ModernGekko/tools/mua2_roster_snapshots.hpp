#pragma once
#include "mua2_roster_lifecycle.hpp"

namespace moderngekko::roster {
// Bank addressing and in-memory transfer bounds. The original serialized loops
// deliberately remain 33 records; the versioned save path is a separate task.
// This plan must not be installed independently of the complete roster layout.
inline constexpr std::array SnapshotMembers{
  ManagerMemberSite{0x80099c48u, 0x3bdf2d7cu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80099c68u, 0x801f2d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099c8cu, 0x801f53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099cbcu, 0x38845cb0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x80099ce4u, 0x388445d1u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x80099cf8u, 0x3bff7790u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80099d68u, 0x3b9d2d7cu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80099db0u, 0x80032d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099dd4u, 0x801d53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099e0cu, 0x38845cb0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x80099e24u, 0x38c65cb0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x80099e40u, 0x88042d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80099e48u, 0x98042d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80099e4cu, 0x80035f6cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x80099e6cu, 0x388445d1u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x80099e80u, 0x3bbd7790u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80099ee8u, 0x38847790u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80099efcu, 0x988345d0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x80099f38u, 0x881f45d0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x80099f50u, 0x38847790u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80099f5cu, 0x981f45d0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x80099f9cu, 0x3bde2d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80099fa0u, 0x801f53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099fc4u, 0x801f2d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a034u, 0x3bde2d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a038u, 0x801f53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a05cu, 0x801f2d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a0c0u, 0x880445d1u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009a0e0u, 0x3bde2d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a0e4u, 0x3b9c7790u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a0e8u, 0x801f53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a10cu, 0x801f2d58u, 65536u, 131072u, 4u}
};
inline std::optional<std::vector<InstructionPatch>> SnapshotPatches() {
  std::vector<InstructionPatch> patches;
  for (const auto& site : SnapshotMembers) {
    const auto member = RelocateMember(Members, site.old_bias +
                                      std::int16_t(site.original & 0xffff), site.width);
    if (!member) return {};
    const auto word = SignedImmediate(site.original, std::int64_t(*member) - site.new_bias);
    if (!word) return {};
    patches.push_back({site.address, site.original, *word});
  }
  patches.push_back({0x80099c38u, 0x3fe30001u, *SignedImmediate(0x3fe30001u, 2)});
  patches.push_back({0x80099cb0u, 0x3c9b0002u, *SignedImmediate(0x3c9b0002u, 4)});
  patches.push_back({0x80099cd8u, 0x3c9b0002u, *SignedImmediate(0x3c9b0002u, 4)});
  patches.push_back({0x80099cecu, 0x3c7b0001u, *SignedImmediate(0x3c7b0001u, 2)});
  patches.push_back({0x80099d60u, 0x3fbe0001u, *SignedImmediate(0x3fbe0001u, 2)});
  patches.push_back({0x80099da8u, 0x3c7e0001u, *SignedImmediate(0x3c7e0001u, 2)});
  patches.push_back({0x80099e00u, 0x3c9e0002u, *SignedImmediate(0x3c9e0002u, 4)});
  patches.push_back({0x80099e14u, 0x3cde0002u, *SignedImmediate(0x3cde0002u, 4)});
  patches.push_back({0x80099e18u, 0x3cbe0001u, *SignedImmediate(0x3cbe0001u, 2)});
  patches.push_back({0x80099e60u, 0x3c9e0002u, *SignedImmediate(0x3c9e0002u, 4)});
  patches.push_back({0x80099e74u, 0x3c7e0001u, *SignedImmediate(0x3c7e0001u, 2)});
  patches.push_back({0x80099ee0u, 0x3c830001u, *SignedImmediate(0x3c830001u, 2)});
  patches.push_back({0x80099ef0u, 0x3c1e0002u, *SignedImmediate(0x3c1e0002u, 4)});
  patches.push_back({0x80099f30u, 0x3c030002u, *SignedImmediate(0x3c030002u, 4)});
  patches.push_back({0x80099f48u, 0x3c830001u, *SignedImmediate(0x3c830001u, 2)});
  patches.push_back({0x80099f88u, 0x3fc30001u, *SignedImmediate(0x3fc30001u, 2)});
  patches.push_back({0x8009a020u, 0x3fc30001u, *SignedImmediate(0x3fc30001u, 2)});
  patches.push_back({0x8009a0b8u, 0x3c830002u, *SignedImmediate(0x3c830002u, 4)});
  patches.push_back({0x8009a0ccu, 0x3fc30001u, *SignedImmediate(0x3fc30001u, 2)});
  patches.push_back({0x80099cf4u, 0x3be36720u, 0x3fe30001u}); // addis: second bank
  patches.push_back({0x80099e7cu, 0x3ba36720u, 0x3fa30001u}); // addis: second bank
  patches.push_back({0x8009a0d4u, 0x3b9e6720u, 0x3f9e0001u}); // addis: second bank
  patches.push_back({0x80099edcu, 0x1c046720u, 0x5480801eu}); // slwi r0,r4,16
  patches.push_back({0x80099f44u, 0x1c046720u, 0x5480801eu}); // slwi r0,r4,16
  patches.push_back({0x80099fe8u, 0x2c1c0021u, *SignedImmediate(0x2c1c0021u, Expanded.snapshots_per_bank)});
  patches.push_back({0x8009a080u, 0x2c1c0021u, *SignedImmediate(0x2c1c0021u, Expanded.snapshots_per_bank)});
  patches.push_back({0x8009a130u, 0x2c1b0021u, *SignedImmediate(0x2c1b0021u, Expanded.snapshots_per_bank)});
  std::sort(patches.begin(), patches.end(), [](const auto& a, const auto& b) {
    return a.address < b.address;
  });
  return patches;
}
} // namespace moderngekko::roster
