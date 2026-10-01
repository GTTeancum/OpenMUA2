#include "frontend_config.hpp"
#include "Common/IniFile.h"
#include "InputCommon/ControlReference/ExpressionParser.h"
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>

// A process-local device: never initializes SDL, discovers hardware or emits
// host input. Exercise the real generated profile with Dolphin's real parser.
class TestPad final : public ciface::Core::Device {
  class Input final : public Device::Input {
  public:
    Input(std::string name, double& value) : m_name(std::move(name)), m_value(value) {}
    std::string GetName() const override { return m_name; }
    double GetState() const override { return m_value; }
  private:
    std::string m_name;
    double& m_value;
  };
public:
  TestPad() {
    for (const char* name : {"Button A", "Button B", "Button X", "Button Y",
        "Shoulder L", "Shoulder R", "Trigger L", "Trigger R", "Start", "Back",
        "Pad N", "Pad S", "Pad W", "Pad E", "Left X+", "Left X-", "Left Y+",
        "Left Y-", "Right X+", "Right X-", "Right Y+", "Right Y-"}) {
      AddInput(new Input(name, values[name]));
    }
  }
  std::string GetName() const override { return "Test Pad"; }
  std::string GetSource() const override { return "Test"; }
  std::map<std::string, double> values;
};
class TestDevices final : public ciface::Core::DeviceContainer {
public:
  explicit TestDevices(std::shared_ptr<TestPad> pad) { m_devices.push_back(std::move(pad)); }
};

