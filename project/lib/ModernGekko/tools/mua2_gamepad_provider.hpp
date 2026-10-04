#pragma once
#include <bit>
#include <cmath>
#include <array>
#include <optional>
#include <cstdint>
#include <span>

namespace moderngekko::controls {
// Logical player slots may be remapped to physical ports. Engine-owned input
// slots (network/script control) retain their native always-available behavior.
inline std::optional<bool> GamepadConnectionStatus(std::span<const std::uint8_t> manager,
    unsigned logical, const std::array<bool,4>& connected) {
  if(manager.size()!=60 || logical>=4) return std::nullopt;
  unsigned physical=0;
  for(unsigned i=0;i<4;++i) physical=(physical<<8)|manager[4+logical*4+i];
  if(physical>=4) return false;
  return manager[20+physical]!=0 || connected[physical];
}

// PS2 shared tutorial semantics: a native action-9 edge schedules close after
// 0.2 game-clock seconds, without entering Wii pointer readiness. The runtime
// calls this only at the verified acceptance site, before native input cleanup.
inline bool AcceptGamepadTutorial(std::span<std::uint8_t> help, unsigned owner, float now) {
  if(help.size()!=80 || owner>=4 || !std::isfinite(now) || now<0 ||
     help[20]!=1 || help[64]!=0 || help[65]!=1 || help[66]!=0 || help[67]>4 ||
     help[68]>1) return false;
  const auto word=[&](unsigned offset) {
    std::uint32_t value=0;for(unsigned i=0;i<4;++i)value=(value<<8)|help[offset+i];return value;
  };
  if(word(24)!=0x80564248 || (word(76)!=0xffffffff && word(76)!=owner))return false;
  const float deadline=now+0.2f;
  if(!std::isfinite(deadline) || deadline<=now)return false;
  const auto bits=std::bit_cast<std::uint32_t>(deadline);
  for(unsigned i=0;i<4;++i)help[72+i]=std::uint8_t(bits>>(24-8*i));
  help[64]=help[65]=0;
  return true;
}

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
