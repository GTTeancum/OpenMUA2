// Copyright 2026 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>
#include <vector>

#include "Common/CommonTypes.h"

namespace Core
{
class System;
}

// Default-off game-library replacement experiment, restricted to the known
// temporary logger buffer and code identity. Shadow mode executes original code.
class JitSimpleFormatter
{
public:
  void Init();
  void Finish();
  void ClearPending();
  bool Handles(u32 pc) const;
  static bool Invoke(JitSimpleFormatter* self, Core::System* system, u32 pc);

private:
  enum class Mode { Disabled, Shadow, Replace };
  struct Pending
  {
    u32 sp;
    u32 output;
    u32 length;
    u32 arguments;
    u32 fpscr;
    std::array<u8, 12> argument_state;
    std::array<u8, 1024> expected;
    std::array<u32, 32> gpr{};
    std::array<u64, 32> ps0{}, ps1{};
    std::array<u32, 8> cr{};
  };
  bool Run(Core::System& system, u32 pc);
  Mode m_mode = Mode::Disabled;
  std::vector<Pending> m_pending;
  u64 m_entries = 0;
  u64 m_eligible = 0;
  u64 m_replaced = 0;
  u64 m_compared = 0;
  u64 m_mismatches = 0;
  u64 m_guard_rejects = 0;
  u64 m_unsupported = 0;
  struct UnsupportedFormat { u32 address = 0; u64 count = 0; };
  std::array<UnsupportedFormat, 64> m_unsupported_formats{};
  u64 m_unsupported_overflow = 0;
  u64 m_abandoned = 0;
  u64 m_floating = 0;
  u64 m_fpscr_mismatches = 0;
  u32 m_changed_gpr = 0, m_changed_ps0 = 0, m_changed_ps1 = 0, m_changed_cr = 0;
};
