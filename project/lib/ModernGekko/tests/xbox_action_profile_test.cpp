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
  class Output final : public Device::Output {
  public:
    Output(std::string name, double& value) : m_name(std::move(name)), m_value(value) {}
    std::string GetName() const override { return m_name; }
    void SetState(ControlState value) override { m_value = value; }
  private:
    std::string m_name;
    double& m_value;
  };
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
    AddOutput(new Output("Motor L", motors[0]));
    AddOutput(new Output("Motor R", motors[1]));
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
  double motors[2]{};
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
  std::string rumble;
  if (!section->Get("Rumble/Motor", &rumble)) return 30;
  auto output = ep::ParseExpression(rumble);
  if (!output.expr) return 31;
  output.expr->UpdateReferences(env);
  if (output.expr->CountNumControls() != 2) return 32;
  for (double value : {0.75, 0.0}) {
    output.expr->SetValue(value);
    if (pad->motors[0] != value || pad->motors[1] != value) return 33;
  }
  // The prior Wiimote-only output cannot resolve against Xbox motor names.
  auto old_output = ep::ParseExpression("Motor");
  if (!old_output.expr) return 34;
  old_output.expr->UpdateReferences(env);
  if (old_output.expr->CountNumControls() != 0) return 35;
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
      !test({"Button A"},{"Buttons/A"}) ||
      !test({"Button B"},{"Buttons/B"}) ||
      !test({"Button X"},{"Buttons/A","Buttons/B"}) ||
      !test({"Button Y"},{"Nunchuk/Buttons/C"}) ||
      !test({"Shoulder L"},{"Nunchuk/Buttons/Z"}) ||
      !test({"Shoulder R"},{}) ||
      !test({"Start"},{"Buttons/2"}) ||
      !test({"Back"},{"Buttons/1","Buttons/2"}) ||
      !test({"Start","Right X+"},{"Buttons/2"}) ||
      !test({"Back","Right X-"},{"Buttons/1","Buttons/2"}) ||
      !test({"Right X+"},{"Buttons/1","Tilt/Right"}) ||
      !test({"Right X-"},{"Buttons/1","Tilt/Left"}) ||
      !test({"Left X+"},{"Nunchuk/Stick/Right"}) ||
      !test({"Left X-"},{"Nunchuk/Stick/Left"}) ||
      !test({"Left Y+"},{"Nunchuk/Stick/Up"}) ||
      !test({"Left Y-"},{"Nunchuk/Stick/Down"}) ||
      !test({"Trigger R"},{}) ||
      !test({"Trigger R","Button A"},{"D-Pad/Down"}) ||
      !test({"Trigger R","Button B"},{"D-Pad/Right"}) ||
      !test({"Trigger R","Button X"},{"D-Pad/Left"}) ||
      !test({"Trigger R","Button Y"},{"D-Pad/Up"}) ||
      !test({"Pad N"},{"Buttons/+","D-Pad/Up"}) ||
      !test({"Pad S"},{"Buttons/+","D-Pad/Down"}) ||
      !test({"Pad W"},{"Buttons/+","D-Pad/Left"}) ||
      !test({"Pad E"},{"Buttons/+","D-Pad/Right"}) ||
      !test({"Trigger R","Pad N"},{}) ||
      !test({"Trigger L"},{"Buttons/-","Nunchuk/Buttons/Z","Nunchuk/Shake/X"}) ||
      !test({"Trigger L","Button A"},{"Buttons/-","Nunchuk/Buttons/Z","Buttons/A","D-Pad/Down"}) ||
      !test({"Trigger L","Button B"},{"Buttons/-","Nunchuk/Buttons/Z","Buttons/A","D-Pad/Right"}) ||
      !test({"Trigger L","Button X"},{"Buttons/-","Nunchuk/Buttons/Z","Buttons/A","D-Pad/Left"}) ||
      !test({"Trigger L","Button Y"},{"Buttons/-","Nunchuk/Buttons/Z","Buttons/A","D-Pad/Up"}) ||
      !test({"Trigger L","Right X+"},{"Nunchuk/Buttons/Z","IR/Right"}) ||
      !test({"Trigger L","Right X-","Button A"},{"Nunchuk/Buttons/Z","IR/Left","Buttons/A"}) ||
      !test({"Trigger L","Trigger R","Button A"},{"Buttons/-"}) ||
      !test({},{})) return 5;
  // The threshold is strict: exactly 0.5 leaves ordinary attacks active.
  pad->values["Button A"]=1;pad->values["Trigger R"]=0.5;
  if(expressions.at("Buttons/A")->GetValue()!=1 || expressions.at("D-Pad/Down")->GetValue()!=0) return 7;
  pad->values["Trigger R"]=0.5001;
  if(expressions.at("Buttons/A")->GetValue()!=0 || expressions.at("D-Pad/Down")->GetValue()!=1) return 8;
  if(!test({},{})) return 9;
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
