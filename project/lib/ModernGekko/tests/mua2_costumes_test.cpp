#include "mua2_costumes.hpp"
#include "mua2_gamepad_actions.hpp"
#include <iostream>
#include <cstdlib>
using namespace moderngekko::controls;
static void require(bool ok) { if(!ok) {std::cerr<<"costume regression failed\n";std::exit(1);} }
int main() {
  for(unsigned pool=7;pool<=10;++pool) {
    const auto caller=0x801e37fc+(pool-7)*0x18;
    require(CostumeArenaReservation(0x806a3b60,pool,0x159999,4,1,caller)==2*1024*1024);
    require(CostumeArenaReservation(0x806a3b60,pool,0x159999,4,1,caller+4)==0x159999);
    require(CostumeArenaReservation(0x806a3b60,pool,0x159999,4,0,caller)==0x159999);
    require(CostumeArenaReservation(0x806a3b60,pool,0x159999,32,1,caller)==0x159999);
    require(CostumeArenaReservation(0x806a3b60,pool,0x200000,4,1,caller)==0x200000);
    require(CostumeArenaReservation(0x806a3b64,pool,0x159999,4,1,caller)==0x159999);
  }
  require(CostumeArenaReservation(0x806a3b60,6,0x159999,4,1,0x801e37e4)==0x159999);
  require(CostumeArenaReservation(0x806a3b60,11,0x159999,4,1,0x801e385c)==0x159999);
  const auto scene=CostumeArenaReservation(0x806a3b60,11,0x110cccc,4,1,0x801e3860);
  require(scene==0xe73334);
  require(CostumeArenaReservation(0x806a3b60,11,0x110cccc,4,1,0x801e3864)==0x110cccc);
  require(CostumeArenaReservation(0x806a3b60,11,0x110cccc,4,0,0x801e3860)==0x110cccc);
  require(CostumeArenaReservation(0x806a3b60,11,0x110cccc,32,1,0x801e3860)==0x110cccc);
  require(CostumeArenaReservation(0x806a3b64,11,0x110cccc,4,1,0x801e3860)==0x110cccc);
  // Reproduce native arena alignment and reservation, not only the byte sum:
  // every pool following the scene arena must retain its original address.
  auto retail=std::uint32_t{0x90a00800},expanded=retail;
  for(unsigned pool=7;pool<=11;++pool) {
    const auto bytes=pool==11?0x110ccccu:0x159999u;
    const auto caller=pool==11?0x801e3860u:0x801e37fc+(pool-7)*0x18;
    retail=((retail+4)&~4u)+bytes;
    expanded=((expanded+4)&~4u)+CostumeArenaReservation(0x806a3b60,pool,bytes,4,1,caller);
  }
  require(expanded==retail && retail==0x92073b34);
  // All 40 roster definitions have independent ten-slot storage. Selection
  // wraps without touching another hero; model residency is tested in-game.
  std::array<CostumeSet,40> roster;
  for(unsigned hero=0;hero<roster.size();++hero) {
    for(unsigned slot=0;slot<10;++slot) {
      const auto key=slot==0?"skin":slot==9?"skin_10":"skin_0"+std::to_string(slot+1);
      require(roster[hero].Set(key,std::to_string(10000+hero*10+slot)));
    }
  }
  for(unsigned hero=0;hero<roster.size();++hero) {
    unsigned current=0;
    for(unsigned press=1;press<=30;++press) {
      current=roster[hero].Next(current,[](const auto&){return true;});
      require(current==press%10);
      require(roster[hero].Model(current)==std::to_string(10000+hero*10+current));
    }
  }
  CostumeSet set;
  set.Set("skin","25301");set.Set("skin_03","25303");
  set.Set("skin_04","25304");set.Set("skin_07","25307");set.Set("skin_10","25310");
  auto installed=[](const std::string& model){return model!="25304";};
  require(set.Next(0,installed)==2);require(set.Next(2,installed)==6);
  require(set.Next(6,installed)==9);require(set.Next(9,installed)==0);
  require(!set.Set("skin_11","25311"));require(!set.Set("skin_03_name","label"));
  require(set.Model(10).empty());require(set.Model(6)=="25307");
  require(set.Next(0,[](const auto&){return false;})==0);
  set.Set("skin_03","../bad");require(set.Model(2).empty());
  set.Set("skin_03","3");require(set.Model(2)=="25303");
  CostumeSet leading;leading.Set("skin","0301");leading.Set("skin_10","0310");
  require(leading.Model(9)=="0310");
  GamepadSample pad;pad.connected=true;
  auto down=[](const GamepadActions& a,unsigned id){return a.active[(id/32)*4+3-(id%32)/8]&(1u<<(id%8));};
  pad.inputs[unsigned(GamepadInput::X)]=1;
  auto a=BuildGamepadActions(pad,false,false,true,true,true);
  require(down(a,106)&&!down(a,101)&&!down(a,121));
  auto other=BuildGamepadActions(pad,false,false,true,true,false);
  require(down(other,101)&&!down(other,106));
  pad.inputs.fill(0);pad.inputs[unsigned(GamepadInput::Y)]=1;
  a=BuildGamepadActions(pad,false,false,true,true,true);
  require(down(a,101)&&!down(a,106)&&!down(a,109));
  pad.inputs.fill(0);a=BuildGamepadActions(pad,false,false,true,true,true);
  require(!down(a,101)&&!down(a,106));
  std::cout<<"costume slot, availability and scoped binding checks passed\n";
}
