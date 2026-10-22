#pragma once

#include "mua2_gamepad_actions.hpp"
#include <deque>
#include <string_view>

namespace moderngekko::controls {
// The profile/preset and primary/secondary binding model follows OpenXML1xbox's
// pc_controls. Actions here target MUA2's verified gamepad path, not XML1's
// health/energy actions, guest addresses, or camera implementation.
enum class PcAction : unsigned {
  Attack, HeavyAttack, Use, Jump, Block, NextPage, Fusion, Powers, Pause, Heroes,
  HeroUp, HeroDown, HeroLeft, HeroRight, MoveRight, MoveLeft, MoveUp, MoveDown,
  CameraRight, CameraLeft, CameraUp, CameraDown,
  Power1, Power2, Power3, Power4, Fusion1, Fusion2, Fusion3, Fusion4, Walk, CameraDrag,
  Count
};
constexpr unsigned PcActionCount = static_cast<unsigned>(PcAction::Count);
constexpr std::array<std::string_view, PcActionCount> PcActionNames{
  "Attack", "HeavyAttack", "Use", "Jump", "Block", "NextPage", "Fusion", "Powers",
  "Pause", "Heroes", "HeroUp", "HeroDown", "HeroLeft", "HeroRight",
  "MoveRight", "MoveLeft", "MoveUp", "MoveDown", "CameraRight", "CameraLeft",
  "CameraUp", "CameraDown", "Power1", "Power2", "Power3", "Power4",
  "Fusion1", "Fusion2", "Fusion3", "Fusion4", "Walk", "CameraDrag"
};
struct PcBinding {
  unsigned primary = 0, secondary = 0;
  bool operator==(const PcBinding&) const = default;
};
struct PcProfile {
  std::array<PcBinding, PcActionCount> keys{};
  // Sources are GamepadInput + 1; zero means unbound.
  std::array<PcBinding, PcActionCount> buttons{};
  bool operator==(const PcProfile&) const = default;
};
struct PcSettings {
  // Zero preserves the existing display configuration until explicitly changed.
  unsigned output_width = 0, output_height = 0;
  bool keyboard_enabled = true;
  unsigned keyboard_player = 0;
  bool separate_controllers = false, invert_camera_y = false;
  unsigned mouse_sensitivity = 100;
  std::array<PcProfile, 4> profiles{};
  bool operator==(const PcSettings&) const = default;
};
struct PcOutputMode { unsigned width, height; };
inline constexpr std::array<PcOutputMode,4> PcOutputModes{{
  {1280,720},{1920,1080},{2560,1440},{3840,2160}
}};
inline bool PcOutputSupported(unsigned width,unsigned height) {
  if ((width==0 && height==0) || (width==640 && height==480)) return true;
  return std::any_of(PcOutputModes.begin(),PcOutputModes.end(),[&](const auto& mode) {
    return mode.width==width && mode.height==height;
  });
}
inline void PcNextOutputMode(PcSettings& settings) {
  unsigned next=0;
  for(unsigned i=0;i<PcOutputModes.size();++i)
    if(settings.output_width==PcOutputModes[i].width && settings.output_height==PcOutputModes[i].height)
      next=(i+1)%PcOutputModes.size();
  settings.output_width=PcOutputModes[next].width;
  settings.output_height=PcOutputModes[next].height;
}
inline bool PcPreset(PcProfile& profile, unsigned preset) {
  if (preset < 1 || preset > 3) return false;
  profile = {};
  // Portable key IDs use Win32 virtual-key values. Only the window adapter
  // knows about HWNDs; tests and the mapping core never poll or inject OS input.
  constexpr std::array<unsigned, PcActionCount> keys{
    0x64,0x66,'E',0x20,0x11,0,'Q',0x65,0x1b,0x70,
    0x26,0x28,0x25,0x27,'D','A','W','S','L','J','I','K',
    '1','2','3','4','5','6','7','8',0x10,'V'
  };
  for (unsigned i=0; i<PcActionCount; ++i) profile.keys[i].primary=keys[i];
  for (unsigned i=0; i<22; ++i) profile.buttons[i].primary=i+1;
  profile.keys[unsigned(PcAction::Attack)].secondary=1; // left mouse
  profile.keys[unsigned(PcAction::HeavyAttack)].secondary=2;
  profile.keys[unsigned(PcAction::CameraDrag)].secondary=4;
  auto set=[&](PcAction a,unsigned key){profile.keys[unsigned(a)].primary=key;};
  if (preset==2) {
    set(PcAction::Attack,'J'); set(PcAction::HeavyAttack,'K'); set(PcAction::Use,'H');
    set(PcAction::CameraUp,0x24); set(PcAction::CameraDown,0x23);
    set(PcAction::CameraLeft,0x2e); set(PcAction::CameraRight,0x22);
  } else if (preset==3) {
    set(PcAction::Attack,'Q'); set(PcAction::HeavyAttack,'E'); set(PcAction::Use,'R');
    set(PcAction::Fusion,'F');
    set(PcAction::HeroUp,0x68); set(PcAction::HeroDown,0x62);
    set(PcAction::HeroLeft,0x64); set(PcAction::HeroRight,0x66);
    set(PcAction::CameraUp,0x26); set(PcAction::CameraDown,0x28);
    set(PcAction::CameraLeft,0x25); set(PcAction::CameraRight,0x27);
  }
  return true;
}
inline PcSettings DefaultPcSettings() {
  PcSettings s;
  for (auto& profile:s.profiles) PcPreset(profile,1);
  return s;
}
inline int PcBindingConflict(const std::array<PcBinding,PcActionCount>& bindings,
                             unsigned action,unsigned key) {
  if (!key) return -1;
  for (unsigned i=0;i<PcActionCount;++i)
    if (i!=action && (bindings[i].primary==key || bindings[i].secondary==key)) return int(i);
  return -1;
}
inline std::string_view ValidatePcSettings(const PcSettings& s) {
  if (!PcOutputSupported(s.output_width,s.output_height)) return "Unsupported output resolution";
  if (s.keyboard_player>=4) return "Keyboard player must be 1 through 4";
  if (s.mouse_sensitivity<10 || s.mouse_sensitivity>300) return "Mouse sensitivity must be 10 through 300";
  for (const auto& p:s.profiles) for (unsigned i=0;i<PcActionCount;++i) {
    for (const auto k:{p.keys[i].primary,p.keys[i].secondary}) {
      // Enter/Backspace are menu fallbacks. Alt, Windows keys and F10/F11
      // belong to window/system navigation, not configurable gameplay.
      if (k>=256 || k==13 || k==8 || k==0x12 || k==0x5b || k==0x5c || k==0x79 || k==0x7a)
        return "Invalid or reserved keyboard binding";
      if (PcBindingConflict(p.keys,i,k)>=0) return "Keyboard binding already assigned";
    }
    for (const auto k:{p.buttons[i].primary,p.buttons[i].secondary}) {
      if (k>22) return "Invalid controller binding";
      if (PcBindingConflict(p.buttons,i,k)>=0) return "Controller binding already assigned";
    }
  }
  return {};
}
// Stable physical device assignment. A reserved keyboard player shifts later
// gamepads instead of allowing two devices to accidentally join the same slot.
inline int PcPhysicalPort(const PcSettings& s,unsigned player) {
  if (player>=4) return -1;
  if (!s.keyboard_enabled || !s.separate_controllers) return int(player);
  if (player==s.keyboard_player) return -1;
  return int(player)-(player>s.keyboard_player ? 1:0);
}
struct PcInputFrame {
  std::array<bool,256> held{};
  double mouse_dx=0,mouse_dy=0;
};
// Window-thread events are latched once per game input frame under the caller's
// lock. Multiple guest reads share that frame. Fast down/up taps survive until
// sampled; OS key-repeat cannot turn a held Use key into repeated QTE presses.
class PcInputEvents {
public:
  void Focus(bool focused) {
    m_focused=focused; m_frame={}; m_physical.fill(false);
    for (auto& q:m_edges) q.clear();
    m_dx=m_dy=0;
  }
  void Key(unsigned key,bool down) {
    if (!m_focused || key==0 || key>=256 || m_physical[key]==down) return;
    m_physical[key]=down;
    auto& q=m_edges[key];
    // Bounded queue; on overflow discard stale taps and retain the current
    // physical state, so neither a stuck key nor unbounded latency can result.
    if (q.size()>=16) q.clear();
    q.push_back(down);
  }
  void MouseDelta(double x,double y) {
    if (m_focused && std::isfinite(x) && std::isfinite(y)) {
      m_dx=std::clamp(m_dx+x,-10000.0,10000.0);
      m_dy=std::clamp(m_dy+y,-10000.0,10000.0);
    }
  }
  const PcInputFrame& AdvanceFrame() {
    for(unsigned i=0;i<256;++i) if(!m_edges[i].empty()) {
      m_frame.held[i]=m_edges[i].front(); m_edges[i].pop_front();
    }
    m_frame.mouse_dx=m_dx; m_frame.mouse_dy=m_dy; m_dx=m_dy=0;
    return m_frame;
  }
private:
  bool m_focused=true;
  PcInputFrame m_frame;
  std::array<bool,256> m_physical{};
  std::array<std::deque<bool>,256> m_edges;
  double m_dx=0,m_dy=0;
};
inline GamepadSample MapPcInput(const PcSettings& settings,unsigned player,
                                const PcInputFrame& frame,const GamepadSample& physical,
                                bool menu,double seconds) {
  GamepadSample result;
  if(player>=4) return result;
  const bool keyboard=settings.keyboard_enabled && settings.keyboard_player==player;
  result.connected=keyboard || physical.connected;
  const auto& p=settings.profiles[player];
  const auto value=[&](unsigned action) {
    double v=0;
    for (auto key:{p.keys[action].primary,p.keys[action].secondary})
      if(keyboard && key>0 && key<256 && frame.held[key]) v=1;
    for (auto button:{p.buttons[action].primary,p.buttons[action].secondary})
      if(button>0 && button<=22) v=std::max(v,physical.Value(GamepadInput(button-1)));
    return v;
  };
  for(unsigned i=0;i<22;++i) result.inputs[i]=value(i);
  auto put=[&](GamepadInput input,double v) {
    auto& dst=result.inputs[unsigned(input)]; dst=std::max(dst,std::clamp(v,0.0,1.0));
  };
  if(menu) {
    if(keyboard && frame.held[13]) put(GamepadInput::A,1);
    if(keyboard && frame.held[8]) put(GamepadInput::B,1);
    return result;
  }
  for(unsigned i=0;i<4;++i) {
    if(value(unsigned(PcAction::Power1)+i)>0.5) {
      put(GamepadInput::RT,1); put(GamepadInput(i),1);
    }
    if(value(unsigned(PcAction::Fusion1)+i)>0.5) {
      put(GamepadInput::LT,1); put(GamepadInput(i),1);
    }
  }
  if(value(unsigned(PcAction::Walk))>0.5)
    for(unsigned i=14;i<18;++i) result.inputs[i]*=0.5;
  const double x=result.inputs[14]-result.inputs[15],y=result.inputs[16]-result.inputs[17];
  const double length=std::hypot(x,y);
  if(length>1) for(unsigned i=14;i<18;++i) result.inputs[i]/=length;
  if(keyboard && value(unsigned(PcAction::CameraDrag))>0.5 &&
     std::isfinite(seconds) && seconds>0) {
    const double scale=(settings.mouse_sensitivity/100.0)/(600.0*seconds);
    const double dx=frame.mouse_dx*scale;
    const double dy=-frame.mouse_dy*scale*(settings.invert_camera_y ? -1:1);
    put(GamepadInput::RightRight,dx); put(GamepadInput::RightLeft,-dx);
    put(GamepadInput::RightUp,dy); put(GamepadInput::RightDown,-dy);
  }
  return result;
}
} // namespace moderngekko::controls