int main(int argc, char** argv) {
  namespace fs = std::filesystem;
  namespace ep = ciface::ExpressionParser;
  const auto directory = fs::temp_directory_path() / ("openmua2-xbox-profile-" +
      std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  std::string error;
  if (!moderngekko::frontend::GenerateControllerConfig(directory, "Test/0/Test Pad", &error))
    return 1;
  Common::IniFile ini;
  if (!ini.Load((directory / "Config/WiimoteNew.ini").string())) return 2;
  auto* section = ini.GetSection("Wiimote1");
  auto pad = std::make_shared<TestPad>();
  TestDevices devices(pad);
  ciface::Core::DeviceQualifier qualifier("Test", 0, "Test Pad");
  ep::ControlEnvironment::VariableContainer vars;
  ep::ControlEnvironment env(devices, qualifier, vars);
  std::map<std::string, std::unique_ptr<ep::Expression>> expressions;
  for (const char* key : {"Buttons/A", "Buttons/B", "Buttons/1", "Buttons/2",
      "Buttons/-", "Buttons/+", "Buttons/Home", "D-Pad/Up", "D-Pad/Down",
      "D-Pad/Left", "D-Pad/Right", "Nunchuk/Buttons/C", "Nunchuk/Buttons/Z",
      "Nunchuk/Stick/Up", "Nunchuk/Stick/Down", "Nunchuk/Stick/Left", "Nunchuk/Stick/Right",
      "Nunchuk/Shake/X", "Nunchuk/Shake/Y", "Nunchuk/Shake/Z", "Shake/X", "Shake/Y",
      "Shake/Z", "Swing/Up", "Swing/Down", "Tilt/Left", "Tilt/Right", "IR/Up", "IR/Down", "IR/Left", "IR/Right"}) {
    std::string value;
    if (!section->Get(key, &value)) return 3;
    auto parsed = ep::ParseExpression(value);
    if (parsed.status == ep::ParseStatus::SyntaxError) {
      std::cerr << key << ": " << parsed.description.value_or("parse error") << '\n';
      return 4;
    }
    if (parsed.expr) parsed.expr->UpdateReferences(env);
    expressions[key] = std::move(parsed.expr);
  }
  int checks = 0;
  const auto test = [&](std::initializer_list<const char*> pressed,
                       std::initializer_list<const char*> expected) {
    for (auto& [name, value] : pad->values) value = 0;
    for (const char* name : pressed) pad->values.at(name) = 1;
    std::set<std::string> wanted(expected.begin(), expected.end());
    for (auto& [key, expr] : expressions) {
      const double value = expr ? expr->GetValue() : 0;
      if (std::abs(value - (wanted.contains(key) ? 1.0 : 0.0)) > 0.00001) {
        std::cerr << "case " << checks << ' ' << key << " got " << value << '\n';
        return false;
      }
    }
    ++checks;
    return true;
  };
  if (!test({}, {}) ||
      !test({"Button A"}, {"Buttons/A"}) ||
      !test({"Button B"}, {"Buttons/B"}) ||
      !test({"Button X"}, {"Buttons/A", "Buttons/B"}) ||
      !test({"Button Y"}, {"Nunchuk/Buttons/C"}) ||
      !test({"Start"}, {"Buttons/2"}) ||
      !test({"Shoulder L"}, {"Nunchuk/Buttons/Z"}) ||
      !test({"Shoulder R", "Button A"}, {"D-Pad/Down"}) ||
      !test({"Shoulder R", "Button B"}, {"D-Pad/Right"}) ||
      !test({"Shoulder R", "Button X"}, {"D-Pad/Left"}) ||
      !test({"Shoulder R", "Button Y"}, {"D-Pad/Up"}) ||
      !test({"Shoulder L", "Shoulder R"}, {"Nunchuk/Buttons/Z", "Nunchuk/Shake/X"}) ||
      !test({"Trigger L"}, {"Buttons/-"}) ||
      !test({"Trigger R"}, {"Buttons/+"}) ||
      !test({"Pad N"}, {"D-Pad/Up"}) ||
      !test({"Pad S"}, {"D-Pad/Down"}) ||
      !test({"Pad W"}, {"D-Pad/Left"}) ||
      !test({"Pad E"}, {"D-Pad/Right"}) ||
      !test({"Left Y+"}, {"Nunchuk/Stick/Up"}) ||
      !test({"Left Y-"}, {"Nunchuk/Stick/Down"}) ||
      !test({"Left X+"}, {"Nunchuk/Stick/Right"}) ||
      !test({"Left X-"}, {"Nunchuk/Stick/Left"}) ||
      !test({"Right X+"}, {"Buttons/1", "Tilt/Right"}) ||
      !test({"Right X-"}, {"Buttons/1", "Tilt/Left"}) ||
      !test({"Back"}, {}) ||
      !test({"Back", "Right X+"}, {"Shake/X"}) ||
      !test({"Back", "Right X-"}, {"Shake/X"}) ||
      !test({"Back", "Right Y+"}, {"Swing/Up"}) ||
      !test({"Back", "Right Y-"}, {"Swing/Down"}) ||
      !test({"Back", "Shoulder L", "Shoulder R", "Right Y+"},
            {"Nunchuk/Buttons/Z", "IR/Up"}) ||
      !test({"Shoulder L", "Shoulder R", "Right X-"},
            {"Nunchuk/Buttons/Z", "IR/Left"}) ||
      !test({}, {})) return 5;
  // Aim is established before the confirm edge; powers/jump remain suppressed.
  for (auto& [name, value] : pad->values) value = 0;
  for (const char* name : {"Shoulder L", "Shoulder R", "Button X"}) pad->values.at(name) = 1;
  if (expressions.at("Buttons/A")->GetValue() != 0 ||
      expressions.at("IR/Left")->GetValue() != 0.62 ||
      expressions.at("IR/Up")->GetValue() != 0.50 ||
      expressions.at("Nunchuk/Shake/X")->GetValue() != 0 ||
      expressions.at("D-Pad/Left")->GetValue() != 0) return 7;
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  if (expressions.at("Buttons/A")->GetValue() != 1) return 8;
  if (!test({}, {})) return 9;
  // Manual aiming takes priority over corner presets and must not shake,
  // rotate the camera, jump, or select a normal power while confirming.
  for (auto& [name, value] : pad->values) value = 0;
  for (const char* name : {"Shoulder L", "Shoulder R", "Button A"}) pad->values.at(name) = 1;
  pad->values.at("Right X-") = 0.28;
  pad->values.at("Right Y-") = 0.4;
  if (std::abs(expressions.at("IR/Left")->GetValue() - 0.28) > 0.00001 ||
      std::abs(expressions.at("IR/Down")->GetValue() - 0.4) > 0.00001 ||
      expressions.at("IR/Up")->GetValue() != 0 ||
      expressions.at("IR/Right")->GetValue() != 0 ||
      expressions.at("Nunchuk/Shake/X")->GetValue() != 0 ||
      expressions.at("Buttons/1")->GetValue() != 0 ||
      expressions.at("D-Pad/Down")->GetValue() != 0 ||
      expressions.at("Tilt/Left")->GetValue() != 0 ||
      expressions.at("Buttons/A")->GetValue() != 0) return 10;
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  if (expressions.at("Buttons/A")->GetValue() != 1) return 11;
  if (!test({}, {})) return 12;
  for (const char* name : {"Shoulder L", "Shoulder R", "Button X"}) pad->values.at(name) = 1;
  pad->values.at("Right X-") = 0.1;
  if (expressions.at("IR/Left")->GetValue() != 0.62 ||
      expressions.at("IR/Up")->GetValue() != 0.5) return 13;
  if (!test({}, {}) ||
      !test({"Shoulder L", "Shoulder R", "Right X+"}, {"Nunchuk/Buttons/Z", "IR/Right"}) ||
      !test({"Shoulder L", "Shoulder R", "Right Y+"}, {"Nunchuk/Buttons/Z", "IR/Up"}) ||
      !test({"Shoulder L", "Shoulder R", "Right Y-"}, {"Nunchuk/Buttons/Z", "IR/Down"}) ||
      !test({}, {})) return 14;
  std::cout << checks << " Xbox profile action/isolation cases passed\n";
  // Optional diagnostic output lets a process-local gameplay harness replay
  // the evaluated mapping without injecting OS keyboard or controller events.
  if (argc > 1) {
    for (auto& [name, value] : pad->values) value = 0;
    for (int i = 1; i < argc; ++i) {
      if (!pad->values.contains(argv[i])) return 6;
      pad->values.at(argv[i]) = 1;
    }
    for (auto& [key, expr] : expressions)
      std::cout << key << '=' << (expr ? expr->GetValue() : 0) << '\n';
  }
  fs::remove_all(directory);
  return 0;
}
