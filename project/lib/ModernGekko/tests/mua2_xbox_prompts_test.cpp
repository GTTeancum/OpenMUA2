#include "mua2_xbox_prompts.hpp"
#include <iostream>
using namespace moderngekko::controls;
int main() {
  unsigned failures = 0;
  const auto check = [&](bool ok, const char* message) {
    if (!ok) { std::cerr << message << '\n'; ++failures; }
  };
  check(XboxActionPrompt("Action") == XboxPrompt::X, "Context use must show X, not chord's first button");
  check(XboxActionPrompt("ACTION") == XboxPrompt::X, "Native statue interaction uses uppercase ACTION");
  check(XboxActionPrompt("USEGRABICON") == XboxPrompt::X, "Legacy use token must agree");
  check(XboxActionPrompt("MenuAccept") == XboxPrompt::A, "Menu accept");
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
  return failures ? 1 : 0;
}
