#include "mua2_xbox_prompts.hpp"
#include <iostream>
using namespace moderngekko::controls;
int main() {
  unsigned failures = 0;
  const auto check = [&](bool ok, const char* message) {
    if (!ok) { std::cerr << message << '\n'; ++failures; }
  };
  check(XboxActionPrompt("MenuAddSkillPoint",false,true)==XboxPrompt::RB &&
        XboxActionPrompt("MenuRemoveSkillPoint",false,true)==XboxPrompt::LB &&
        XboxActionPrompt("MenuReallocate",false,true)==XboxPrompt::LT, "Direct menu point prompts");
  check(!XboxActionPrompt("MenuAddSkillPoint") && !XboxActionPrompt("MenuReallocate"),
        "Do not advertise missing legacy-profile bindings");
  check(XboxActionPrompt("MENU_OTHER",false,true)==XboxPrompt::X &&
        XboxActionPrompt("MenuViewDetails",false,true)==XboxPrompt::X &&
        XboxActionPrompt("MenuAssignPowers",false,true)==XboxPrompt::A, "Direct menu face prompts");
  check(XboxActionPrompt("Block",false,true)==XboxPrompt::LB &&
        XboxActionPrompt("FlyDown",false,true)==XboxPrompt::LB, "Menu map must not relabel gameplay LB");
  check(XboxActionPrompt("LEFT_STICK")==XboxPrompt::LeftStick, "Native power description stick token");
  check(XboxActionPrompt("Action") == XboxPrompt::X, "Context use must show X, not chord's first button");
  check(XboxActionPrompt("ACTION") == XboxPrompt::X, "Native statue interaction uses uppercase ACTION");
  check(XboxActionPrompt("USEGRABICON") == XboxPrompt::X, "Legacy use token must agree");
  check(XboxActionPrompt("MenuAccept") == XboxPrompt::A, "Menu accept");
  check(XboxActionPrompt("MENU_OK",true) == XboxPrompt::Start, "Title/profile continue uses Start");
  check(XboxActionPrompt("MenuExit",true,true) == XboxPrompt::Start, "Profile-ready Start Game glyph");
  check(!XboxActionPrompt("MenuExit",false,true) && !XboxActionPrompt("MenuExit",true,false),
        "Preserve MenuExit outside Start-accept/provider context");
  check(!XboxActionPrompt("MENU_OK") && !XboxActionPrompt("MENU_OK",false),
        "Other action-105 contexts must not claim a working Start/A binding");
  check(XboxActionPrompt("MENU_ACCEPT",true) == XboxPrompt::A,
        "Start-accept context must not change ordinary A prompts");
  check(XboxActionPrompt("MENU_OTHER") == XboxPrompt::LB, "Other/details share native action 101");
  check(XboxActionPrompt("MENU_SUBTRACT") == XboxPrompt::Y &&
        XboxActionPrompt("AUTOSPEND") == XboxPrompt::Y, "Verified C-button menu actions use Y");
  check(XboxActionPrompt("BACKBUTTON") == XboxPrompt::B, "Menu back must not show View");
  check(XboxActionPrompt("PAUSEBUTTON") == XboxPrompt::Start, "Pause must not show Wii 2");
  check(XboxActionPrompt("Block") == XboxPrompt::LB, "Block must not show use X");
  check(XboxActionPrompt("DPAD") == XboxPrompt::DPad, "Native DPAD must retain its own glyph");
  check(XboxActionPrompt("DPAD") != XboxActionPrompt("ACTION"), "D-pad must never display contextual X");
  check(XboxActionPrompt("XLT") == XboxPrompt::LT && XboxActionPrompt("XRT") == XboxPrompt::RT, "Trigger glyphs");
  check(XboxActionPrompt("XA") == XboxPrompt::A && XboxActionPrompt("XB") == XboxPrompt::B &&
        XboxActionPrompt("XX") == XboxPrompt::X && XboxActionPrompt("XY") == XboxPrompt::Y, "Explicit face glyphs");
  check(XboxActionPrompt("XD") == XboxPrompt::DPad && XboxActionPrompt("XL") == XboxPrompt::LeftStick &&
        XboxActionPrompt("XR") == XboxPrompt::RightStick && XboxActionPrompt("XV") == XboxPrompt::View, "Auxiliary glyphs");
  check(XboxActionPrompt("MENU") == XboxPrompt::View && XboxActionPrompt("MENU_DETAILS") == XboxPrompt::LB, "Verified menu actions");
  check(XboxActionPrompt("FlyUp") == XboxPrompt::Y && XboxActionPrompt("FlyDown") == XboxPrompt::LB, "Flight directions");
  check(!XboxActionPrompt("Power1") && !XboxActionPrompt("HOMEBUTTON"), "Unresolved actions must not acquire invented prompts");
  check(!XboxActionPrompt("MenuAcceptExtra") && !XboxActionPrompt("menuaccept"), "Exact tokens only");
  check(!XboxActionPrompt(std::string_view("MenuAccept\0garbage",18)), "Embedded NUL is not a matching token");
  for (unsigned port=0; port<4; ++port) {
    const auto object=0x81313274+port*0xbe00;
    check(XboxPromptPort(1u<<port,object), "Managed port rejected");
    check(!XboxPromptPort(15u^(1u<<port),object), "Custom port modified");
    check(!XboxPromptPort(15,object+4), "Interior pointer accepted");
  }
  check(!XboxPromptPort(15,0x81313270) && !XboxPromptPort(15,0x81342a74), "Out-of-range object accepted");
  check(XboxPowerHudSprite(29,false,false)==43 && XboxPowerHudSprite(29,true,false)==42 &&
        XboxPowerHudSprite(29,false,true)==114, "PS2 samepowerhold must show A, not D-pad or steering");
  check(XboxPowerHudSprite(30,false,false)==99, "Power 2 must show static X, not LB or rapid tapping");
  check(XboxPowerHudSprite(31,false,false)==45 && XboxPowerHudSprite(32,false,false)==49,
        "Power 3/4 must show B/Y");
  for(unsigned action=0;action<61;++action)
    if(action<29 || action>32)
      check(!XboxPowerHudSprite(action,false,false), "Unrelated HUD actions must retain native handling");
  return failures ? 1 : 0;
}
