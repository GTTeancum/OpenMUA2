// Copyright 2026 OpenMUA2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "Common/CommonTypes.h"

namespace JitCommon
{
// Conservative half-open guest ranges. Wrapping or empty ranges are not safe
// inputs to the replacement; callers must still establish mapped RAM access.
constexpr bool SimpleFormatRangesDisjoint(u32 a, u32 a_size, u32 b, u32 b_size)
{
  const u64 a_end = u64{a} + a_size;
  const u64 b_end = u64{b} + b_size;
  return a_size && b_size && a_end <= (u64{1} << 32) && b_end <= (u64{1} << 32) &&
         (a_end <= b || b_end <= a);
}

// The known caller's frame contains the register save area, va_list, saved
// nonvolatile registers and return address. Publishing an output must not
// overwrite them; va_list updates must not modify the format being consumed.
constexpr bool SimpleFormatLayoutSafe(u32 sp, u32 output, u32 format, u32 arguments)
{
  constexpr u32 frame_size = 0x98;
  return !(sp & 15) && u64{sp} + frame_size <= (u64{1} << 32) &&
         u64{arguments} == u64{sp} + 0x68 &&
         SimpleFormatRangesDisjoint(output, 1024, sp, frame_size) &&
         SimpleFormatRangesDisjoint(format, 512, sp, frame_size) &&
         SimpleFormatRangesDisjoint(output, 1024, format, 512);
}

// EABI caller-preserved state. Volatile differences remain diagnostic; they do
// not by themselves violate a function replacement's ABI. This does not prove
// instruction timing or interrupt-observable architectural equivalence.
constexpr bool SimpleFormatPreservedStateChanged(u32 gpr, u32 ps0, u32 ps1, u32 cr)
{
  return (gpr & 0xffffe006u) || (ps0 & 0xffffc000u) || (ps1 & 0xffffc000u) ||
         (cr & 0x1cu);
}
}  // namespace JitCommon
