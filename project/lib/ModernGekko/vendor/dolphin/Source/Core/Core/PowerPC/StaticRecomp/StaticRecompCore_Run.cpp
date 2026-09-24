// RecompCore: StaticRecomp CPU core - Main execution loop.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Core/PowerPC/StaticRecomp/StaticRecompCore.h"
#include "Core/System.h"
#include "Core/PowerPC/PowerPC.h"
#include "Core/PowerPC/Interpreter/Interpreter.h"
#include "Core/PowerPC/StaticRecomp/StaticRecompLockstep.h"
#include "Core/CoreTiming.h"
#include "Core/HW/CPU.h"
#include "Core/Config/MainSettings.h"
#include "Core/Config/ConfigManager.h"
#include "Core/HW/SystemTimers.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <vector>

namespace
{
constexpr u32 SYNC_EXCEPTION_MASK = ~static_cast<u32>(
    EXCEPTION_EXTERNAL_INT | EXCEPTION_DECREMENTER | EXCEPTION_PERFORMANCE_MONITOR);

struct FileCloser
{
  void operator()(std::FILE* file) const
  {
    if (file)
      std::fclose(file);
  }
};

using FilePtr = std::unique_ptr<std::FILE, FileCloser>;

FilePtr OpenDispatchTrace()
{
  const char* path = std::getenv("STATICRECOMP_TRACE_FILE");
  if (!path || !*path)
    return {};

  FilePtr file(std::fopen(path, "w"));
  if (file)
  {
    std::fprintf(file.get(), "dispatch,pc,lr,ctr,cr,timebase,ppc_downcount\n");
    std::fflush(file.get());
  }
  return file;
}

class DispatchProfiler
{
public:
  DispatchProfiler() : m_enabled(std::getenv("STATICRECOMP_PROFILE_DISPATCH") != nullptr) {}

  bool Enabled() const { return m_enabled; }

  void Record(u32 pc, std::chrono::steady_clock::duration elapsed)
  {
    if (!m_enabled)
      return;
    auto& sample = m_samples[pc];
    ++sample.count;
    sample.nanos +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
  }

  void Print() const
  {
    if (!m_enabled)
      return;

    std::vector<std::pair<u32, Sample>> sorted(m_samples.begin(), m_samples.end());
    std::sort(sorted.begin(), sorted.end(), [](const auto& lhs, const auto& rhs) {
      return lhs.second.nanos > rhs.second.nanos;
    });
    const std::size_t limit = std::min<std::size_t>(sorted.size(), 24);
    for (std::size_t i = 0; i < limit; ++i)
    {
      const auto& [pc, sample] = sorted[i];
      std::fprintf(stderr,
                   "[staticrecomp] dispatch-time pc=%08x count=%llu wall_ms=%.3f avg_ns=%.1f\n",
                   pc, static_cast<unsigned long long>(sample.count),
                   static_cast<double>(sample.nanos) / 1000000.0,
                   sample.count == 0 ? 0.0 :
                                       static_cast<double>(sample.nanos) /
                                           static_cast<double>(sample.count));
    }
  }

private:
  struct Sample
  {
    u64 count = 0;
    u64 nanos = 0;
  };

  bool m_enabled = false;
  std::unordered_map<u32, Sample> m_samples;
};
}

