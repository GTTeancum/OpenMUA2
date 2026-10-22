#pragma once
#include "mua2_gamepad_actions.hpp"
#include <optional>
#include <utility>

namespace moderngekko::controls {
// Release-to-arm and release-to-finish prevent the activation button and the
// newly bound input from leaking into menu navigation or gameplay.
class PcBindingCapture {
public:
  struct Result { unsigned source; bool cancelled; };
  bool active=false, controller=false;
  unsigned player=0, action=0;
  bool alternate=false;
  void Begin(unsigned target,unsigned slot,bool secondary,bool pad) {
    action=target;player=slot;alternate=secondary;controller=pad;
    active=true;armed=false;pending.reset();pad_neutral=!pad;
    if(!pad && Neutral())armed=true;
  }
  void Key(unsigned key,bool down,bool repeat=false) {
    if(key>=keys.size())return;
    const bool edge=down && !keys[key] && !repeat;
    keys[key]=down;
    if(!active)return;
    if(armed && edge && !pending && (!controller || key==0x1b || key==0x2e))
      pending=Result{key==0x2e?0:key,key==0x1b};
    if(!armed && Neutral() && pad_neutral)armed=true;
  }
  void Pad(unsigned slot,const GamepadSample& sample) {
    if(!active || !controller || slot!=player)return;
    pad_neutral=std::none_of(sample.inputs.begin(),sample.inputs.end(),[](double v){return v>0.5;});
    if(!sample.connected){pending=Result{0,true};pad_neutral=true;return;}
    if(!armed){if(pad_neutral && Neutral())armed=true;return;}
    if(!pending)for(unsigned i=0;i<sample.inputs.size();++i)
      if(sample.inputs[i]>0.5){pending=Result{i+1,false};break;}
  }
  std::optional<Result> Take() {
    if(!active || !pending || !Neutral() || !pad_neutral)return {};
    active=false;return std::exchange(pending,std::nullopt);
  }
  void Reset() {active=false;pending.reset();keys.fill(false);armed=false;pad_neutral=true;}
  void FocusLost() {if(active)pending=Result{0,true};keys.fill(false);pad_neutral=true;}
private:
  bool Neutral() const {return std::none_of(keys.begin(),keys.end(),[](bool v){return v;});}
  bool armed=false,pad_neutral=true;
  std::array<bool,256> keys{};
  std::optional<Result> pending;
};
}
