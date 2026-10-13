// Included after ProviderCode. Development-only until selection and persistence
// acceptance are complete. Installation happens at DOL startup, before native
// constructors and before any of the replaced blocks can be executing.
bool s_roster_boot_attempted = false;
bool s_roster_storage_installed = false;
bool s_roster_menu_installed = false;
#include "mua2_roster_persistence.inl"
#include "mua2_roster_menu_items.inl"

void ClearRosterMenuLocations(const Core::CPUThreadGuard& guard) {
  using namespace moderngekko::roster;
  if (!s_roster_menu_installed) return;
  const u32 menu = guard.GetSystem().GetPPCState().gpr[3];
  auto* object = guard.GetSystem().GetMemory().GetPointerForRange(menu, MenuSlotBytes);
  if (!object) return;
  std::fill_n(object + MenuLocationFlags, MenuLocationCount, EmptyMenuLocation);
}

void ObserveRosterMenuConstructor(const Core::CPUThreadGuard& guard) {
  using namespace moderngekko::roster;
  if (!s_roster_storage_installed) return;
  if (!s_roster_menu_installed) {
    if (!ProviderCode(guard, 0x80f712c4, 16, "59ED9902B4D979C177EC8B816E536245067B63D4")) return;
    auto& system = guard.GetSystem();
    auto& memory = system.GetMemory();
    const auto read = [&](u32 address) -> std::optional<std::uint32_t> {
      const auto* bytes = memory.GetPointerForRange(address, 4);
      if (!bytes) return {};
      return (u32(bytes[0]) << 24) | (u32(bytes[1]) << 16) |
             (u32(bytes[2]) << 8) | bytes[3];
    };
    // The file hash was checked before boot. Check the loaded instruction words
    // as one transaction too; these sites contain no REL relocations.
    if (InspectPatchWords(MenuPatches, read) != PatchState::Original) {
      std::fprintf(stderr, "[openmua2] roster menu rejected: instruction mismatch\n");
      return;
    }
    for (const auto& patch : MenuPatches) {
      auto* bytes = memory.GetPointerForRange(patch.address, 4);
      for (unsigned b = 0; b < 4; ++b) bytes[b] = u8(patch.replacement >> (24 - b * 8));
      system.GetPPCState().iCache.Invalidate(memory, system.GetJitInterface(), patch.address);
    }
    Host_JitCacheInvalidation();
    s_roster_menu_installed = true;
    std::fprintf(stderr, "[openmua2] roster menu installed: 40 heroes, 256 location bytes\n");
  }
  ClearRosterMenuLocations(guard);
}

void ObserveRosterBoot(const Core::CPUThreadGuard& guard) {
  using namespace moderngekko::roster;
  if (s_roster_boot_attempted) return;
  s_roster_boot_attempted = true;
  if (!ProviderCode(guard, 0x80194dd8, 112, "1C4B3BF0560FAA1199581A5D8F9981B23E9819F8")) return;
  const auto plan = PreparedDolPatches();
  if (!plan) {
    std::fprintf(stderr, "[openmua2] roster storage rejected: invalid patch plan\n");
    return;
  }
  for (const auto& span : StorageCodeGuards)
    if (!ProviderCode(guard, span.address, span.bytes, span.sha1)) {
      std::fprintf(stderr, "[openmua2] roster storage rejected: code guard %08x\n", span.address);
      return;
    }
  for (const auto& span : CharacterInfoCodeGuards)
    if (!ProviderCode(guard, span.address, span.bytes, span.sha1)) {
      std::fprintf(stderr, "[openmua2] roster storage rejected: character-info guard %08x\n", span.address);
      return;
    }
  auto& system = guard.GetSystem();
  auto& memory = system.GetMemory();
  const auto read = [&](u32 address) -> std::optional<std::uint32_t> {
    const auto* bytes = memory.GetPointerForRange(address, 4);
    if (!bytes) return {};
    return (u32(bytes[0]) << 24) | (u32(bytes[1]) << 16) |
           (u32(bytes[2]) << 8) | bytes[3];
  };
  if (InspectPatchWords(*plan, read) != PatchState::Original) {
    std::fprintf(stderr, "[openmua2] roster storage rejected: original instruction mismatch\n");
    return;
  }
  // Validate all writable ranges before the first mutation. Guest execution is
  // stopped in this CPU-thread callback; installation cannot be partially used.
  std::vector<u8*> destinations;
  destinations.reserve(plan->size());
  for (const auto& patch : *plan) {
    auto* bytes = memory.GetPointerForRange(patch.address, 4);
    if (!bytes) return;
    destinations.push_back(bytes);
  }
  for (std::size_t i = 0; i < plan->size(); ++i) {
    const auto word = (*plan)[i].replacement;
    for (unsigned b = 0; b < 4; ++b) destinations[i][b] = u8(word >> (24 - b * 8));
  }
  for (const auto& patch : *plan)
    system.GetPPCState().iCache.Invalidate(memory, system.GetJitInterface(), patch.address);
  Host_JitCacheInvalidation();
  if (!roster_io::Install(guard)) {
    roster_io::Abort(guard, "save/load hooks could not be installed");
    return;
  }
  s_roster_storage_installed = true;
  std::fprintf(stderr,
      "[openmua2] roster storage installed: %zu sites, manager=%u bytes, "
      "charinfo=%u, fullstats=%u, hero-state=%u; named save/load enabled\n",
      plan->size(), Expanded.size, PlayableHeroes, Expanded.stats.capacity,
      Expanded.auxiliary.capacity);
}

void InstallRosterExpansion(const Core::CPUThreadGuard& guard, const std::filesystem::path& root) {
  s_roster_boot_attempted = false;
  s_roster_storage_installed = false;
  s_roster_menu_installed = false;
  std::ifstream input(root / "files" / "Marvel-rev-fin-plf2.rel", std::ios::binary);
  const std::vector<char> bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
  if (input.bad() || Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(bytes)) !=
                         "A3653F6739E572F7D93218102BB5D05BC141E9E7") {
    std::fprintf(stderr, "[openmua2] roster expansion rejected: unsupported REL\n");
    return;
  }
  roster_menu::Reset();
  const bool factory = HLE::SetExternalStartObserver(guard, 0x80fd4e28, roster_menu::Install);
  const bool constructor = HLE::SetExternalStartObserver(guard, 0x80f712c4, ObserveRosterMenuConstructor);
  const bool refresh = HLE::SetExternalStartObserver(guard, 0x80f80acc, ClearRosterMenuLocations);
  if (!factory || !constructor || !refresh) {
    std::fprintf(stderr, "[openmua2] roster expansion rejected: menu lifecycle hooks\n");
    return;
  }
  const bool ok = HLE::SetExternalStartObserver(guard, 0x80005f70, ObserveRosterBoot);
  std::fprintf(stderr, "[openmua2] roster startup observer %s\n",
               ok ? "armed" : "rejected");
}
