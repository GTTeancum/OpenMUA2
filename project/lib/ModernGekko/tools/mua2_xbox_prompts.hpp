#pragma once
#include <cstdint>
#include <optional>
#include <string_view>

namespace moderngekko::controls {
// Icon IDs, before the native text formatter adds 162. These IDs require the
// matching private Xbox menu font pack. They are not raw controller buttons.
enum class XboxPrompt : std::uint8_t {
  Start = 1, A = 2, B = 3, Y = 4, LB = 5, DPad = 6,
  LT = 63, RT = 64, View = 65, LeftStick = 66, RightStick = 67, RB = 69, X = 72
};

inline std::optional<XboxPrompt> XboxActionPrompt(std::string_view token,
                                                bool start_accept_screen = false,
                                                bool direct_gamepad = false) {
  // MENU_OK is action 105, not ordinary A/accept. The managed Start adapter
  // emits it only on the verified title/profile screens. Keep other contexts
  // unresolved rather than displaying a button that cannot perform the action.
  if (token == "MENU_OK")
    return start_accept_screen ? std::optional{XboxPrompt::Start} : std::nullopt;
  // Profile-ready labels Start Game with the legacy MenuExit token.
  // Only advertise Start where the matching Start-accept binding is active.
  if (token == "MenuExit")
    return direct_gamepad && start_accept_screen ? std::optional{XboxPrompt::Start} : std::nullopt;
  // Shared menu action tokens are resolved from the provider's menu map.
  // Keep legacy profile prompts intact until the paired provider pack is used.
  if (direct_gamepad) {
    if (token == "MenuAddSkillPoint") return XboxPrompt::RB;
    if (token == "MenuRemoveSkillPoint") return XboxPrompt::LB;
    if (token == "MenuReallocate" || token == "MenuSwitchToPrevHero") return XboxPrompt::LT;
    if (token == "MenuSwitchToNextHero") return XboxPrompt::RT;
    if (token == "MenuAssignPowers") return XboxPrompt::A;
    if (token == "MenuViewDetails" || token == "MENU_DETAILS" || token == "MENU_OTHER" ||
        token == "MenuRemoveBoost" || token == "MenuDeleteProfile" || token == "MenuCycleHeroes")
      return XboxPrompt::X;
    if (token == "MenuToggleAutoSpend" || token == "MenuMinimize") return XboxPrompt::Y;
  }
  // Short, explicit Xbox tokens in the paired private tutorial pack.
  if (token == "XA") return XboxPrompt::A;
  if (token == "XB") return XboxPrompt::B;
  if (token == "XX") return XboxPrompt::X;
  if (token == "XY") return XboxPrompt::Y;
  if (token == "XLT") return XboxPrompt::LT;
  if (token == "XRT") return XboxPrompt::RT;
  if (token == "XRB") return XboxPrompt::RB;
  if (token == "XD" || token == "DPAD") return XboxPrompt::DPad;
  if (token == "XL" || token == "MoveX" || token == "MoveY" ||
      token == "MOVE_X" || token == "MOVE_Y" || token == "LEFT_STICK" || token == "MOVE") return XboxPrompt::LeftStick;
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
  if (token == "Jump" || token == "FlyUp" || token == "CBUTTON" ||
      token == "MENU_SUBTRACT" || token == "AUTOSPEND") return XboxPrompt::Y;
  if (token == "Block" || token == "BLOCK" || token == "FlyDown" ||
      token == "MenuViewDetails" || token == "MENU_DETAILS" || token == "MENU_OTHER" ||
      token == "ZBUTTON") return XboxPrompt::LB;
  if (token == "Pause" || token == "PAUSE" || token == "PAUSEBUTTON" ||
      token == "2BUTTON") return XboxPrompt::Start;
  // Do not label unresolved power/fusion/motion instructions as working inputs.
  return std::nullopt;
}

// PS2 0x4b6c10 resolves power actions 29..32 to their face-button cells.
// A samepowerhold hint is a power-button instruction, not stick steering.
// Preserve its static/alternate-frame selection using the paired Xbox HUD pack.
inline std::optional<std::uint32_t> XboxPowerHudSprite(
    std::uint32_t action, bool alternate, bool static_icon) {
  // PS2 suppresses power-action hints in the compact/static-icon branch.
  if(action>=29 && action<=32 && static_icon)return 114u; // native no-icon sentinel
  switch (action) {
  case 29: return alternate ? 42u : 43u; // A
  case 30: return 99u; // X: shared static Xbox X cell, not rapid-tap poses
  case 31: return alternate ? 44u : 45u; // B
  case 32: return alternate ? 48u : 49u; // Y
  default: return std::nullopt;
  }
}

inline bool XboxPromptPort(std::uint8_t managed_ports, std::uint32_t object) {
  constexpr std::uint32_t first = 0x81313274, stride = 0xbe00;
  if (object < first || object >= first + 4 * stride || (object-first)%stride)
    return false;
  return (managed_ports & (1u << ((object-first)/stride))) != 0;
}
}
