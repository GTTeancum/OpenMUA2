// Standalone helper regression. No interpreter object, game, MMIO or cache is run.
// The reference below is the unchanged pinned production source, not a model.
#include <bit>
#include <cfenv>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <vector>
#include <xmmintrin.h>
extern "C" {
#include "cpu/cpu.h"
}
#include "Core/PowerPC/Interpreter/Interpreter_FloatMisc.cpp"

namespace test {
constexpr uint64_t SENTINEL = UINT64_C(0x123456789abcdef0);
constexpr uint32_t FI = 0x00020000, FR = 0x00040000, FPRF = 0x0001f000;
constexpr uint32_t XX = 0x02000000, VX = 0x20000000, FEX = 0x40000000;
constexpr uint32_t FX = 0x80000000, VXCVI = 0x100, VXSNAN = 0x01000000;
constexpr int host_modes[] = {FE_TONEAREST, FE_TOWARDZERO, FE_UPWARD, FE_DOWNWARD};
uint64_t seed = UINT64_C(0x91af83476d09ebc2);
uint64_t random64() {
  seed ^= seed >> 12; seed ^= seed << 25; seed ^= seed >> 27;
  return seed * UINT64_C(2685821657736338717);
}
struct Suite {
  std::unique_ptr<PowerPC::PowerPCState> reference = std::make_unique<PowerPC::PowerPCState>();
  CPUState native{};
  uint64_t cases = 0, failed = 0, fpscr_fail = 0, result_fail = 0, write_fail = 0;
  uint64_t fprf_fail = 0, invalid_cases = 0, nan_cases = 0;
  uint64_t exact_cases = 0, inexact_cases = 0, blocked_cases = 0;
  uint64_t invariant_fail = 0, printed = 0;

  void run(uint64_t bits, uint32_t initial, bool toward_zero) {
    const double value = std::bit_cast<double>(bits);
    const unsigned mode = toward_zero ? 1 : initial & 3;
    if (std::fesetround(host_modes[initial & 3]) != 0) std::abort();
    // NI is preserved as a guest control bit. This helper test intentionally has
    // host FTZ/DAZ off; emulated host-control setup is a separate runtime layer.
    _mm_setcsr(_mm_getcsr() & ~0x8040u);
    native.fpscr = initial;
    native.msr = 0; // no guest FE0/FE1 trap delivery in the scalar helper fixture
    ppc_fpscr_updated(&native); // normalize derived VX/FEX in the common input
    initial = native.fpscr;
    reference->msr.Hex = 0;
    reference->fpscr.Hex = initial;
    reference->ps[1].SetPS0(bits);
    reference->ps[2].SetPS0(SENTINEL);
    UGeckoInstruction inst{};
    inst.FB = 1; inst.FD = 2; inst.Rc = 0;
    ConvertToInteger(*reference, inst, static_cast<RoundingMode>(mode));
    uint64_t actual = SENTINEL;
    const bool wrote = ppc_fctiw(&native, value, toward_zero, &actual);
    const uint64_t expected = reference->ps[2].PS0AsU64();
    const bool expected_write = expected != SENTINEL;
    const bool a = native.fpscr != reference->fpscr.Hex;
    const bool b = actual != expected;
    const bool c = wrote != expected_write;
    const bool d = (native.fpscr & FPRF) != (initial & FPRF);
    // Independent invariants for the architecture-defined integer result and flags.
    // These do not borrow the reference's FPSCR output or mask comparison bits.
    double rounded;
    switch (mode) {
      case 1: rounded = std::trunc(value); break;
      case 2: rounded = std::ceil(value); break;
      case 3: rounded = std::floor(value); break;
      default: rounded = std::nearbyint(value); break; // host RN is nearest here
    }
    const bool nan = std::isnan(value);
    const bool invalid = nan || rounded >= 2147483648.0 || rounded < -2147483648.0;
    bool invariant = false;
    if (invalid) {
      ++invalid_cases;
      if (nan) ++nan_cases;
      invariant = (native.fpscr & (FI | FR)) != 0 || !(native.fpscr & VXCVI);
      if (initial & 0x80) ++blocked_cases;
      invariant |= wrote != !(initial & 0x80);
    } else {
      const bool inexact = rounded != value;
      if (inexact) ++inexact_cases; else ++exact_cases;
      const uint32_t flag_bits = inexact ? FI | ((std::fabs(rounded) > std::fabs(value)) ? FR : 0) : 0;
      invariant = (native.fpscr & (FI | FR)) != flag_bits;
      invariant |= !wrote;
      invariant |= inexact && !(native.fpscr & XX);
      const uint64_t boxed = UINT64_C(0xfff8000000000000) |
          static_cast<uint32_t>(static_cast<int32_t>(rounded)) |
          ((rounded == 0.0 && std::signbit(value)) ? UINT64_C(0x100000000) : 0);
      invariant |= actual != boxed;
    }
    ++cases;
    fpscr_fail += a; result_fail += b; write_fail += c; fprf_fail += d;
    invariant_fail += invariant;
    if (a || b || c || d || invariant) {
      ++failed;
      if (printed++ < 12) {
        std::fprintf(stderr,
          "FAIL input=%016llx initial=%08x mode=%u tz=%u native=%08x ref=%08x "
          "out=%016llx refout=%016llx wrote=%u/%u invariant=%u\n",
          static_cast<unsigned long long>(bits), initial, mode, toward_zero,
          native.fpscr, reference->fpscr.Hex,
          static_cast<unsigned long long>(actual), static_cast<unsigned long long>(expected),
          wrote, expected_write, invariant);
      }
    }
  }
};
}

