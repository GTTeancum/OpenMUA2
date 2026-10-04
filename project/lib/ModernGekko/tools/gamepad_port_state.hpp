#pragma once
#include "mua2_gamepad_actions.hpp"

namespace moderngekko::controls {
// Own connection/edge state independently of any emulated controller profile.
// A button held across connection or reset must be released before it can act.
struct GamepadPortState {
  GamepadSample sample{};
  std::array<bool,22> pressed{}, released{}, armed{};

  void Update(const GamepadSample& physical) {
    const auto previous=sample;
    sample={}; sample.connected=physical.connected;
    pressed={}; released={};
    if (!physical.connected) armed={};
    for (unsigned i=0;i<sample.inputs.size();++i) {
      const auto key=static_cast<GamepadInput>(i);
      const double value=physical.Value(key);
      if (!previous.connected || !physical.connected) armed[i]=false;
      if (physical.connected && value<=0.15) armed[i]=true;
      sample.inputs[i]=physical.connected && armed[i] ? value : 0.0;
      pressed[i]=sample.Down(key) && !previous.Down(key);
      released[i]=!sample.Down(key) && previous.Down(key);
    }
  }
  void Reset() { *this={}; }
};
}
