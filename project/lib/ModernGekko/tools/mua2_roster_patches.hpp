#pragma once
#include "mua2_roster_layout.hpp"
#include <algorithm>
#include <vector>

namespace moderngekko::roster {
static_assert(Expanded.stats.capacity == 64 && Expanded.auxiliary.capacity == 48,
              "Review the native unrolled constructor when changing capacities");
// Reviewed field accesses in the native pool constructors only. These are a
// subset of the coordinated expansion: NEVER install them on their own. The
// allocation, users, reset, teardown and serialization must use the same layout.
struct ConstructorField {
  std::uint32_t address, original, member;
};
inline constexpr std::array FullStatsConstructorFields{
  ConstructorField{0x800a0c64u, 0x93a32c80u, 76928u},
  ConstructorField{0x800a0c70u, 0x93a32c84u, 76932u},
  ConstructorField{0x800a0c74u, 0x93a32c88u, 76936u},
  ConstructorField{0x800a0c78u, 0x38632c8cu, 76940u},
  ConstructorField{0x800a0c88u, 0x93be2c94u, 76948u},
  ConstructorField{0x800a0c8cu, 0x93be2c80u, 76928u},
  ConstructorField{0x800a0c90u, 0x93be2c84u, 76932u},
  ConstructorField{0x800a0c94u, 0x93be2c88u, 76936u},
  ConstructorField{0x800a0ca0u, 0x809e2c80u, 76928u},
  ConstructorField{0x800a0ca4u, 0x807e2c88u, 76936u},
  ConstructorField{0x800a0cacu, 0x909e2c80u, 76928u},
  ConstructorField{0x800a0cb8u, 0x901e2c88u, 76936u},
  ConstructorField{0x800a0cc0u, 0x93be2c80u, 76928u},
  ConstructorField{0x800a0ce0u, 0x38632bd0u, 76752u},
  ConstructorField{0x800a0d04u, 0x90052d48u, 77128u},
  ConstructorField{0x800a0d0cu, 0x90052d4cu, 77132u},
  ConstructorField{0x800a0d14u, 0x80052d48u, 77128u},
  ConstructorField{0x800a0d1cu, 0x80652d4cu, 77132u},
  ConstructorField{0x800a0d28u, 0x90852d48u, 77128u},
  ConstructorField{0x800a0d30u, 0x90052d4cu, 77132u},
  ConstructorField{0x800a0d5cu, 0x90a42c98u, 76952u},
  ConstructorField{0x800a0d6cu, 0x80e32d4cu, 77132u},
  ConstructorField{0x800a0d84u, 0x90e42c98u, 76952u},
  ConstructorField{0x800a0d8cu, 0x93c42c9cu, 76956u},
  ConstructorField{0x800a0d90u, 0x80e32d4cu, 77132u},
  ConstructorField{0x800a0d9cu, 0x90e42c9cu, 76956u},
  ConstructorField{0x800a0da0u, 0x91842ca0u, 76960u},
  ConstructorField{0x800a0da4u, 0x80e32d4cu, 77132u},
  ConstructorField{0x800a0db0u, 0x90e42ca0u, 76960u},
  ConstructorField{0x800a0db4u, 0x91642ca4u, 76964u},
  ConstructorField{0x800a0db8u, 0x80e32d4cu, 77132u},
  ConstructorField{0x800a0dc4u, 0x90e42ca4u, 76964u},
  ConstructorField{0x800a0dc8u, 0x91442ca8u, 76968u},
  ConstructorField{0x800a0dccu, 0x80e32d4cu, 77132u},
  ConstructorField{0x800a0dd8u, 0x90e42ca8u, 76968u},
  ConstructorField{0x800a0ddcu, 0x91242cacu, 76972u},
  ConstructorField{0x800a0de0u, 0x80e32d4cu, 77132u},
  ConstructorField{0x800a0decu, 0x90e42cacu, 76972u},
  ConstructorField{0x800a0df0u, 0x91042cb0u, 76976u},
  ConstructorField{0x800a0df4u, 0x80e32d4cu, 77132u},
  ConstructorField{0x800a0e00u, 0x90e42cb0u, 76976u},
  ConstructorField{0x800a0e18u, 0x90a72c98u, 76952u},
  ConstructorField{0x800a0e24u, 0x80082d4cu, 77132u},
  ConstructorField{0x800a0e30u, 0x90072c98u, 76952u},
  ConstructorField{0x800a0e38u, 0x80082d4cu, 77132u}
};
inline constexpr std::array AuxiliaryConstructorFields{
  ConstructorField{0x800a06a0u, 0x93fe0f04u, 3844u},
  ConstructorField{0x800a06a4u, 0x387e0f10u, 3856u},
  ConstructorField{0x800a06acu, 0x93fe0f08u, 3848u},
  ConstructorField{0x800a06b4u, 0x93fe0f0cu, 3852u},
  ConstructorField{0x800a06bcu, 0x93fe0f18u, 3864u},
  ConstructorField{0x800a06c4u, 0x93fe0f04u, 3844u},
  ConstructorField{0x800a06c8u, 0x93fe0f08u, 3848u},
  ConstructorField{0x800a06ccu, 0x93fe0f0cu, 3852u},
  ConstructorField{0x800a06d8u, 0x809e0f04u, 3844u},
  ConstructorField{0x800a06dcu, 0x807e0f0cu, 3852u},
  ConstructorField{0x800a06e4u, 0x909e0f04u, 3844u},
  ConstructorField{0x800a06f0u, 0x901e0f0cu, 3852u},
  ConstructorField{0x800a06f8u, 0x93fe0f04u, 3844u},
  ConstructorField{0x800a0718u, 0x38630e80u, 3712u}
};

inline std::optional<std::vector<InstructionPatch>> ConstructorPatches() {
  std::vector<InstructionPatch> patches;
  for (const auto& site : FullStatsConstructorFields) {
    const auto member = RelocateMember(StatsRegions, site.member, 4);
    if (!member) return {};
    const auto instruction = SignedImmediate(site.original, std::int64_t(*member) - 131072);
    if (!instruction) return {};
    patches.push_back({site.address, site.original, *instruction});
  }
  for (const auto& site : AuxiliaryConstructorFields) {
    const auto member = RelocateMember(AuxiliaryRegions, site.member, 4);
    if (!member) return {};
    const auto instruction = SignedImmediate(site.original, *member);
    if (!instruction) return {};
    patches.push_back({site.address, site.original, *instruction});
  }
  // 9 groups of seven generation entries + one tail = 64. The second retail
  // tail entry would overwrite the handle mask and must not be stored.
  patches.push_back({0x800a06e8u, 0x2c040021u, *SignedImmediate(0x2c040021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a06fcu, 0x38000020u, *SignedImmediate(0x38000020u, Expanded.auxiliary.capacity - 1)});
  patches.push_back({0x800a072cu, 0x2c000021u, *SignedImmediate(0x2c000021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x800a0c5cu, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x800a0c80u, 0x3fdf0001u, *SignedImmediate(0x3fdf0001u, 2)});
  patches.push_back({0x800a0cb0u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x800a0cc4u, 0x3800002bu, *SignedImmediate(0x3800002bu, Expanded.stats.capacity - 1)});
  patches.push_back({0x800a0cf4u, 0x2c00002cu, *SignedImmediate(0x2c00002cu, Expanded.stats.capacity)});
  patches.push_back({0x800a0cfcu, 0x3cbf0001u, *SignedImmediate(0x3cbf0001u, 2)});
  patches.push_back({0x800a0d08u, 0x38c0002bu, *SignedImmediate(0x38c0002bu, Expanded.stats.capacity - 1)});
  patches.push_back({0x800a0d3cu, 0x38800006u, *SignedImmediate(0x38800006u, 9)});
  patches.push_back({0x800a0d44u, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x800a0d54u, 0x3c860001u, *SignedImmediate(0x3c860001u, 2)});
  patches.push_back({0x800a0e08u, 0x3d1f0001u, *SignedImmediate(0x3d1f0001u, 2)});
  patches.push_back({0x800a0e34u, 0x90872c9cu, 0x60000000u});
  patches.push_back({0x800a0e44u, 0x90072c9cu, 0x60000000u});
  std::sort(patches.begin(), patches.end(), [](const auto& a, const auto& b) {
    return a.address < b.address;
  });
  return patches;
}
} // namespace moderngekko::roster
