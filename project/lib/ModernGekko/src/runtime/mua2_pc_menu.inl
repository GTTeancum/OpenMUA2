// Native MUA2 menu integration. Entered only from ordinary menu callbacks.
namespace pc_menu {
using roster_io::Bytes;
using roster_io::Word;
using namespace moderngekko::controls;
PcOptionsModel model;
u32 owner=0;
unsigned page=0;
bool controller=false;
void Reconstructed(u32 menu) {
  if(owner!=menu)return;
  owner=0;model.Cancel();
  const std::lock_guard lock(s_pc_mutex);s_pc_capture.Reset();
}
bool SaveSettings(const std::string& data,std::string& error) {
  const auto path=s_pc_settings_path.string();
  const auto temporary=path+".pending";
  File::IOFile file(temporary,"wb");
  if(!file || !file.WriteBytes(data.data(),data.size()) || !file.Flush() || !file.Close()) {
    error="Could not write settings; previous settings retained";return false;
  }
  if(!File::Rename(temporary,path)) {
    error="Could not replace settings; previous settings retained";return false;
  }
  return true;
}
void FinishCapture() {
  const std::lock_guard lock(s_pc_mutex);
  if(const auto result=s_pc_capture.Take()) {
    if(result->cancelled)model.status="Binding cancelled";
    else if(model.Bind(s_pc_capture.action,result->source,s_pc_capture.alternate,s_pc_capture.controller))
      model.status="Binding changed; Apply to save";
    s_pc_events.Focus(false);s_pc_events.Focus(s_pc_focused);s_pc_frame={};
    std::fprintf(stderr,"[openmua2] native binding result: %s\n",model.status.c_str());
  }
}
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
  case 0x24:return "Home";case 0x23:return "End";case 0x2e:return "Delete";
  case 0x22:return "Page Down";case 0x21:return "Page Up";
  default:return "Key "+std::to_string(key);
  }
}
constexpr std::array<std::string_view,22> PadNames{
  "A","B","X","Y","LB","RB","LT","RT","Start","Back",
  "D-pad Up","D-pad Down","D-pad Left","D-pad Right",
  "LS Right","LS Left","LS Up","LS Down","RS Right","RS Left","RS Up","RS Down"
};
constexpr std::array<std::string_view,PcActionCount> ActionLabels{
  "Light attack","Heavy attack","Grab / use","Jump","Block","Next page",
  "Fusion modifier","Power modifier","Pause","Heroes",
  "Hero up","Hero down","Hero left","Hero right",
  "Move right","Move left","Move up","Move down",
  "Camera right","Camera left","Camera up","Camera down",
  "Power 1","Power 2","Power 3","Power 4",
  "Fusion 1","Fusion 2","Fusion 3","Fusion 4","Walk","Camera drag"
};
std::string Value(std::string_view name) {
  const auto& s=model.draft;
  if(name=="pc_window") {
    const bool full=s.window_mode?s.window_mode==2:Config::Get(Config::MAIN_FULLSCREEN);
    return full?"Borderless fullscreen":"Windowed";
  }
  if(name=="pc_fsaa_value") {
    const auto samples=s.msaa?s.msaa:Config::Get(Config::GFX_MSAA);
    return samples<=1?"Off":std::to_string(samples)+"x";
  }
  unsigned device=4;
  if(name=="controls_list")device=0;
  else if(name=="controls_list2")device=1;
  else if(name=="pc_device_3")device=2;
  else if(name=="pc_device_4")device=3;
  if(device<4) {
    const int pad=PcPhysicalPort(s,device);
    std::string label="P"+std::to_string(device+1)+": ";
    if(s.keyboard_enabled && s.keyboard_player==device)label+="Keys";
    if(pad>=0) {if(label.back()!=' ')label+=" + ";label+="Pad "+std::to_string(pad+1);}
    return label;
  }
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
    if(name==labels[row])return std::string(ActionLabels[page+row]);
    if(name==primary[row] || name=="pc_secondary"+std::to_string(row)) {
      const auto& p=s.profiles[model.player];
      const auto& b=(controller?p.buttons:p.keys)[page+row];
      const auto key=name==primary[row]?b.primary:b.secondary;
      return controller?(key && key<=PadNames.size()?std::string(PadNames[key-1]):"--"):KeyName(key);
    }
  }
  return {};
}
// Use the same camera and native world bounds as menu text rendering.
std::optional<moderngekko::controls::PcMenuRect> ProjectedBounds(
    const Core::CPUThreadGuard& guard,u32 item) {
  const auto read=[&](u32 address)->u32 {
    const auto* p=Bytes(guard,address,4);return p?Word(p):0;
  };
  const u32 manager=read(0x8081736c);
  const u32 camera=manager?read(manager+0x200c):0;
  const u32 node=camera?read(camera+0x25c):0;
  const u32 children=node?read(node+0x10):0;
  const u32 array=children?read(children+0x10):0;
  const u32 transform=array?read(array):0;
  const u32 projection=camera?read(camera+0x1a8):0;
  if(!transform || !projection)return {};
  const auto* world_bytes=Bytes(guard,transform+0x20,64);
  const auto* projection_bytes=Bytes(guard,projection+0x110,64);
  const auto* bounds_bytes=Bytes(guard,item+0x6c,24);
  if(!world_bytes || !projection_bytes || !bounds_bytes)return {};
  const auto f=[](const u8* p){return double(std::bit_cast<float>(Word(p)));};
  moderngekko::controls::PcMatrix world{},project{};
  std::array<double,6> bounds{};
  for(unsigned i=0;i<16;++i){world[i]=f(world_bytes+i*4);project[i]=f(projection_bytes+i*4);}
  for(unsigned i=0;i<6;++i)bounds[i]=f(bounds_bytes+i*4);
  return moderngekko::controls::PcProjectMenuBounds(bounds,world,project);
}
void ObserveProjectedBounds(const Core::CPUThreadGuard& guard,u32 item) {
  static const bool enabled=std::getenv("OPENMUA2_PC_MENU_BOUNDS")!=nullptr;
  if(!enabled)return;
  const auto* header=Bytes(guard,item,8);
  if(!header)return;
  const auto menu=GuestString(guard,Word(header)+8);
  if(menu!="options_rev" && menu!="options_controller_xbox")return;
  const auto name=Interned(guard,Word(header+4));
  if(!name.starts_with("label_") && !name.starts_with("pc_"))return;
  static std::map<u32,bool> reported;
  if(reported[item])return;
  if(const auto rect=ProjectedBounds(guard,item)) {
    reported[item]=true;
    std::fprintf(stderr,"[openmua2] PC menu bounds %s/%s item=%08x screen=(%.6f,%.6f)-(%.6f,%.6f)\n",
      menu.c_str(),name.c_str(),item,rect->left,rect->top,rect->right,rect->bottom);
  }
}
bool Update(const Core::CPUThreadGuard& guard) {
  auto& state=guard.GetSystem().GetPPCState();
  const u32 item=state.gpr[3];
  ObserveProjectedBounds(guard,item);
  if(!Item(guard,item))return false;
  FinishCapture();
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
  if(command=="closemenu") {
    model.Cancel();
    {const std::lock_guard lock(s_pc_mutex);s_pc_capture.Reset();}
    return false; // Let MUA2 perform its native close transition.
  }
  if(!command.starts_with("pcnative_"))return false;
  const auto action=command.substr(9);
  {const std::lock_guard lock(s_pc_mutex);if(s_pc_capture.active){state.gpr[3]=3;state.npc=state.spr[8];return true;}}
  model.status.clear();
  auto& s=model.draft;
  bool handled=true;
  if(action=="resolution")PcNextOutputMode(s);
  else if(action=="window") {
    const bool full=s.window_mode?s.window_mode==2:Config::Get(Config::MAIN_FULLSCREEN);
    s.window_mode=full?1:2;
  }
  else if(action=="fsaa")PcNextMsaa(s,g_backend_info.AAModes,s.msaa?s.msaa:Config::Get(Config::GFX_MSAA));
  else if(action.starts_with("device_") && action.size()==8 && action.back()>='1' && action.back()<='4') {
    model.player=action.back()-'1';controller=PcPhysicalPort(s,model.player)>=0;
  }
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
  else if(action=="defaults") {s=DefaultPcSettings();page=0;}
  else if(action=="apply") {
    if(model.Apply(SaveSettings)) {
      const std::lock_guard lock(s_pc_mutex);
      s_pc_settings=model.applied;
      s_pc_events.Focus(false);s_pc_events.Focus(s_pc_focused);s_pc_frame={};
      model.Open(s_pc_settings);model.status="Saved (display changes on next launch)";
    }
  }
  else if(action.starts_with("rowbind_") && action.size()==11 &&
          action[8]>='0' && action[8]<='6' && action[9]=='_' &&
          (action[10]=='0' || action[10]=='1')) {
    const unsigned target=page+action[8]-'0';
    if(target<PcActionCount) {
      const std::lock_guard lock(s_pc_mutex);
      s_pc_capture.Begin(target,model.player,action[10]=='1',controller);
      s_pc_events.Focus(false);s_pc_events.Focus(s_pc_focused);s_pc_frame={};
      model.status=controller?"Release controls, then press or move":"Press a key; Delete clears, Esc cancels";
    }
  }
  else handled=false;
  if(!handled)return false;
  std::fprintf(stderr,"[openmua2] native PC action: %s\n",action.c_str());
  state.gpr[3]=3;state.npc=state.spr[8];return true;
}
// Runs from the normal native menu input update, never from a test command.
// Tail calls retain the native caller's LR and all native focus/command behavior.
bool PointerInput(const Core::CPUThreadGuard& guard) {
  auto& state=guard.GetSystem().GetPPCState();
  const u32 menu=state.gpr[3];
  const auto name=GuestString(guard,menu+8);
  const auto read=[&](u32 address)->u32 {
    const auto* p=Bytes(guard,address,4);return p?Word(p):0;
  };
  const u32 manager=read(0x8081736c);
  const bool supported=name=="options_rev" || name=="options_controller_xbox";
  static u32 pointer_owner=0;
  static u64 consumed=0;
  static u32 slider_item=0;
  static int slider_target=-1;
  const std::lock_guard lock(s_pc_mutex);
  if(!supported || !manager || read(manager+25860)!=menu || !s_pc_enabled ||
     !s_pc_settings.keyboard_enabled || !s_pc_focused || s_pc_capture.active) {
    s_pc_menu_pointer.click=false;s_pc_menu_pointer.wheel={};consumed=s_pc_menu_pointer.serial;pointer_owner=0;
    return false;
  }
  if(pointer_owner!=menu) {
    slider_item=0;slider_target=-1;
    pointer_owner=menu;consumed=s_pc_menu_pointer.serial;s_pc_menu_pointer.click=false;s_pc_menu_pointer.wheel={};
    return false;
  }
  // The native Options close gate uses these same dialog/save managers.
  // Unknown instances fail closed. The pending-dialog flag also blocks clicks
  // during the interval before a newly requested dialog becomes visible.
  const bool blocked=PcMenuModalBlocked([&](u32 a,std::size_t n)->std::span<const u8> {
    const auto* p=Bytes(guard,a,n);return p?std::span<const u8>(p,n):std::span<const u8>{};
  });
  if(std::getenv("OPENMUA2_PC_MENU_BOUNDS") &&
     (s_pc_menu_pointer.click || consumed!=s_pc_menu_pointer.serial))
    std::fprintf(stderr,"[openmua2] pointer gate blocked=%d valid=%d click=%d viewport=(%.4f,%.4f)-(%.4f,%.4f)\n",
      blocked,s_pc_menu_pointer.valid,s_pc_menu_pointer.click,
      s_pc_viewport.left,s_pc_viewport.top,s_pc_viewport.right,s_pc_viewport.bottom);
  if(blocked || !s_pc_menu_pointer.valid) {
    slider_item=0;slider_target=-1;
    s_pc_menu_pointer.click=false;s_pc_menu_pointer.wheel={};consumed=s_pc_menu_pointer.serial;return false;
  }
  if(!s_pc_menu_pointer.down && slider_target<0)slider_item=0;
  if(s_pc_menu_pointer.click) {slider_item=0;slider_target=-1;}
  if(!s_pc_menu_pointer.click && !s_pc_menu_pointer.wheel.Pending() && slider_target<0 && consumed==s_pc_menu_pointer.serial)return false;
  consumed=s_pc_menu_pointer.serial;
  const auto point=PcViewportPoint(s_pc_menu_pointer.x,s_pc_menu_pointer.y,s_pc_viewport);
  u32 hit=0;
  const u32 map=read(menu+moderngekko::roster::MenuMapPointer);
  const auto* items=map?Bytes(guard,map,moderngekko::roster::MenuMapBytes):nullptr;
  if(point && items && Word(items+8)==0x8118fb90 &&
     Word(items+moderngekko::roster::MenuMapCount)<=moderngekko::roster::MenuItems) {
    std::map<std::string,u32> regions, named;
    for(unsigned i=0;i<moderngekko::roster::MenuItems;++i) {
      const u32 address=Word(items+moderngekko::roster::MenuMapValues+4*i);
      const auto* region=address?Bytes(guard,address,8):nullptr;
      if(!region || Word(region)!=menu)continue;
      const auto region_name=Interned(guard,Word(region+4));
      named.emplace(region_name,address);
      if(region_name.starts_with("pc_hit_"))regions.emplace(region_name.substr(7),address);
    }
    for(unsigned i=0;i<moderngekko::roster::MenuItems;++i) {
      const u32 candidate=Word(items+moderngekko::roster::MenuMapValues+4*i);
      const auto* item=candidate?Bytes(guard,candidate,0xac):nullptr;
      // Match the native focus eligibility check at 80fc9410..80fc944c.
      if(!item || Word(item)!=menu || !(item[0x4e]&0x10) || (item[0x4e]&0x2c))continue;
      // Visible decorative scene nodes share these flags but have no action.
      if(!Word(item+0x20) && !Word(item+0x24) && !Word(item+0x28))continue;
      const auto region=regions.find(Interned(guard,Word(item+4)));
      const auto bounds=ProjectedBounds(guard,region==regions.end()?candidate:region->second);
      if(bounds && bounds->Contains((*point)[0],(*point)[1])) {hit=candidate;break;}
    }
    // Bind the actual visible bars to their native labels. Guard both gamevar
    // callbacks and the options manager before interpreting its packed detents.
    // 801342d8/801344a4 call virtual getters 8014d26c/8014d2a4:
    // options+0x21/+0x22, upper five bits, divided by ten.
    if(name=="options_rev" && point && read(0x806356e0)==0x8053ce90) {
      for(const auto& spec:std::array{
          std::tuple{"fx_vol","label_effects_volume",0x801342d8u,0x80134338u,0x80635701u},
          std::tuple{"music_vol","label_music_volume",0x801344a4u,0x80134504u,0x80635702u}}) {
        const auto& [bar_name,label_name,getter,setter,value_address]=spec;
        const auto bar=named.find(bar_name),label=named.find(label_name);
        if(bar==named.end() || label==named.end() || read(bar->second+0x34)!=getter ||
           read(bar->second+0x30)!=setter)continue;
        const auto bounds=ProjectedBounds(guard,bar->second);
        const auto* value=Bytes(guard,value_address,1);
        if(!bounds || !value || (*value>>3)>10)continue;
        const bool begin=s_pc_menu_pointer.click && bounds->Contains((*point)[0],(*point)[1]);
        if(begin || (slider_item==label->second && s_pc_menu_pointer.down)) {
          slider_item=label->second;
          slider_target=PcVolumeTarget((*point)[0],*bounds).value_or(-1);
          s_pc_menu_pointer.click=false;
        }
        if(slider_item!=label->second || slider_target<0)continue;
        hit=slider_item;
        if(read(menu+0x458)!=hit)break; // Focus first, preserving the target.
        const int current=*value>>3;
        if(current==slider_target) {
          slider_target=-1;
          if(!s_pc_menu_pointer.down)slider_item=0;
          state.npc=state.spr[8];return true;
        }
        const int direction=current<slider_target?1:-1;
        const u32 callback=read(read(hit+0x9c)+0x64);
        auto* axis=Bytes(guard,0x812e252c,2);
        if(callback!=0x80fe21c8 || !axis) {slider_item=0;slider_target=-1;break;}
        axis[0]=direction>0?1:0xff;axis[1]=0;
        state.gpr[3]=hit;state.gpr[4]=0x812e252c;state.npc=callback;
        std::fprintf(stderr,"[openmua2] pointer volume: %s %d -> %d\n",bar_name,current,slider_target);
        return true;
      }
    }
  }
  if(!hit) {s_pc_menu_pointer.click=false;s_pc_menu_pointer.wheel={};return false;}
  if(read(menu+0x458)!=hit) {
    state.gpr[4]=hit;state.npc=0x80fc9bb0;
    std::fprintf(stderr,"[openmua2] pointer focus: %s\n",Interned(guard,read(hit+4)).c_str());
    return true;
  }
  if(s_pc_menu_pointer.wheel.Pending()) {
    const int direction=s_pc_menu_pointer.wheel.Take();
    const u32 callback=read(read(hit+0x9c)+0x64);
    if(callback==0x80fe21c8 && read(hit+(direction>0?0x28:0x24))) {
      // Native directional input is two signed bytes. Use the same temporary
      // result buffer as native text updates; this call is synchronous on CPU.
      auto* axis=Bytes(guard,0x812e252c,2);
      if(axis) {
        axis[0]=direction>0?1:0xff;axis[1]=0;
        state.gpr[3]=hit;state.gpr[4]=0x812e252c;state.npc=callback;
        std::fprintf(stderr,"[openmua2] pointer adjust: %s %d\n",Interned(guard,read(hit+4)).c_str(),direction);
        return true;
      }
    }
    s_pc_menu_pointer.wheel={};
  }
  if(!s_pc_menu_pointer.click)return false;
  s_pc_menu_pointer.click=false;
  // Release the mutex before Use can enter binding capture on the next guest step.
  const u32 use=read(read(hit+0x9c)+0x4c);
  if(use!=0x80fe2548)return false;
  state.gpr[3]=hit;state.gpr[4]=0xffffffff;state.npc=use;
  std::fprintf(stderr,"[openmua2] pointer activate: %s\n",Interned(guard,read(hit+4)).c_str());
  return true;
}
void Install(const Core::CPUThreadGuard& guard) {
  const auto* update=Bytes(guard,0x80ff34fc,4);
  const auto* use=Bytes(guard,0x80fe2548,4);
  const auto* setter=Bytes(guard,0x80ff3688,4);
  const auto* input=Bytes(guard,0x80fc9170,4);
  const auto* focus=Bytes(guard,0x80fc9bb0,4);
  const auto* directional=Bytes(guard,0x80fe21c8,4);
  if(!directional || Word(directional)!=0x9421ffe0 || !input || !focus || Word(input)!=0x9421ff40 || Word(focus)!=0x9421fff0 ||
     !update || !use || !setter || Word(update)!=0x9421fff0 || Word(use)!=0x9421fff0 || Word(setter)!=0x9421ffe0 ||
     !HLE::SetExternalFunctionReplacement(guard,0x80ff34fc,Update) ||
     !HLE::SetExternalFunctionReplacement(guard,0x80fe2548,Use) ||
     !HLE::SetExternalFunctionReplacement(guard,0x80fc9170,PointerInput)) {
    roster_io::Abort(guard,"PC menu callback guards rejected");return;
  }
  owner=0;model.active=false;
  std::fprintf(stderr,"[openmua2] native PC menu callbacks installed\n");
}
} // namespace pc_menu
