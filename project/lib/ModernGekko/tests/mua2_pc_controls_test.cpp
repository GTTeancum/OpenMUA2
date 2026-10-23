#include "mua2_pc_menu_geometry.hpp"
#include "mua2_pc_menu_context.hpp"
#include <map>
#include "mua2_pc_controls.hpp"
#include "mua2_pc_settings.hpp"
#include "mua2_pc_binding_capture.hpp"
#include <iostream>
#include <limits>

using namespace moderngekko::controls;
static bool On(const GamepadActions& a,unsigned id) {
  return (a.active[id/32*4+3-(id%32)/8] & (1u<<(id%8)))!=0;
}
#define CHECK(condition) do { if(!(condition)) { std::cerr << "line " << __LINE__ << ": " #condition "\n"; return 1; } } while(false)
int main() {
  std::map<std::uint32_t,std::vector<std::uint8_t>> guest;
  auto put=[&](std::uint32_t a,std::uint32_t v) {
    guest[a]={std::uint8_t(v>>24),std::uint8_t(v>>16),std::uint8_t(v>>8),std::uint8_t(v)};
  };
  const auto read=[&](std::uint32_t a,std::size_t n)->std::span<const std::uint8_t> {
    auto it=guest.find(a);return it!=guest.end() && it->second.size()==n ?
      std::span<const std::uint8_t>(it->second):std::span<const std::uint8_t>{};
  };
  CHECK(PcMenuModalBlocked(read));
  put(0x80817374,0x90000000);put(0x90000000,0x811983b8);
  put(0x808173b0,0x80001000);put(0x80001004,0x80542ba0);
  put(0x90008a0c,6);put(0x800010dc,0);
  const auto flag=0x90000000u+4+6*0x13b8+0x13a9;
  guest[flag]={0};guest[0x90008a32]={0};
  CHECK(!PcMenuModalBlocked(read));
  guest[flag]={0x80};CHECK(PcMenuModalBlocked(read));guest[flag]={0};
  guest[0x90008a32]={0x80};put(0x90008a10,1);CHECK(PcMenuModalBlocked(read));
  put(0x90008a10,0);CHECK(!PcMenuModalBlocked(read));
  put(0x800010dc,3);CHECK(PcMenuModalBlocked(read));put(0x800010dc,0);
  guest.erase(flag);CHECK(PcMenuModalBlocked(read));
  const PcMenuRect letterbox{0,.125,1,.875};
  auto center=PcViewportPoint(.5,.5,letterbox);
  CHECK(center && (*center)[0]==.5 && (*center)[1]==.5);
  CHECK(!PcViewportPoint(.5,.1,letterbox));
  CHECK(!PcViewportPoint(.5,.875,letterbox));
  CHECK(!PcViewportPoint(1,.5,{0,0,1,1}));
  CHECK(!PcViewportPoint(.5,.5,{0,0,0,0}));
  CHECK(!PcViewportPoint(std::numeric_limits<double>::quiet_NaN(),.5,letterbox));
  PcWheelInput wheel;
  for(unsigned i=0;i<3;++i)wheel.Add(.25);
  CHECK(!wheel.Pending() && wheel.Take()==0);
  wheel.Add(.25);CHECK(wheel.Pending() && wheel.Take()==1 && !wheel.Pending());
  wheel.Add(-2.5);CHECK(wheel.Take()==-1 && wheel.Take()==-1 && wheel.Take()==0);
  wheel.Add(.5);CHECK(!wheel.Pending());
  wheel.Add(std::numeric_limits<double>::infinity());CHECK(!wheel.Pending());
  wheel.Add(10000);for(unsigned i=0;i<16;++i)CHECK(wheel.Take()==1);
  CHECK(!wheel.Pending());wheel.Add(-4);wheel={};CHECK(!wheel.Pending());
  PcBindingCapture capture;
  capture.Key(13,true);capture.Begin(unsigned(PcAction::Use),0,false,false);
  capture.Key('Z',true);capture.Key('Z',false); // held activation key: not armed
  CHECK(!capture.Take());capture.Key(13,false);
  capture.Key('Z',true);CHECK(!capture.Take());capture.Key('Z',false);
  auto binding=capture.Take();CHECK(binding && binding->source=='Z' && !binding->cancelled && !capture.active);
  capture.Begin(0,0,true,false);capture.Key(0x2e,true);capture.Key(0x2e,false);
  binding=capture.Take();CHECK(binding && binding->source==0 && !binding->cancelled);
  capture.Begin(0,0,false,false);capture.Key(0x1b,true);capture.Key(0x1b,false);
  binding=capture.Take();CHECK(binding && binding->cancelled);
  capture.Begin(0,0,false,false);capture.FocusLost();
  binding=capture.Take();CHECK(binding && binding->cancelled);
  GamepadSample captured_pad;captured_pad.connected=true;captured_pad.inputs[0]=1;
  capture.Begin(0,0,false,true);capture.Pad(0,captured_pad);CHECK(!capture.Take());
  captured_pad.inputs[0]=0;capture.Pad(0,captured_pad);
  captured_pad.inputs[2]=1;capture.Pad(1,captured_pad);CHECK(!capture.Take());
  capture.Pad(0,captured_pad);CHECK(!capture.Take());
  captured_pad.inputs[2]=0;capture.Pad(0,captured_pad);
  binding=capture.Take();CHECK(binding && binding->source==3 && !binding->cancelled);
  capture.Begin(0,0,false,true);capture.Pad(0,captured_pad);
  capture.Key(0x2e,true);capture.Key(0x2e,false);
  binding=capture.Take();CHECK(binding && binding->source==0 && !binding->cancelled);
  capture.Begin(0,0,false,true);captured_pad.connected=false;capture.Pad(0,captured_pad);
  binding=capture.Take();CHECK(binding && binding->cancelled);
  auto menu_profile=DefaultPcSettings();PcPreset(menu_profile.profiles[0],3);
  PcInputFrame menu_keys;menu_keys.held[0x26]=true;
  auto menu_pad=MapPcInput(menu_profile,0,menu_keys,{},true,1.0/30);
  CHECK(menu_pad.Down(GamepadInput::Up) && !menu_pad.Down(GamepadInput::RightUp));
  menu_profile.profiles[0].keys[unsigned(PcAction::Attack)]={0x26,0};
  menu_pad=MapPcInput(menu_profile,0,menu_keys,{},true,1.0/30);
  CHECK(menu_pad.Down(GamepadInput::Up) && !menu_pad.Down(GamepadInput::A));
  menu_pad=MapPcInput(menu_profile,0,menu_keys,{},false,1.0/30);
  CHECK(menu_pad.Down(GamepadInput::A));
  auto s=DefaultPcSettings();
  CHECK(ValidatePcSettings(s).empty());
  for(unsigned preset=1;preset<=3;++preset) {
    CHECK(PcPreset(s.profiles[0],preset));
    CHECK(ValidatePcSettings(s).empty());
  }
  CHECK(!PcPreset(s.profiles[0],4));
  for(unsigned reserved:{8u,13u,0x12u,0x5bu,0x5cu,0x79u,0x7au}) {
    auto invalid=DefaultPcSettings();
    invalid.profiles[0].keys[0].primary=reserved;
    CHECK(!ValidatePcSettings(invalid).empty());
  }
  s=DefaultPcSettings();
  s.profiles[0].keys[unsigned(PcAction::Block)].secondary='Q';
  CHECK(!ValidatePcSettings(s).empty()); // Must not repeat the LT/block double binding.
  s=DefaultPcSettings();
  PcInputFrame frame;
  auto map=[&](bool menu=false) {return MapPcInput(s,0,frame,{},menu,1.0/60);};
  CHECK(map().connected); // No XInput hardware needed to start playing.
  CHECK(!MapPcInput(s,1,frame,{},false,1.0/60).connected);
  frame.held['Q']=true;
  auto action=BuildGamepadActions(map(),false,false);
  CHECK(On(action,33) && !On(action,12));
  frame={}; frame.held[0x11]=true;
  action=BuildGamepadActions(map(),false,false);
  CHECK(On(action,12) && !On(action,33));
  constexpr unsigned power_actions[]{29,31,30,32};
  for(unsigned slot=0;slot<4;++slot) {
    frame={}; frame.held['1'+slot]=true;
    action=BuildGamepadActions(map(),false,false);
    CHECK(On(action,power_actions[slot]));
    CHECK(!On(action,9) && !On(action,10) && !action.mash_x);
    frame={}; frame.held['5'+slot]=true;
    action=BuildGamepadActions(map(),false,true);
    CHECK(action.fusion_slot==int(slot) && On(action,33) && On(action,9));
    CHECK(!On(action,12));
    CHECK(!map(true).Down(GamepadInput::LT)); // Shortcuts cannot activate menu shoulders.
  }
  frame={}; frame.held['1']=frame.held['5']=true;
  action=BuildGamepadActions(map(),false,true);
  CHECK(!On(action,29) && action.fusion_slot==-1 && !On(action,9));
  frame={}; frame.held[13]=true;
  action=BuildGamepadActions(map(true),false,false,true,true);
  CHECK(On(action,89));
  CHECK(!map().Down(GamepadInput::A));
  frame={}; frame.held[8]=true;
  CHECK(On(BuildGamepadActions(map(true),false,false,true,true),90));
  frame={}; frame.held['W']=frame.held['D']=true;
  CHECK(std::abs(std::hypot(map().Value(GamepadInput::LeftUp),map().Value(GamepadInput::LeftRight))-1)<1e-9);
  frame={}; frame.held['V']=true; frame.mouse_dx=5; frame.mouse_dy=5;
  CHECK(std::abs(map().Value(GamepadInput::RightRight)-0.5)<1e-9);
  CHECK(std::abs(map().Value(GamepadInput::RightDown)-0.5)<1e-9);
  s.invert_camera_y=true;
  CHECK(std::abs(map().Value(GamepadInput::RightUp)-0.5)<1e-9);
  CHECK(MapPcInput(s,0,frame,{},false,0).Value(GamepadInput::RightRight)==0);
  CHECK(MapPcInput(s,0,frame,{},false,std::numeric_limits<double>::quiet_NaN()).Value(GamepadInput::RightRight)==0);
  frame.held['V']=false;
  CHECK(map().Value(GamepadInput::RightRight)==0);

  // A brief click entirely between input frames produces one press then a
  // release. Held-key repeat produces no extra presses; focus loss clears all.
  PcInputEvents events;
  events.Key('E',true); events.Key('E',false);
  frame=events.AdvanceFrame();
  CHECK(BuildGamepadActions(map(),false,false).mash_x);
  frame=events.AdvanceFrame();
  CHECK(!BuildGamepadActions(map(),false,false).mash_x);
  events.Key('E',true);
  for(int i=0;i<30;++i) events.Key('E',true);
  CHECK(events.AdvanceFrame().held['E']);
  events.Key('E',false);
  CHECK(!events.AdvanceFrame().held['E']);
  events.MouseDelta(10,20);
  CHECK(events.AdvanceFrame().mouse_dx==10);
  CHECK(events.AdvanceFrame().mouse_dx==0);
  events.Key('E',true); events.Focus(false);
  events.Key('E',true);
  CHECK(!events.AdvanceFrame().held['E']);
  events.Focus(true);
  CHECK(!events.AdvanceFrame().held['E']);

  s=DefaultPcSettings();
  s.separate_controllers=true; s.keyboard_player=1;
  CHECK(PcPhysicalPort(s,0)==0 && PcPhysicalPort(s,1)==-1 && PcPhysicalPort(s,2)==1 && PcPhysicalPort(s,3)==2);
  s.keyboard_enabled=false;
  GamepadSample physical; physical.connected=true;
  for(unsigned i=0;i<22;++i) {
    physical.inputs.fill(0); physical.inputs[i]=1;
    CHECK(MapPcInput(s,0,{},physical,false,1.0/60).inputs==physical.inputs);
  }
  // Rebinding only player 2 leaves player 1's pad and keyboard profile alone.
  s.profiles[1].buttons[unsigned(PcAction::Attack)]={3,0};
  s.profiles[1].buttons[unsigned(PcAction::Use)]={1,0};
  CHECK(ValidatePcSettings(s).empty());
  physical.inputs.fill(0); physical.inputs[0]=1;
  CHECK(MapPcInput(s,0,{},physical,false,1.0/60).Down(GamepadInput::A));
  CHECK(MapPcInput(s,1,{},physical,false,1.0/60).Down(GamepadInput::X));
  // Captured native Options camera and Effects Volume world bounds.
  const PcMatrix camera_world{1,0,0,0, 0,0,1,0, 0,-1,0,0, 255,-1000.5,191.5,1};
  const PcMatrix projection{.0029296875,0,0,0, 0,.0046875,0,0,
      0,0,-.002,0, 0,0,-1.2,1};
  const auto effects_rect=PcProjectMenuBounds({43.4,287.05,-120.5,57.2,9.1,0},camera_world,projection);
  CHECK(effects_rect);
  CHECK(std::abs(effects_rect->left-.1900390625)<1e-7);
  CHECK(std::abs(effects_rect->top-.2760546875)<1e-7);
  CHECK(std::abs(effects_rect->right-.273828125)<1e-7);
  CHECK(std::abs(effects_rect->bottom-.2973828125)<1e-7);
  CHECK(effects_rect->Contains(.23,.285));
  CHECK(!effects_rect->Contains(.4,.285));
  CHECK(!effects_rect->Contains(effects_rect->right,.285));
  CHECK(!PcProjectMenuBounds({43.4,287,-120,0,9,0},camera_world,projection));
  CHECK(!PcProjectMenuBounds({10000,287,-120,57,9,0},camera_world,projection));
  CHECK(!PcProjectMenuBounds({43,287,-120,57,9,0},PcMatrix{},projection));
  auto behind=projection;behind[15]=-1;
  CHECK(!PcProjectMenuBounds({43,287,-120,57,9,0},camera_world,behind));
  std::string error;
  auto display=DefaultPcSettings();
  // Pin the user-facing HD choices independently of the implementation table.
  constexpr std::array<PcOutputMode,4> expected_modes{{
      {1280,720},{1920,1080},{2560,1440},{3840,2160}}};
  for(const auto mode:expected_modes) {
    PcNextOutputMode(display);
    CHECK(display.output_width==mode.width && display.output_height==mode.height);
    auto loaded=DefaultPcSettings();
    CHECK(DecodePcSettings(EncodePcSettings(display),loaded,error));
    CHECK(loaded==display);
  }
  PcNextOutputMode(display);
  CHECK(display.output_width==1280 && display.output_height==720);
  CHECK(DecodePcSettings("Version=1\nOutputWidth=3840\nOutputHeight=2160\n",display,error));
  CHECK(display.output_width==3840 && display.output_height==2160);
  CHECK(!DecodePcSettings("Version=1\nOutputWidth=3840\nOutputHeight=1440\n",display,error));
  CHECK(display.output_width==3840 && display.output_height==2160);
  CHECK(DecodePcSettings("Version=1\n",display,error));
  CHECK(display.output_width==0 && display.output_height==0);
  display.window_mode=2;display.msaa=4;
  auto display_loaded=DefaultPcSettings();
  CHECK(DecodePcSettings(EncodePcSettings(display),display_loaded,error));
  CHECK(display_loaded==display);
  CHECK(!DecodePcSettings("Version=1\nWindowMode=3\n",display_loaded,error));
  CHECK(!DecodePcSettings("Version=1\nMSAA=3\n",display_loaded,error));
  CHECK(display_loaded==display);
  const std::array<unsigned,4> supported_samples{4,1,8,2};
  PcNextMsaa(display,supported_samples,4);CHECK(display.msaa==8);
  PcNextMsaa(display,supported_samples,8);CHECK(display.msaa==1);
  const std::array<unsigned,1> no_multisampling{1};
  PcNextMsaa(display,no_multisampling,1);CHECK(display.msaa==1);
  auto restored=DefaultPcSettings();
  CHECK(DecodePcSettings(EncodePcSettings(s),restored,error));
  CHECK(restored==s);
  for(const auto bad:{"Version=2\n", "Version=1\nKeyboardEnabled=2\n",
      "Version=1\nKeyboardPlayer=-1\n", "Version=1\nKeyboardPlayer=4\n",
      "Version=1\nVersion=1\n", "Version=1\nPlayer1.Key.Attack.Primary=13\n",
      "Version=1\nPlayer5.Key.Attack.Primary=0\n", "Version=1\nMouseSensitivity=40junk\n",
      "Version=1\nUnknown=5\n", "Version=1\nPlayer1.Key.Attack.Primary=69\n"}) {
    CHECK(!DecodePcSettings(bad,restored,error));
    CHECK(restored==s && !error.empty());
  }
  const PcMenuRect volume_bar{.2,.3,.8,.4};
  CHECK(PcVolumeTarget(.2,volume_bar)==0);
  CHECK(PcVolumeTarget(.5,volume_bar)==5);
  CHECK(PcVolumeTarget(.8,volume_bar)==10);
  CHECK(PcVolumeTarget(-1,volume_bar)==0);
  CHECK(PcVolumeTarget(2,volume_bar)==10);
  CHECK(!PcVolumeTarget(.5,PcMenuRect{.8,.3,.2,.4}));
  CHECK(!PcVolumeTarget(NAN,volume_bar));
  PcOptionsModel options;
  options.Open(s);
  CHECK(options.Bind(unsigned(PcAction::Use),'Z',false,false));
  CHECK(!options.Apply([](const auto&,std::string& why){why="Disk full";return false;}));
  CHECK(options.active && options.applied==s && options.status=="Disk full");
  options.Cancel(); CHECK(options.draft==s);
  options.Open(s);
  CHECK(options.Bind(unsigned(PcAction::Use),'Z',false,false));
  std::string saved;
  CHECK(options.Apply([&](const auto& data,std::string&){saved=data;return true;}));
  CHECK(DecodePcSettings(saved,restored,error));
  CHECK(restored==options.applied && restored!=s && !options.active);
  std::cout << "MUA2 PC input mapping, profiles, presets, focus and tap lifetime passed\n";
}
