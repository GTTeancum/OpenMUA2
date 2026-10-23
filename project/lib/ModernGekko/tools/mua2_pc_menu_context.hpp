#pragma once
#include "mua2_action_bindings.hpp"

namespace moderngekko::controls {
// Match the native Options close gate, including queued dialogs. Unknown or
// unavailable manager instances block pointer input rather than guessing.
template<typename Reader> bool PcMenuModalBlocked(Reader read) {
  const auto word=[&](std::uint32_t a) {
    const auto b=read(a,4);return b.size()==4?ReadBE(b,0):0u;
  };
  const auto dialog=word(0x80817374),save=word(0x808173b0);
  if(!dialog || word(dialog)!=0x811983b8 || !save || word(save+4)!=0x80542ba0)return true;
  const auto slot=word(dialog+0x8a0c);
  const auto flags=read(dialog+4+(slot<=6?slot:0)*0x13b8+0x13a9,1);
  const auto pending=read(dialog+0x8a32,1);
  const auto saving=read(save+0xdc,4);
  return flags.size()!=1 || pending.size()!=1 || saving.size()!=4 ||
      (flags[0]&0x80) || ((pending[0]&0x80) && word(dialog+0x8a10)) || ReadBE(saving,0)!=0;
}
} // namespace moderngekko::controls
