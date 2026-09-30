// Copyright 2026 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Core/PowerPC/JitCommon/JitSimpleFormatter.h"

#include <algorithm>
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string_view>

#include <fmt/format.h>

#include "Common/Crypto/SHA1.h"
#include "Common/Swap.h"
#include "Core/HW/Memmap.h"
#include "Core/PowerPC/BreakPoints.h"
#include "Core/PowerPC/JitCommon/SimpleFormat.h"
#include "Core/PowerPC/MMU.h"
#include "Core/PowerPC/PowerPC.h"
#include "Core/System.h"

namespace
{
constexpr u32 ENTRY = 0x803c63bc;
constexpr u32 RETURN = 0x801e844c;
constexpr u32 CODE_BEGIN = 0x803c57c8;
constexpr u32 CODE_SIZE = 3196;
constexpr Common::SHA1::Digest CODE_HASH = {
    0x59, 0x8f, 0x6c, 0x17, 0xb1, 0xe0, 0xe7, 0x16, 0xfa, 0x9a,
    0xda, 0xc8, 0xe1, 0xbb, 0xc3, 0x03, 0x2e, 0xcc, 0x18, 0xdc};

// Require the canonical, directly BAT-mapped MEM1 alias. No MMIO, page-table
// translation, cache emulation, guest exceptions or unchecked host pointers.
u8* DirectRAM(Core::System& system, u32 address, u32 length, bool instruction = false)
{
  auto& state = system.GetPPCState();
  auto& memory = system.GetMemory();
  if (!length || !memory.GetRAM() || address < 0x80000000 ||
      address - 0x80000000 >= memory.GetRamSizeReal() ||
      length > memory.GetRamSizeReal() - (address - 0x80000000) ||
      (instruction ? !state.msr.IR : !state.msr.DR))
    return nullptr;
  const auto& bat = instruction ? system.GetMMU().GetIBATTable() : system.GetMMU().GetDBATTable();
  for (u32 page = address >> PowerPC::BAT_INDEX_SHIFT;
       page <= (address + length - 1) >> PowerPC::BAT_INDEX_SHIFT; ++page)
  {
    const u32 expected = (page << PowerPC::BAT_INDEX_SHIFT) - 0x80000000;
    if ((bat[page] & (PowerPC::BAT_MAPPED_BIT | PowerPC::BAT_WI_BIT)) != PowerPC::BAT_MAPPED_BIT ||
        (bat[page] & PowerPC::BAT_RESULT_MASK) != expected)
      return nullptr;
  }
  return memory.GetRAM() + address - 0x80000000;
}
}  // namespace

void JitSimpleFormatter::Init()
{
  *this = {};
  const char* setting = std::getenv("MODERNGEKKO_SIMPLE_FORMAT");
  if (setting && std::string_view(setting) == "shadow")
  {
    m_mode = Mode::Shadow;
    m_pending.reserve(16);
  }
  else if (setting && std::string_view(setting) == "on")
    m_mode = Mode::Replace;
}

bool JitSimpleFormatter::Handles(u32 pc) const
{
  return m_mode != Mode::Disabled && (pc == ENTRY || (m_mode == Mode::Shadow && pc == RETURN));
}

void JitSimpleFormatter::ClearPending()
{
  m_abandoned += m_pending.size();
  m_pending.clear();
}

void JitSimpleFormatter::Finish()
{
  if (m_mode != Mode::Disabled)
  {
    for (const auto& item : m_unsupported_formats)
      if (item.count)
        fmt::print(stderr, "Unsupported formatter: address={:08x} count={}\n", item.address, item.count);
    fmt::print(stderr, "Unsupported formatter overflow: {}\n", m_unsupported_overflow);
  }
  if (m_mode != Mode::Disabled)
    fmt::print(stderr, "Simple formatter: mode={} entries={} eligible={} replaced={} "
                       "compared={} mismatches={} guard_rejects={} unsupported={} "
                       "abandoned={} pending={} floating={} fpscr_mismatches={} "
                       "changed_gpr={:08x} changed_ps0={:08x} changed_ps1={:08x} changed_cr={:02x}\n",
                 m_mode == Mode::Shadow ? "shadow" : "on", m_entries, m_eligible, m_replaced,
                 m_compared, m_mismatches, m_guard_rejects, m_unsupported, m_abandoned, m_pending.size(),
                 m_floating, m_fpscr_mismatches, m_changed_gpr, m_changed_ps0, m_changed_ps1,
                 m_changed_cr);
}

