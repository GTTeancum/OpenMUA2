// Copyright 2026 OpenMUA2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <thread>
#include <vector>

namespace Common::RuntimeTiming
{
enum class Kind { Throttle, GpuPacing, GpuWorker, GpuFence, GpuSubmit, GpuPresent, Present,
                  JitCompile, ShaderCompile, PipelineCompile, JitAnalyze, JitEmit, JitFinalize, JitInstruction, JitBackpatch, JitEntryMap, JitRanges, JitLinks, GpuDecodeSlow, JitBackpatchRehash };
inline constexpr const char* Names[] = {
    "throttle", "gpu_pacing", "gpu_worker", "gpu_fence", "gpu_submit", "gpu_present", "present",
    "jit_compile", "shader_compile", "pipeline_compile", "jit_analyze", "jit_emit", "jit_finalize", "jit_instruction", "jit_backpatch", "jit_entry_map", "jit_ranges", "jit_links", "gpu_decode_slow", "jit_backpatch_rehash"};
inline constexpr std::int64_t GpuDecodeMinimumNs = 100000;
inline std::int64_t Now()
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count();
}

// Record concurrently into unique preallocated slots. Write only after all
// producer threads have joined. Nested spans overlap and must not be summed.
class Trace
{
public:
  explicit Trace(std::size_t capacity = 262144) : m_rows(capacity) {}
  void Record(Kind kind, std::int64_t begin, std::int64_t end,
              std::int64_t target, std::uint64_t thread, std::int64_t cpu_ns = -1,
              std::int64_t cycles = -1, std::uint32_t address = 0, std::uint32_t instructions = 0)
  {
    const auto index = m_count.fetch_add(1, std::memory_order_relaxed);
    if (index < m_rows.size()) m_rows[index] = {kind, begin, end, target, thread, cpu_ns, cycles, address, instructions};
  }
  void Write(std::ostream& out) const
  {
    const auto count = m_count.load();
    out << "# steady_clock spans; elapsed time includes preemption; nested spans overlap\n"
        << "# gpu_decode_slow_min_ns=" << GpuDecodeMinimumNs << '\n'
        << "# dropped_samples=" << (count > m_rows.size() ? count - m_rows.size() : 0) << '\n'
        << "kind,begin_ns,end_ns,target_ns,thread,cpu_ns,cycles,address,instructions\n";
    for (std::size_t i = 0; i < std::min(count, m_rows.size()); ++i) {
      const auto& r = m_rows[i];
      out << Names[static_cast<int>(r.kind)] << ',' << r.begin << ',' << r.end << ','
          << r.target << ',' << r.thread << ',' << r.cpu_ns << ',' << r.cycles << ','
          << r.address << ',' << r.instructions << '\n';
    }
  }
private:
  struct Row { Kind kind{}; std::int64_t begin{}, end{}, target{}; std::uint64_t thread{};
               std::int64_t cpu_ns{-1}, cycles{-1}; std::uint32_t address{}, instructions{}; };
  std::vector<Row> m_rows;
  std::atomic<std::size_t> m_count{};
};

class Profile
{
public:
  Profile()
  {
    if (const char* path = std::getenv("OPENMUA2_RUNTIME_SPANS")) m_path = path;
    if (!m_path.empty()) trace = std::make_unique<Trace>();
  }
  void Flush()
  {
    if (!trace) return;
    std::ofstream out(m_path);
    trace->Write(out);
  }
  std::unique_ptr<Trace> trace;
private:
  std::string m_path;
};
inline Profile& Get()
{
  // Process lifetime avoids destruction racing any runtime teardown callback.
  static Profile* profile = new Profile;
  return *profile;
}
inline std::int64_t Begin() { return Get().trace ? Now() : 0; }
inline void End(Kind kind, std::int64_t begin, std::int64_t target = 0)
{
  if (!begin) return;
  const auto end = Now();
  static thread_local const auto thread = std::hash<std::thread::id>{}(std::this_thread::get_id());
  Get().trace->Record(kind, begin, end, target, thread);
}
class Scope
{
public:
  explicit Scope(Kind kind, std::int64_t target = 0, std::int64_t minimum_ns = 0)
      : m_kind(kind), m_target(target), m_begin(Begin()), m_minimum_ns(minimum_ns) {}
  ~Scope() {
    if (!m_minimum_ns || (m_begin && Now() - m_begin >= m_minimum_ns))
      End(m_kind, m_begin, m_target);
  }
private:
  Kind m_kind;
  std::int64_t m_target, m_begin, m_minimum_ns;
};
}  // namespace Common::RuntimeTiming
