#pragma once
#include "mua2_roster_consumers.hpp"
#include "mua2_roster_snapshots.hpp"
#include "mua2_roster_character_info.hpp"
#include "mua2_roster_menu_strings.hpp"

namespace moderngekko::roster {
struct CodeSpanGuard {
  std::uint32_t address, bytes;
  const char* sha1;
};
inline constexpr std::array StorageCodeGuards{
    CodeSpanGuard{0x80098910, 0x8554, "8E4420A7405E954F2EC7669AFB24B233AE3CFFA4"},
    CodeSpanGuard{0x800b2d04, 0x21c, "6DB452248C6A4C11B6F893C5CEA058FF3CB4E7FE"}};
inline constexpr std::array CharacterInfoCodeGuards{
    CodeSpanGuard{0x80016768, 0x13c, "09221BF1112C7F8D565E87CA5CCF5F6874F50E3E"},
    CodeSpanGuard{0x80231310, 0x4c, "0ECC150A1966C8BA24A84BD785C3B854C6C2E4C3"},
    CodeSpanGuard{0x800a0e74, 0x1e4, "541768CA18F0442C77710EE2AFAA17C0FBDBE862"},
    CodeSpanGuard{0x800b2f20, 0x1400, "E1E4A6B7138E4A45F2C48990458BC1535E9BBD04"}};
// Prepared storage changes only. Character-info relocation, menu integration
// and versioned persistence are still required before enabling the expansion.
inline std::optional<std::vector<InstructionPatch>> PreparedStoragePatches() {
  std::vector<InstructionPatch> out;
  for (const auto& plan : {ConstructorPatches(), LifecyclePatches(),
                           ConsumerPatches(), SnapshotPatches()}) {
    if (!plan) return {};
    out.insert(out.end(), plan->begin(), plan->end());
  }
  std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
    return a.address < b.address;
  });
  for (std::size_t i = 0; i < out.size(); ++i)
    if ((out[i].address & 3) || out[i].original == out[i].replacement ||
        (i && out[i - 1].address == out[i].address)) return {};
  return out;
}
inline std::optional<std::vector<InstructionPatch>> PreparedDolPatches() {
  auto out = PreparedStoragePatches();
  const auto info = CharacterInfoPatches();
  if (!out || !info) return {};
  out->insert(out->end(), info->begin(), info->end());
  out->insert(out->end(), MenuStringDolPatches.begin(), MenuStringDolPatches.end());
  std::sort(out->begin(), out->end(), [](const auto& a, const auto& b) {
    return a.address < b.address;
  });
  for (std::size_t i = 1; i < out->size(); ++i)
    if ((*out)[i - 1].address == (*out)[i].address) return {};
  return out;
}
} // namespace moderngekko::roster
