#pragma once
#include <cstdint>
#include <optional>
#include <string_view>

namespace moderngekko::controls {
// Icon IDs, before the native text formatter adds 162. These IDs require the
// matching private Xbox menu font pack. They are not raw controller buttons.
enum class XboxPrompt : std::uint8_t {
  Start = 1, A = 2, B = 3, Y = 4, LB = 5, DPad = 6,
  LT = 15, RT = 16, View = 17, LeftStick = 18, RightStick = 19, X = 24
};

inline std::optional<XboxPrompt> XboxActionPrompt(std::string_view token) {
  // Short, explicit Xbox tokens in the paired private tutorial pack.
  if (token == "XA") return XboxPrompt::A;
  if (token == "XB") return XboxPrompt::B;
  if (token == "XX") return XboxPrompt::X;
  if (token == "XY") return XboxPrompt::Y;
  if (token == "XLT") return XboxPrompt::LT;
  if (token == "XRT") return XboxPrompt::RT;
  if (token == "XD" || token == "DPAD") return XboxPrompt::DPad;
  if (token == "XL" || token == "MoveX" || token == "MoveY" ||
      token == "MOVE_X" || token == "MOVE_Y" || token == "MOVE") return XboxPrompt::LeftStick;
  if (token == "XR") return XboxPrompt::RightStick;
  if (token == "XV" || token == "MENU" || token == "HeroManagement") return XboxPrompt::View;
  // Prefer action semantics: the original grab/use descriptors are chords,
  // while the managed Xbox layout exposes one contextual X button.
  if (token == "Grab" || token == "Action" || token == "ACTION" || token == "GRAB" ||
      token == "USEGRABICON" || token == "GUARD") return XboxPrompt::X;
  if (token == "Attack" || token == "ATTACK" || token == "LIGHTATTACKICON" ||
      token == "MenuAccept" || token == "MENU_ACCEPT" || token == "ABUTTON")
    return XboxPrompt::A;
  if (token == "Smash" || token == "SMASH" || token == "HEAVYATTACKICON" ||
      token == "CHARGEICON" || token == "MenuBack" || token == "MENU_BACK" ||
      token == "BACKBUTTON" || token == "BBUTTON") return XboxPrompt::B;
  if (token == "Jump" || token == "FlyUp" || token == "CBUTTON") return XboxPrompt::Y;
  if (token == "Block" || token == "BLOCK" || token == "FlyDown" ||
      token == "MenuViewDetails" || token == "MENU_DETAILS" || token == "ZBUTTON") return XboxPrompt::LB;
  if (token == "Pause" || token == "PAUSE" || token == "PAUSEBUTTON" ||
      token == "2BUTTON") return XboxPrompt::Start;
  // Do not label unresolved power/fusion/motion instructions as working inputs.
  return std::nullopt;
}

inline bool XboxPromptPort(std::uint8_t managed_ports, std::uint32_t object) {
  constexpr std::uint32_t first = 0x81313274, stride = 0xbe00;
  if (object < first || object >= first + 4 * stride || (object-first)%stride)
    return false;
  return (managed_ports & (1u << ((object-first)/stride))) != 0;
}
}
