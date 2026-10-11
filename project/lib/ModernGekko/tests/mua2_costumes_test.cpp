#include "mua2_costumes.hpp"
#include "mua2_gamepad_actions.hpp"
#include <iostream>
#include <cstdlib>
using namespace moderngekko::controls;
static void require(bool ok) { if(!ok) {std::cerr<<"costume regression failed\n";std::exit(1);} }
int main() {
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
