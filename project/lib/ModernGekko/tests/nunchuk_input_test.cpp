#include "Common/IniFile.h"
#include "Core/HW/Wiimote.h"
#include "Core/HW/WiimoteEmu/Extension/DesiredExtensionState.h"
#include "Core/HW/WiimoteEmu/Extension/Nunchuk.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"
#include "InputCommon/ControllerEmu/ControlGroup/Attachments.h"
#include "InputCommon/ControllerInterface/Touch/InputOverrider.h"
#include "InputCommon/InputConfig.h"
#include "UICommon/UICommon.h"

#include <chrono>
#include <filesystem>
#include <algorithm>

int main()
{
  namespace fs = std::filesystem;
  using namespace ciface::Touch;
  const auto directory = fs::temp_directory_path() /
      ("moderngekko-nunchuk-input-" + std::to_string(
          std::chrono::steady_clock::now().time_since_epoch().count()));
  fs::create_directories(directory);
  UICommon::SetUserDirectory(directory.string());
  UICommon::CreateDirectories();
  UICommon::Init();
  auto* config = Wiimote::GetConfig();
  // Create only an emulated controller. No device discovery, OS input or game
  // is involved; exercise the same overrider and serialized report as runtime.
  config->CreateController<WiimoteEmu::Wiimote>(0);
  auto* wiimote = static_cast<WiimoteEmu::Wiimote*>(config->GetController(0));
  auto& attachments = static_cast<ControllerEmu::Attachments*>(
      wiimote->GetWiimoteGroup(WiimoteEmu::WiimoteGroup::Attachments))->GetAttachmentList();
  auto* nunchuk = static_cast<WiimoteEmu::Nunchuk*>(
      attachments[WiimoteEmu::ExtensionNumber::NUNCHUK].get());
  nunchuk->Reset();
  RegisterWiiInputOverrider(0);
  const auto read = [&] {
    WiimoteEmu::DesiredExtensionState desired;
    nunchuk->BuildDesiredExtensionState(&desired);
    return std::get<WiimoteEmu::Nunchuk::DataFormat>(desired.data);
  };
  int result = 0;
  const auto neutral = read();
  SetControlState(0, NUNCHUK_Z_BUTTON, 1);
  SetControlState(0, NUNCHUK_ACCEL_DELTA_X, 20);
  SetControlState(0, NUNCHUK_ACCEL_DELTA_Y, -20);
  SetControlState(0, NUNCHUK_ACCEL_DELTA_Z, -9.80665);
  const auto positive = read();
  if (positive.GetButtons() != WiimoteEmu::Nunchuk::BUTTON_Z ||
      positive.GetAccelX() <= neutral.GetAccelX() + 300 ||
      positive.GetAccelY() + 300 >= neutral.GetAccelY() ||
      positive.GetAccelZ() + 150 >= neutral.GetAccelZ())
    result = 1;
  SetControlState(0, NUNCHUK_ACCEL_DELTA_X, -20);
  const auto negative = read();
  if (negative.GetAccelX() + 300 >= neutral.GetAccelX() ||
      negative.GetButtons() != WiimoteEmu::Nunchuk::BUTTON_Z)
    result = 2;
  for (int id = FIRST_WII_CONTROL; id <= LAST_WII_CONTROL; ++id)
    SetControlState(0, static_cast<ControlID>(id), 0);
  const auto cleared = read();
  if (cleared.GetButtons() != 0 || cleared.GetAccelX() != neutral.GetAccelX() ||
      cleared.GetAccelY() != neutral.GetAccelY() || cleared.GetAccelZ() != neutral.GetAccelZ())
    result = 3;
  SetControlState(0, NUNCHUK_Z_BUTTON, 1);
  SetControlState(0, NUNCHUK_SHAKE_X, 1);
  u16 minimum = 1023, maximum = 0;
  for (int i = 0; i < 200; ++i)
  {
    const auto sample = read();
    minimum = std::min(minimum, sample.GetAccelX());
    maximum = std::max(maximum, sample.GetAccelX());
    if (sample.GetButtons() != WiimoteEmu::Nunchuk::BUTTON_Z)
      result = 4;
  }
  // Native shake must change the actual wire report with no host input gate.
  if (maximum - minimum < 400)
    result = 5;
  UnregisterWiiInputOverrider(0);
  config->ClearControllers();
  UICommon::Shutdown();
  fs::remove_all(directory);
  return result;
}
