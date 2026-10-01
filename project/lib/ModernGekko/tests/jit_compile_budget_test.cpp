// Copyright 2026 OpenMUA2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#include "Core/PowerPC/JitCommon/JitCompileBudget.h"

#include <cstdlib>
#include <iostream>
#include <limits>

static void Require(bool condition, const char* label)
{
  if (!condition)
  {
    std::cerr << label << '\n';
    std::exit(1);
  }
}

int main()
{
  for (const char* text : {"", "0", "1", "249", "16001", "-4000", "+4000",
                           "4000x", " 4000", "4000 ", "4294967296"})
    Require(JitCompileBudget::ParseNanoseconds(text) == 0, "invalid limit enabled budgeting");
  Require(JitCompileBudget::ParseNanoseconds("250") == 250000, "minimum limit");
  Require(JitCompileBudget::ParseNanoseconds("4000") == 4000000, "microsecond conversion");
  Require(JitCompileBudget::ParseNanoseconds("16000") == 16000000, "maximum limit");

  JitCompileBudget budget;
  budget.Observe(0, 729000000);
  budget.Charge(3999999);
  Require(!budget.Exhausted(4000000), "premature exhaustion");
  budget.Charge(1);
  Require(budget.Exhausted(4000000), "inclusive budget boundary");
  Require(!budget.Exhausted(0), "disabled budget");
  budget.Observe(24299999, 729000000);
  Require(budget.Exhausted(4000000), "same window lost its charge");
  budget.Observe(24300000, 729000000);
  Require(!budget.Exhausted(4000000), "window rollover did not replenish");
  budget.Charge(4000000);
  budget.Observe(0, 729000000);
  Require(!budget.Exhausted(4000000), "clock rollback did not replenish");
  budget.Charge(std::numeric_limits<std::uint64_t>::max());
  budget.Charge(1);
  Require(budget.Exhausted(std::numeric_limits<std::uint64_t>::max()), "charge overflow");
  budget.Reset();
  Require(!budget.Exhausted(1), "cache reset retained a charge");
  budget.Observe(0, 0);
  budget.Charge(4000000);
  budget.Observe(1, 0);
  Require(budget.Exhausted(4000000), "zero clock frequency must remain bounded");

  JitColdCodeVisits visits;
  for (const std::uint32_t pc : {0u, 0x80000000u, 0x80000004u, 0xfffffffcu})
  {
    visits.Clear();
    Require(!visits.Revisited(pc, 0), "first visit promoted");
    Require(visits.Revisited(pc, 0), "second visit not promoted");
    Require(!visits.Revisited(pc, 1), "different CPU features conflated");
    Require(visits.Revisited(pc, 1), "second feature-specific visit not promoted");
    visits.Clear();
    Require(!visits.Revisited(pc, 1), "cache reset retained a hotness hint");
  }
  visits.Clear();
  Require(!visits.Revisited(0, 1), "first collision key");
  Require(!visits.Revisited(0x4010, 1), "colliding address must not be a hit");
  Require(!visits.Revisited(0, 1), "evicted address must not be a hit");
  Require(visits.Revisited(0, 1), "replacement key must retain its hint");
  return 0;
}
