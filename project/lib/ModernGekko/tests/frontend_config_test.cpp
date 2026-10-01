#include "frontend_config.hpp"
#include "legacy_xbox_profiles.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#ifdef MODERNGEKKO_GAMECUBE_CONTROLLERS
constexpr const char *CONTROLLER_CONFIG_NAME = "GCPadNew.ini";
#else
constexpr const char *CONTROLLER_CONFIG_NAME = "WiimoteNew.ini";
#endif

int main() {
  namespace fs = std::filesystem;
  const fs::path directory =
      fs::temp_directory_path() /
      ("moderngekko-frontend-config-" +
       std::to_string(
           std::chrono::steady_clock::now().time_since_epoch().count()));

  std::string error;
  const std::string controller = "SDL/0/Test Controller";
  if (!moderngekko::frontend::SaveConfig(directory, "1920x1080", false,
                                         controller, &error))
    return 1;

  const auto loaded = moderngekko::frontend::LoadConfig(directory, false);
  if (!loaded || loaded.dolphin_scale != 3 || loaded.show_fps_in_title ||
      loaded.controller != controller ||
      loaded.graphics_backend != "Vulkan") {
    return 2;
  }

  moderngekko::frontend::ConfigResult netplay_config = loaded;
  netplay_config.graphics_backend = "OpenGL";
  netplay_config.fullscreen = true;
  netplay_config.controllers = {controller, "SDL/1/Second Controller"};
  netplay_config.controller = controller;
  netplay_config.netplay_nickname = "Kirby";
  netplay_config.netplay_address = "192.168.1.50";
  netplay_config.netplay_port = 34567;
  netplay_config.netplay_buffer = "auto";
  if (!moderngekko::frontend::SaveConfig(directory, netplay_config, &error))
    return 6;
  const auto netplay_loaded =
      moderngekko::frontend::LoadConfig(directory, false);
  if (!netplay_loaded ||
      netplay_loaded.controllers != netplay_config.controllers ||
      netplay_loaded.netplay_nickname != "Kirby" ||
      netplay_loaded.netplay_address != "192.168.1.50" ||
      netplay_loaded.netplay_port != 34567 ||
      netplay_loaded.netplay_buffer != "auto" ||
      netplay_loaded.graphics_backend != "OGL" ||
      !netplay_loaded.fullscreen) {
    return 7;
  }

  auto invalid_netplay = netplay_config;
  invalid_netplay.netplay_address = "not a host";
  if (moderngekko::frontend::SaveConfig(directory, invalid_netplay, &error))
    return 8;
  invalid_netplay = netplay_config;
  invalid_netplay.netplay_nickname = std::string(31, 'K');
  if (moderngekko::frontend::SaveConfig(directory, invalid_netplay, &error))
    return 9;
  invalid_netplay = netplay_config;
  invalid_netplay.graphics_backend = "Direct3D 9";
  if (moderngekko::frontend::SaveConfig(directory, invalid_netplay, &error))
    return 13;
  if (!moderngekko::frontend::GenerateControllerConfig(
          directory, netplay_config.controllers, &error))
    return 3;
  if (moderngekko::frontend::ReadConfiguredController(directory) != controller)
    return 4;
  if (moderngekko::frontend::ReadConfiguredControllers(directory) !=
      netplay_config.controllers)
    return 10;

  // Scoped so the handle is closed before the cleanup below. Windows refuses
  // to delete a file that is still open, where POSIX allows it, so leaving
  // these open makes remove_all throw there and only there.
  std::string generated;
  {
    std::ifstream input(directory / "Config" / CONTROLLER_CONFIG_NAME);
    generated.assign(std::istreambuf_iterator<char>(input),
                     std::istreambuf_iterator<char>());
  }
#ifdef MODERNGEKKO_GAMECUBE_CONTROLLERS
  if (!generated.contains("Buttons/A = `Button A`\n") ||
      !generated.contains("Buttons/Z = `Shoulder R`\n") ||
      !generated.contains("Main Stick/Up = `Left Y+`\n") ||
      !generated.contains("C-Stick/Up = `Right Y+`\n") ||
      !generated.contains("Triggers/L-Analog = `Trigger L`\n") ||
      !generated.contains("Rumble/Motor = `Motor L` | `Motor R`\n") ||
      !generated.contains("[GCPad2]\nDevice = SDL/1/Second Controller\n") ||
      generated.contains("[Wiimote") || generated.contains("[BalanceBoard]")) {
    return 5;
  }
#else
  if (!generated.contains("# OpenMUA2 Xbox action layout v5\n") ||
      !generated.contains("Buttons/2 = Start | `Back`\n") ||
      !generated.contains("Buttons/Home =\n") ||
      !generated.contains("Extension = Nunchuk\n") ||
      !generated.contains("Nunchuk/Buttons/C = !(`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & `Button Y`\n") ||
      !generated.contains("Nunchuk/Buttons/Z = (`Shoulder L` | (`Trigger L` > 0.5)) & !(`Trigger R` > 0.5)\n") ||
      !generated.contains("Nunchuk/Stick/Up = `Left Y+`\n") ||
      !generated.contains("Nunchuk/Shake/X = (`Trigger L` > 0.5) & !(`Trigger R` > 0.5) & !(`Button A` | `Button B` | `Button X` | `Button Y`) & !") ||
      !generated.contains("Options/Sideways Wiimote = False\n") ||
      !generated.contains("[Wiimote2]\nDevice = SDL/1/Second Controller\n") ||
      generated.contains("Buttons/Home = Guide\n")) {
    return 5;
  }
#endif

#ifdef MODERNGEKKO_GAMECUBE_CONTROLLERS
  const std::string custom =
      "[GCPad1]\nDevice = SDL/9/Custom Controller\nButtons/A = Custom\n";
#else
  const std::string custom =
      "[Wiimote1]\nDevice = SDL/9/Custom Controller\nButtons/1 = Custom\n";
#endif
  {
    std::ofstream output(directory / "Config" / CONTROLLER_CONFIG_NAME,
                         std::ios::trunc);
    output << custom;
  }
  if (!moderngekko::frontend::EnsureControllerConfig(
          directory, netplay_config.controllers, &error))
    return 11;
  std::string preserved;
  {
    std::ifstream custom_input(directory / "Config" / CONTROLLER_CONFIG_NAME);
    preserved.assign(std::istreambuf_iterator<char>(custom_input),
                     std::istreambuf_iterator<char>());
  }
  if (preserved != custom || moderngekko::frontend::ReadConfiguredController(
                                 directory) != "SDL/9/Custom Controller")
    return 12;


#ifndef MODERNGEKKO_GAMECUBE_CONTROLLERS
  const auto config_path = directory / "Config" / CONTROLLER_CONFIG_NAME;
  const auto read = [](const fs::path& path) {
    std::ifstream input(path); return std::string(std::istreambuf_iterator<char>(input), {});
  };
  const std::vector<std::string> fixture_devices{"SDL/0/Test Controller"};
  const auto fixture_path=fs::path(__FILE__).parent_path()/"data"/"xbox_v4_generated.ini";
  auto expected_v4=moderngekko::frontend::legacy::Profile(fixture_devices,4);
  // INI trailing whitespace has no meaning; keep the independent fixture clean.
  for(auto at=expected_v4.find(" \n");at!=std::string::npos;at=expected_v4.find(" \n"))
    expected_v4.erase(at,1);
  if(read(fixture_path)!=expected_v4) return 25;
  for (int version : {1, 2, 3, 4}) {
   for (bool persisted : {false, true}) {
    const auto original = moderngekko::frontend::legacy::Profile(netplay_config.controllers, version, persisted);
    { std::ofstream output(config_path); output << original; }
    // The devices in the old profile win over an unrelated current selection.
    if (!moderngekko::frontend::EnsureControllerConfig(directory, "SDL/9/Other", &error) ||
        !read(config_path).contains("# OpenMUA2 Xbox action layout v5") ||
        moderngekko::frontend::ReadConfiguredControllers(directory) != netplay_config.controllers)
      return 20;
    bool backup_found = false;
    for (const auto& entry : fs::directory_iterator(config_path.parent_path()))
      if (entry.path().extension() == ".bak" && read(entry.path()) == original) backup_found = true;
    if (!backup_found) return 21;
    auto customized = original + "# My personal settings\n";
    { std::ofstream output(config_path); output << customized; }
    if (!moderngekko::frontend::EnsureControllerConfig(directory, controller, &error) ||
        read(config_path) != customized) return 22;
    customized = original;
    const auto key = customized.find("Nunchuk/Stick/Dead Zone = 15.0");
    if (key == std::string::npos) return 23;
    customized.replace(key, std::string("Nunchuk/Stick/Dead Zone = 15.0").size(), "Nunchuk/Stick/Dead Zone = 22.0");
    { std::ofstream output(config_path); output << customized; }
    if (!moderngekko::frontend::EnsureControllerConfig(directory, controller, &error) ||
        read(config_path) != customized) return 24;
  }
  }
#endif
  // A shutdown-created empty profile must not suppress first-run discovery.
  // Existing nonempty/custom profiles above must still remain untouched.
  for (const auto empty : {"", " \t\r\n"}) {
    { std::ofstream out(directory / "Config" / CONTROLLER_CONFIG_NAME); out << empty; }
    if (moderngekko::frontend::ControllerConfigExists(directory)) return 26;
    if (!moderngekko::frontend::EnsureControllerConfig(directory, controller, &error) ||
        moderngekko::frontend::ReadConfiguredController(directory) != controller)
      return 27;
  }
  fs::remove_all(directory);
  return 0;
}
