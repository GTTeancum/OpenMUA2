#pragma once
#include <cstdint>
#include <span>

namespace moderngekko::controls {
// CInput's common post-evaluation bookkeeping. Queued actions are one-shot;
// the release mask survives while an action remains held. Layout is guarded by
// executable and object identity in the runtime before calling this helper.
inline bool MergeNativeActionQueue(std::span<std::uint8_t> input,
                                   std::span<std::uint8_t> active) {
  if(input.size()!=0xbe00 || active.size()!=20) return false;
  for(unsigned i=0;i<20;++i) {
    active[i]|=input[0x5f30+i];
    input[0x5f30+i]=0;
  }
  std::uint32_t counter=0;
  for(unsigned i=0;i<4;++i) counter=(counter<<8)|input[0xbdb4+i];
  if(counter) {
    --counter;
    for(unsigned i=0;i<4;++i) input[0xbdb4+i]=std::uint8_t(counter>>(24-8*i));
  } else {
    for(unsigned i=0;i<20;++i) input[0xbdcc+i]&=active[i];
  }
  return true;
}
}
