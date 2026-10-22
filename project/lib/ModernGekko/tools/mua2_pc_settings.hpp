#pragma once
#include "mua2_pc_controls.hpp"
#include <charconv>
#include <sstream>
#include <string>
#include <unordered_set>

namespace moderngekko::controls {
// Stable action names make saved bindings independent of menu order. Unknown,
// duplicate, malformed and conflicting fields fail without replacing live data.
inline std::string EncodePcSettings(const PcSettings& s) {
  std::ostringstream out;
  out << "Version=1\nKeyboardEnabled=" << s.keyboard_enabled
      << "\nOutputWidth=" << s.output_width << "\nOutputHeight=" << s.output_height
      << "\nKeyboardPlayer=" << s.keyboard_player
      << "\nSeparateControllers=" << s.separate_controllers
      << "\nInvertCameraY=" << s.invert_camera_y
      << "\nMouseSensitivity=" << s.mouse_sensitivity << '\n';
  for(unsigned player=0;player<4;++player) for(unsigned a=0;a<PcActionCount;++a) {
    const auto& p=s.profiles[player];
    for(unsigned device=0;device<2;++device) {
      const auto b=device ? p.buttons[a]:p.keys[a];
      for(unsigned alternate=0;alternate<2;++alternate)
        out << "Player" << player+1 << '.' << (device ? "Pad.":"Key.")
            << PcActionNames[a] << (alternate ? ".Secondary=":".Primary=")
            << (alternate ? b.secondary:b.primary) << '\n';
    }
  }
  return out.str();
}
inline bool DecodePcSettings(std::string_view text,PcSettings& output,std::string& error) {
  error.clear();
  auto fail=[&](std::string_view message){error=message;return false;};
  if(text.size()>128*1024) return fail("Settings file is too large");
  PcSettings next=DefaultPcSettings();
  bool version=false;
  std::unordered_set<std::string> seen;
  while(!text.empty()) {
    const auto end=text.find('\n');
    auto line=text.substr(0,end);
    text=end==text.npos ? std::string_view{}:text.substr(end+1);
    if(!line.empty() && line.back()=='\r') line.remove_suffix(1);
    if(line.empty() || line.front()=='#') continue;
    const auto equal=line.find('=');
    if(equal==line.npos) return fail("Settings line is missing '='");
    const auto name=line.substr(0,equal),value=line.substr(equal+1);
    if(!seen.emplace(name).second) return fail("Duplicate settings field");
    unsigned number=0;
    const auto parsed=std::from_chars(value.data(),value.data()+value.size(),number);
    if(parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size()) return fail("Invalid numeric setting");
    if(name=="Version") {if(number!=1) return fail("Unsupported settings version");version=true;}
    else if(name=="KeyboardPlayer") next.keyboard_player=number;
    else if(name=="OutputWidth") next.output_width=number;
    else if(name=="OutputHeight") next.output_height=number;
    else if(name=="MouseSensitivity") next.mouse_sensitivity=number;
    else if(name=="KeyboardEnabled" || name=="SeparateControllers" || name=="InvertCameraY") {
      if(number>1) return fail("Boolean setting must be zero or one");
      if(name=="KeyboardEnabled") next.keyboard_enabled=number!=0;
      if(name=="SeparateControllers") next.separate_controllers=number!=0;
      if(name=="InvertCameraY") next.invert_camera_y=number!=0;
    } else {
      if(name.size()<8 || !name.starts_with("Player") || name[6]<'1' || name[6]>'4' || name[7]!='.')
        return fail("Unknown settings field");
      auto field=name.substr(8);
      const bool pad=field.starts_with("Pad.");
      if(!pad && !field.starts_with("Key.")) return fail("Unknown binding device");
      field.remove_prefix(4);
      const bool alternate=field.ends_with(".Secondary");
      if(!alternate && !field.ends_with(".Primary")) return fail("Unknown binding column");
      field.remove_suffix(alternate ? 10:8);
      const auto action=std::find(PcActionNames.begin(),PcActionNames.end(),field);
      if(action==PcActionNames.end()) return fail("Unknown binding action");
      auto& p=next.profiles[name[6]-'1'];
      auto& b=(pad ? p.buttons:p.keys)[action-PcActionNames.begin()];
      (alternate ? b.secondary:b.primary)=number;
    }
  }
  if(!version) return fail("Missing settings version");
  if(const auto issue=ValidatePcSettings(next);!issue.empty()) return fail(issue);
  output=next;
  return true;
}
// The native menu edits a draft. Cancel never mutates applied controls; save
// failure likewise leaves the live profile intact. The host owns atomic I/O.
struct PcOptionsModel {
  PcSettings applied=DefaultPcSettings(),draft=applied;
  unsigned player=0;
  bool active=false;
  std::string status;
  void Open(const PcSettings& current) {applied=draft=current;active=true;status.clear();}
  void Cancel() {draft=applied;active=false;status.clear();}
  bool Bind(unsigned action,unsigned source,bool alternate,bool controller) {
    if(player>=4 || action>=PcActionCount) return false;
    auto candidate=draft;
    auto& p=candidate.profiles[player];
    auto& b=(controller ? p.buttons:p.keys)[action];
    (alternate ? b.secondary:b.primary)=source;
    // Moving an existing binding from one column to the other is not a conflict.
    if(source && b.primary==b.secondary) (alternate ? b.primary:b.secondary)=0;
    status=ValidatePcSettings(candidate);
    if(!status.empty()) return false;
    draft=candidate;
    return true;
  }
  template<class Save> bool Apply(Save&& save) {
    status=ValidatePcSettings(draft);
    if(!status.empty() || !save(EncodePcSettings(draft),status)) return false;
    applied=draft;active=false;
    return true;
  }
};
} // namespace moderngekko::controls
