// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <windows.h>
#include <xinput.h>

namespace ciface::XInput
{
// XInput does not promise to write the output buffer on failure. Never expose
// the previous held controls (or a partially written packet) after a failed poll.
template <typename Poll>
bool PollState(DWORD index, XINPUT_STATE& state, Poll poll)
{
  XINPUT_STATE next{};
  const bool connected = poll(index, &next) == ERROR_SUCCESS;
  state = connected ? next : XINPUT_STATE{};
  return connected;
}
}  // namespace ciface::XInput
