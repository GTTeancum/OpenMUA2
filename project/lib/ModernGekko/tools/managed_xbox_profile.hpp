#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <sstream>
#include <string>
#include <string_view>

namespace moderngekko::controls {
inline constexpr std::string_view XboxBodyV5 = R"PROFILE(# OpenMUA2 Xbox action layout v5
Buttons/A = (!(`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & (`Button A` | `Button X`)) | ((`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & (`Button A` | `Button B` | `Button X` | `Button Y`))
Buttons/B = !(`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & (`Button B` | `Button X`)
Buttons/1 = `Back` | (!`Back` & !Start & !(`Trigger L` > 0.5) & ((`Right X+` > 0.2) | (`Right X-` > 0.2)))
Buttons/2 = Start | `Back`
Buttons/- = (`Trigger L` > 0.5) & !((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15))
Buttons/+ = !(`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & (`Pad N` | `Pad S` | `Pad W` | `Pad E`)
Buttons/Home =
D-Pad/Up = (!(`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & `Pad N`) | (((`Trigger R` > 0.5) & !(`Trigger L` > 0.5)) | ((`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & !((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15)))) & `Button Y`
D-Pad/Down = (!(`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & `Pad S`) | (((`Trigger R` > 0.5) & !(`Trigger L` > 0.5)) | ((`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & !((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15)))) & `Button A`
D-Pad/Left = (!(`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & `Pad W`) | (((`Trigger R` > 0.5) & !(`Trigger L` > 0.5)) | ((`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & !((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15)))) & `Button X`
D-Pad/Right = (!(`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & `Pad E`) | (((`Trigger R` > 0.5) & !(`Trigger L` > 0.5)) | ((`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & !((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15)))) & `Button B`
IR/Up = (`Trigger L` > 0.5) & `Right Y+`
IR/Down = (`Trigger L` > 0.5) & `Right Y-`
IR/Left = (`Trigger L` > 0.5) & `Right X-`
IR/Right = (`Trigger L` > 0.5) & `Right X+`
IR/Hide = !(`Trigger L` > 0.5)
Tilt/Left = !`Back` & !Start & !(`Trigger L` > 0.5) & `Right X-`
Tilt/Right = !`Back` & !Start & !(`Trigger L` > 0.5) & `Right X+`
Tilt/Dead Zone = 20.0
Shake/X =
Swing/Up =
Swing/Down =
Shake/Y =
Shake/Z =
Rumble/Motor = `Motor L` | `Motor R`
Extension = Nunchuk
Nunchuk/Buttons/C = !(`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & `Button Y`
Nunchuk/Buttons/Z = (`Shoulder L` | (`Trigger L` > 0.5)) & !(`Trigger R` > 0.5)
Nunchuk/Stick/Up = `Left Y+`
Nunchuk/Stick/Down = `Left Y-`
Nunchuk/Stick/Left = `Left X-`
Nunchuk/Stick/Right = `Left X+`
Nunchuk/Stick/Calibration = 100.00
Nunchuk/Stick/Dead Zone = 15.0
Nunchuk/Shake/X = (`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & !(`Button A` | `Button B` | `Button X` | `Button Y`) & !((`Right X+` > 0.15) | (`Right X-` > 0.15) | (`Right Y+` > 0.15) | (`Right Y-` > 0.15))
Nunchuk/Shake/Y =
Nunchuk/Shake/Z =
Options/Sideways Wiimote = False
)PROFILE";
using ProfileSection = std::map<std::string,std::string>;
using ProfileSections = std::map<std::string,ProfileSection>;
inline std::string ProfileTrim(std::string s) {
  const auto first=s.find_first_not_of(" \t\r\n");
  if(first==std::string::npos) return {};
  return s.substr(first,s.find_last_not_of(" \t\r\n")-first+1);
}
// Reject ambiguous duplicate sections/keys. Comments and serialization whitespace
// are harmless, but changed bindings or additional settings are not managed.
inline bool ParseXboxProfile(std::string_view text, ProfileSections& sections) {
  std::istringstream input{std::string(text)};std::string line,current;
  while(std::getline(input,line)) {
    line=ProfileTrim(line);
    if(line.empty() || line[0]=='#' || line[0]==';') continue;
    if(line.front()=='[' && line.back()==']') {
      current=line.substr(1,line.size()-2);
      if(current.empty() || sections.contains(current)) return false;
      sections.emplace(current,ProfileSection{});continue;
    }
    auto equals=line.find('=');
    if(current.empty() || equals==std::string::npos) return false;
    auto key=ProfileTrim(line.substr(0,equals));
    if(key.empty() || !sections[current].emplace(key,ProfileTrim(line.substr(equals+1))).second)
      return false;
  }
  return true;
}
inline std::uint8_t ManagedXboxPorts(std::string_view text) {
  ProfileSections sections,expected;
  if(!ParseXboxProfile(text,sections) ||
     !ParseXboxProfile("[Wiimote1]\n"+std::string(XboxBodyV5),expected)) return 0;
  std::uint8_t mask=0;
  for(unsigned port=0;port<4;++port) {
    auto it=sections.find("Wiimote"+std::to_string(port+1));
    if(it==sections.end()) continue;
    auto body=it->second;auto device=body.find("Device");
    if(device==body.end() || device->second.empty()) continue;
    body.erase(device);
    if(body==expected.at("Wiimote1")) mask|=std::uint8_t(1u<<port);
  }
  return mask;
}
inline bool XboxPortEnabled(std::uint8_t mask,std::uint32_t input) {
  constexpr std::uint32_t base=0x81313274,stride=0xbe00;
  return input>=base && input<base+4*stride && (input-base)%stride==0 &&
         (mask&(1u<<((input-base)/stride)));
}
}
