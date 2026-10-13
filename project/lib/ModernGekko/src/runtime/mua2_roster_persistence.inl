// Included by mua2_roster.inl. All callbacks run on the guest CPU thread.
namespace roster_io {
namespace layout = moderngekko::roster;
namespace format = moderngekko::roster::save;
struct Hero { u32 id, object; std::string name; };
struct PendingLoad { u32 manager=0, stream=0, cursor=0; format::Decoded decoded; };
std::optional<PendingLoad> pending;
bool failed=false;

u8* Bytes(const Core::CPUThreadGuard& guard,u32 address,std::size_t size) {
  return guard.GetSystem().GetMemory().GetPointerForRange(address,size);
}
u32 Word(const u8* b) { return (u32(b[0])<<24)|(u32(b[1])<<16)|(u32(b[2])<<8)|b[3]; }
void PutWord(u8* b,u32 v) { for(unsigned i=0;i<4;++i)b[i]=u8(v>>(24-8*i)); }
std::string Name(const u8* b) {
  const auto* end=std::find(b,b+format::NameBytes,0);
  if(end==b+format::NameBytes)return {};
  std::string out(b,end);return format::ValidName(out)?out:std::string{};
}
bool Abort(const Core::CPUThreadGuard& guard,const char* reason) {
  if(!failed) {
    failed=true;
    std::fprintf(stderr,"[openmua2] roster persistence rejected: %s; stopping before native stream continuation\n",reason);
    Core::QueueHostJob([](Core::System& system){ Core::Stop(system); });
  }
  guard.GetSystem().GetPPCState().npc=guard.GetSystem().GetPPCState().pc;
  return true;
}
std::optional<std::vector<Hero>> Heroes(const Core::CPUThreadGuard& guard,u32 manager) {
  const auto* b=Bytes(guard,manager,layout::Expanded.size);if(!b)return {};
  const u32 count=Word(b+layout::Expanded.definition_count);
  const u32 mask=Word(b+layout::Expanded.stats_base+layout::Expanded.stats.handle_mask);
  if(count>layout::DefinitionCapacity || mask!=63)return {};
  std::vector<Hero> out;std::set<std::string> names;
  for(u32 id=1;id<count;++id) {
    const auto* definition=b+layout::Expanded.definitions+id*28;
    const u32 handle=Word(definition+16);
    if(!(definition[25]&4) || !handle)continue;
    // Native snapshots use definition IDs, including the null/default entries.
    if(id>=layout::Expanded.snapshots_per_bank)return {};
    const u32 offset=layout::Expanded.stats_base+layout::Expanded.stats.records+(handle&mask)*1744;
    const auto name=Name(b+offset+748);
    if(name.empty() || !names.insert(name).second)return {};
    out.push_back({id,manager+offset,name});
  }
  if(out.size()>layout::Expanded.snapshots_per_bank)return {};
  return out;
}
// Definition names use the native 8192-bucket intern table, independently of
// the UI string table. Unlock records include mission NPCs as well as heroes.
std::optional<std::vector<std::string>> Definitions(const Core::CPUThreadGuard& guard,u32 manager) {
  const auto* b=Bytes(guard,manager,layout::Expanded.size);
  const auto* global=Bytes(guard,0x808168f0,4);
  if(!b || !global)return {};
  const u32 pool=Word(global),count=Word(b+layout::Expanded.definition_count);
  const auto* table=Bytes(guard,pool,32776);
  if(!table || count>layout::DefinitionCapacity)return {};
  std::vector<std::string> names(count);std::set<std::string> unique;
  for(u32 id=1;id<count;++id) {
    const u32 handle=Word(b+layout::Expanded.definitions+id*28);
    const u32 offset=Word(table+4+(handle&8191)*4);
    if(offset>=121856)return {};
    const auto* text=Bytes(guard,pool+32776+offset,format::NameBytes);
    if(!text)return {};
    names[id]=Name(text);
    if(names[id].empty() || !unique.insert(names[id]).second)return {};
  }
  return names;
}
struct Parts { std::array<u8*,5> bytes; };
std::optional<Parts> RecordParts(const Core::CPUThreadGuard& guard,u32 object,bool snapshot) {
  auto* b=Bytes(guard,object,snapshot?800:1744);if(!b)return {};
  auto* auxiliary=snapshot?b+592:Bytes(guard,Word(b+592),112);
  if(!auxiliary || Word(auxiliary)!=0x805467d8)return {};
  return Parts{{b,b+32,auxiliary+64,b+(snapshot?704:596),b+(snapshot?792:684)}};
}
constexpr std::array<std::size_t,5> lengths{24,544,48,88,1};
format::Record Pack(const Parts& parts) {
  format::Record record{};std::size_t at=0;
  for(unsigned i=0;i<lengths.size();++i) {
    std::copy_n(parts.bytes[i],lengths[i],record.begin()+at);at+=lengths[i];
  }
  return record;
}
void Apply(const Parts& parts,const format::Record& record) {
  std::size_t at=0;
  for(unsigned i=0;i<lengths.size();++i) {
    std::copy_n(record.begin()+at,lengths[i],parts.bytes[i]);at+=lengths[i];
  }
}
std::optional<std::pair<u32,std::span<u8>>> Stream(const Core::CPUThreadGuard& guard,u32 stream) {
  auto* b=Bytes(guard,stream,format::StreamCapacity+4);if(!b)return {};
  const u32 cursor=Word(b+format::StreamCapacity);
  if(cursor<stream || u64(cursor)>=u64(stream)+format::StreamCapacity)return {};
  return std::pair{cursor,std::span(b+(cursor-stream),format::StreamCapacity-(cursor-stream)-1)};
}
bool Save(const Core::CPUThreadGuard& guard) {
  if(!s_roster_storage_installed)return false;
  auto& state=guard.GetSystem().GetPPCState();
  if(failed)return Abort(guard,"prior failure");
  const u32 manager=state.gpr[3],stream=state.gpr[4];
  const auto heroes=Heroes(guard,manager);const auto output=Stream(guard,stream);
  if(!heroes || !output)return Abort(guard,"save roster/stream bounds");
  auto* b=Bytes(guard,manager,layout::Expanded.size);
  const auto definitions=Definitions(guard,manager);
  if(!definitions)return Abort(guard,"save definition identities");
  format::Section section;section.snapshot_valid=b[layout::Expanded.snapshot_flags+1]!=0;
  for(const auto& hero:*heroes) {
    const auto parts=RecordParts(guard,hero.object,false);
    const auto snapshot=RecordParts(guard,manager+layout::Expanded.snapshots+layout::Expanded.bank_stride+hero.id*800,true);
    if(!parts || !snapshot)return Abort(guard,"save record layout");
    section.heroes.push_back({hero.name,Pack(*parts)});
    section.snapshots.push_back({hero.name,Pack(*snapshot)});
  }
  const u32 count=Word(b+layout::Expanded.unlocked_list+700);
  if(count>layout::DefinitionCapacity)return Abort(guard,"unlock list bounds");
  for(u32 i=0;i<count;++i) {
    const auto* cell=b+layout::Expanded.unlocked_list+i*2;
    const u32 id=(u32(cell[0])<<8)|cell[1];
    if(!id || id>=definitions->size())return Abort(guard,"unknown unlocked identity");
    section.unlocked.push_back((*definitions)[id]);
  }
  const auto encoded=format::Encode(section,output->second.size());
  if(!encoded)return Abort(guard,"save section validation/capacity");
  std::copy(encoded->begin(),encoded->end(),output->second.begin());
  PutWord(Bytes(guard,stream+format::StreamCapacity,4),output->first+u32(encoded->size()));
  std::fprintf(stderr,"[openmua2] roster saved: %zu identities, %zu bytes\n",section.heroes.size(),encoded->size());
  state.npc=state.spr[8];return true;
}
bool BeginLoad(const Core::CPUThreadGuard& guard) {
  if(!s_roster_storage_installed)return false;
  if(failed)return Abort(guard,"prior failure");
  auto& state=guard.GetSystem().GetPPCState();
  if(pending)return Abort(guard,"nested roster load");
  const auto input=Stream(guard,state.gpr[4]);const auto heroes=Heroes(guard,state.gpr[3]);
  if(!input || !heroes)return Abort(guard,"load roster/stream bounds");
  const auto bytes=input->second;
  const bool keyed=bytes.size()>=format::Magic.size() && std::equal(format::Magic.begin(),format::Magic.end(),bytes.begin());
  const auto definitions=Definitions(guard,state.gpr[3]);
  if(!definitions)return Abort(guard,"load definition identities");
  std::vector<std::string> npc_names;
  if(!keyed) {
    std::set<u32> hero_ids;for(const auto& hero:*heroes)hero_ids.insert(hero.id);
    std::string order;
    for(u32 id=1;id<definitions->size();++id)if(!hero_ids.contains(id)) {
      npc_names.push_back((*definitions)[id]);
      if(!order.empty())order+='\n';order+=(*definitions)[id];
    }
    // Only migrate the verified stock NPC order. Unversioned custom NPC tables
    // have no identity metadata and must not be guessed from numeric indices.
    if(Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(order))!=
       "B6721128E36AFBB4895465E918B15C4B9B682B1A")return Abort(guard,"unsupported legacy NPC order");
  }
  auto decoded=keyed?format::Decode(bytes):format::DecodeRetail(bytes,npc_names);
  if(!decoded)return Abort(guard,"unsupported or malformed roster save");
  pending=PendingLoad{state.gpr[3],state.gpr[4],input->first,std::move(*decoded)};
  std::fprintf(stderr,"[openmua2] roster load: %s, %zu identities\n",keyed?"named v1":"retail migration",pending->decoded.section.heroes.size());
  return false; // Native reset, resource preparation and per-hero iteration stay intact.
}
bool LoadRecord(const Core::CPUThreadGuard& guard) {
  if(!pending)return false;
  auto& state=guard.GetSystem().GetPPCState();
  if(failed)return Abort(guard,"prior failure");
  if(state.gpr[4]!=pending->stream)return Abort(guard,"unexpected nested record stream");
  const auto* object=Bytes(guard,state.gpr[3],1744);
  const auto parts=RecordParts(guard,state.gpr[3],false);
  if(!object || !parts)return Abort(guard,"load record layout");
  const auto name=Name(object+748);
  const auto& records=pending->decoded.section.heroes;
  const auto found=std::find_if(records.begin(),records.end(),[&](const auto& r){return r.name==name;});
  if(found!=records.end())Apply(*parts,found->bytes);
  // A newly installed hero keeps its freshly initialized native defaults.
  state.npc=state.spr[8];return true;
}
bool FinishLoad(const Core::CPUThreadGuard& guard) {
  if(!pending)return false;
  if(failed)return Abort(guard,"prior failure");
  const auto heroes=Heroes(guard,pending->manager);
  if(!heroes)return Abort(guard,"restored roster bounds");
  auto* b=Bytes(guard,pending->manager,layout::Expanded.size);
  const auto& section=pending->decoded.section;
  const auto definitions=Definitions(guard,pending->manager);
  if(!definitions)return Abort(guard,"restored definition identities");
  std::vector<std::pair<Parts,format::Record>> snapshots;
  std::vector<u32> unlocked;
  for(const auto& hero:*heroes) {
    const auto target=RecordParts(guard,pending->manager+layout::Expanded.snapshots+layout::Expanded.bank_stride+hero.id*800,true);
    const auto current=RecordParts(guard,hero.object,false);
    if(!target || !current)return Abort(guard,"snapshot restore layout");
    const auto found=std::find_if(section.snapshots.begin(),section.snapshots.end(),[&](const auto& r){return r.name==hero.name;});
    snapshots.emplace_back(*target,found==section.snapshots.end()?Pack(*current):found->bytes);

  }
  for(u32 id=1;id<definitions->size();++id)
    if(std::find(section.unlocked.begin(),section.unlocked.end(),(*definitions)[id])!=section.unlocked.end())unlocked.push_back(id);
  for(const auto& [parts,record]:snapshots)Apply(parts,record);
  b[layout::Expanded.snapshot_flags+1]=section.snapshot_valid;
  std::fill_n(b+layout::Expanded.unlocked_list,704,0);
  for(std::size_t i=0;i<unlocked.size();++i) {
    const u32 id=unlocked[i];auto* cell=b+layout::Expanded.unlocked_list+i*2;
    cell[0]=u8(id>>8);cell[1]=u8(id);b[layout::Expanded.definitions+id*28+25]|=0x80;
  }
  PutWord(b+layout::Expanded.unlocked_list+700,u32(unlocked.size()));
  PutWord(Bytes(guard,pending->stream+format::StreamCapacity,4),pending->cursor+u32(pending->decoded.consumed));
  std::fprintf(stderr,"[openmua2] roster restored: %zu current identities, %zu unlocked\n",heroes->size(),unlocked.size());
  pending.reset();guard.GetSystem().GetPPCState().npc=0x80099ea0;return true;
}
bool Install(const Core::CPUThreadGuard& guard) {
  pending.reset();failed=false;
  return HLE::SetExternalFunctionReplacement(guard,0x80099c24,Save) &&
         HLE::SetExternalFunctionReplacement(guard,0x80099d30,BeginLoad) &&
         HLE::SetExternalFunctionReplacement(guard,0x80194dd8,LoadRecord) &&
         HLE::SetExternalBranchReplacement(guard,0x80099dfc,FinishLoad);
}
} // namespace roster_io
