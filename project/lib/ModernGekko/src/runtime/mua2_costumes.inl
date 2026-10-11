// Included in dolphin_runtime.cpp's private namespace after ProviderCode.
struct CostumeDraft { std::string name; moderngekko::controls::CostumeSet set; };
std::map<u32,CostumeDraft> s_costume_drafts;
std::map<std::string,moderngekko::controls::CostumeSet> s_costumes;
std::set<std::string> s_costume_files;
std::filesystem::path s_costume_root;
bool s_costume_hooks=false, s_costume_menu_hooks=false;
std::set<u32> s_costume_registered_hooks;
struct CostumeRefresh {u32 menu=0, object=0, widget=0, pool=0, stack=0; unsigned phase=0;};
CostumeRefresh s_costume_refresh;
std::map<u32,u32> s_costume_preview_widgets;
struct CostumeAnimation {std::string name;double start=0,blend=0;};
std::map<u32,CostumeAnimation> s_costume_preview_animations;
u32 s_costume_changed=0, s_costume_refresh_actor=0, s_costume_actor_stack=0;
unsigned s_costume_actor_phase=0;
void InstallCostumeHooks(const Core::CPUThreadGuard& guard);

std::string CostumeString(const Core::CPUThreadGuard& guard,u32 address,unsigned limit=64) {
  std::string out;
  for(unsigned i=0;i<limit;++i) {
    const u64 a=u64(address)+i;
    if(!((a>=0x80000000 && a<0x81800000)||(a>=0x90000000 && a<0x94000000)))return {};
    const auto* b=guard.GetSystem().GetMemory().GetPointerForRange(u32(a),1);
    if(!b)return {};if(!*b)return out;if(*b<32 || *b>126)return {};
    out.push_back(char(*b));
  }
  return {};
}
u32 CostumeBrowseMenu(const Core::CPUThreadGuard& guard) {
  const auto read=[&](u32 a,std::size_t n)->std::span<const u8> {
    const auto* b=guard.GetSystem().GetMemory().GetPointerForRange(a,n);
    return b?std::span<const u8>(b,n):std::span<const u8>{};
  };
  if(moderngekko::controls::ActiveMenuType(read)!=0x8118f978)return 0;
  const u32 manager=moderngekko::controls::ReadBE(read(0x8081736c,4),0);
  const u32 menu=moderngekko::controls::ReadBE(read(manager+25860,4),0);
  const auto page=read(menu+5300,1);
  return page.size()==1 && page[0]==0?menu:0;
}
bool CostumeInstalled(std::string name,const std::string& model) {
  std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return char(std::tolower(c));});
  const auto stem="packages/generated/characters/"+name+"_"+model;
  // Both menu and world packages are required. A declaration with missing
  // resources must not replace a working model with a failed load.
  for(const auto& suffix:{".fb","_nc.fb"}) {
    const auto path=stem+suffix;
    if(!s_costume_files.contains(path) && !std::filesystem::is_regular_file(s_costume_root/path))return false;
  }
  return true;
}
const moderngekko::controls::CostumeSet* CostumeDefinition(const Core::CPUThreadGuard& guard,u32 object) {
  auto it=s_costumes.find(CostumeString(guard,object+0x2ec,32));
  return it==s_costumes.end()?nullptr:&it->second;
}
void ObserveCostumeReset(const Core::CPUThreadGuard& guard) {
  if(s_gamepad_provider && ProviderCode(guard,0x8018b690,64,"4F583C4CA077C3C83B6F5D4D038913F08EBE0468")) {
    InstallCostumeHooks(guard);
    s_costume_drafts.erase(guard.GetSystem().GetPPCState().gpr[29]);
  }
}
bool ReplaceCostumeProperty(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !ProviderCode(guard,0x8018be8c,64,"7AEA6DBBB3AC406990505BE9F4132C0864F284E5"))return false;
  auto& state=guard.GetSystem().GetPPCState();
  const auto key=CostumeString(guard,state.gpr[4]),value=CostumeString(guard,state.gpr[5]);
  const auto slot=moderngekko::controls::CostumeSet::Slot(key);
  if(key!="name" && !slot)return false;
  auto& draft=s_costume_drafts[state.gpr[3]];
  if(key=="name")draft.name=value;else draft.set.Set(key,value);
  if(!draft.name.empty() && !draft.set.skins[0].empty())s_costumes[draft.name]=draft.set;
  if(!slot || *slot==0)return false; // Native name/default parsing stays intact.
  if(*slot<6) {
    const auto model=draft.set.Model(*slot);
    auto* cell=guard.GetSystem().GetMemory().GetPointerForRange(state.gpr[3]+0x41c+*slot,1);
    if(!cell)return false;
    *cell=model.size()>=2?u8(std::stoi(model.substr(model.size()-2))):0;
  }
  state.gpr[3]=1;state.npc=state.spr[8];return true;
}
bool ReplaceCostumeModel(const Core::CPUThreadGuard& guard) {
  auto& state=guard.GetSystem().GetPPCState();
  struct Site {u32 pc;unsigned object;const char* hash;};
  static constexpr Site sites[]{
    {0x8018dce8,26,"E803F40A43CD68B7676407E31A91E9300A37B00B"},
    {0x8018de10,28,"6264F51C6F9AB134537EAE77FD5431B63EA82692"},
    {0x8018df20,28,"33BECB949B74F5E5DB832E2D5530EC50F58D8CA9"},
    {0x8019455c,27,"4E447B3E865A97F9774C5929D9C9AC8F970AF2E8"},
    {0x801946f4,27,"85B13CA773B879D8948BE87966DB232A7C2639CA"},
    {0x8019489c,27,"D05CE088BA7AB541894437D228182479D22CEEFD"}};
  if(!s_gamepad_provider)return false;
  for(const auto& site:sites)if(state.pc==site.pc) {
    if(!ProviderCode(guard,site.pc,64,site.hash))return false;
    const auto* set=CostumeDefinition(guard,state.gpr[site.object]);if(!set)return false;
    const auto model=set->Model(state.gpr[0]);if(model.empty())return false;
    if(state.gpr[4]<model.size()+1 || state.gpr[4]>256)return false;
    auto* dest=guard.GetSystem().GetMemory().GetPointerForRange(state.gpr[3],model.size()+1);
    if(!dest)return false;std::memcpy(dest,model.c_str(),model.size()+1);
    state.gpr[3]=u32(model.size());state.npc=site.pc+4;return true;
  }
  return false;
}
bool CycleCostume(const Core::CPUThreadGuard& guard,u32 object) {
  const auto* set=CostumeDefinition(guard,object);if(!set)return false;
  auto* slot=guard.GetSystem().GetMemory().GetPointerForRange(object+0x2ac,1);if(!slot)return false;
  const auto name=CostumeString(guard,object+0x2ec,32);
  const unsigned next=set->Next(*slot,[&](const auto& model){return CostumeInstalled(name,model);});
  if(next==*slot)return false;
  std::fprintf(stderr,"[openmua2] costume character=%s slot=%u -> %u model=%s\n",name.c_str(),unsigned(*slot)+1,next+1,set->Model(next).c_str());
  *slot=u8(next);return true;
}
bool ReplaceCostumeCycle(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !ProviderCode(guard,0x8018da08,64,"B667D3AFCA57AEDBE377A36BD41D556C8FC44E2C"))return false;
  auto& state=guard.GetSystem().GetPPCState();const u32 object=state.gpr[3];
  if(!CostumeDefinition(guard,object))return false;
  CycleCostume(guard,object);
  const auto* slot=guard.GetSystem().GetMemory().GetPointerForRange(object+0x2ac,1);if(!slot)return false;
  state.gpr[3]=*slot;state.npc=state.spr[8];return true;
}
bool ReplaceCostumeUnlocked(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !ProviderCode(guard,0x8018db8c,64,"9048E33FB085EFF925336540A22AACB3699CBE3F"))return false;
  auto& state=guard.GetSystem().GetPPCState();const auto* set=CostumeDefinition(guard,state.gpr[3]);
  if(!set)return false;const auto model=set->Model(state.gpr[4]);
  const bool available=!model.empty() && (state.gpr[4]==0 || CostumeInstalled(CostumeString(guard,state.gpr[3]+0x2ec,32),model));
  state.gpr[3]=available;state.npc=state.spr[8];return true;
}
bool ReplaceCostumeMenuInput(const Core::CPUThreadGuard& guard) {
  auto& state=guard.GetSystem().GetPPCState();const auto menu=CostumeBrowseMenu(guard);
  if(!s_gamepad_provider || !s_costume_hooks || !s_costume_menu_hooks || !menu || state.gpr[31]!=menu)return false;
  const bool browse=state.pc==0x80f7d994;
  if(!ProviderCode(guard,state.pc,64,browse?"656677E4D275C5CEA50C157A8C3AD28C9DFA200D":"164A4C1088EF5C531D7C19ECFAF982FF438D6276"))return false;
  // Execute the native ownership sequence on the game thread. In particular,
  // no menu preview may retain the old skeleton while the live actor is rebuilt.
  if(s_costume_refresh.phase && s_costume_refresh.menu==menu && (s_costume_refresh.stack==state.gpr[1] || (s_costume_refresh.phase==5 && s_costume_refresh.stack==state.gpr[1]+0x100))) {
    auto& pending=s_costume_refresh;
    state.spr[8]=state.pc;
    if(pending.phase==1) {
      pending.phase=2;
      state.gpr[3]=pending.object;state.gpr[4]=pending.pool;
      state.gpr[5]=u32(-1);state.gpr[6]=2;state.npc=0x801945f8;return true;
    }
    if(pending.phase==2) {
      pending.phase=3;s_costume_changed=pending.object;
      auto* dirty=guard.GetSystem().GetMemory().GetPointerForRange(0x812d0e44,4);
      if(dirty) {
        const u32 value=moderngekko::controls::ReadBE(std::span<const u8>(dirty,4),0)|(1u<<(pending.pool-7));
        for(unsigned i=0;i<4;++i)dirty[i]=u8(value>>(24-8*i));
      }
      state.gpr[3]=menu;state.npc=0x80f836d4;return true;
    }
    if(pending.phase==3) {
      pending.phase=4;
      // A selected-slot refresh also clears its native preview attachments.
      state.gpr[3]=menu;state.gpr[4]=pending.pool-7;state.npc=0x80f80acc;return true;
    }
    if(pending.phase==4) {
      const auto& animation=s_costume_preview_animations.at(pending.widget);
      auto* scratch=guard.GetSystem().GetMemory().GetPointerForRange(pending.stack-0x100,0x100);
      std::memset(scratch,0,0x100);
      for(unsigned i=0;i<4;++i)scratch[i]=u8(pending.stack>>(24-8*i));
      std::memcpy(scratch+0x10,animation.name.c_str(),animation.name.size()+1);
      pending.phase=5;state.gpr[1]=pending.stack-0x100;
      state.gpr[3]=pending.widget;state.gpr[4]=state.gpr[1]+0x10;
      state.ps[1].SetPS0(animation.start);state.ps[2].SetPS0(animation.blend);
      state.npc=0x80fe534c;return true;
    }
    state.gpr[1]=pending.stack;
    pending={};s_costume_changed=0;
    state.gpr[3]=1;state.npc=browse?0x80f7db5c:0x80f7c9d8;return true;
  }
  if(!ProviderCode(guard,0x80fe4c74,64,"B690A03C8D813BEE5FD0E396A3BDE0E949FC132E") ||
     !ProviderCode(guard,0x801945f8,64,"5D93DB09D64A9E384C4B8252482F195A60F1934C") ||
     !ProviderCode(guard,0x80f836d4,64,"90E7C5C47E1E3BA3B46393DB659ED3F7F0B10D7D") ||
     !ProviderCode(guard,0x80f80acc,64,"FCB6B111CBEE677589266E98FFCB1E42403CEE81") ||
     !ProviderCode(guard,0x80fe534c,64,"A31C0D3582BB17DB94BC23B992D4DFB7E3C40472"))return false;
  const auto* bits=guard.GetSystem().GetMemory().GetPointerForRange(state.gpr[3],20);
  if(!bits || !(moderngekko::controls::ReadBE(std::span<const u8>(bits,20),12)&(1u<<10)))return false; // menu edge action106
  const auto* selected=guard.GetSystem().GetMemory().GetPointerForRange(0x812d0e58,4);
  const auto* flags=guard.GetSystem().GetMemory().GetPointerForRange(0x812d0e48,1);
  if(!selected || !flags)return false;
  const u32 object=moderngekko::controls::ReadBE(std::span<const u8>(selected,4),0);
  const auto found=s_costume_preview_widgets.find(object);
  if(found==s_costume_preview_widgets.end() || ((*flags>>5)&7)>3)return false;
  const u32 widget=found->second;
  const auto* displayed=guard.GetSystem().GetMemory().GetPointerForRange(widget+0x100,4);
  if(!displayed || moderngekko::controls::ReadBE(std::span<const u8>(displayed,4),0)!=object)return false;
  const auto animation=s_costume_preview_animations.find(widget);
  if(animation==s_costume_preview_animations.end() || animation->second.name.empty() || animation->second.name.size()>127 ||
     state.gpr[1]<0x80000100 || state.gpr[1]>=0x81800000 || (state.gpr[1]&15) ||
     !guard.GetSystem().GetMemory().GetPointerForRange(state.gpr[1]-0x100,0x100))return false;
  if(!CycleCostume(guard,object)) {state.gpr[3]=0;state.npc=state.pc+4;return true;}
  s_costume_refresh={menu,object,widget,u32(7+((*flags>>5)&7)),state.gpr[1],1};
  state.gpr[3]=widget;state.gpr[4]=0;state.gpr[5]=s_costume_refresh.pool;
  state.spr[8]=state.pc;state.npc=0x80fe4c74;return true;
}

