// SPDX-License-Identifier: GPL-2.0-or-later
#include "InputCommon/ControllerInterface/XInput/PollState.h"
#include <cstdio>
#include <cstring>

int main()
{
  XINPUT_STATE state{};
  auto pressed = [](DWORD index, XINPUT_STATE* out) -> DWORD {
    if (index != 2) return ERROR_DEVICE_NOT_CONNECTED;
    out->dwPacketNumber = 42;
    out->Gamepad.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_DPAD_RIGHT;
    out->Gamepad.bLeftTrigger = 255;
    out->Gamepad.bRightTrigger = 200;
    out->Gamepad.sThumbLX = -32768;
    out->Gamepad.sThumbRY = 32767;
    return ERROR_SUCCESS;
  };
  auto neutral = [&] {
    const XINPUT_STATE zero{};
    return std::memcmp(&state, &zero, sizeof(state)) == 0;
  };
  auto check = [](bool passed, const char* message) {
    if (!passed) std::fprintf(stderr, "FAIL: %s\n", message);
    return passed;
  };
  using ciface::XInput::PollState;
  if (!check(PollState(2, state, pressed) && state.Gamepad.wButtons ==
      (XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_DPAD_RIGHT), "connected input")) return 1;
  // Real failure contract: no output write, including repeated disconnected polls.
  auto unplugged = [](DWORD, XINPUT_STATE*) -> DWORD { return ERROR_DEVICE_NOT_CONNECTED; };
  for (int i = 0; i < 3; ++i)
    if (!check(!PollState(2, state, unplugged) && neutral(), "disconnect releases all controls")) return 1;
  if (!check(PollState(2, state, pressed) && state.Gamepad.bLeftTrigger == 255 &&
      state.Gamepad.sThumbLX == -32768, "same-slot reconnection")) return 1;
  auto partial_error = [](DWORD, XINPUT_STATE* out) -> DWORD {
    out->Gamepad.wButtons = XINPUT_GAMEPAD_B;
    return ERROR_GEN_FAILURE;
  };
  if (!check(!PollState(2, state, partial_error) && neutral(), "failed partial packet discarded")) return 1;
  auto released = [](DWORD, XINPUT_STATE* out) -> DWORD { *out = {}; return ERROR_SUCCESS; };
  if (!check(PollState(2, state, pressed) && PollState(2, state, released) && neutral(),
      "normal release after reconnect")) return 1;
  std::puts("PASS: XInput held controls, disconnect, repeated failure, reconnect and release");
  return 0;
}
