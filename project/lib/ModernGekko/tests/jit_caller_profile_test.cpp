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
  return 0;
}
