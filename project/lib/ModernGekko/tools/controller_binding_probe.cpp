#include "Core/HW/Wiimote.h"
#include "InputCommon/InputConfig.h"
#include "InputCommon/ControllerEmu/ControllerEmu.h"
#include "InputCommon/ControllerEmu/ControlGroup/Attachments.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "UICommon/UICommon.h"
#include <chrono>
#include <cstdio>
#include <thread>

// Read-only hardware/profile diagnostic. Never submits output or host input.
static void Report(ControllerEmu::ControlGroupContainer& controller, const std::string& prefix)
{
  for (auto& group : controller.groups)
  {
    for (auto& control : group->controls)
    {
      auto& ref = *control->control_ref;
      if (!ref.IsInput() || ref.GetExpression().empty()) continue;
      std::printf("binding %s%s/%s count=%d value=%.6f expression=%s\n", prefix.c_str(),
          group->name.c_str(), control->name.c_str(), ref.BoundCount(), ref.State(),
          ref.GetExpression().c_str());
    }
    if (group->type == ControllerEmu::GroupType::Attachments)
    {
      auto& attachments = static_cast<ControllerEmu::Attachments&>(*group);
      auto& selected = *attachments.GetAttachmentList().at(attachments.GetSelectedAttachment());
      Report(selected, prefix + selected.GetName() + "/");
    }
  }
}
int main(int argc, char** argv)
{
  if (argc != 2) return 2;
  UICommon::SetUserDirectory(argv[1]);
  UICommon::CreateDirectories();
  UICommon::Init();
  WindowSystemInfo wsi;
  wsi.type = WindowSystemType::Headless;
  UICommon::InitControllers(wsi);
  // Include completion of asynchronous device discovery/hotplug rebinding.
  std::this_thread::sleep_for(std::chrono::seconds(2));
  g_controller_interface.UpdateInput();
  for (const auto& device : g_controller_interface.GetAllDeviceStrings())
    std::printf("device %s\n", device.c_str());
  {
    auto lock = ControllerEmu::EmulatedController::GetStateLock();
    auto* pad = Wiimote::GetConfig()->GetController(0);
    std::printf("profile %s connected=%d gate=%d\n", pad->GetDefaultDevice().ToString().c_str(),
                pad->IsDefaultDeviceConnected(), ControlReference::GetInputGate());
    Report(*pad, "");
  }
  std::fflush(stdout);
  UICommon::ShutdownControllers();
  UICommon::Shutdown();
}
