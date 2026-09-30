#pragma once

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace moderngekko::frontend
{
struct SingleProcessorAffinity
{
  DWORD_PTR mask = 0;
  DWORD error = ERROR_SUCCESS;
};

// All threads in this process share one logical processor, including workers
// created later. No global setting or other process is changed. A single logical
// processor also excludes using the second SMT thread on the same physical core.
inline SingleProcessorAffinity PinProcessToOneProcessor()
{
  const HANDLE process = GetCurrentProcess();
  DWORD_PTR allowed = 0;
  DWORD_PTR system = 0;
  if (!GetProcessAffinityMask(process, &allowed, &system))
    return {0, GetLastError()};
  if (!allowed)
    return {0, ERROR_NOT_SUPPORTED};
  const DWORD_PTR selected = allowed & (~allowed + 1);
  if (!SetProcessAffinityMask(process, selected))
    return {0, GetLastError()};
  DWORD_PTR applied = 0;
  if (!GetProcessAffinityMask(process, &applied, &system))
    return {0, GetLastError()};
  if (applied != selected)
    return {0, ERROR_INVALID_DATA};
  return {applied, ERROR_SUCCESS};
}
}  // namespace moderngekko::frontend
#endif
