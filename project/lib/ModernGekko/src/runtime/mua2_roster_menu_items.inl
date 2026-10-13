// Included after roster persistence helpers. This relocates the shared menu-item
// tree, not the CMenu object or any derived class fields. No menu is forced open.
namespace roster_menu {
using namespace moderngekko::roster;
using roster_io::Bytes;
using roster_io::Word;
using roster_io::PutWord;
bool installed=false;
std::map<u32,u32> allocations;
struct Allocation {u32 menu=0, lr=0;std::array<u32,32> registers{};};
std::optional<Allocation> pending_allocation;
u32 inserting_menu=0, object_pool=0;
constexpr u32 ObjectAllocationOffset=(MenuMapBytes+15)&~15u;

u32 Map(const Core::CPUThreadGuard& guard,u32 menu) {
  const auto it=allocations.find(menu);
  if(it==allocations.end())return 0;
  const auto* field=Bytes(guard,menu+MenuMapPointer,4);
  return field && Word(field)==it->second ? it->second:0;
}
void Initialize(const Core::CPUThreadGuard& guard,u32 menu,u32 map) {
  auto* b=Bytes(guard,map,MenuMapBytes);
  auto* slot=Bytes(guard,menu+MenuMapPointer,4);
  if(!b || !slot) {roster_io::Abort(guard,"menu item allocation is unmapped");return;}
  std::fill_n(b,MenuMapBytes,0);
  PutWord(b,0x3fffffff);PutWord(b+4,0xffffffff);PutWord(b+8,0x8118fb90);
  PutWord(b+MenuMapFree,MenuItems);
  for(u32 i=0;i<MenuItems;++i)PutWord(b+MenuMapQueue+i*4,i);
  PutWord(slot,map);
}
// All native menu constructors converge at this epilogue before loading XML.
// One retained allocation per native menu pool slot lives for this game boot.
bool Constructed(const Core::CPUThreadGuard& guard) {
  if(!installed)return false;
  auto& st=guard.GetSystem().GetPPCState();
  if(pending_allocation) {
    const auto saved=*pending_allocation;const u32 map=st.gpr[3];
    pending_allocation.reset();
    std::copy(saved.registers.begin(),saved.registers.end(),std::begin(st.gpr));
    st.spr[8]=saved.lr;
    if(!map)return roster_io::Abort(guard,"menu item heap allocation failed");
    if(!object_pool) {
      const auto* manager_slot=Bytes(guard,0x8081736c,4);
      const u32 manager=manager_slot?Word(manager_slot):0;
      auto* old_count=manager?Bytes(guard,manager+27728+276432,4):nullptr;
      if(!old_count || Word(old_count)!=0)
        return roster_io::Abort(guard,"shared menu pool was not empty at installation");
      object_pool=map+ObjectAllocationOffset;
      auto* bytes=Bytes(guard,object_pool,MenuObjectBytes);
      if(!bytes)return roster_io::Abort(guard,"shared menu pool allocation is unmapped");
      std::fill_n(bytes,MenuObjectBytes,0);
      PutWord(bytes+MenuObjectFree,MenuObjectCapacity);
      for(u32 i=0;i<MenuObjectCapacity;++i)PutWord(bytes+MenuObjectQueue+4*i,i);
      PutWord(Bytes(guard,manager+27728,4),object_pool);
    }
    allocations[saved.menu]=map;Initialize(guard,saved.menu,map);
    return false;
  }
  const u32 menu=st.gpr[3];if(!menu)return false;
  if(const auto it=allocations.find(menu);it!=allocations.end()) {
    Initialize(guard,menu,it->second);return false;
  }
  if(allocations.size()>=16)return roster_io::Abort(guard,"unexpected menu pool count");
  const auto* old=Bytes(guard,menu+MenuMapPointer,5096);
  if(!old || Word(old+8)!=0x8118fb90 || Word(old+4252)!=0)
    return roster_io::Abort(guard,"menu map was not empty after construction");
  Allocation saved;saved.menu=menu;saved.lr=st.spr[8];
  std::copy(std::begin(st.gpr),std::end(st.gpr),saved.registers.begin());
  pending_allocation=saved;
  st.gpr[3]=object_pool?MenuMapBytes:ObjectAllocationOffset+MenuObjectBytes;st.gpr[4]=3;st.gpr[5]=0x80530380;
  st.gpr[6]=0x8052f0f8;st.gpr[7]=0;st.spr[8]=st.pc;st.npc=0x801f1380;
  return true;
}
void Count(const Core::CPUThreadGuard& guard) {
  if(!installed)return;
  auto& st=guard.GetSystem().GetPPCState();
  const u32 map=Map(guard,st.gpr[st.pc==0x80fcb204?27:30]);
  const auto* count=map?Bytes(guard,map+MenuMapCount,4):nullptr;
  if(!count) {roster_io::Abort(guard,"menu count has no sidecar");return;}
  st.gpr[0]=Word(count);
}
// Preserve the native insertion/balancing algorithm. Its inlined accesses are
// all map-relative in this bounded block; r27 resumes its menu role afterward.
void BeginInsert(const Core::CPUThreadGuard& guard) {
  if(!installed)return;
  auto& st=guard.GetSystem().GetPPCState();const u32 menu=st.gpr[27];
  const u32 map=Map(guard,menu);
  if(!map || inserting_menu) {roster_io::Abort(guard,"invalid menu insertion context");return;}
  inserting_menu=menu;st.gpr[27]=map-MenuMapPointer;
}
void EndInsert(const Core::CPUThreadGuard& guard) {
  if(!installed)return;
  if(!inserting_menu) {roster_io::Abort(guard,"missing menu insertion context");return;}
  guard.GetSystem().GetPPCState().gpr[27]=inserting_menu;inserting_menu=0;
}
void Install(const Core::CPUThreadGuard& guard) {
  if(installed || !s_roster_storage_installed)return;
  auto& system=guard.GetSystem();auto& memory=system.GetMemory();
  const auto read=[&](u32 a)->std::optional<std::uint32_t> {
    const auto* b=Bytes(guard,a,4);return b?std::optional(Word(b)):std::nullopt;
  };
  if(InspectPatchWords(MenuObjectPatches,read)!=PatchState::Original ||
     InspectPatchWords(MenuStringRelPatches,read)!=PatchState::Original ||
     InspectPatchWords(MenuItemPatches,read)!=PatchState::Original ||
     read(0x80fd6c58)!=(0x48000001u | ((0x80514174u-0x80fd6c58u)&0x03fffffcu)) || read(0x80fcb7f8)!=0x80610014) {
    for(const auto& p:MenuItemPatches)if(read(p.address)!=p.original)
      std::fprintf(stderr,"[openmua2] menu guard %08x expected=%08x actual=%08x\n",p.address,p.original,read(p.address).value_or(0));
    std::fprintf(stderr,"[openmua2] menu epilogue guard=%08x end=%08x\n",read(0x80fd6c58).value_or(0),read(0x80fcb7f8).value_or(0));
    roster_io::Abort(guard,"menu item instruction guard mismatch");return;
  }
  const bool ok=HLE::SetExternalBranchReplacement(guard,0x80fd6c58,Constructed) &&
    HLE::SetExternalStartObserver(guard,0x80fcb204,Count) &&
    HLE::SetExternalStartObserver(guard,0x80fd018c,Count) &&
    HLE::SetExternalStartObserver(guard,0x80fcb6f0,BeginInsert) &&
    HLE::SetExternalStartObserver(guard,0x80fcb7f8,EndInsert);
  if(!ok) {
    std::fprintf(stderr,"[openmua2] menu item hooks rejected; registered=%u\n",HLE::GetExternalHookCount());
    roster_io::Abort(guard,"menu item lifecycle hooks rejected");return;
  }
  for(const auto plan:{std::span<const InstructionPatch>(MenuItemPatches),std::span<const InstructionPatch>(MenuStringRelPatches),std::span<const InstructionPatch>(MenuObjectPatches)})
  for(const auto& patch:plan) {
    PutWord(Bytes(guard,patch.address,4),patch.replacement);
    system.GetPPCState().iCache.Invalidate(memory,system.GetJitInterface(),patch.address);
  }
  Host_JitCacheInvalidation();

  installed=true;
  std::fprintf(stderr,"[openmua2] roster menu item capacity: %u, %zu guarded edits\n",MenuItems,MenuItemPatches.size()+MenuStringRelPatches.size()+MenuObjectPatches.size());
  std::fprintf(stderr,"[openmua2] registered runtime hooks: %u\n",HLE::GetExternalHookCount());
}
void Reset() {installed=false;allocations.clear();pending_allocation.reset();inserting_menu=0;object_pool=0;}
} // namespace roster_menu
