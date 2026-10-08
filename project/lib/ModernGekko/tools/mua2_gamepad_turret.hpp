#pragma once
#include "mua2_gamepad_actions.hpp"
#include <optional>

namespace moderngekko::controls {
struct TurretRotation { float pitch, yaw; };
// PS2 CCHWeapon rotates from stick deflection and native game delta, rather
// than constructing a pointer ray. Limits are relative to the reset entity.
inline std::optional<TurretRotation> RotateGamepadTurret(
    TurretRotation current, TurretRotation origin, float down_degrees,
    float up_degrees, float seconds, const GamepadSample& pad) {
  for(float v : {current.pitch,current.yaw,origin.pitch,origin.yaw,
                 down_degrees,up_degrees,seconds})
    if(!std::isfinite(v))return std::nullopt;
  if(seconds<0 || down_degrees<0 || up_degrees<0)return std::nullopt;
  const auto axis=[&](GamepadInput positive,GamepadInput negative) {
    const float v=static_cast<float>(pad.Value(positive)-pad.Value(negative));
    return std::abs(v)<=0.1f ? 0.0f : v;
  };
  current.pitch-=2.0071287155151367f*axis(GamepadInput::LeftUp,GamepadInput::LeftDown)*seconds;
  current.yaw-=3.0019662380218506f*axis(GamepadInput::LeftRight,GamepadInput::LeftLeft)*seconds;
  constexpr float radians=0.01745329238474369f, yaw_limit=1.3089969158172607f;
  current.pitch=std::clamp(current.pitch,origin.pitch-radians*down_degrees,
                         origin.pitch+radians*up_degrees);
  current.yaw=std::clamp(current.yaw,origin.yaw-yaw_limit,origin.yaw+yaw_limit);
  return current;
}
} // namespace moderngekko::controls
