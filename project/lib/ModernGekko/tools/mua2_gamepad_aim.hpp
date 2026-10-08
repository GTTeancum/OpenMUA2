#pragma once
#include "mua2_action_bindings.hpp"
#include "mua2_gamepad_actions.hpp"
#include <optional>

namespace moderngekko::controls {
// Shared aiming ABI. Network Game state is deliberately absent: local gamepad
// aim is a device capability, independent of CNetPlayManager session flags.
inline bool GamepadAimContext(std::span<const std::uint8_t> manager) {
  return manager.size()==25864 && ReadBE(manager,0)==0x81198830 &&
    // Native getter81015AC8 rotates left25 and masks bit31: source bit7.
    (manager[5925]&0x80)!=0 && ReadBE(manager,5192)<=2 &&
    ReadBE(manager,25860)==0;
}
inline std::optional<unsigned> GamepadAimRecord(std::span<const std::uint8_t> manager,
                                               unsigned port) {
  if(port>=4 || !GamepadAimContext(manager))return std::nullopt;
  return 4904+72*port;
}
// PS2 00523A78 centers on entry to mazehack. Use the native mode transition
// instead of a host-side history flag, so save-state restoration stays coherent.
inline bool CenterGamepadAimOnModeChange(std::span<std::uint8_t> manager,
    std::uint32_t previous, std::uint32_t next, std::uint32_t mazehack) {
  if(!mazehack || previous==next || next!=mazehack || !GamepadAimContext(manager))
    return false;
  for(unsigned port=0;port<4;++port)
    std::fill_n(manager.begin()+4904+72*port,8,std::uint8_t{0});
  return true;
}
// PS2 00523570, default mode3 (00736BC0): accumulate each axis /20 per
// native update outside +/-0.1, clamp to [-1,1], retain position on release.
// Only the sample prefix changes. Native projection/collision stays native.
inline bool WriteGamepadAim(std::span<std::uint8_t> record,const GamepadSample& pad,
                            bool joined, bool instruction_visible=false) {
  if(record.size()!=72)return false;
  const float x=std::bit_cast<float>(ReadBE(record,0));
  const float y=std::bit_cast<float>(ReadBE(record,4));
  if(!std::isfinite(x) || !std::isfinite(y))return false;
  const bool valid=pad.connected && joined;
  const auto axis=[&](float current,GamepadInput positive,GamepadInput negative) {
    const float value=static_cast<float>(pad.Value(positive)-pad.Value(negative));
    // PS2 00523A64..74 centers while CHudPointerError instructions are visible.
    // Confirmation must not leave a target displaced by stick use in the panel.
    if(instruction_visible)return 0.0f;
    const float change=valid && std::abs(value)>0.1f ? value/20.0f : 0.0f;
    return std::clamp(current+change,-1.0f,1.0f);
  };
  const std::array<std::uint32_t,3> words{
    std::bit_cast<std::uint32_t>(axis(x,GamepadInput::LeftRight,GamepadInput::LeftLeft)),
    std::bit_cast<std::uint32_t>(axis(y,GamepadInput::LeftUp,GamepadInput::LeftDown)),
    valid?1u:0u};
  for(unsigned word=0;word<3;++word)for(unsigned byte=0;byte<4;++byte)
    record[word*4+byte]=std::uint8_t(words[word]>>(24-byte*8));
  return true;
}
}
