// Copyright 2026 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <algorithm>
#include <array>
#include <span>

#include "Common/CommonTypes.h"

namespace JitCommon
{
// Cache a verified byte sequence, never a pointer or an unchecked identity result.
// Every successful call compares all live bytes. The validator sees the captured
// copy, so it cannot authenticate different bytes from those retained here.
// Caller must serialize access with writes to the source and to this object.
template <std::size_t Size>
class VerifiedCodeIdentity
{
public:
  template <typename Validator>
  bool Matches(std::span<const u8> code, Validator&& validate)
  {
    if (code.size() != Size)
      return false;
    if (!m_verified)
    {
      std::copy(code.begin(), code.end(), m_bytes.begin());
      if (!validate(std::span<const u8>(m_bytes)))
        return false;
      m_verified = true;
    }
    return std::equal(code.begin(), code.end(), m_bytes.begin());
  }

private:
  std::array<u8, Size> m_bytes{};
  bool m_verified = false;
};
}  // namespace JitCommon
