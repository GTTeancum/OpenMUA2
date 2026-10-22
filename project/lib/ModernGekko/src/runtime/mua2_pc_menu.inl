// Native MUA2 menu integration. Entered only from ordinary menu callbacks.
namespace pc_menu {
using roster_io::Bytes;
using roster_io::Word;
using namespace moderngekko::controls;
PcOptionsModel model;
u32 owner=0;
unsigned page=0;
bool controller=false;
std::string GuestString(const Core::CPUThreadGuard& guard,u32 address,unsigned limit=128) {
  const auto* bytes=Bytes(guard,address,limit);
  if(!bytes)return {};
  const auto* end=std::find(bytes,bytes+limit,0);
  return end==bytes+limit ? std::string{}:std::string(reinterpret_cast<const char*>(bytes),end-bytes);
}
std::string Interned(const Core::CPUThreadGuard& guard,u32 hash) {
  if(!hash || (hash&0xffffff)>=1024)return {};
  const auto* global=Bytes(guard,0x808168f8,4);
  if(!global)return {};
  const u32 table=Word(global);
  const auto* entry=Bytes(guard,table+4+4*(hash&0xffffff),4);
  return entry ? GuestString(guard,table+0x1008+Word(entry)):std::string{};
}
bool Item(const Core::CPUThreadGuard& guard,u32 item) {
  const auto* bytes=Bytes(guard,item,0xac);
  if(!bytes)return false;
  const u32 menu=Word(bytes);
  if(GuestString(guard,menu+8)!="options_controller_xbox")return false;
  if(owner!=menu) {
    const std::lock_guard lock(s_pc_mutex);
    owner=menu;page=0;controller=false;model.Open(s_pc_settings);
    std::fprintf(stderr,"[openmua2] native PC options opened\n");
  }
  return model.active;
}
std::string KeyName(unsigned key) {
  if(!key)return "--";
  if((key>='A'&&key<='Z') || (key>='0'&&key<='9'))return std::string(1,char(key));
  if(key>=0x60 && key<=0x69)return "Num "+std::to_string(key-0x60);
  if(key>=0x70 && key<=0x7b)return "F"+std::to_string(key-0x6f);
  switch(key) {
  case 1:return "Mouse 1";case 2:return "Mouse 2";case 4:return "Mouse 3";
  case 0x10:return "Shift";case 0x11:return "Ctrl";case 0x20:return "Space";
  case 0x1b:return "Esc";case 0x25:return "Left";case 0x26:return "Up";
  case 0x27:return "Right";case 0x28:return "Down";
  default:return "Key "+std::to_string(key);
  }
}
std::string Value(std::string_view name) {
  const auto& s=model.draft;
  if(name=="pc_resolution_value")return s.output_width ? std::to_string(s.output_width)+" x "+std::to_string(s.output_height):"Current";
  if(name=="label_combat_music")return "Keyboard player: "+std::to_string(s.keyboard_player+1);
  if(name=="label_view_angle")return "Mouse sensitivity: "+std::to_string(s.mouse_sensitivity)+"%";
  if(name=="label_view_cycle")return std::string("Invert camera: ")+(s.invert_camera_y?"On":"Off");
  if(name=="label_view_follow")return std::string("Separate devices: ")+(s.separate_controllers?"On":"Off");
  if(name=="label_vibration")return std::string("Keyboard: ")+(s.keyboard_enabled?"On":"Off");
  if(name=="pc_binding_device")return controller?"Controller bindings":"Keyboard bindings";
  if(name=="pc_scroll_range")return std::to_string(page+1)+"-"+std::to_string(std::min(page+7,PcActionCount))+" / "+std::to_string(PcActionCount);
  if(name=="pc_status")return model.status.empty()?" ":model.status;
  if(name.starts_with("pc_player") && name.size()==10)return std::string(name.back()-'1'==model.player?"> Player ":"Player ")+name.back();
  constexpr std::array<std::string_view,7> labels{"label_button01","label_button02","label_button03","label_button04","label_button05","label_button06","label_stick01"};
  constexpr std::array<std::string_view,7> primary{"label_button07","label_button08","label_button09","label_button10","label_button11","label_button12","label_stick02"};
  for(unsigned row=0;row<7 && page+row<PcActionCount;++row) {
    if(name==labels[row])return std::string(PcActionNames[page+row]);
    if(name==primary[row] || name=="pc_secondary"+std::to_string(row)) {
      const auto& p=s.profiles[model.player];
      const auto& b=(controller?p.buttons:p.keys)[page+row];
      const auto key=name==primary[row]?b.primary:b.secondary;
      return controller?(key?"Input "+std::to_string(key):"--"):KeyName(key);
    }
  }
  return {};
}
bool Update(const Core::CPUThreadGuard& guard) {
  auto& state=guard.GetSystem().GetPPCState();
  const u32 item=state.gpr[3];
  if(!Item(guard,item))return false;
  const auto value=Value(Interned(guard,Word(Bytes(guard,item+4,4))));
  if(value.empty())return false;
  // The original update uses this same 256-byte string result buffer before
  // calling the native virtual text setter. Tail-call it with the original LR;
  // native interning and all text-layout invalidation remain intact.
  auto* buffer=Bytes(guard,0x812e252c,256);
  if(!buffer || value.size()>=256)return false;
  std::copy(value.begin(),value.end(),buffer);buffer[value.size()]=0;
  state.gpr[4]=0x812e252c;state.npc=0x80ff3688;
  return true;
}
bool Use(const Core::CPUThreadGuard& guard) {
  auto& state=guard.GetSystem().GetPPCState();
  if(!Item(guard,state.gpr[3]))return false;
  auto command=Interned(guard,Word(Bytes(guard,state.gpr[3]+0x20,4)));
  if(!command.starts_with("pcnative_"))return false;
  const auto action=command.substr(9);
  auto& s=model.draft;
  bool handled=true;
  if(action=="resolution")PcNextOutputMode(s);
  else if(action=="player")s.keyboard_player=(s.keyboard_player+1)%4;
  else if(action=="keyboard")s.keyboard_enabled=!s.keyboard_enabled;
  else if(action=="sharing")s.separate_controllers=!s.separate_controllers;
  else if(action=="invert")s.invert_camera_y=!s.invert_camera_y;
  else if(action=="sensitivity")s.mouse_sensitivity=s.mouse_sensitivity>=300?25:s.mouse_sensitivity+25;
  else if(action=="binding_device")controller=!controller;
  else if(action=="scroll_next")page=std::min(page+7,PcActionCount-7);
  else if(action=="scroll_prev")page=page<7?0:page-7;
  else if(action.starts_with("profile_") && action.size()==9 && action.back()>='1' && action.back()<='4')model.player=action.back()-'1';
  else if(action.starts_with("preset_") && action.size()==8 && action.back()>='1' && action.back()<='3')PcPreset(s.profiles[model.player],action.back()-'0');
  else handled=false;
  if(!handled)return false;
  model.status.clear();
  std::fprintf(stderr,"[openmua2] native PC action: %s\n",action.c_str());
  state.gpr[3]=3;state.npc=state.spr[8];return true;
}
void Install(const Core::CPUThreadGuard& guard) {
  const auto* update=Bytes(guard,0x80ff34fc,4);
  const auto* use=Bytes(guard,0x80fe2548,4);
  const auto* setter=Bytes(guard,0x80ff3688,4);
  if(!update || !use || !setter || Word(update)!=0x9421fff0 || Word(use)!=0x9421fff0 || Word(setter)!=0x9421ffe0 ||
     !HLE::SetExternalFunctionReplacement(guard,0x80ff34fc,Update) ||
     !HLE::SetExternalFunctionReplacement(guard,0x80fe2548,Use)) {
    roster_io::Abort(guard,"PC menu callback guards rejected");return;
  }
  owner=0;model.active=false;
  std::fprintf(stderr,"[openmua2] native PC menu callbacks installed\n");
}
} // namespace pc_menu
