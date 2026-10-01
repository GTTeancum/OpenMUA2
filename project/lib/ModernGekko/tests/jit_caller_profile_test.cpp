#include "Core/PowerPC/JitCommon/JitIndirectProfile.h"
#include "Core/PowerPC/JitCommon/JitCallerProfile.h"

int main()
{
  JitCallerProfile profile;
  unsigned captures = 0;
  const auto capture = [&](JitCallerProfile::Sample& sample) {
    sample.gpr[0] = ++captures;
    sample.stack_lr[0] = 0x80001000;
  };
  for (u32 i = 0; i < JitCallerProfile::CAPACITY; ++i)
    profile.Record(0x80000000 + i * 4, capture);
  profile.Record(0x80000000, capture);
  profile.Record(0x80000100, capture);
  profile.Record(0x80000100, capture);
  if (profile.size != 64 || profile.overflow != 2 || captures != 64 ||
      profile.samples[0].count != 2 || profile.samples[0].gpr[0] != 1 ||
      profile.samples[0].stack_lr[0] != 0x80001000)
    return 1;
  u64 accounted = profile.overflow;
  for (std::size_t i = 0; i < profile.size; ++i)
    accounted += profile.samples[i].count;
  if (accounted != 67)
    return 2;
  profile = {};
  profile.Record(0x80000100, capture);
  if (profile.size != 1 || profile.overflow != 0 || profile.samples[0].count != 1 ||
      profile.samples[0].gpr[0] != 65)
    return 3;
  for (const auto text : {"", "0", "00000000", "8036f9b5", "0x8036f9b4", "8036f9b4junk", "100000000"})
    if (JitCommon::ParseIndirectProfileAddress(text)) return 4;
  if (JitCommon::ParseIndirectProfileAddress("8036f9b4") != 0x8036f9b4u) return 5;
  JitCommon::IndirectTargetCounts targets;
  for (u32 i = 0; i < 64; ++i) targets.Record(0x80000000 + (i / 2) * 4, i % 2);
  targets.Record(0x80000000, 0);
  targets.Record(0x80000000, 1);
  targets.Record(0x90000000, 0);
  if (targets.size != 64 || targets.total != 67 || targets.overflow != 1 ||
      targets.entries[0].count != 2 || targets.entries[1].count != 2) return 6;
  u64 sum = targets.overflow;
  for (const auto& e : targets.entries) sum += e.count;
  if (sum != targets.total) return 7;
  for (const auto text : {"", "80000000", "80000000:", "80000001:80001000",
                         "80000000:80001000,", "80000000:80001000,80001000",
                         "80000000:80001000,80002000,80003000,80004000",
                         "80000000:80001000:80002000", "80000000:0"})
    if (JitCommon::ParseIndirectHints(text)) return 8;
  const auto hints = JitCommon::ParseIndirectHints("80000000:80001000,80002000,80003000");
  if (!hints || hints->origin != 0x80000000 || hints->size != 3 ||
      hints->targets[0] != 0x80001000 || hints->targets[2] != 0x80003000) return 9;
  const auto single = JitCommon::ParseIndirectHints("80000000:80001000");
  if (!single || single->size != 1 || single->targets[0] != 0x80001000) return 10;
  return 0;
}