int main() {
  using namespace test;
  std::fenv_t saved_env;
  if (std::fegetenv(&saved_env)) return 2;
  const auto saved_mxcsr = _mm_getcsr();
  std::vector<uint64_t> values;
  for (double v : {0., -0., 0.25, -0.25, 0.5, -0.5, 0.75, -0.75,
      1., -1., 1.5, -1.5, 2., -2., 2.5, -2.5, 3.5, -3.5,
      42.125, -42.125, 65535.875, -65535.875,
      2147483647., 2147483647.25, 2147483647.5, 2147483647.75,
      2147483648., -2147483648., -2147483648.25, -2147483648.5,
      -2147483648.75, -2147483649., 4503599627370496., -4503599627370496.,
      1e300, -1e300, 1e-300, -1e-300}) {
    values.push_back(std::bit_cast<uint64_t>(v));
    values.push_back(std::bit_cast<uint64_t>(std::nextafter(v, INFINITY)));
    values.push_back(std::bit_cast<uint64_t>(std::nextafter(v, -INFINITY)));
  }
  for (uint64_t bits : {UINT64_C(1), UINT64_C(0x8000000000000001),
       UINT64_C(0x000fffffffffffff), UINT64_C(0x800fffffffffffff),
       UINT64_C(0x0010000000000000), UINT64_C(0x8010000000000000),
       UINT64_C(0x7fefffffffffffff), UINT64_C(0xffefffffffffffff),
       UINT64_C(0x7ff0000000000000), UINT64_C(0xfff0000000000000),
       UINT64_C(0x7ff8000000000000), UINT64_C(0xfff8000000000042),
       UINT64_C(0x7ff0000000000001), UINT64_C(0xfff0000000004321)})
    values.push_back(bits);
  Suite suite;
  // Every FPRF value, both pre-existing flag bits, all guest rounding modes,
  // both instructions and enabled/disabled invalid exception result writes.
  const uint32_t sticky[] = {0u, 0x82000000u, 0x81000100u, 0x12000008u};
  for (auto bits : values)
    for (unsigned fprf = 0; fprf < 32; ++fprf)
      for (unsigned fifr = 0; fifr < 4; ++fifr)
        for (unsigned rn = 0; rn < 4; ++rn)
          for (unsigned ve = 0; ve < 2; ++ve)
            for (unsigned tz = 0; tz < 2; ++tz)
              for (auto prior : sticky)
                suite.run(bits, prior | (fprf << 12) | (fifr << 17) | rn | (ve << 7) | 4, tz);
  const uint64_t matrix_cases = suite.cases;
  // Reproducible random IEEE-754 bit patterns, not just decimal samples.
  for (unsigned i = 0; i < 250000; ++i) {
    auto bits = random64();
    uint32_t initial = static_cast<uint32_t>(random64());
    // Keep reserved bits clear; deliberately vary exception enables/stickies.
    initial &= 0xfffff7ffu;
    suite.run(bits, initial, (random64() & 1) != 0);
  }
  if (std::fesetenv(&saved_env)) return 2;
  _mm_setcsr(saved_mxcsr);
  std::printf("{\"cases\":%llu,\"matrix_cases\":%llu,\"random_cases\":250000,"
      "\"failures\":%llu,\"fpscr_mismatches\":%llu,\"result_mismatches\":%llu,"
      "\"write_mismatches\":%llu,\"fprf_preservation_failures\":%llu,"
      "\"independent_invariant_failures\":%llu,\"invalid_cases\":%llu,"
      "\"nan_cases\":%llu,\"exact_cases\":%llu,\"inexact_cases\":%llu,"
      "\"blocked_invalid_writes\":%llu,\"seed\":\"91af83476d09ebc2\","
      "\"host_fp_traps\":false,\"host_ftz_daz\":false,\"gameplay_tested\":false}\n",
      (unsigned long long)suite.cases, (unsigned long long)matrix_cases,
      (unsigned long long)suite.failed, (unsigned long long)suite.fpscr_fail,
      (unsigned long long)suite.result_fail, (unsigned long long)suite.write_fail,
      (unsigned long long)suite.fprf_fail, (unsigned long long)suite.invariant_fail,
      (unsigned long long)suite.invalid_cases, (unsigned long long)suite.nan_cases,
      (unsigned long long)suite.exact_cases, (unsigned long long)suite.inexact_cases,
      (unsigned long long)suite.blocked_cases);
  return suite.failed ? 1 : 0;
}
