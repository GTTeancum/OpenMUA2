#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <span>

namespace moderngekko::controls {
// Raw physical gamepad samples. No Wii buttons, gestures, or cursor markers.
enum class GamepadInput : unsigned {
  A, B, X, Y, LB, RB, LT, RT, Start, Back, Up, Down, Left, Right,
  LeftRight, LeftLeft, LeftUp, LeftDown, RightRight, RightLeft, RightUp, RightDown
};
struct GamepadSample {
  bool connected = false;
  std::array<double,22> inputs{};
  double Value(GamepadInput key) const {
    const double v=inputs[static_cast<unsigned>(key)];
    return connected && std::isfinite(v) ? std::clamp(v,0.0,1.0) : 0.0;
  }
  bool Down(GamepadInput key) const { return Value(key)>0.5; }
};
struct GamepadActions {
  std::array<std::uint8_t,20> active{};
  std::array<std::uint8_t,124*4> values{};
  int hero_slot=-1, fusion_slot=-1;
  bool mash_x=false;
  void Set(unsigned action,float value=1.0f) {
    if(action>=124 || !std::isfinite(value)) return;
    const auto word=std::bit_cast<std::uint32_t>(value);
    for(unsigned i=0;i<4;++i) values[action*4+i]=std::uint8_t(word>>(24-8*i));
    if(value!=0) active[(action/32)*4+3-(action%32)/8]|=std::uint8_t(1u<<(action%8));
  }
};
inline GamepadActions BuildGamepadActions(const GamepadSample& pad,
                                          bool start_accept, bool fusion_request) {
  using K=GamepadInput;
  GamepadActions out;
  if(!pad.connected) return out;
  const bool lt=pad.Down(K::LT),rt=pad.Down(K::RT);
  const auto axis=[&](K positive,K negative) {
    const double v=pad.Value(positive)-pad.Value(negative);
    return std::abs(v)<=0.15 ? 0.0f : static_cast<float>(v);
  };
  out.Set(0,axis(K::LeftRight,K::LeftLeft));
  out.Set(1,axis(K::LeftUp,K::LeftDown));
  // Menu/hero management takes priority over gameplay on the same sample.
  if(pad.Down(K::Back)) { out.Set(40); return out; }
  if(pad.Down(K::Start)) { out.Set(39);out.Set(start_accept?105:90);return out; }
  constexpr std::array<K,4> face{K::A,K::B,K::X,K::Y};
  if(lt || rt) {
    if(lt && rt) return out; // Ambiguous modifier: no attack/selection leakage.
    if(lt) {
      out.Set(33); // Native FusionPower request, without Nunchuk shake.
      int slot=-1;unsigned count=0;
      for(unsigned i=0;i<4;++i) if(pad.Down(face[i])) {slot=int(i);++count;}
      if(count==1) { out.fusion_slot=slot;if(fusion_request) out.Set(9); }
    } else {
      constexpr std::array<unsigned,4> power{29,31,30,32};
      for(unsigned i=0;i<4;++i) if(pad.Down(face[i])) out.Set(power[i]);
    }
    return out;
  }
  if(pad.Down(K::A)) {out.Set(9);out.Set(89);out.Set(43);}
  if(pad.Down(K::B)) {out.Set(10);out.Set(90);out.Set(44);}
  if(pad.Down(K::X)) {out.Set(11);out.Set(21);out.Set(45);out.mash_x=true;}
  if(pad.Down(K::Y)) for(unsigned id:{8u,19u,41u,46u,55u,106u,109u}) out.Set(id);
  if(pad.Down(K::LB)) for(unsigned id:{12u,20u,38u,49u,100u,101u,102u,120u,121u}) out.Set(id);
  const float camera=axis(K::RightRight,K::RightLeft);
  out.Set(2,camera);
  if(camera!=0) out.Set(7);
  if(camera>0.65f) out.Set(61);
  if(camera<-0.65f) out.Set(62);
  constexpr std::array<K,4> directions{K::Up,K::Right,K::Down,K::Left};
  constexpr std::array<unsigned,4> menu{95,98,96,97};
  // The shipped menu routes also consume this second action quartet. Its
  // names are rotated relative to physical directions in the native table;
  // preserve the verified physical direction, not the misleading label.
  constexpr std::array<unsigned,4> route{94,93,91,92}; // Up, Right, Down, Left
  unsigned count=0;
  for(unsigned i=0;i<4;++i) if(pad.Down(directions[i])) {
    out.Set(menu[i]);out.Set(route[i]);out.hero_slot=int(i);++count;
  }
  // Stick navigation supplies both menu action families, without hero selection.
  if(pad.Value(K::LeftUp)>0.5) {out.Set(95);out.Set(94);}
  if(pad.Value(K::LeftDown)>0.5) {out.Set(96);out.Set(91);}
  if(pad.Value(K::LeftLeft)>0.5) {out.Set(97);out.Set(92);}
  if(pad.Value(K::LeftRight)>0.5) {out.Set(98);out.Set(93);}
  if(count==1) out.Set(13);else out.hero_slot=-1;
  return out;
}
} // namespace moderngekko::controls
