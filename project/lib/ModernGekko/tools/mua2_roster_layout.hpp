#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>

namespace moderngekko::roster {

inline constexpr std::uint32_t PlayableHeroes = 40;
inline constexpr std::uint32_t ActiveTeam = 4;
inline constexpr std::uint32_t DefinitionCapacity = 350;
inline constexpr std::uint32_t NpcAuxiliaryCapacity = 13;

constexpr std::uint32_t Align16(std::uint32_t value) {
  return (value + 15u) & ~15u;
}

// Offsets are guest bytes, not host C++ structure offsets. Keep the native
// record formats and the two-word bitmaps. Capacities above 64 require a
// different bitmap/handle implementation, not just a different allocation.
struct PoolLayout {
  std::uint32_t capacity, stride, records, free_indices, head, tail, free_count;
  std::uint32_t occupied, active_count, generations, handle_mask, handle_bits, size;

  static constexpr PoolLayout Make(std::uint32_t capacity, std::uint32_t stride,
                                   bool handles) {
    PoolLayout p{};
    p.capacity = capacity;
    p.stride = stride;
    p.records = 16;
    p.free_indices = p.records + capacity * stride;
    p.head = p.free_indices + capacity * 4;
    p.tail = p.head + 4;
    p.free_count = p.tail + 4;
    p.occupied = p.free_count + 4;
    p.active_count = p.occupied + 8;
    const auto end = p.active_count + 4;
    if (handles) {
      p.generations = end;
      p.handle_mask = p.generations + capacity * 4;
      p.handle_bits = p.handle_mask + 4;
      p.size = p.handle_bits + 4;
    } else {
      p.size = Align16(end);
    }
    return p;
  }
};

struct CharactersLayout {
  PoolLayout stats, auxiliary;
  std::uint32_t snapshots_per_bank, bank_stride;
  std::uint32_t stats_base, definitions, definition_count, definition_map;
  std::uint32_t tree_sentinel, snapshots, snapshot_flags, auxiliary_base;
  std::uint32_t npc_auxiliary, unlocked_list, playable_list, size;