void StaticRecompCore::Run()
{
  auto& core_timing = m_system.GetCoreTiming();
  auto& power_pc = m_system.GetPowerPC();
  auto& ppc = power_pc.GetPPCState();
  auto& interpreter = m_system.GetInterpreter();
  auto& memory = m_system.GetMemory();
  const CPU::State* state_ptr = m_system.GetCPU().GetStatePtr();
  FilePtr dispatch_trace = OpenDispatchTrace();
  DispatchProfiler dispatch_profiler;

  m_guest.ram = memory.GetRAM();
  m_guest.ram_size = memory.GetRamSizeReal();
  m_guest.exram = memory.GetEXRAM();
  m_guest.exram_size = memory.GetExRamSizeReal();
  InitLookupTable(m_guest.ram_size, m_guest.exram_size);
  const bool lockstep_enabled = m_lockstep_verifier->IsEnabled();
  const auto fast_dispatchable_at = [this](u32 address, u32* chunk_index,
                                             u32* linked_address, u32* rel_section_index,
                                             u32 rel_section_hint) {
    if (m_has_rel_modules || !m_forced_fallback_ranges.empty())
      return FastDispatchableAt(address, chunk_index, linked_address, rel_section_index,
                                rel_section_hint);
    if (!m_module_active || m_chunk_lookup_table.empty())
      return false;

    int lookup_index = -1;
    if (address >= 0x80000000u && address < 0x80000000u + m_lookup_ram_size)
    {
      lookup_index = static_cast<int>((address - 0x80000000u) >> 2);
    }
    else if (address >= 0x90000000u && address < 0x90000000u + m_lookup_exram_size)
    {
      lookup_index = static_cast<int>((m_lookup_ram_size >> 2) + ((address - 0x90000000u) >> 2));
    }
    if (lookup_index < 0 || lookup_index >= static_cast<int>(m_chunk_lookup_table.size()))
      return false;
    const int chunk = m_chunk_lookup_table[lookup_index];
    if (chunk < 0 || m_chunk_state[chunk] != CHUNK_VERIFIED)
      return false;
    if (chunk_index)
      *chunk_index = static_cast<u32>(chunk);
    if (linked_address)
      *linked_address = address;
    if (rel_section_index)
      *rel_section_index = 0xffffffffu;
    return true;
  };
  const auto chunk_contains_host_call = [this](u32 chunk_index) {
    if (!m_module_source.host_call_contains || chunk_index >= m_chunk_host_call_state.size())
      return false;
    const u8 state = m_chunk_host_call_state[chunk_index];
    return state == 0 ? ChunkContainsHostCall(chunk_index) : state == 2;
  };
  const auto host_call_at = [this, &chunk_contains_host_call](u32 address, u32 chunk_index) {
    return m_guest.host_call && chunk_contains_host_call(chunk_index) &&
           IsHostCallAddress(address);
  };
  const auto fast_native_continue = [&](u32 address, u32* linked_address,
                                          u32* rel_section_index) {
    u32 chunk_index = 0;
    const u32 rel_section_hint = rel_section_index ? *rel_section_index : 0xffffffffu;
    return fast_dispatchable_at(address, &chunk_index, linked_address, rel_section_index,
                                rel_section_hint) &&
           !host_call_at(address, chunk_index);
  };

  const std::string initial_game_id = SConfig::GetInstance().GetGameID();
  const auto after_mtmsr = [this](u32 pc) {
    if (pc < 4u || (pc & 3u) != 0)
      return false;

    const u32 instruction_pc = pc - 4u;
    const u8* code = nullptr;
    if (instruction_pc >= 0x80000000u &&
        instruction_pc - 0x80000000u + 4u <= m_guest.ram_size)
    {
      code = m_guest.ram + instruction_pc - 0x80000000u;
    }
    else if (instruction_pc >= 0x90000000u &&
             instruction_pc - 0x90000000u + 4u <= m_guest.exram_size)
    {
      code = m_guest.exram + instruction_pc - 0x90000000u;
    }
    if (!code)
      return false;

    const u32 raw = static_cast<u32>(code[0]) << 24 | static_cast<u32>(code[1]) << 16 |
                    static_cast<u32>(code[2]) << 8 | code[3];
    return (raw & 0xFC0007FEu) == 0x7C000124u;
  };
  const auto record_jit_fallback = [&]() {
    ++m_jit_fallback_runs;
    if (m_collect_fallback_samples)
      IncrementSample(m_jit_fallback_pc_samples, ppc.pc);
    if (m_jit_fallback_sample_count >= m_jit_fallback_samples.size())
      return;
    auto& sample = m_jit_fallback_samples[m_jit_fallback_sample_count++];
    sample.run = m_jit_fallback_runs;
    sample.pc = ppc.pc;
    sample.lr = ppc.spr[SPR_LR];
    sample.ctr = ppc.spr[SPR_CTR];
    sample.cr = ppc.cr.Get();
    sample.exceptions = ppc.Exceptions;
    sample.downcount = ppc.downcount;
  };
  m_module_active = m_module && (initial_game_id.empty() || initial_game_id == m_module->game_id);

  if (!m_module_active && m_fallback_jit && !m_guest.host_call)
  {
    record_jit_fallback();
    m_fallback_jit->Run();
    return;
  }

  while (*state_ptr == CPU::State::Running)
  {
    core_timing.Advance();
    const std::string current_game_id = SConfig::GetInstance().GetGameID();
    m_module_active = m_module && (current_game_id.empty() || current_game_id == m_module->game_id);

    do
    {
      // MSR.FP needs no gate here: generated FPU instructions raise the
      // FP-unavailable exception themselves (ppc_fp_available).
      u32 entry_chunk_index = 0;
      u32 linked_dispatch_address = ppc.pc;
      u32 dispatch_rel_section_index = 0xffffffffu;
      if (m_module_active &&
          DispatchableAt(ppc.pc, &entry_chunk_index, &linked_dispatch_address,
                         &dispatch_rel_section_index) &&
          !host_call_at(ppc.pc, entry_chunk_index))
      {
        SyncIn();
        ++m_bursts;
        do
        {
          if (dispatch_trace && (m_native_dispatches & 0xFFFFFu) == 0)
          {
            std::fprintf(dispatch_trace.get(), "%llu,%08x,%08x,%08x,%08x,%llu,%d\n",
                         static_cast<unsigned long long>(m_native_dispatches), m_guest.pc,
                         m_guest.lr, m_guest.ctr, m_guest.cr,
                         static_cast<unsigned long long>(m_guest.timebase), ppc.downcount);
            std::fflush(dispatch_trace.get());
          }
          const bool do_ls = lockstep_enabled && m_lockstep_verifier->ShouldCheck(m_guest.pc);
          if (do_ls)
          {
            m_lockstep_verifier->Prepare(m_guest);
          }

          if (m_collect_dispatch_samples && (m_native_dispatches & 4095u) == 0)
            ++m_dispatch_samples[m_guest.pc];
          const u32 runtime_dispatch_address = m_guest.pc;
          m_guest.pc = linked_dispatch_address;
          const auto dispatch_start = dispatch_profiler.Enabled() ?
                                          std::chrono::steady_clock::now() :
                                          std::chrono::steady_clock::time_point{};
          m_module->dispatch(&m_guest, linked_dispatch_address);
          if (dispatch_profiler.Enabled())
          {
            dispatch_profiler.Record(runtime_dispatch_address,
                                     std::chrono::steady_clock::now() - dispatch_start);
          }
          if (m_has_rel_modules)
            m_guest.pc = TranslateRelAddress(m_guest.pc, dispatch_rel_section_index);
          if (m_collect_dispatch_samples)
          {
            auto& trace = m_dispatch_trace_samples[m_dispatch_trace_next];
            trace.dispatch = m_native_dispatches;
            trace.runtime_pc = runtime_dispatch_address;
            trace.linked_pc = linked_dispatch_address;
            trace.result_pc = m_guest.pc;
            trace.lr = m_guest.lr;
            trace.ctr = m_guest.ctr;
            trace.cr = m_guest.cr;
            trace.r3 = m_guest.gpr[3];
            trace.r4 = m_guest.gpr[4];
            trace.exception = m_guest.exception;
            trace.downcount = m_guest.downcount;
            m_dispatch_trace_next =
                (m_dispatch_trace_next + 1) % static_cast<u32>(m_dispatch_trace_samples.size());
            if (m_dispatch_trace_count < m_dispatch_trace_samples.size())
              ++m_dispatch_trace_count;
          }
          ++m_native_dispatches;

          if (do_ls)
          {
            m_lockstep_verifier->Verify(m_guest);
          }

          // Flush the module's per-block cycle charges into Dolphin's
          // downcount. A dispatch that charged nothing (PC-switch default,
          // pure embedded data) still costs 1 so the burst always makes
          // downcount progress; this per-dispatch flush is also the
          // dispatcher back-edge timing check — CoreTiming regains control
          // with at least CachedInterpreter's per-block frequency, so
          // external-interrupt latency matches stock.
          const s64 charge = -m_guest.downcount;
          m_guest.downcount = 0;
          const u64 effective_charge = static_cast<u64>(charge > 0 ? charge : 1);
          ppc.downcount -= static_cast<int>(effective_charge);
          m_charged_cycles += effective_charge;
          const u64 total_cycles = m_timebase_cycle_remainder + effective_charge;
          m_guest.timebase += total_cycles / SystemTimers::TIMER_RATIO;
          m_timebase_cycle_remainder = total_cycles % SystemTimers::TIMER_RATIO;

          // Idle loop skipping for configured target loops (e.g. Wii Menu OSIdleThread)
          if (m_guest.pc == m_idle_pc && m_idle_pc != 0)
          {
            m_system.GetCoreTiming().Idle();
          }

          // ctx->timebase is refreshed at burst start (SyncIn), and here we
          // incrementally advance it by the exact block cycle charges to
          // prevent guest busy-wait loops from spinning on a stale timebase.
          if (m_guest.exception)
          {
            // DolRecomp's runtime already redirected pc/msr/srr to the guest
            // exception vector; the flag only signals that it happened.
            if (m_native_exception_sample_count < m_native_exception_samples.size())
            {
              auto& sample = m_native_exception_samples[m_native_exception_sample_count++];
              sample.pc = m_guest.pc;
              sample.lr = m_guest.lr;
              sample.srr0 = m_guest.srr0;
              sample.srr1 = m_guest.srr1;
              sample.msr = m_guest.msr;
              sample.program_exception = m_guest.program_exception;
            }
            m_guest.exception = 0;
            m_guest.program_exception = 0;
            ++m_native_exceptions;
          }
          if ((ppc.Exceptions & SYNC_EXCEPTION_MASK) != 0)
            break;  // Hook-raised synchronous exception: deliver via Dolphin below.
          if ((ppc.Exceptions & EXCEPTION_EXTERNAL_INT) != 0 &&
              (m_guest.msr & 0x8000u) != 0 && after_mtmsr(m_guest.pc))
            break;
        } while (fast_native_continue(m_guest.pc, &linked_dispatch_address,
                                      &dispatch_rel_section_index) &&
                 ppc.downcount > 0 && *state_ptr == CPU::State::Running);
        SyncOut();
        if ((ppc.Exceptions & SYNC_EXCEPTION_MASK) != 0)
          power_pc.CheckExceptions();
        else if ((ppc.Exceptions & EXCEPTION_EXTERNAL_INT) != 0 && ppc.msr.EE &&
                 after_mtmsr(ppc.pc))
          power_pc.CheckExternalExceptions();
      }
      else
      {
        if (m_guest.host_call && IsHostCallAddress(ppc.pc))
        {
          SyncIn();
          bool handled = m_guest.host_call(&m_guest, m_guest.pc);
          if (!handled && m_guest.pc < m_guest.ram_size)
            handled = m_guest.host_call(&m_guest, m_guest.pc | 0x80000000u);
          if (m_fallback_jit && IsHostCallAddress(m_guest.lr))
            m_fallback_jit->GetBlockCache()->InvalidateICache(m_guest.lr, 4, true);
          if (handled)
          {
            const s64 charge = -m_guest.downcount;
            m_guest.downcount = 0;
            const u64 effective_charge = static_cast<u64>(charge > 0 ? charge : 1);
            ppc.downcount -= static_cast<int>(effective_charge);
            AdvanceGuestTimebase(effective_charge);
            SyncOut();
            continue;
          }
          SyncOut();
          if (m_fallback_jit)
          {
            m_host_call_passthrough_pc = ppc.pc;
            m_host_call_passthrough = true;
          }
        }
        // SingleStepInner delivers synchronous exceptions itself; external
        // interrupts are delivered at slice start, as in Interpreter::Run.
        if (m_module_active && IsForcedFallbackAddress(ppc.pc))
        {
          ppc.downcount -= interpreter.SingleStepInner();
          ++m_fallback_steps;
        }
        else if (m_fallback_jit)
        {
          record_jit_fallback();
          m_fallback_jit->Run();
        }
        else
        {
          do
          {
            ppc.downcount -= interpreter.SingleStepInner();
            ++m_fallback_steps;
          } while (!(m_module_active && DispatchableAt(ppc.pc)) &&
                   !IsHostCallAddress(ppc.pc) && ppc.downcount > 0 &&
                   *state_ptr == CPU::State::Running);
        }
      }
    } while (ppc.downcount > 0 && *state_ptr == CPU::State::Running);
  }
  dispatch_profiler.Print();
}

void StaticRecompCore::SingleStep()
{
  // Debugger stepping runs through the interpreter; state outside Run() lives
  // in PowerPCState, so no sync is needed.
  auto& system = m_system;
  system.GetCoreTiming().Advance();
  system.GetPPCState().downcount -= system.GetInterpreter().SingleStepInner();
}
