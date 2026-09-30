// Copyright 2026 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>
#include <cstddef>

#include "Common/CommonTypes.h"

// Bounded diagnostic samples. Capture arguments only on the first occurrence
// of each LR; they are examples, not the arguments of every invocation.
struct JitCallerProfile
{
  struct Sample
  {
    u32 lr = 0;
    u64 count = 0;
    std::array<u32, 8> gpr{};
    std::array<u32, 8> stack_lr{};
  };

  static constexpr std::size_t CAPACITY = 64;
  std::array<Sample, CAPACITY> samples{};
  std::size_t size = 0;
  u64 overflow = 0;

  template <typename Capture>
  void Record(u32 lr, Capture capture)
  {
    for (std::size_t i = 0; i < size; ++i)
    {
      if (samples[i].lr == lr)
      {
        ++samples[i].count;
        return;
      }
    }
    if (size == CAPACITY)
    {
      ++overflow;
      return;
    }
    auto& sample = samples[size++];
    sample.lr = lr;
    sample.count = 1;
    capture(sample);
  }
};