bool JitSimpleFormatter::Invoke(JitSimpleFormatter* self, Core::System* system, u32 pc)
{
  return self->Run(*system, pc);
}

bool JitSimpleFormatter::Run(Core::System& system, u32 pc)
{
  auto& state = system.GetPPCState();
  if (pc == RETURN)
  {
    const auto it = std::find_if(m_pending.begin(), m_pending.end(), [&](const Pending& item) {
      return item.sp == state.gpr[1];
    });
    if (it != m_pending.end())
    {
      const auto* output = DirectRAM(system, it->output, static_cast<u32>(it->expected.size()));
      const auto* arguments = DirectRAM(system, it->arguments, static_cast<u32>(it->argument_state.size()));
      ++m_compared;
      // Aggregate register numbers only, never guest register contents. This
      // exposes state clobbered by the original that the replacement preserves;
      // an output-byte match alone cannot establish caller-state equivalence.
      for (u32 i = 0; i < 32; ++i)
      {
        if (state.gpr[i] != it->gpr[i]) m_changed_gpr |= u32{1} << i;
        if (state.ps[i].PS0AsU64() != it->ps0[i]) m_changed_ps0 |= u32{1} << i;
        if (state.ps[i].PS1AsU64() != it->ps1[i]) m_changed_ps1 |= u32{1} << i;
      }
      for (u32 i = 0; i < 8; ++i)
        if (state.cr.GetField(i) != it->cr[i]) m_changed_cr |= u32{1} << i;
      if (state.fpscr.Hex != it->fpscr)
        ++m_fpscr_mismatches;
      if (!output || !arguments || state.gpr[3] != it->length ||
          std::memcmp(output, it->expected.data(), it->expected.size()) ||
          std::memcmp(arguments, it->argument_state.data(), it->argument_state.size()) ||
          state.fpscr.Hex != it->fpscr)
        ++m_mismatches;
      m_pending.erase(it);
    }
    return false;
  }

  ++m_entries;
  const u32 destination = state.gpr[3];
  const u32 format_address = state.gpr[5];
  const auto* format = DirectRAM(system, format_address, 512);
  auto* output = DirectRAM(system, destination, 1024);
  auto* arguments = DirectRAM(system, state.gpr[6], 12);
  if (LR(state) != RETURN || state.gpr[4] != 1024 ||
      destination < 0x806b3af0 || destination >= 0x806b4af0 ||
      (destination - 0x806b3af0) % 1024 || !format || !output || !arguments ||
      state.gpr[6] != state.gpr[1] + 0x68 ||
      state.m_enable_dcache || system.GetPowerPC().GetMemChecks().HasAny() ||
      (format_address < destination + 1024 && destination < format_address + 512))
  {
    ++m_guard_rejects;
    return false;
  }
  Pending candidate{};
  candidate.fpscr = state.fpscr.Hex;
  std::memcpy(candidate.argument_state.data(), arguments, candidate.argument_state.size());
  std::memcpy(candidate.expected.data(), output, candidate.expected.size());
  auto length = JitCommon::TryLiteralFormat({format, 512}, candidate.expected);
  bool floating = false;
  if (!length && state.fpscr.RN == 0)
  {
    const auto fields = JitCommon::CountFixedFloatFields({format, 512});
    const u32 first = arguments[1];
    const u32 save_area = Common::swap32(arguments + 8);
    if (fields && *fields && first <= 8 && *fields <= 8 - first && !(save_area & 7) &&
        save_area == state.gpr[1] + 8)
    {
      const auto* source = DirectRAM(system, save_area + 32 + first * 8, *fields * 8);
      if (source)
      {
        std::array<double, 8> values{};
        for (u32 i = 0; i < *fields; ++i)
          values[i] = std::bit_cast<double>(Common::swap64(source + i * 8));
        length = JitCommon::TryFixedFloatFormat({format, 512}, {values.data(), *fields}, candidate.expected);
        if (length)
        {
          candidate.argument_state[1] = static_cast<u8>(first + *fields);
          floating = true;
        }
      }
    }
  }
  if (!length)
  {
    const u32 save_area = Common::swap32(arguments + 8);
    u32 general = arguments[0], floating_index = arguments[1];
    bool used_float = false;
    if (!(save_area & 7) && save_area == state.gpr[1] + 8 && general <= 8 && floating_index <= 8)
    {
      length = JitCommon::TryBasicFormat({format, 512}, candidate.expected,
          [&](u8 type, std::span<u8> target) -> std::optional<u32> {
        if (type == 'f')
        {
          if (floating_index == 8 || state.fpscr.RN != 0) return {};
          const auto* source = DirectRAM(system, save_area + 32 + floating_index * 8, 8);
          if (!source) return {};
          const std::array<double, 1> value{std::bit_cast<double>(Common::swap64(source))};
          const std::array<u8, 3> pattern{'%', 'f', 0};
          const auto result = JitCommon::TryFixedFloatFormat(pattern, value, target);
          if (result) { ++floating_index; used_float = true; }
          return result;
        }
        if (general == 8) return {};
        const auto* source = DirectRAM(system, save_area + general * 4, 4);
        if (!source) return {};
        const u32 value = Common::swap32(source);
        ++general;
        if (type == 'd')
        {
          auto* begin = reinterpret_cast<char*>(target.data());
          const auto result = std::to_chars(begin, begin + target.size(), std::bit_cast<s32>(value));
          if (result.ec != std::errc{}) return {};
          return static_cast<u32>(result.ptr - begin);
        }
        const auto* text = DirectRAM(system, value, 512);
        // Original output/va_list writes could otherwise alter an aliased string.
        if (!text || (value < destination + 1024 && destination < value + 512) ||
            (value < state.gpr[6] + 12 && state.gpr[6] < value + 512)) return {};
        const auto* end = static_cast<const u8*>(std::memchr(text, 0, 512));
        if (!end || static_cast<std::size_t>(end - text) > target.size()) return {};
        std::memcpy(target.data(), text, end - text);
        return static_cast<u32>(end - text);
      });
      if (length)
      {
        candidate.argument_state[0] = static_cast<u8>(general);
        candidate.argument_state[1] = static_cast<u8>(floating_index);
        floating = used_float;
      }
    }
  }
  if (!length)
  {
    ++m_unsupported;
    // Bounded numeric census. Keep proprietary format strings out of logs.
    const auto slot = std::find_if(m_unsupported_formats.begin(), m_unsupported_formats.end(),
                                  [&](const auto& item) {
                                    return !item.count || item.address == format_address;
                                  });
    if (slot == m_unsupported_formats.end())
      ++m_unsupported_overflow;
    else
    {
      slot->address = format_address;
      ++slot->count;
    }
    return false;
  }
  const auto* code = DirectRAM(system, CODE_BEGIN, CODE_SIZE, true);
  if (!code || Common::SHA1::CalculateDigest(code, CODE_SIZE) != CODE_HASH)
  {
    ++m_guard_rejects;
    return false;
  }
  ++m_eligible;
  m_floating += floating;
  if (m_mode == Mode::Shadow)
  {
    if (m_pending.size() == 16 || std::ranges::any_of(m_pending, [&](const Pending& item) {
          return item.sp == state.gpr[1];
        }))
    {
      ++m_abandoned;
      return false;
    }
    candidate.sp = state.gpr[1];
    candidate.output = destination;
    candidate.length = *length;
    candidate.arguments = state.gpr[6];
    for (u32 i = 0; i < 32; ++i)
    {
      candidate.gpr[i] = state.gpr[i];
      candidate.ps0[i] = state.ps[i].PS0AsU64();
      candidate.ps1[i] = state.ps[i].PS1AsU64();
    }
    for (u32 i = 0; i < 8; ++i) candidate.cr[i] = state.cr.GetField(i);
    m_pending.push_back(candidate);
    return false;
  }
  // Do the complete supported output operation. The caller still advances its
  // ring, the sink still filters/emits, and unsupported formats execute normally.
  std::memcpy(output, candidate.expected.data(), *length + 1);
  arguments[0] = candidate.argument_state[0];
  arguments[1] = candidate.argument_state[1];
  state.gpr[3] = *length;
  ++m_replaced;
  return true;
}