  static constexpr CharactersLayout Make(std::uint32_t stats_capacity,
                                         std::uint32_t hero_capacity,
                                         std::uint32_t bank_stride) {
    CharactersLayout c{};
    c.stats = PoolLayout::Make(stats_capacity, 1744, true);
    c.auxiliary = PoolLayout::Make(hero_capacity, 112, false);
    c.snapshots_per_bank = hero_capacity;
    c.bank_stride = bank_stride;
    c.stats_base = 16;
    c.definitions = c.stats_base + c.stats.size;
    c.definition_count = c.definitions + DefinitionCapacity * 28;
    c.definition_map = c.definition_count + 4;
    // The existing 350-definition lookup and sentinel are not roster arrays.
    c.tree_sentinel = c.definition_map + 9172;
    c.snapshots = c.tree_sentinel + 16;
    c.snapshot_flags = c.snapshots + 2 * bank_stride;
    c.auxiliary_base = Align16(c.snapshot_flags + 2);
    c.npc_auxiliary = c.auxiliary_base + c.auxiliary.size;
    c.unlocked_list = c.npc_auxiliary + 1968;
    c.playable_list = c.unlocked_list + 704;
    c.size = c.playable_list + 704;
    return c;
  }
};

inline constexpr auto Retail = CharactersLayout::Make(44, 33, 26400);
// Spare hero-state entries accommodate the native null/default entries. The
// public selectable limit is still 40. Leave NPC auxiliary storage at 13.
// A 64 KiB bank stride is representable by one PPC addis/shift instruction;
// 48*800 is not a positive signed 16-bit addi immediate.
inline constexpr auto Expanded = CharactersLayout::Make(64, 48, 65536);
inline constexpr std::uint32_t CharacterInfoCount = 4 + PlayableHeroes * 12;
inline constexpr std::uint32_t CharacterInfoBytes = CharacterInfoCount + 4;
// CMenuTeam is placement-constructed in the shared menu pool: the factory at
// 80fd4f50 uses 22400-byte slots. Its last observed member ends at 11196.
// Reserve flags for the complete byte-valued menulocation domain so malformed
// authored locations cannot address the following native pointers. The intended
// selectable limit remains 40; this is storage, not a roster cap.
inline constexpr std::uint32_t MenuObjectEnd = 11196;
inline constexpr std::uint32_t MenuSlotBytes = 22400;
inline constexpr std::uint32_t MenuLocationFlags = Align16(MenuObjectEnd);
inline constexpr std::uint32_t MenuLocationFlagBytes = 256;

static_assert(Retail.size == 156208 && Retail.definitions == 77152);
static_assert(Retail.snapshots == 96144 && Retail.auxiliary_base == 148960);
static_assert(Retail.npc_auxiliary == 152832 && Retail.playable_list == 155504);
static_assert(Expanded.size == 271264 && Expanded.definitions == 112192);
static_assert(Expanded.snapshots == 131184 && Expanded.auxiliary_base == 262272);
static_assert(Expanded.npc_auxiliary == 267888);
static_assert(Expanded.stats.capacity <= 64 && Expanded.auxiliary.capacity <= 64);
static_assert(Expanded.snapshots_per_bank * 800 <= Expanded.bank_stride);
static_assert(Expanded.auxiliary.capacity + NpcAuxiliaryCapacity <= Expanded.stats.capacity);
static_assert(CharacterInfoBytes == 488);
static_assert(MenuLocationFlags + MenuLocationFlagBytes <= MenuSlotBytes);

// A member relocation is valid only after identifying the containing native
// structure. Never apply this to arbitrary instructions matching a number.
struct Region {
  std::uint32_t old_begin, new_begin, bytes;
};

constexpr auto PoolRegions(PoolLayout old, PoolLayout next) {
  return std::array{
    Region{0, 0, old.records},
    Region{old.records, next.records, old.capacity * old.stride},
    Region{old.free_indices, next.free_indices, old.capacity * 4},
    Region{old.head, next.head, 12},
    Region{old.occupied, next.occupied, 12},
    Region{old.generations, next.generations, old.generations ? old.capacity * 4 : 0},
    Region{old.handle_mask, next.handle_mask, old.generations ? 8u : 0u}
  };
}

template<std::size_t N>
constexpr std::optional<std::uint32_t> RelocateMember(
    const std::array<Region, N>& regions, std::uint32_t offset, std::uint32_t bytes = 1) {
  if (!bytes) return {};
  for (const auto& r : regions) {
    if (offset >= r.old_begin && std::uint64_t(offset) + bytes <=
        std::uint64_t(r.old_begin) + r.bytes)
      return r.new_begin + offset - r.old_begin;
  }
  return {};
}

inline constexpr auto StatsRegions = PoolRegions(Retail.stats, Expanded.stats);
inline constexpr auto AuxiliaryRegions = PoolRegions(Retail.auxiliary, Expanded.auxiliary);

constexpr auto CharacterRegions() {
  std::array<Region, 24> out{};
  std::size_t index = 0;
  out[index++] = {0, 0, Retail.stats_base};
  for (const auto& p : StatsRegions)
    out[index++] = {Retail.stats_base + p.old_begin,
                    Expanded.stats_base + p.new_begin, p.bytes};
  out[index++] = {Retail.definitions, Expanded.definitions, DefinitionCapacity * 28 + 4};
  out[index++] = {Retail.definition_map, Expanded.definition_map, 9172};
  out[index++] = {Retail.tree_sentinel, Expanded.tree_sentinel, 16};
  for (unsigned bank = 0; bank < 2; ++bank)
    out[index++] = {Retail.snapshots + bank * Retail.bank_stride,
                    Expanded.snapshots + bank * Expanded.bank_stride, Retail.bank_stride};
  out[index++] = {Retail.snapshot_flags, Expanded.snapshot_flags, 2};
  for (const auto& p : AuxiliaryRegions)
    out[index++] = {Retail.auxiliary_base + p.old_begin,
                    Expanded.auxiliary_base + p.new_begin, p.bytes};
  out[index++] = {Retail.npc_auxiliary, Expanded.npc_auxiliary, 1968};
  out[index++] = {Retail.unlocked_list, Expanded.unlocked_list, 704};
  out[index++] = {Retail.playable_list, Expanded.playable_list, 704};
  return out;
}
inline constexpr auto Members = CharacterRegions();

// Instruction validation is kept separate from the actual patch write. The
// runtime must validate every address/range and the complete supported code
// hashes before any write; an individual matching instruction is insufficient.
struct InstructionPatch {
  std::uint32_t address, original, replacement;
};
enum class PatchState { Original, Installed, Invalid };

template<class ReadWord>
PatchState InspectPatchWords(std::span<const InstructionPatch> patches, ReadWord read) {
  if (patches.empty()) return PatchState::Invalid;
  bool original = true, installed = true;
  std::uint32_t previous = 0;
  for (const auto& p : patches) {
    if ((p.address & 3) || p.address <= previous || p.original == p.replacement)
      return PatchState::Invalid;
    previous = p.address;
    const std::optional<std::uint32_t> word = read(p.address);
    if (!word) return PatchState::Invalid;
    original &= *word == p.original;
    installed &= *word == p.replacement;
  }
  if (original) return PatchState::Original;
  if (installed) return PatchState::Installed;
  return PatchState::Invalid; // Includes partially installed layouts.
}

// Only for reviewed D-form signed-immediate instructions. Overflow must fail,
// rather than wrap into an access before the expanded object.
constexpr std::optional<std::uint32_t> SignedImmediate(std::uint32_t instruction,
                                                       std::int64_t value) {
  if (value < -32768 || value > 32767) return {};
  return (instruction & 0xffff0000u) | (std::uint32_t(value) & 0xffffu);
}

} // namespace moderngekko::roster
