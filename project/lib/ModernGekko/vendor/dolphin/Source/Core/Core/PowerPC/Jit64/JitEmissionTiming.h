// Copyright 2026 OpenMUA2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "Common/RuntimeTiming.h"
#include <charconv>
#include <string_view>
#ifdef _WIN32
#include <windows.h>
#endif

// Diagnostic only. Windows CPU accounting is quantized; cycle counts must not
// be converted to time using a nominal clock. Neither field includes off-CPU waits.
class JitEmissionTiming
{
  struct Counters
  {
    std::int64_t cpu_ns = -1, cycles = -1;
    static Counters Read()
    {
      Counters result;
#ifdef _WIN32
      FILETIME created{}, exited{}, kernel{}, user{};
      const auto thread = GetCurrentThread();
      if (GetThreadTimes(thread, &created, &exited, &kernel, &user))
      {
        const auto ticks = [](FILETIME value) {
          return (std::uint64_t(value.dwHighDateTime) << 32) | value.dwLowDateTime;
        };
        result.cpu_ns = static_cast<std::int64_t>((ticks(kernel) + ticks(user)) * 100);
      }
      ULONG64 cycles = 0;
      if (QueryThreadCycleTime(thread, &cycles)) result.cycles = static_cast<std::int64_t>(cycles);
#endif
      return result;
    }
  };
public:
  JitEmissionTiming(std::uint32_t address, std::uint32_t instructions,
                    Common::RuntimeTiming::Kind kind = Common::RuntimeTiming::Kind::JitEmit)
      : m_kind(kind), m_address(address), m_instructions(instructions), m_begin(Common::RuntimeTiming::Begin())
  {
    if (m_begin && m_kind != Common::RuntimeTiming::Kind::JitInstruction) m_start = Counters::Read();
  }
  bool TraceInstructions() const
  {
    if (!m_begin) return false;
    static const std::uint32_t target = [] {
      const char* env = std::getenv("OPENMUA2_JIT_EMISSION_ADDRESS");
      std::uint32_t value = 0;
      if (!env) return value;
      const std::string_view text(env);
      const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value, 16);
      return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size() ? value : 0u;
    }();
    return target != 0 && (m_address == target || target == UINT32_MAX);
  }
  ~JitEmissionTiming()
  {
    if (!m_begin) return;
    const bool instruction = m_kind == Common::RuntimeTiming::Kind::JitInstruction;
    const auto finish = instruction ? Counters{} : Counters::Read();
    const auto end = Common::RuntimeTiming::Now();
    // Retain only slow instruction emissions; avoid OS counter queries per instruction.
    if (instruction && end - m_begin < 100000) return;
    const auto difference = [](std::int64_t before, std::int64_t after) {
      return before >= 0 && after >= before ? after - before : std::int64_t{-1};
    };
    static thread_local const auto thread = std::hash<std::thread::id>{}(std::this_thread::get_id());
    Common::RuntimeTiming::Get().trace->Record(m_kind,
        m_begin, end, 0, thread, difference(m_start.cpu_ns, finish.cpu_ns),
        difference(m_start.cycles, finish.cycles), m_address, m_instructions);
  }
private:
  Common::RuntimeTiming::Kind m_kind;
  std::uint32_t m_address, m_instructions;
  std::int64_t m_begin;
  Counters m_start;
};
