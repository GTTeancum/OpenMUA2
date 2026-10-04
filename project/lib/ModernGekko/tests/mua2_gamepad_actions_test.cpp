#include "mua2_gamepad_actions.hpp"
#include "gamepad_port_state.hpp"
#include "mua2_gamepad_provider.hpp"
#include <iostream>
#include <limits>
using namespace moderngekko::controls;
bool on(const GamepadActions& a,unsigned id) {
 return (a.active[id/32*4+3-(id%32)/8]&(1u<<(id%8)))!=0;
}
int main() {
 GamepadPortState port;
 GamepadSample raw;raw.connected=true;raw.inputs[0]=1;
 port.Update(raw);if(port.sample.Down(GamepadInput::A)||port.pressed[0])return 50;
 raw.inputs[0]=0;port.Update(raw);
 raw.inputs[0]=1;port.Update(raw);if(!port.sample.Down(GamepadInput::A)||!port.pressed[0])return 51;
 port.Update(raw);if(port.pressed[0]||!port.sample.Down(GamepadInput::A))return 52;
 raw.connected=false;port.Update(raw);if(port.sample.connected||!port.released[0])return 53;
 raw.connected=true;port.Update(raw);if(port.sample.Down(GamepadInput::A)||port.pressed[0])return 54;
 raw.inputs[0]=0;port.Update(raw);raw.inputs[0]=1;port.Update(raw);if(!port.pressed[0])return 55;
 GamepadPortState other;other.Update({});if(other.pressed[0]||!port.pressed[0])return 56;
 port.Reset();port.Update(raw);if(port.sample.Down(GamepadInput::A))return 57;
 raw.inputs[0]=std::numeric_limits<double>::quiet_NaN();port.Update(raw);if(port.sample.Value(GamepadInput::A)!=0)return 58;
 // Four simultaneous ports must keep ownership through independent disconnects.
 std::array<GamepadPortState,4> ports;
 std::array<GamepadSample,4> samples;
 for(unsigned i=0;i<4;++i) {samples[i].connected=true;ports[i].Update(samples[i]);}
 for(unsigned i=0;i<4;++i) {samples[i].inputs[i]=1;ports[i].Update(samples[i]);}
 for(unsigned i=0;i<4;++i) for(unsigned j=0;j<4;++j)
   if(ports[i].sample.Down(static_cast<GamepadInput>(j))!=(i==j))return 60;
 samples[2].connected=false;ports[2].Update(samples[2]);
 if(!ports[2].released[2]||ports[2].sample.connected)return 61;
 for(unsigned i:{0u,1u,3u})if(!ports[i].sample.Down(static_cast<GamepadInput>(i)))return 62;
 samples[2].connected=true;ports[2].Update(samples[2]);if(ports[2].sample.Down(GamepadInput::X))return 63;
 samples[2].inputs[2]=0;ports[2].Update(samples[2]);samples[2].inputs[2]=1;ports[2].Update(samples[2]);
 if(!ports[2].pressed[2])return 64;
 std::array<std::uint8_t,0xbe00> native{};
 std::array<std::uint8_t,20> bits{};
 native[0x5f30]=0x80;native[0xbdcc]=0xff;bits[0]=1;
 if(!MergeNativeActionQueue(native,bits)||bits[0]!=0x81||native[0x5f30]||native[0xbdcc]!=0x81)return 65;
 bits={};MergeNativeActionQueue(native,bits);if(bits[0]||native[0xbdcc])return 66;
 native[0xbdb7]=2;native[0xbdcc]=0xff;MergeNativeActionQueue(native,bits);
 if(native[0xbdb7]!=1||native[0xbdcc]!=0xff)return 67;
 MergeNativeActionQueue(native,bits);if(native[0xbdb7]||native[0xbdcc]!=0xff)return 68;
 MergeNativeActionQueue(native,bits);if(native[0xbdcc])return 69;
 if(MergeNativeActionQueue({},bits)||MergeNativeActionQueue(native,{}))return 70;
 using K=GamepadInput;
 GamepadSample p;p.connected=true;
 auto set=[&](K key,double value=1.0){p.inputs[unsigned(key)]=value;};
 set(K::X);auto a=BuildGamepadActions(p,false,false);
 if(!on(a,11)||!on(a,21)||!a.mash_x||on(a,9)||on(a,10)||on(a,89)||on(a,90)) return 1;
 p.inputs={};a=BuildGamepadActions(p,false,false);
 for(auto v:a.active) if(v) return 2;
 for(auto v:a.values) if(v) return 3;
 set(K::LT);set(K::B);a=BuildGamepadActions(p,false,true);
 if(!on(a,33)||!on(a,9)||a.fusion_slot!=1||on(a,10)||on(a,14)||on(a,31)||on(a,56)) return 4;
 set(K::A);a=BuildGamepadActions(p,false,true);
 if(a.fusion_slot!=-1||on(a,9)) return 5;
 set(K::RT);a=BuildGamepadActions(p,false,true);
 if(on(a,33)||on(a,9)||a.fusion_slot!=-1) return 6;
 p.inputs={};set(K::RT);set(K::X);a=BuildGamepadActions(p,false,false);
 if(!on(a,30)||on(a,11)||a.mash_x||on(a,33)) return 7;
 p.inputs={};set(K::Right);a=BuildGamepadActions(p,false,false);
 if(a.hero_slot!=1||!on(a,13)||!on(a,98)||!on(a,93)||on(a,31)||on(a,105)) return 8;
 set(K::Up);a=BuildGamepadActions(p,false,false);
 if(a.hero_slot!=-1||on(a,13)) return 9;
 p.inputs={};set(K::Start);set(K::X);a=BuildGamepadActions(p,true,false);
 if(!on(a,105)||on(a,90)||on(a,11)||a.mash_x) return 10;
 a=BuildGamepadActions(p,false,false);if(!on(a,90)||on(a,105)) return 11;
 p.inputs.fill(1);p.connected=false;a=BuildGamepadActions(p,true,true);
 for(auto v:a.active) if(v) return 12;
 for(auto v:a.values) if(v) return 13;
 p.connected=true;p.inputs={};set(K::RightRight,std::numeric_limits<double>::quiet_NaN());
 a=BuildGamepadActions(p,false,false);if(on(a,2)||on(a,7)) return 14;
 // Every raw input must keep pointer, gesture, debug and network actions clear.
 for(unsigned key=0;key<22;++key) {
  p.inputs={};p.inputs[key]=1;a=BuildGamepadActions(p,false,true);
  for(unsigned id:{4u,5u,6u,14u,56u,57u,58u,59u,60u,69u,70u,71u,74u,75u,76u,77u,78u,79u,80u,81u,82u,83u,84u,85u,86u,87u,88u})
   if(on(a,id)) return 15;
 }
 std::cout<<"Direct gamepad actions: release, modifiers, isolation, no pointer/motion/debug aliases passed\n";
}