void ObserveCostumePreviewOwner(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !ProviderCode(guard,0x80fe4ca0,64,"B18418FCE304EADE5FF97F0DDFA2069D96181BB4"))return;
  const auto& st=guard.GetSystem().GetPPCState();
  // Record ownership from the native model-binding call, not UI focus: focus
  // can temporarily point at a collision item while the selected hero stays put.
  if(st.gpr[4] && CostumeDefinition(guard,st.gpr[4]))
    s_costume_preview_widgets[st.gpr[4]]=st.gpr[29];
}
bool RetainCostumeLiveResources(const Core::CPUThreadGuard& guard) {
  auto& st=guard.GetSystem().GetPPCState();
  if(!s_gamepad_provider || s_costume_refresh.phase!=2 || st.gpr[27]!=s_costume_refresh.object ||
     st.gpr[3]!=s_costume_refresh.pool ||
     !ProviderCode(guard,0x801947b8,64,"4B3A13DDE233FA563B7009F3F246D5933FAE385C"))return false;
  // A costume change keeps the live actor: its animation resources must remain
  // valid while the replacement package loads. Normal hero teardown still owns
  // the pool and releases all of its resources when the hero is removed.
  st.npc=st.pc+4;return true;
}
bool ReplaceCostumeActorRefresh(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !ProviderCode(guard,0x80f81c70,64,"C1E208BE85220E59F670B39034C2ACDEA2F4E022"))return false;
  auto& state=guard.GetSystem().GetPPCState();
  if(s_costume_actor_phase==1) {
    const u32 pool=state.gpr[3];
    s_costume_actor_phase=2;
    state.gpr[3]=s_costume_refresh_actor;state.gpr[4]=state.gpr[1]+0x10;
    state.gpr[5]=pool;state.gpr[6]=0;
    state.spr[8]=state.pc;state.npc=0x80068738;return true;
  }
  if(s_costume_actor_phase==2) {
    if(!state.gpr[3])std::fprintf(stderr,"[openmua2] costume model load failed for actor %08x\n",s_costume_refresh_actor);
    state.gpr[1]=s_costume_actor_stack;state.gpr[3]=s_costume_refresh_actor;
    s_costume_actor_phase=0;s_costume_actor_stack=0;s_costume_refresh_actor=0;
    return false; // Continue the original native stat refresh.
  }
  if(!s_costume_changed)return false;
  const u32 actor=state.gpr[3];
  const auto* stats=guard.GetSystem().GetMemory().GetPointerForRange(actor+0x398,4);
  if(!stats || moderngekko::controls::ReadBE(std::span<const u8>(stats,4),0)!=s_costume_changed)return false;
  const auto* definition=CostumeDefinition(guard,s_costume_changed);
  const auto* slot=guard.GetSystem().GetMemory().GetPointerForRange(s_costume_changed+0x2ac,1);
  if(!definition || !slot)return false;
  const auto model=definition->Model(*slot);if(model.empty() || model.size()>63)return false;
  if(!ProviderCode(guard,0x80045510,64,"6763E66321FAB2D039D5C0D79D32D408D87B1189") ||
     !ProviderCode(guard,0x80068738,64,"A0235E2896E1ACE0D863E906E6D2373CFA354372"))return false;
  const u32 stack=state.gpr[1];
  if(stack<0x80000080 || stack>=0x81800000 || (stack&15))return false;
  auto* scratch=guard.GetSystem().GetMemory().GetPointerForRange(stack-0x80,0x80);
  if(!scratch)return false;
  std::memset(scratch,0,0x80);
  for(unsigned i=0;i<4;++i)scratch[i]=u8(stack>>(24-8*i));
  std::memcpy(scratch+0x10,model.c_str(),model.size()+1);
  s_costume_changed=0;s_costume_refresh_actor=actor;s_costume_actor_stack=stack;s_costume_actor_phase=1;
  // Match the native script's skin-change path: use the actor's resource pool
  // and preserve animation state instead of resetting its entire body.
  state.gpr[1]=stack-0x80;state.spr[8]=state.pc;state.npc=0x80045510;return true;
}
void ObserveCostumeAnimationSelected(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !CostumeBrowseMenu(guard) ||
     !ProviderCode(guard,0x80fe534c,64,"A31C0D3582BB17DB94BC23B992D4DFB7E3C40472"))return;
  const auto& st=guard.GetSystem().GetPPCState();
  const auto name=CostumeString(guard,st.gpr[4],128);
  if(!name.empty())s_costume_preview_animations[st.gpr[3]]={name,st.ps[1].PS0AsDouble(),st.ps[2].PS0AsDouble()};
}
void ObserveCostumeWidgetText(const Core::CPUThreadGuard& guard) {
  const auto menu=CostumeBrowseMenu(guard);if(!s_gamepad_provider || !menu)return;
  const auto& st=guard.GetSystem().GetPPCState();
  // The native formatted-label setter owns a 1024-byte stack buffer here.
  // Change this page's real Details label before the native widget consumes it.
  if(st.spr[8]!=0x80fd050c || st.gpr[4]!=st.gpr[1]+0x78 || !st.gpr[5])return;
  if(!ProviderCode(guard,0x80fcfe20,64,"2CEF8282D101C08B5D1B071D5C3F138BFDB55154"))return;
  if(CostumeString(guard,st.gpr[4],128)!="$MENU_OTHER Details")return;
  constexpr char text[]="$XX Skin   $XY Details";
  auto* dest=guard.GetSystem().GetMemory().GetPointerForRange(st.gpr[4],sizeof(text));
  if(dest)std::memcpy(dest,text,sizeof(text));
}
void InstallCostumes(const Core::CPUThreadGuard& guard,const std::filesystem::path& root) {
  s_costume_drafts.clear();s_costumes.clear();s_costume_files.clear();s_costume_root=root/"files";
  std::error_code error;
  for(const auto& entry:std::filesystem::directory_iterator(s_costume_root/"z",error))
    if(entry.is_regular_file() && entry.path().extension()==".wad")moderngekko::controls::ReadCostumeArchiveNames(entry.path(),s_costume_files);
  HLE::SetExternalStartObserver(guard,0x8018b690,ObserveCostumeReset);
  s_costume_hooks=false;s_costume_menu_hooks=false;s_costume_registered_hooks.clear();
  s_costume_preview_widgets.clear();s_costume_preview_animations.clear();s_costume_refresh={};s_costume_changed=0;s_costume_refresh_actor=0;s_costume_actor_stack=0;s_costume_actor_phase=0;
}
void InstallCostumeHooks(const Core::CPUThreadGuard& guard) {
  // BootCore is asynchronous: install only after each code region is present.
  // Remember each successful registration so a partial attempt can retry safely.
  const auto install=[&](u32 address,auto callback,auto registrar) {
    if(s_costume_registered_hooks.contains(address))return true;
    if(!registrar(guard,address,callback))return false;
    s_costume_registered_hooks.insert(address);return true;
  };
  if(!s_costume_hooks && ProviderCode(guard,0x8018be8c,64,"7AEA6DBBB3AC406990505BE9F4132C0864F284E5")) {
    bool ok=install(0x8018be8c,ReplaceCostumeProperty,HLE::SetExternalFunctionReplacement);
    for(u32 pc:{0x8018dce8u,0x8018de10u,0x8018df20u,0x8019455cu,0x801946f4u,0x8019489cu})
      ok=install(pc,ReplaceCostumeModel,HLE::SetExternalBranchReplacement) && ok;
    ok=install(0x801947b8,RetainCostumeLiveResources,HLE::SetExternalBranchReplacement) && ok;
    ok=install(0x8018da08,ReplaceCostumeCycle,HLE::SetExternalFunctionReplacement) && ok;
    ok=install(0x8018db8c,ReplaceCostumeUnlocked,HLE::SetExternalFunctionReplacement) && ok;
    s_costume_hooks=ok;
    std::fprintf(stderr,"[openmua2] costume definition hooks %s\n",ok?"installed":"incomplete");
  }
  if(!s_costume_menu_hooks && ProviderCode(guard,0x80f7d994,64,"656677E4D275C5CEA50C157A8C3AD28C9DFA200D")) {
    bool ok=install(0x80fcfe20,ObserveCostumeWidgetText,HLE::SetExternalStartObserver);
    ok=install(0x80f7d994,ReplaceCostumeMenuInput,HLE::SetExternalBranchReplacement) && ok;
    ok=install(0x80f7c718,ReplaceCostumeMenuInput,HLE::SetExternalBranchReplacement) && ok;
    ok=install(0x80fe4ca0,ObserveCostumePreviewOwner,HLE::SetExternalStartObserver) && ok;
    ok=install(0x80fe534c,ObserveCostumeAnimationSelected,HLE::SetExternalStartObserver) && ok;
    ok=install(0x80f81c70,ReplaceCostumeActorRefresh,HLE::SetExternalBranchReplacement) && ok;
    s_costume_menu_hooks=ok;
    std::fprintf(stderr,"[openmua2] costume menu hooks %s\n",ok?"installed":"incomplete");
  }
}
