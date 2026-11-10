// SPDX-License-Identifier: GPL-3.0-or-later
#include "memory_census.hpp"

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

namespace {
int g_failures = 0;
void Check(bool ok, const char* what) {
  if (!ok) {
    std::fprintf(stderr, "FAIL: %s\n", what);
    ++g_failures;
  }
}
}  // namespace

int main() {
  using moderngekko::telemetry::MemoryCensus;
  constexpr std::size_t P = MemoryCensus::kPageSize;

  std::vector<std::uint8_t> mem1(8 * P, 0);
  std::vector<std::uint8_t> mem2(4 * P + 100, 0);  // partial final page

  MemoryCensus census(2);  // capacity 2 -> third sample is dropped from the log

  // Sample 1: one page of MEM1 loaded; nothing counts as changed yet.
  mem1[0 * P + 10] = 1;
  census.Sample(1, 0, mem1, mem2);
  auto c1 = census.Latest(0);
  Check(c1.pages == 8, "mem1 pages");
  Check(c1.nonzero == 1 && c1.ever_nonzero == 1, "first sample nonzero");
  Check(c1.changed == 0 && c1.ever_changed == 0, "first sample has no changes");
  Check(census.Latest(1).pages == 5, "mem2 partial page counted");

  // Sample 2: page 0 static, page 3 written, page 4 in MEM2 written (partial page).
  mem1[3 * P] = 7;
  mem2[4 * P + 50] = 9;
  census.Sample(2, 16, mem1, mem2);
  auto c2 = census.Latest(0);
  Check(c2.nonzero == 2 && c2.ever_nonzero == 2, "second sample nonzero");
  Check(c2.changed == 1 && c2.ever_changed == 1, "only page 3 changed");
  auto m2 = census.Latest(1);
  Check(m2.nonzero == 1 && m2.changed == 1, "mem2 partial page change detected");

  // Sample 3 (dropped from the log but still tracked): page 3 cleared back to zero.
  mem1[3 * P] = 0;
  census.Sample(3, 32, mem1, mem2);
  Check(census.SampleCount() == 3, "sample count includes dropped sample");

  std::ostringstream out;
  census.Write(out);
  const std::string text = out.str();
  Check(text.find("dropped_samples=1") != std::string::npos, "dropped reported");
  Check(text.find("# peak mem1: pages=8 nonzero=2 (8 KiB) ever_nonzero=2 (8 KiB) "
                  "changed=1 (4 KiB) ever_changed=1 (4 KiB)") != std::string::npos,
        "mem1 peak line");
  Check(text.find("# mem1_mib_ever_nonzero_pages(of 256): 2\n") != std::string::npos,
        "mem1 per-MiB histogram");
  Check(text.find("2,16,8,2,2,1,1,5,1,1,1,1\n") != std::string::npos, "csv row 2");

  // Same-content rewrite is not a change; different content of equal "shape" is.
  MemoryCensus hashes;
  std::vector<std::uint8_t> a(P, 0);
  a[0] = 1;
  hashes.Sample(0, 0, a, {});
  hashes.Sample(1, 0, a, {});
  Check(hashes.Latest(0).changed == 0, "identical content unchanged");
  a[0] = 0;
  a[1] = 1;
  hashes.Sample(2, 0, a, {});
  Check(hashes.Latest(0).changed == 1, "moved byte detected as change");

  if (g_failures == 0)
    std::puts("memory_census_test: OK");
  return g_failures == 0 ? 0 : 1;
}
