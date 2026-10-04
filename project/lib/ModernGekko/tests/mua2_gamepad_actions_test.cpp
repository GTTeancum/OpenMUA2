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
 std::array<std::uint8_t,60> manager{};
 std::array<bool,4> connections{false,false,false,true};
 for(unsigned i=0;i<4;++i)manager[7+i*4]=std::uint8_t(i);
 for(unsigned i=0;i<4;++i)if(GamepadConnectionStatus(manager,i,connections)!=std::optional<bool>(i==3))return 71;
 manager[7]=3;if(GamepadConnectionStatus(manager,0,connections)!=true)return 72;
 connections[3]=false;if(GamepadConnectionStatus(manager,0,connections)!=false)return 73;
 manager[21]=1;if(GamepadConnectionStatus(manager,1,connections)!=true)return 74;
 manager[15]=255;if(GamepadConnectionStatus(manager,2,connections)!=false)return 75;
 if(GamepadConnectionStatus({},0,connections)||GamepadConnectionStatus(manager,4,connections))return 76;
 // All five shared tutorial kinds schedule native close; no ready-icon writes.
 for(unsigned kind=0;kind<5;++kind) for(unsigned owner=0;owner<4;++owner) {
   std::array<std::uint8_t,80> tutorial{};
   tutorial[20]=1;tutorial[65]=1;tutorial[67]=std::uint8_t(kind);
   tutorial[24]=0x80;tutorial[25]=0x56;tutorial[26]=0x42;tutorial[27]=0x48;
   tutorial[79]=std::uint8_t(owner);
   if(AcceptGamepadTutorial(tutorial,(owner+1)%4,10))return 77;
   if(!AcceptGamepadTutorial(tutorial,owner,10)||tutorial[65])return 78;
   std::uint32_t deadline=0;for(unsigned i=0;i<4;++i)deadline=(deadline<<8)|tutorial[72+i];
   if(std::bit_cast<float>(deadline)!=10.2f)return 79;
   for(unsigned slot:{32u,40u,48u,56u})if(tutorial[slot]||tutorial[slot+1])return 80;
   if(AcceptGamepadTutorial(tutorial,owner,10))return 81;
   tutorial[65]=1;if(AcceptGamepadTutorial(tutorial,owner,std::numeric_limits<float>::infinity()))return 82;
   tutorial[67]=5;if(AcceptGamepadTutorial(tutorial,owner,10))return 83;
 }
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
 // The shared profile/hero chooser must navigate independently on each pad.
 for(auto key:{K::Left,K::LeftLeft,K::Right,K::LeftRight}) {
  p.inputs={};set(key);a=BuildGamepadActions(p,false,false);
  const bool left=key==K::Left || key==K::LeftLeft;
  if(!on(a,left?63:64)||on(a,left?64:63))return 84;
  p.inputs={};a=BuildGamepadActions(p,false,false);if(on(a,63)||on(a,64))return 85;
 }
 p.inputs={};set(K::Right);set(K::A);a=BuildGamepadActions(p,false,false,false);
 if(!on(a,64)||!on(a,89)||on(a,13)||on(a,9)||a.hero_slot!=-1||a.mash_x)return 86;
 p.inputs={};set(K::LeftRight);a=BuildGamepadActions(p,false,false,false);
 if(!on(a,64)||on(a,0))return 87;
 p.inputs={};set(K::LT);set(K::A);a=BuildGamepadActions(p,false,true,false);
 if(on(a,33)||on(a,9)||a.fusion_slot!=-1)return 88;
 std::array<std::uint8_t,60> join_manager{};
 std::array<std::uint8_t,128> players{};
 join_manager[0]=0x81;join_manager[1]=0x1b;join_manager[2]=0x42;join_manager[3]=0x98;
 players[0]=0x80;players[1]=0x53;players[2]=0xd1;players[3]=0xa0;
 for(unsigned i=0;i<4;++i)join_manager[7+i*4]=std::uint8_t(3-i);
 players[42]=1;
 for(unsigned i=0;i<4;++i)if(GamepadHasJoinedPlayer(join_manager,players,i)!=(i==2))return 89;
 players[42]=0;if(GamepadHasJoinedPlayer(join_manager,players,2))return 90;
 if(GamepadHasJoinedPlayer({},players,0)||GamepadHasJoinedPlayer(join_manager,{},0))return 91;
 // Every raw input must keep pointer, gesture, debug and network actions clear.
 for(unsigned key=0;key<22;++key) {
  p.inputs={};p.inputs[key]=1;a=BuildGamepadActions(p,false,true);
  for(unsigned id:{4u,5u,6u,14u,56u,57u,58u,59u,60u,69u,70u,71u,74u,75u,76u,77u,78u,79u,80u,81u,82u,83u,84u,85u,86u,87u,88u})
   if(on(a,id)) return 15;
 }
 std::cout<<"Direct gamepad actions: release, modifiers, isolation, no pointer/motion/debug aliases passed\n";
}
