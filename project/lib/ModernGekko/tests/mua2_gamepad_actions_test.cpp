#include "mua2_gamepad_actions.hpp"
#include "mua2_gamepad_aim.hpp"
#include "mua2_gamepad_turret.hpp"
#include "gamepad_port_state.hpp"
#include "mua2_gamepad_provider.hpp"
#include "mua2_tutorial_ready.hpp"
#include <iostream>
#include <limits>
using namespace moderngekko::controls;
bool on(const GamepadActions& a,unsigned id) {
 return (a.active[id/32*4+3-(id%32)/8]&(1u<<(id%8)))!=0;
}
int main() {
 // Direct turret rotation: sustained stick, neutral retention, native limits,
 // disconnect, deadzone, invalid data and equal elapsed-time partitions.
 GamepadSample turret;turret.connected=true;
 turret.inputs[unsigned(GamepadInput::LeftRight)]=1;
 turret.inputs[unsigned(GamepadInput::LeftUp)]=1;
 const auto first=RotateGamepadTurret({0,0},{0,0},45,45,0.1f,turret);
 if(!first || std::abs(first->pitch+0.20071287f)>1e-6f ||
    std::abs(first->yaw+0.30019662f)>1e-6f)return 140;
 auto split=TurretRotation{0,0};
 for(unsigned i=0;i<10;++i)split=*RotateGamepadTurret(split,{0,0},45,45,0.01f,turret);
 if(std::abs(split.pitch-first->pitch)>1e-6f || std::abs(split.yaw-first->yaw)>1e-6f)return 141;
 const auto limit=RotateGamepadTurret({0,0},{0,0},30,40,1,turret);
 if(!limit || std::abs(limit->pitch+0.5235988f)>1e-6f ||
    std::abs(limit->yaw+1.3089969f)>1e-6f)return 142;
 turret.connected=false;
 auto hold=RotateGamepadTurret(*first,{0,0},45,45,0.1f,turret);
 if(!hold || hold->pitch!=first->pitch || hold->yaw!=first->yaw)return 143;
 turret.connected=true;turret.inputs.fill(0);
 turret.inputs[unsigned(GamepadInput::LeftRight)]=0.1;
 hold=RotateGamepadTurret(*first,{0,0},45,45,0.1f,turret);
 if(!hold || hold->pitch!=first->pitch || hold->yaw!=first->yaw)return 144;
 if(RotateGamepadTurret({0,0},{0,0},45,45,-1,turret) ||
    RotateGamepadTurret({0,0},{0,0},-1,45,0.1f,turret) ||
    RotateGamepadTurret({std::numeric_limits<float>::quiet_NaN(),0},{0,0},45,45,0.1f,turret))return 145;

 // PS2 accumulated aim: four independent records, release holds position;
 // disconnect invalidates the sample without erasing its last target.
 std::array<std::uint8_t,25864> aim_manager{};
 const auto aim_put=[&](unsigned off,std::uint32_t value) {
   for(unsigned i=0;i<4;++i)aim_manager[off+i]=std::uint8_t(value>>(24-8*i));
 };
 aim_put(0,0x81198830);aim_manager[5925]=0x40;
 if(GamepadAimContext(aim_manager))return 150;
 aim_manager[5925]=0x80;
 if(!GamepadAimContext(aim_manager))return 151;
 for(unsigned port=0;port<4;++port) {
   const auto offset=GamepadAimRecord(aim_manager,port);
   if(!offset || *offset!=4904+72*port)return 120;
   auto record=std::span(aim_manager).subspan(*offset,72);
   std::fill(record.begin(),record.end(),0x5a);
   std::fill_n(record.begin(),12,0);
   GamepadSample aim;aim.connected=true;
   aim.inputs[unsigned(GamepadInput::LeftRight)]=0.75;
   aim.inputs[unsigned(GamepadInput::LeftDown)]=0.5;
   if(!WriteGamepadAim(record,aim,true) ||
      std::abs(std::bit_cast<float>(ReadBE(record,0))-0.0375f)>1e-6f ||
      std::abs(std::bit_cast<float>(ReadBE(record,4))+0.025f)>1e-6f ||
      ReadBE(record,8)!=1)return 121;
   for(unsigned i=12;i<72;++i)if(record[i]!=0x5a)return 122;
   const auto held_x=ReadBE(record,0),held_y=ReadBE(record,4);
   aim.connected=false;WriteGamepadAim(record,aim,true);
   if(ReadBE(record,0)!=held_x || ReadBE(record,4)!=held_y || ReadBE(record,8))return 123;
   aim.connected=true;WriteGamepadAim(record,aim,false);
   if(ReadBE(record,8))return 124;
   aim.inputs[unsigned(GamepadInput::LeftRight)]=std::numeric_limits<double>::quiet_NaN();
   aim.inputs[unsigned(GamepadInput::LeftDown)]=0.1;
   WriteGamepadAim(record,aim,true);
   if(ReadBE(record,0)!=held_x || ReadBE(record,4)!=held_y || ReadBE(record,8)!=1)return 125;
   aim.inputs.fill(0);aim.inputs[unsigned(GamepadInput::LeftRight)]=1;
   aim.inputs[unsigned(GamepadInput::LeftDown)]=1;
   for(unsigned step=0;step<40;++step)WriteGamepadAim(record,aim,true);
   if(ReadBE(record,0)!=0x3f800000 || ReadBE(record,4)!=0xbf800000)return 146;
   aim.inputs.fill(0);WriteGamepadAim(record,aim,true);
   if(ReadBE(record,0)!=0x3f800000 || ReadBE(record,4)!=0xbf800000)return 147;
   aim.inputs[unsigned(GamepadInput::LeftLeft)]=1;
   aim.inputs[unsigned(GamepadInput::LeftUp)]=1;
   for(unsigned step=0;step<20;++step)WriteGamepadAim(record,aim,true);
   if(std::abs(std::bit_cast<float>(ReadBE(record,0)))>1e-6f ||
      std::abs(std::bit_cast<float>(ReadBE(record,4)))>1e-6f)return 148;
   // Instruction-panel input stays centered; closing resumes accumulation.
   std::array<std::uint8_t,60> projection{};
   std::copy(record.begin()+12,record.end(),projection.begin());
   WriteGamepadAim(record,aim,true,true);
   if(ReadBE(record,0)||ReadBE(record,4)||ReadBE(record,8)!=1)return 158;
   aim.connected=false;WriteGamepadAim(record,aim,true,true);
   if(ReadBE(record,0)||ReadBE(record,4)||ReadBE(record,8))return 159;
   aim.connected=true;WriteGamepadAim(record,aim,true,false);
   if(std::abs(std::bit_cast<float>(ReadBE(record,0))+0.05f)>1e-6f ||
      std::abs(std::bit_cast<float>(ReadBE(record,4))-0.05f)>1e-6f ||
      !std::equal(projection.begin(),projection.end(),record.begin()+12))return 160;
   // The next player's untouched record must not inherit this target.
   if(port<3 && ReadBE(aim_manager,4904+72*(port+1)))return 149;
 }
 // Entry and re-entry center every owner's target, preserving native validity
 // and projection data. Same-mode updates and exit must retain position.
 const auto before_reset=aim_manager;
 if(CenterGamepadAimOnModeChange(aim_manager,1,2,0) ||
    CenterGamepadAimOnModeChange(aim_manager,2,2,2) ||
    CenterGamepadAimOnModeChange(aim_manager,2,1,2) || aim_manager!=before_reset)return 152;
 for(unsigned port=0;port<4;++port) {
   aim_put(4904+72*port,0x3f000000);aim_put(4908+72*port,0xbf000000);
 }
 const auto displaced=aim_manager;
 if(!CenterGamepadAimOnModeChange(aim_manager,1,2,2))return 153;
 for(unsigned i=0;i<aim_manager.size();++i) {
   const bool axis=i>=4904 && i<4904+72*4 && (i-4904)%72<8;
   if(aim_manager[i]!=(axis?0:displaced[i]))return 154;
 }
 auto record=std::span(aim_manager).subspan(4904,72);
 GamepadSample steer;steer.connected=true;steer.inputs[unsigned(GamepadInput::LeftRight)]=1;
 WriteGamepadAim(record,steer,true);
 if(CenterGamepadAimOnModeChange(aim_manager,2,2,2) ||
    std::abs(std::bit_cast<float>(ReadBE(record,0))-0.05f)>1e-6f)return 155;
 if(!CenterGamepadAimOnModeChange(aim_manager,1,2,2) || ReadBE(record,0))return 156;
 if(CenterGamepadAimOnModeChange({},1,2,2))return 157;
 if(GamepadAimRecord(aim_manager,4)||GamepadAimRecord({},0))return 126;
 aim_put(25860,0x90000000);if(GamepadAimContext(aim_manager))return 127;
 aim_put(25860,0);aim_put(5192,3);if(GamepadAimContext(aim_manager))return 128;
 aim_put(5192,0);aim_manager[5925]=0;if(GamepadAimContext(aim_manager))return 129;
 if(WriteGamepadAim({},GamepadSample{},true))return 130;

 std::array<std::uint8_t,80> help{};
 auto put=[&](unsigned o,std::uint32_t v){for(unsigned i=0;i<4;++i)help[o+i]=std::uint8_t(v>>(24-i*8));};
 put(24,0x80564248);help[20]=1;help[65]=1;put(76,0xffffffff);
 for(unsigned kind=0;kind<5;++kind) {
   help[67]=std::uint8_t(kind);
   for(unsigned port=0;port<4;++port) for(unsigned slot:{32u,40u,48u,56u})
     if(TutorialReadySlot(help,port,slot,false)!=slot) return 40;
 }
 put(76,2);if(TutorialReadySlot(help,0,32,false))return 41;
 if(!TutorialReadySlot(help,2,40,false))return 42;
 help[68]=1;if(TutorialReadySlot(help,2,40,true))return 43;
 if(!TutorialReadySlot(help,2,32,true))return 44;
 help[65]=0;if(TutorialReadySlot(help,2,32,true))return 45;
 help[65]=1;help[67]=5;if(TutorialReadySlot(help,2,32,true))return 46;
 GamepadSample confirm;confirm.connected=true;confirm.inputs[unsigned(GamepadInput::A)]=1;
 if(!BuildGamepadActions(confirm,false,true).ready_confirm)return 47;
 for(auto modifier:{GamepadInput::LT,GamepadInput::RT,GamepadInput::X,GamepadInput::B,
                    GamepadInput::Y,GamepadInput::LB,GamepadInput::RB,GamepadInput::Start,GamepadInput::Back}) {
   confirm.inputs[unsigned(modifier)]=1;
   if(BuildGamepadActions(confirm,false,true).ready_confirm)return 48;
   confirm.inputs[unsigned(modifier)]=0;
 }
 confirm.connected=false;if(BuildGamepadActions(confirm,false,true).ready_confirm)return 49;
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
 // A live native menu must not emit combat, camera, fusion or QTE actions,
 // including when a trigger and a face button are pressed together.
 using K=GamepadInput;
 {
  GamepadSample menu;menu.connected=true;menu.inputs.fill(1);
  menu.inputs[unsigned(K::Back)]=menu.inputs[unsigned(K::Start)]=0;
  auto actions=BuildGamepadActions(menu,false,true,true,true);
  for(unsigned id=0;id<63;++id) if(on(actions,id)) return 101;
  if(actions.hero_slot!=-1||actions.fusion_slot!=-1||actions.mash_x) return 102;
  for(unsigned id:{103u,104u,116u,117u,118u,119u,122u,123u})
    if(!on(actions,id)) return 103;
  menu.inputs={};actions=BuildGamepadActions(menu,false,true,true,true);
  for(auto v:actions.active) if(v) return 104;
  menu.inputs.fill(1);menu.connected=false;
  actions=BuildGamepadActions(menu,true,true,true,true);
  for(auto v:actions.active) if(v) return 105;
  menu.connected=true;menu.inputs={};menu.inputs[unsigned(K::RB)]=1;
  actions=BuildGamepadActions(menu,false,false,true,true);
  if(!on(actions,118)||on(actions,119)||on(actions,101)) return 106;
  menu.inputs={};menu.inputs[unsigned(K::LB)]=1;
  actions=BuildGamepadActions(menu,false,false,true,true);
  if(!on(actions,119)||on(actions,118)||on(actions,101)) return 107;
 }
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
 // Both camera axes use the native camera-enable gate. Vertical input must
 // not become movement, pointer validity, or a fusion selection.
 for(auto key:{K::RightUp,K::RightDown}) {
  p.inputs={};set(key,0.75);a=BuildGamepadActions(p,false,false);
  const float value=std::bit_cast<float>(ReadBE(a.values,3*4));
  if(value!=(key==K::RightUp?0.75f:-0.75f) || !on(a,7) || on(a,2) ||
     on(a,0)||on(a,1)||on(a,6)||a.fusion_slot!=-1)return 92;
  set(key,0.15);a=BuildGamepadActions(p,false,false);
  if(on(a,3)||on(a,7)||ReadBE(a.values,3*4))return 93;
  set(key,0.75);set(K::LT);a=BuildGamepadActions(p,false,false);
  if(on(a,3)||on(a,7))return 94;
  p.inputs={};a=BuildGamepadActions(p,false,false);
  if(on(a,3)||on(a,7)||ReadBE(a.values,3*4))return 95;
 }
 // Every raw input must keep pointer, gesture, debug and network actions clear.
 for(unsigned key=0;key<22;++key) {
  p.inputs={};p.inputs[key]=1;a=BuildGamepadActions(p,false,true);
  for(unsigned id:{4u,5u,6u,14u,56u,57u,58u,59u,60u,69u,70u,71u,74u,75u,76u,77u,78u,79u,80u,81u,82u,83u,84u,85u,86u,87u,88u})
   if(on(a,id)) return 15;
 }
 std::cout<<"Direct gamepad actions: release, modifiers, isolation, no pointer/motion/debug aliases passed\n";
}
