#pragma once
#include <span>
#include <string>
#include <string_view>

namespace moderngekko::frontend::legacy {
// Exact templates from our own v1/v2 generators, used only to recognize
// unmodified managed profiles. Any changed binding or extra setting is preserved.
inline constexpr std::string_view BodyV1 = R"PROFILE(# OpenMUA2 Xbox action layout v1
Buttons/A = ((!`Shoulder R`) & (`Button A` | `Button X`)) | hold(`Shoulder L` & `Shoulder R` & (`Button A` | `Button B` | `Button X` | `Button Y`), 0.12)
Buttons/B = ((!`Shoulder R`) & (`Button B` | `Button X`))
Buttons/1 = (!(`Shoulder L` & `Shoulder R`)) & ((`Right X+` > 0.2) | (`Right X-` > 0.2))
Buttons/2 = Start
Buttons/- = `Trigger L` > 0.5
Buttons/+ = `Trigger R` > 0.5
Buttons/Home = 
D-Pad/Up = `Pad N` | (`Shoulder R` & !`Shoulder L` & `Button Y`)
D-Pad/Down = `Pad S` | (`Shoulder R` & !`Shoulder L` & `Button A`)
D-Pad/Left = `Pad W` | (`Shoulder R` & !`Shoulder L` & `Button X`)
D-Pad/Right = `Pad E` | (`Shoulder R` & !`Shoulder L` & `Button B`)
IR/Up = 0.50 * (`Shoulder L` & `Shoulder R` & (`Button A` | `Button X` | `Button Y`))
IR/Down = 0.50 * (`Shoulder L` & `Shoulder R` & `Button B`)
IR/Left = 0.62 * (`Shoulder L` & `Shoulder R` & (`Button A` | `Button X`))
IR/Right = 0.62 * (`Shoulder L` & `Shoulder R` & (`Button B` | `Button Y`))
IR/Hide = !(`Shoulder L` & `Shoulder R`)
Tilt/Left = `Right X-` & !(`Shoulder L` & `Shoulder R`)
Tilt/Right = `Right X+` & !(`Shoulder L` & `Shoulder R`)
Tilt/Dead Zone = 20.0
Shake/X = 
Shake/Y = 
Shake/Z = 
Rumble/Motor = Motor
Extension = Nunchuk
Nunchuk/Buttons/C = `Button Y` & !`Shoulder R`
Nunchuk/Buttons/Z = `Shoulder L`
Nunchuk/Stick/Up = `Left Y+`
Nunchuk/Stick/Down = `Left Y-`
Nunchuk/Stick/Left = `Left X-`
Nunchuk/Stick/Right = `Left X+`
Nunchuk/Stick/Calibration = 100.00
Nunchuk/Stick/Dead Zone = 15.0
Nunchuk/Shake/X = `Shoulder L` & `Shoulder R` & !(`Button A` | `Button B` | `Button X` | `Button Y`)
Nunchuk/Shake/Y = 
Nunchuk/Shake/Z = 
Options/Sideways Wiimote = False
)PROFILE";
inline constexpr std::string_view BodyV2 = R"PROFILE(# OpenMUA2 Xbox action layout v2
Buttons/A = ((!`Shoulder R`) & (`Button A` | `Button X`)) | hold(`Shoulder L` & `Shoulder R` & (`Button A` | `Button B` | `Button X` | `Button Y`), 0.12)
Buttons/B = ((!`Shoulder R`) & (`Button B` | `Button X`))
Buttons/1 = (!(`Shoulder L` & `Shoulder R`)) & ((`Right X+` > 0.2) | (`Right X-` > 0.2))
Buttons/2 = Start
Buttons/- = `Trigger L` > 0.5
Buttons/+ = `Trigger R` > 0.5
Buttons/Home = 
D-Pad/Up = `Pad N` | (`Shoulder R` & !`Shoulder L` & `Button Y`)
D-Pad/Down = `Pad S` | (`Shoulder R` & !`Shoulder L` & `Button A`)
D-Pad/Left = `Pad W` | (`Shoulder R` & !`Shoulder L` & `Button X`)
D-Pad/Right = `Pad E` | (`Shoulder R` & !`Shoulder L` & `Button B`)
IR/Up = (`Shoulder L` & `Shoulder R`) & ((`Right Y+` & ((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15))) | (0.50 * (`Button A` | `Button X` | `Button Y`) & !((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15))))
IR/Down = (`Shoulder L` & `Shoulder R`) & ((`Right Y-` & ((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15))) | (0.50 * `Button B` & !((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15))))
IR/Left = (`Shoulder L` & `Shoulder R`) & ((`Right X-` & ((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15))) | (0.62 * (`Button A` | `Button X`) & !((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15))))
IR/Right = (`Shoulder L` & `Shoulder R`) & ((`Right X+` & ((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15))) | (0.62 * (`Button B` | `Button Y`) & !((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15))))
IR/Hide = !(`Shoulder L` & `Shoulder R`)
Tilt/Left = `Right X-` & !(`Shoulder L` & `Shoulder R`)
Tilt/Right = `Right X+` & !(`Shoulder L` & `Shoulder R`)
Tilt/Dead Zone = 20.0
Shake/X = 
Shake/Y = 
Shake/Z = 
Rumble/Motor = Motor
Extension = Nunchuk
Nunchuk/Buttons/C = `Button Y` & !`Shoulder R`
Nunchuk/Buttons/Z = `Shoulder L`
Nunchuk/Stick/Up = `Left Y+`
Nunchuk/Stick/Down = `Left Y-`
Nunchuk/Stick/Left = `Left X-`
Nunchuk/Stick/Right = `Left X+`
Nunchuk/Stick/Calibration = 100.00
Nunchuk/Stick/Dead Zone = 15.0
Nunchuk/Shake/X = `Shoulder L` & `Shoulder R` & !(`Button A` | `Button B` | `Button X` | `Button Y`) & !((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15))
Nunchuk/Shake/Y = 
Nunchuk/Shake/Z = 
Options/Sideways Wiimote = False
)PROFILE";
inline std::string Profile(std::span<const std::string> devices, int version) {
  const auto body = version == 1 ? BodyV1 : BodyV2;
  std::string result;
  for (std::size_t i = 0; i < 4; ++i) {
    result += "[Wiimote" + std::to_string(i + 1) + "]\n";
    if (i < devices.size()) result += "Device = " + devices[i] + "\n" + std::string(body);
  }
  return result + "[BalanceBoard]\n";
}
} // namespace moderngekko::frontend::legacy
