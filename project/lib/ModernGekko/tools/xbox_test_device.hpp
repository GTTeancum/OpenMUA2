#pragma once
#include "InputCommon/ControllerInterface/CoreDevice.h"
#include <array>
#include <string_view>
namespace moderngekko::automation {
inline constexpr std::array<std::string_view, 22> XboxInputNames = {
    "Button A", "Button B", "Button X", "Button Y", "Shoulder L", "Shoulder R",
    "Trigger L", "Trigger R", "Start", "Back", "Pad N", "Pad S", "Pad W", "Pad E",
    "Left X+", "Left X-", "Left Y+", "Left Y-", "Right X+", "Right X-", "Right Y+", "Right Y-"};
inline constexpr std::array<std::string_view, 22> XboxFieldNames = {
    "a", "b", "x", "y", "lb", "rb", "lt", "rt", "start", "back",
    "dpad_up", "dpad_down", "dpad_left", "dpad_right", "left_right", "left_left",
    "left_up", "left_down", "right_right", "right_left", "right_up", "right_down"};
using XboxState = std::array<double, XboxInputNames.size()>;
// Only the targeted process sees this device. It cannot emit host OS input.
// Values and readers share Dolphin's emulated-controller state lock.
class XboxTestDevice final : public ciface::Core::Device {
  class Input final : public Device::Input {
  public:
    Input(std::string name, const double& value) : m_name(std::move(name)), m_value(value) {}
    std::string GetName() const override { return m_name; }
    double GetState() const override { return m_value; }
  private:
    std::string m_name;
    const double& m_value;
  };
public:
  XboxTestDevice() {
    for (std::size_t i = 0; i < XboxInputNames.size(); ++i)
      AddInput(new Input(std::string(XboxInputNames[i]), values[i]));
  }
  std::string GetName() const override { return "Xbox Profile Test"; }
  std::string GetSource() const override { return "OpenMUA2Test"; }
  bool IsVirtualDevice() const override { return true; }
  XboxState values{};
};
}
