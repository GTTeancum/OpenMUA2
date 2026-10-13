#include "mua2_roster_layout.hpp"
#include "mua2_roster_patches.hpp"
#include "mua2_roster_lifecycle.hpp"
#include "mua2_roster_consumers.hpp"
#include "mua2_roster_patch_plan.hpp"
#include "mua2_roster_menu.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

using namespace moderngekko::roster;

static void Require(bool ok, const char* message) {
  if (!ok) {
    std::cerr << "roster layout: " << message << '\n';
    std::exit(1);
  }
}

int main(int argc, char** argv) {
  Require(MenuPatches.size() == 7 && MenuPatches.front().replacement == 0x2c150100,
          "sparse stage locations must clear the complete byte-valued domain");
  Require(MenuPatches[5].replacement == (0x38000000u | (PlayableHeroes + 1)),
          "menu iteration must include the native default entry");
  for (std::size_t i : {1u, 2u, 3u, 4u, 6u})
    Require((MenuPatches[i].replacement & 0xffff) == MenuLocationFlags,
            "menu lookup/write offsets must move together");
  Require(EmptyMenuLocation > PlayableHeroes && MenuLocationCount == 256,
          "unassigned sparse locations must reject as outside the playable list");
  const auto constructor_patches = ConstructorPatches();
  Require(constructor_patches.has_value() && constructor_patches->size() == 75,
          "constructor patch generation");
  const auto lifecycle_patches = LifecyclePatches();
  Require(lifecycle_patches.has_value() && lifecycle_patches->size() == 62,
          "lifecycle patch generation");
  const auto consumer_patches = ConsumerPatches();
  Require(consumer_patches.has_value() && consumer_patches->size() == 453,
          "consumer patch generation");
  const auto storage_patches = PreparedStoragePatches();
  Require(storage_patches.has_value() && storage_patches->size() == 649,
          "complete prepared storage plan");
  const auto dol_patches = PreparedDolPatches();
  Require(dol_patches.has_value() && dol_patches->size() == 693,
          "prepared DOL plan with native character-info allocation");
  if (argc == 2 && (std::string_view(argv[1]) == "--dump-patches" ||
                    std::string_view(argv[1]) == "--dump-lifecycle-patches" ||
                    std::string_view(argv[1]) == "--dump-consumer-patches" ||
                    std::string_view(argv[1]) == "--dump-storage-patches" ||
                    std::string_view(argv[1]) == "--dump-dol-patches")) {
    // Read-only export for checking this exact compiled plan against a locally
    // supplied original DOL. No proprietary executable is needed in CI.
    std::cout << "address,expected,replacement\n" << std::hex;
    const auto& plan = std::string_view(argv[1]) == "--dump-patches" ?
                          *constructor_patches :
                      std::string_view(argv[1]) == "--dump-lifecycle-patches" ?
                          *lifecycle_patches :
                      std::string_view(argv[1]) == "--dump-consumer-patches" ?
                          *consumer_patches :
                      std::string_view(argv[1]) == "--dump-storage-patches" ?
                          *storage_patches : *dol_patches;
    for (const auto& p : plan)
      std::cout << p.address << ',' << p.original << ',' << p.replacement << '\n';
    return 0;
  }
  Require(argc == 1, "unknown argument");
  const auto getter_target = [](unsigned index) {
    const auto word = CharacterInfoGetterCode[index];
    const std::int32_t delta = std::int32_t((word & 0x03fffffc) << 6) >> 6;
    return CharacterInfoGetter + index * 4 + delta;
  };
  Require(getter_target(19) == 0x801f1380 && getter_target(25) == 0x800065bc,
          "character-info getter calls incorrect native allocator or memset");
  for (const auto entry : {std::pair{8u, 12u}, std::pair{11u, 32u}, std::pair{21u, 32u}})
    Require(entry.first * 4 + std::int16_t(CharacterInfoGetterCode[entry.first] & 0xfffc) ==
                entry.second * 4, "character-info getter branch target");
  Require((CharacterInfoGetterCode[12] & 0xffff) == CharacterInfoBytes &&
              (CharacterInfoGetterCode[24] & 0xffff) == CharacterInfoBytes,
          "character-info allocation and clear sizes differ");
  Require(CharacterInfoGetterCode.back() == 0x4e800020,
          "character-info getter must return before unreachable old constructor");
  for (const auto& p : *storage_patches) {
    Require(p.address != 0x80099d10 && p.address != 0x80099e98,
            "legacy serialized snapshot counts changed without compatibility handling");
    unsigned guards = 0;
    for (const auto& guard : StorageCodeGuards)
      if (p.address >= guard.address && std::uint64_t(p.address) + 4 <=
          std::uint64_t(guard.address) + guard.bytes) ++guards;
    Require(guards == 1, "patch not covered by exactly one complete-code guard");
  }
  const auto snapshot_patches = SnapshotPatches();
  Require(snapshot_patches.has_value() && snapshot_patches->size() == 59,
          "snapshot patch generation");
  const auto snapshot_word = [&](std::uint32_t pc) {
    for (const auto& p : *snapshot_patches) if (p.address == pc) return p.replacement;
    Require(false, "missing snapshot instruction");
    return 0u;
  };
  for (const auto pair : {std::pair{0x80099cf4u, 0x80099cf8u},
                          std::pair{0x80099e7cu, 0x80099e80u},
                          std::pair{0x8009a0d4u, 0x8009a0e4u}}) {
    const auto high = snapshot_word(pair.first), low = snapshot_word(pair.second);
    Require((high >> 26) == 15, "second bank requires addis");
    const auto address = 131072u + ((high & 0xffffu) << 16) +
                         std::int16_t(low & 0xffffu);
    Require(address == 196720, "save/load/restore second bank mismatch");
  }
  Require(snapshot_word(0x80099edc) == 0x5480801e &&
              snapshot_word(0x80099f44) == 0x5480801e,
          "indexed snapshot bank stride mismatch");
  const auto lifecycle_word = [&](std::uint32_t address) {
    for (const auto& p : *lifecycle_patches)
      if (p.address == address) return p.replacement;
    Require(false, "missing lifecycle instruction");
    return 0u;
  };
  const auto pair_value = [&](std::uint32_t high_pc, std::uint32_t low_pc) {
    return ((lifecycle_word(high_pc) & 0xffffu) << 16) +
           std::int16_t(lifecycle_word(low_pc) & 0xffffu);
  };
  Require(pair_value(0x800a04a8, 0x800a04b8) == 271264, "allocation instruction pair");
  Require(pair_value(0x800a04f0, 0x800a04f4) == 112192, "definition construction address");
  Require(pair_value(0x800a0520, 0x800a0524) == 131184, "snapshot construction address");
  Require(pair_value(0x8009fb58, 0x8009fb60) == 131184, "snapshot destruction address");
  Require(pair_value(0x800a052c, 0x800a0530) == 262272, "auxiliary construction address");
  Require(pair_value(0x8009fb48, 0x8009fb50) == 262272, "auxiliary destruction address");
  Require(pair_value(0x800a0538, 0x800a053c) == 267888, "NPC construction address");
  Require(pair_value(0x8009fb38, 0x8009fb40) == 267888, "NPC destruction address");
  Require(lifecycle_word(0x800a0764) == 0x3cc00001 &&
              lifecycle_word(0x8009feec) == 0x3ca00001,
          "snapshot stride must use addis-zero (lis), not overflowing addi");
  auto all_patches = *constructor_patches;
  all_patches.insert(all_patches.end(), lifecycle_patches->begin(), lifecycle_patches->end());
  all_patches.insert(all_patches.end(), consumer_patches->begin(), consumer_patches->end());
  std::sort(all_patches.begin(), all_patches.end(), [](const auto& a, const auto& b) {
    return a.address < b.address;
  });
  Require(InspectPatchWords(all_patches, [&](std::uint32_t address)
                            -> std::optional<std::uint32_t> {
            for (const auto& p : all_patches)
              if (p.address == address) return p.original;
            return {};
          }) == PatchState::Original, "prepared patch plans conflict");
  for (const auto& p : *consumer_patches) {
    Require((p.original & 0xffff0000) == (p.replacement & 0xffff0000),
            "consumer edit altered an opcode or register");
    Require(p.address != 0x8009afb0 && p.address != 0x800a09e0,
            "350-definition bitmap size must stay 44 bytes");
    Require(p.address != 0x8009f0c8 && p.address != 0x8009f110,
            "NPC capacity must stay 13");
    Require(p.address != 0x8009a318 && p.address != 0x8009fa24,
            "four-slot team and bitmap word arithmetic must not change");
  }
  Require(InspectPatchWords(*constructor_patches, [&](std::uint32_t address)
                               -> std::optional<std::uint32_t> {
            for (const auto& p : *constructor_patches)
              if (p.address == address) return p.original;
            return {};
          }) == PatchState::Original, "constructor plan has invalid/duplicate sites");
  for (const auto& p : *constructor_patches) {
    if (p.address == 0x800a0e34 || p.address == 0x800a0e44)
      Require(p.replacement == 0x60000000, "64th-index generation tail must not store");
    else
      Require((p.original & 0xffff0000) == (p.replacement & 0xffff0000),
              "constructor edit altered an opcode or register");
  }
  // Independently observed retail member offsets, relative to each pool.
  Require(RelocateMember(StatsRegions, 76752, 4) == Expanded.stats.free_indices,
          "full-stat free queue moved to the wrong region");
  Require(RelocateMember(StatsRegions, 76928, 4) == Expanded.stats.head,
          "full-stat queue head");
  Require(RelocateMember(StatsRegions, 76940, 8) == Expanded.stats.occupied,
          "full-stat occupied bitmap");
  Require(RelocateMember(StatsRegions, 76952, 4) == Expanded.stats.generations,
          "full-stat generation table");
  Require(RelocateMember(StatsRegions, 77128, 4) == Expanded.stats.handle_mask,
          "full-stat handle mask");
  Require(RelocateMember(AuxiliaryRegions, 3712, 4) == Expanded.auxiliary.free_indices,
          "hero auxiliary free queue");
  Require(RelocateMember(AuxiliaryRegions, 3844, 12) == Expanded.auxiliary.head,
          "hero auxiliary queue metadata");
  Require(RelocateMember(AuxiliaryRegions, 3856, 8) == Expanded.auxiliary.occupied,
          "hero auxiliary bitmap");
  Require(!RelocateMember(AuxiliaryRegions, 3868, 4), "padding is not a member");
  Require(!RelocateMember(AuxiliaryRegions, 3710, 4), "cross-region member rejected");
  Require(!RelocateMember(StatsRegions, 0xffffffffu, 8), "offset overflow rejected");
  Require(!RelocateMember(StatsRegions, 16, 0), "zero-byte member rejected");
  Require(RelocateMember(Members, 77152, 9804) == 112192, "definition table and count");
  Require(RelocateMember(Members, 96144, 800) == 131184, "first snapshot");
  Require(RelocateMember(Members, 122544, 800) == 196720, "second snapshot bank");
  Require(RelocateMember(Members, 148945) == 262257, "second bank valid flag");
  Require(RelocateMember(Members, 156204, 4) == 271260, "playable-list count at object end");
  Require(!RelocateMember(Members, 156208, 4), "past object end rejected");
  Require(!RelocateMember(Members, 148948, 4), "alignment gap is not a member");
  for (std::size_t a = 0; a < Members.size(); ++a) {
    const auto& first = Members[a];
    if (!first.bytes) continue;
    Require(first.old_begin + first.bytes <= Retail.size, "retail region overrun");
    Require(first.new_begin + first.bytes <= Expanded.size, "relocated region overrun");
    for (std::size_t b = a + 1; b < Members.size(); ++b) {
      const auto& second = Members[b];
      if (!second.bytes) continue;
      Require(first.old_begin + first.bytes <= second.old_begin ||
                  second.old_begin + second.bytes <= first.old_begin,
              "ambiguous source member relocation");
      Require(first.new_begin + first.bytes <= second.new_begin ||
                  second.new_begin + second.bytes <= first.new_begin,
              "relocated native regions overlap");
    }
  }
  Require(MenuLocationFlags >= MenuObjectEnd && MenuLocationFlags + 255 < MenuSlotBytes,
          "byte-valued menu location escapes its own pool slot");

  // Verify the bitmaps and all three independently indexed arrays retain space
  // for the last allowed item, without requiring a third bitmap word.
  for (const auto& p : {Expanded.stats, Expanded.auxiliary}) {
    Require(p.capacity > PlayableHeroes && p.capacity <= 64, "capacity/headroom");
    Require(p.records + (p.capacity - 1) * p.stride + p.stride == p.free_indices,
            "last record ends at free queue");
    Require(p.free_indices + p.capacity * 4 == p.head, "last free index boundary");
    Require((p.capacity - 1) / 32 < 2, "bitmap word boundary");
    if (p.generations)
      Require(p.generations + p.capacity * 4 == p.handle_mask, "generation boundary");
  }
  Require(Expanded.snapshot_flags == Expanded.snapshots + 2 * 65536,
          "bank flags follow both complete banks");
  Require(Expanded.auxiliary_base >= Expanded.snapshot_flags + 2,
          "auxiliary pool overlaps snapshot flags");
  Require((Expanded.auxiliary_base & 15) == 0 && (Expanded.npc_auxiliary & 15) == 0,
          "native record alignment");
  Require(Expanded.unlocked_list - Expanded.npc_auxiliary == 1968,
          "NPC pool was inadvertently resized");
  Require(ActiveTeam == 4 && DefinitionCapacity == 350, "unrelated capacities changed");

  Require(SignedImmediate(0x38630000, -32768) == 0x38638000, "negative immediate");
  Require(SignedImmediate(0x38630000, 32767) == 0x38637fff, "positive immediate");
  Require(!SignedImmediate(0x38630000, 38400), "snapshot stride would wrap addi");
  Require(!SignedImmediate(0x38630000, -32769), "negative immediate overflow");

  const std::array patches{
      InstructionPatch{0x80000010, 0x2c000024, 0x2c000028},
      InstructionPatch{0x80000014, 0x806301b4, 0x806301e4}};
  auto words = std::array{patches[0].original, patches[1].original};
  auto read = [&](std::uint32_t address) -> std::optional<std::uint32_t> {
    if (address < 0x80000010 || address > 0x80000014) return {};
    return words[(address - 0x80000010) / 4];
  };
  Require(InspectPatchWords(patches, read) == PatchState::Original, "original patch set");
  words[0] = patches[0].replacement;
  Require(InspectPatchWords(patches, read) == PatchState::Invalid, "partial patch rejected");
  words[1] = patches[1].replacement;
  Require(InspectPatchWords(patches, read) == PatchState::Installed, "installed patch set");
  words[1] ^= 0x10000;
  Require(InspectPatchWords(patches, read) == PatchState::Invalid, "wrong register rejected");
  Require(InspectPatchWords(patches, [](auto) -> std::optional<std::uint32_t> { return {}; })
              == PatchState::Invalid, "unmapped code rejected");
  auto duplicate = patches;
  duplicate[1].address = duplicate[0].address;
  Require(InspectPatchWords(duplicate, read) == PatchState::Invalid, "duplicate patch rejected");
  auto unaligned = patches;
  unaligned[0].address++;
  Require(InspectPatchWords(unaligned, read) == PatchState::Invalid, "unaligned patch rejected");
  Require(InspectPatchWords(std::span<const InstructionPatch>{}, read) == PatchState::Invalid,
          "empty patch set rejected");
  std::cout << "roster member relocation, capacity boundaries and patch guards passed\n";
}
