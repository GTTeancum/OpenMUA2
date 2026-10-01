#pragma once
#include <cstdint>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace moderngekko::telemetry {
// Diagnostic cumulative counters. Windows CPU time is quantized; cycle counts
// must not be converted to elapsed time using a nominal processor frequency.
struct FrameCpuCounters {
  std::int64_t thread_ns = -1, process_ns = -1, thread_cycles = -1, thread_id = -1;
  static FrameCpuCounters Read() {
    FrameCpuCounters result;
#ifdef _WIN32
    FILETIME created{}, exited{}, kernel{}, user{};
    const auto ns = [](FILETIME k, FILETIME u) {
      const auto ticks = [](FILETIME x) {
        return (std::uint64_t(x.dwHighDateTime) << 32) | x.dwLowDateTime;
      };
      return static_cast<std::int64_t>((ticks(k) + ticks(u)) * 100);
    };
    if (GetThreadTimes(GetCurrentThread(), &created, &exited, &kernel, &user))
      result.thread_ns = ns(kernel, user);
    if (GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user))
      result.process_ns = ns(kernel, user);
    ULONG64 cycles = 0;
    if (QueryThreadCycleTime(GetCurrentThread(), &cycles))
      result.thread_cycles = static_cast<std::int64_t>(cycles);
    result.thread_id = GetCurrentThreadId();
#endif
    return result;
  }
};
}
