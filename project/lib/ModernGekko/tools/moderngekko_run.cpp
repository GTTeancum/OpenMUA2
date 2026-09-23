#include "cache_affinity.hpp"
#include "frontend_config.hpp"
#include "dol_patch.hpp"
#include "moderngekko/game.hpp"
#include "moderngekko/runtime.hpp"
#include "netplay_session.hpp"

#include "Common/StringUtil.h"

#include <SDL3/SDL.h>

#include <charconv>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
// For the affinity/priority setup in main(). WIN32_LEAN_AND_MEAN keeps the
// winsock and GDI surface out of a translation unit that only needs the
// processor-topology and process calls.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {
#ifndef MODERNGEKKO_RUNNER_NAME
#define MODERNGEKKO_RUNNER_NAME "moderngekko-run"
#endif

#ifndef MODERNGEKKO_USER_DIRECTORY_NAME
#define MODERNGEKKO_USER_DIRECTORY_NAME "moderngekko"
#endif

volatile std::sig_atomic_t s_stop_requested = 0;

void HandleStopSignal(int) { s_stop_requested = 1; }

void Usage() {
  std::cerr << "usage: " MODERNGEKKO_RUNNER_NAME
               " [--game <extracted-root>] [--module <path>]\n"
               "       [--user-dir <path>] [--title <text>] [--load-state <path>]\n"
               "       [--graphics <backend>] [--audio <backend>]\n"
               "       [--mods <directory>] [--no-mods]\n"
               "       [--wayland] [-X11] [--headless] [--allow-interpreter]\n"
               "       [--controller-diagnostics] [--controller-diagnostics-seconds <n>]\n"
               "       [--controller-diagnostics-rumble]\n"
               "       [--netplay-host | --netplay-join <host>] "
               "[--netplay-port <port>]\n"
               "       [--nickname <name>] [--buffer <auto|1-20>] "
               "[--controller <device>]...\n"
               "       With no --game, boots the path in "
               "<user-dir>/default-game.txt.\n";
}

struct ControllerDiagnosticsOptions {
  bool enabled = false;
  bool rumble = false;
  double seconds = 5.0;
};

struct ControllerOption {
  std::string label;
  std::string device;
  SDL_JoystickID joystick_id = 0;
  bool gamepad = false;
};

std::vector<ControllerOption> EnumerateControllers() {
  std::vector<ControllerOption> result;
  std::unordered_map<std::string, int> device_ids;
  int count = 0;
  SDL_JoystickID *joystick_ids = SDL_GetJoysticks(&count);
  for (int i = 0; i < count; ++i) {
    const SDL_JoystickID joystick_id = joystick_ids[i];
    const bool gamepad = SDL_IsGamepad(joystick_id);
    const char *name_value =
        gamepad ? SDL_GetGamepadNameForID(joystick_id)
                : SDL_GetJoystickNameForID(joystick_id);
    const std::string name =
        name_value && *name_value ? name_value : "Unknown Controller";
    const int device_id = device_ids[name]++;
    ControllerOption option;
    option.label = device_id == 0 ? name : name + " (" + std::to_string(device_id + 1) + ")";
    option.device = "SDL/" + std::to_string(device_id) + "/" + name;
    option.joystick_id = joystick_id;
    option.gamepad = gamepad;
    result.emplace_back(std::move(option));
  }
  SDL_free(joystick_ids);
  return result;
}

int RunControllerDiagnostics(const ControllerDiagnosticsOptions &options) {
  if (!SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC)) {
    std::cerr << "controller diagnostics: SDL_Init failed: " << SDL_GetError() << '\n';
    return 1;
  }

  const auto controllers = EnumerateControllers();
  std::cout << "controller diagnostics: joystick_count=" << controllers.size() << '\n';
  for (std::size_t i = 0; i < controllers.size(); ++i) {
    const ControllerOption &controller = controllers[i];
    std::cout << "controller[" << i << "]: device=\"" << controller.device
              << "\" label=\"" << controller.label << "\" gamepad="
              << (controller.gamepad ? "true" : "false") << '\n';
  }

  if (controllers.empty()) {
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC);
    return 0;
  }

  const ControllerOption *selected = nullptr;
  for (const ControllerOption &controller : controllers) {
    if (controller.gamepad) {
      selected = &controller;
      break;
    }
  }
  if (!selected) {
    std::cout << "controller diagnostics: no SDL gamepad-compatible controller found\n";
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC);
    return 0;
  }

  SDL_Gamepad *gamepad = SDL_OpenGamepad(selected->joystick_id);
  if (!gamepad) {
    std::cerr << "controller diagnostics: SDL_OpenGamepad failed: " << SDL_GetError() << '\n';
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC);
    return 1;
  }

  char *mapping = SDL_GetGamepadMapping(gamepad);
  std::cout << "controller diagnostics: selected=\"" << selected->device << "\"\n";
  std::cout << "controller diagnostics: mapping=\""
            << (mapping && *mapping ? mapping : "<none>") << "\"\n";
  SDL_free(mapping);

  if (options.rumble) {
    const bool rumble_ok = SDL_RumbleGamepad(gamepad, 0x4000, 0x6000, 250);
    std::cout << "controller diagnostics: rumble="
              << (rumble_ok ? "requested" : SDL_GetError()) << '\n';
  }

  std::cout << "controller diagnostics: sampling_seconds=" << options.seconds << '\n';
  std::vector<Sint16> last_axes(SDL_GAMEPAD_AXIS_COUNT);
  std::vector<bool> last_buttons(SDL_GAMEPAD_BUTTON_COUNT);
  std::vector<bool> seen_axes(SDL_GAMEPAD_AXIS_COUNT);
  std::vector<bool> seen_buttons(SDL_GAMEPAD_BUTTON_COUNT);
  bool guide_seen = false;
  const auto start = std::chrono::steady_clock::now();
  while (std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() <
         options.seconds) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
    }
    SDL_UpdateGamepads();
    for (int axis = 0; axis < SDL_GAMEPAD_AXIS_COUNT; ++axis) {
      const auto sdl_axis = static_cast<SDL_GamepadAxis>(axis);
      const Sint16 value = SDL_GetGamepadAxis(gamepad, sdl_axis);
      if (std::abs(static_cast<int>(value)) > 16384)
        seen_axes[axis] = true;
      if (std::abs(static_cast<int>(value) - static_cast<int>(last_axes[axis])) > 4096) {
        last_axes[axis] = value;
        std::cout << "axis " << SDL_GetGamepadStringForAxis(sdl_axis)
                  << '=' << value << '\n';
      }
    }
    for (int button = 0; button < SDL_GAMEPAD_BUTTON_COUNT; ++button) {
      const auto sdl_button = static_cast<SDL_GamepadButton>(button);
      const bool pressed = SDL_GetGamepadButton(gamepad, sdl_button);
      if (pressed) {
        seen_buttons[button] = true;
        if (sdl_button == SDL_GAMEPAD_BUTTON_GUIDE)
          guide_seen = true;
      }
      if (pressed != last_buttons[button]) {
        last_buttons[button] = pressed;
        std::cout << "button " << SDL_GetGamepadStringForButton(sdl_button)
                  << '=' << (pressed ? "down" : "up") << '\n';
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  const auto print_axis = [&](const char *label, SDL_GamepadAxis axis) {
    std::cout << "controller diagnostics: expected-axis " << label << '='
              << (seen_axes[axis] ? "seen" : "missing") << '\n';
  };
  const auto print_button = [&](const char *label, SDL_GamepadButton button) {
    std::cout << "controller diagnostics: expected-button " << label << '='
              << (seen_buttons[button] ? "seen" : "missing") << '\n';
  };
  print_axis("Left Stick X", SDL_GAMEPAD_AXIS_LEFTX);
  print_axis("Left Stick Y", SDL_GAMEPAD_AXIS_LEFTY);
  print_axis("Right Stick X", SDL_GAMEPAD_AXIS_RIGHTX);
  print_axis("Right Stick Y", SDL_GAMEPAD_AXIS_RIGHTY);
  print_axis("LT", SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
  print_axis("RT", SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
  print_button("A", SDL_GAMEPAD_BUTTON_SOUTH);
  print_button("B", SDL_GAMEPAD_BUTTON_EAST);
  print_button("X", SDL_GAMEPAD_BUTTON_WEST);
  print_button("Y", SDL_GAMEPAD_BUTTON_NORTH);
  print_button("LB", SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
  print_button("RB", SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
  print_button("Start", SDL_GAMEPAD_BUTTON_START);
  print_button("Back/View", SDL_GAMEPAD_BUTTON_BACK);
  print_button("D-pad Up", SDL_GAMEPAD_BUTTON_DPAD_UP);
  print_button("D-pad Down", SDL_GAMEPAD_BUTTON_DPAD_DOWN);
  print_button("D-pad Left", SDL_GAMEPAD_BUTTON_DPAD_LEFT);
  print_button("D-pad Right", SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
  print_button("Left Stick Click", SDL_GAMEPAD_BUTTON_LEFT_STICK);
  print_button("Right Stick Click", SDL_GAMEPAD_BUTTON_RIGHT_STICK);
  std::cout << "controller diagnostics: ignored-button Guide/Home="
            << (guide_seen ? "seen-unmapped" : "not-pressed")
            << " (Wii Close Game is intentionally not bound)\n";

  SDL_CloseGamepad(gamepad);
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC);
  return 0;
}

std::filesystem::path
ReadDefaultGame(const std::filesystem::path &user_directory,
                const std::filesystem::path &executable_directory) {
#ifdef MODERNGEKKO_PORTABLE_DEFAULT_GAME
  const std::filesystem::path default_file =
      executable_directory / "default-game.txt";
#else
  const std::filesystem::path default_file =
      user_directory / "default-game.txt";
#endif
  std::ifstream file(default_file);
  std::string path;
  std::getline(file, path);
  if (!path.empty() && path.back() == '\r')
    path.pop_back();
  std::filesystem::path result(path);
  if (result.is_relative())
    result = executable_directory / result;
  return result;
}

std::filesystem::path DefaultUserDirectory() {
#ifdef MODERNGEKKO_USER_DIRECTORY_IN_DOCUMENTS
#if defined(_WIN32)
  if (const char *user_profile = std::getenv("USERPROFILE"))
    return std::filesystem::path(user_profile) / "Documents" /
           MODERNGEKKO_USER_DIRECTORY_NAME;
#endif
  if (const char *home = std::getenv("HOME"))
    return std::filesystem::path(home) / "Documents" /
           MODERNGEKKO_USER_DIRECTORY_NAME;
#endif
#if defined(_WIN32)
  if (const char *local_app_data = std::getenv("LOCALAPPDATA"))
    return std::filesystem::path(local_app_data) /
           MODERNGEKKO_USER_DIRECTORY_NAME;
#endif
  if (const char *xdg = std::getenv("XDG_DATA_HOME"))
    return std::filesystem::path(xdg) / MODERNGEKKO_USER_DIRECTORY_NAME;
  if (const char *home = std::getenv("HOME"))
    return std::filesystem::path(home) / ".local" / "share" /
           MODERNGEKKO_USER_DIRECTORY_NAME;
  return std::string(MODERNGEKKO_USER_DIRECTORY_NAME) + "-user";
}

std::string LibrarySuffix() {
#if defined(_WIN32)
  return ".dll";
#elif defined(__APPLE__)
  return ".dylib";
#else
  return ".so";
#endif
}

std::filesystem::path ExecutableDirectory(const char *argv0) {
  std::error_code ec;
#if defined(__linux__)
  const std::filesystem::path proc_executable =
      std::filesystem::read_symlink("/proc/self/exe", ec);
  if (!ec)
    return proc_executable.parent_path();
  ec.clear();
#endif
  const std::filesystem::path executable =
      std::filesystem::weakly_canonical(argv0, ec);
  return ec ? std::filesystem::current_path() : executable.parent_path();
}
} // namespace

int RunMain(int argc, char **argv) {
#if defined(_WIN32)
  if (!std::getenv("SDL_GAMECONTROLLER_ALLOW_STEAM_VIRTUAL_GAMEPAD"))
    _putenv_s("SDL_GAMECONTROLLER_ALLOW_STEAM_VIRTUAL_GAMEPAD", "1");
#else
  setenv("SDL_GAMECONTROLLER_ALLOW_STEAM_VIRTUAL_GAMEPAD", "1", 0);
#endif
  moderngekko::RuntimeConfig config;
  config.user_directory = DefaultUserDirectory();
  const std::filesystem::path executable_directory =
      ExecutableDirectory(argv[0]);
#ifdef MODERNGEKKO_DEFAULT_WINDOW_TITLE
  config.window_title = MODERNGEKKO_DEFAULT_WINDOW_TITLE;
#endif
  std::filesystem::path module_path;
  bool use_default_mods = true;
  std::optional<moderngekko::frontend::NetplayRole> netplay_role;
  std::string netplay_address;
  std::optional<std::uint16_t> netplay_port;
  std::string netplay_nickname;
  std::string netplay_buffer;
  std::vector<std::string> netplay_controllers;
  ControllerDiagnosticsOptions controller_diagnostics;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    const auto value = [&](const char *option) -> const char * {
      if (i + 1 >= argc) {
        std::cerr << option << " requires a value\n";
        std::exit(2);
      }
      return argv[++i];
    };
    if (arg == "--game")
      config.game_root = value("--game");
    else if (arg == "--module")
      module_path = value("--module");
    else if (arg == "--user-dir")
      config.user_directory = value("--user-dir");
    else if (arg == "--title")
      config.window_title = value("--title");
    else if (arg == "--load-state")
    {
      // Checked here rather than left to the boot path: a state that is not
      // there would otherwise boot to the title screen and look like it worked.
      std::filesystem::path state = StringToPath(value("--load-state"));
      if (!std::filesystem::is_regular_file(state))
      {
        std::cerr << "savestate not found: " << PathToString(state) << '\n';
        return 2;
      }
      config.load_state_path = std::move(state);
    }
    else if (arg == "--graphics")
      config.graphics.backend = value("--graphics");
    else if (arg == "--audio")
      config.audio.backend = value("--audio");
    else if (arg == "--mods")
      config.mod_directories.emplace_back(value("--mods"));
    else if (arg == "--no-mods")
      use_default_mods = false;
    else if (arg == "-X11" || arg == "--x11")
      config.window_system = moderngekko::WindowSystem::X11;
    else if (arg == "--wayland")
      config.window_system = moderngekko::WindowSystem::Wayland;
    else if (arg == "--mute")
      config.audio.mute = true;
    else if (arg == "--automation-dir")
      config.automation.directory = value("--automation-dir");
    else if (arg == "--headless")
      config.headless = true;
    else if (arg == "--allow-interpreter")
      config.allow_interpreter = true;
    else if (arg == "--controller-diagnostics")
      controller_diagnostics.enabled = true;
    else if (arg == "--controller-diagnostics-rumble")
      controller_diagnostics.rumble = true;
    else if (arg == "--controller-diagnostics-seconds") {
      const std::string seconds_value = value("--controller-diagnostics-seconds");
      const auto parsed = std::from_chars(
          seconds_value.data(), seconds_value.data() + seconds_value.size(),
          controller_diagnostics.seconds);
      if (parsed.ec != std::errc{} ||
          parsed.ptr != seconds_value.data() + seconds_value.size() ||
          controller_diagnostics.seconds < 0.0 ||
          controller_diagnostics.seconds > 120.0) {
        std::cerr << "--controller-diagnostics-seconds must be between 0 and 120\n";
        return 2;
      }
    }
    else if (arg == "--netplay-host")
      netplay_role = moderngekko::frontend::NetplayRole::Host;
    else if (arg == "--netplay-join") {
      netplay_role = moderngekko::frontend::NetplayRole::Join;
      netplay_address = value("--netplay-join");
    } else if (arg == "--netplay-port") {
      const std::string port_value = value("--netplay-port");
      unsigned int port = 0;
      const auto parsed = std::from_chars(
          port_value.data(), port_value.data() + port_value.size(), port);
      if (parsed.ec != std::errc{} ||
          parsed.ptr != port_value.data() + port_value.size() || port == 0 ||
          port > 65535) {
        std::cerr << "--netplay-port must be between 1 and 65535\n";
        return 2;
      }
      netplay_port = static_cast<std::uint16_t>(port);
    } else if (arg == "--nickname")
      netplay_nickname = value("--nickname");
    else if (arg == "--buffer")
      netplay_buffer = value("--buffer");
    else if (arg == "--controller")
      netplay_controllers.emplace_back(value("--controller"));
    else if (arg == "--help" || arg == "-h") {
      Usage();
      return 0;
    } else {
      std::cerr << "unknown option: " << arg << '\n';
      Usage();
      return 2;
    }
  }
  if (controller_diagnostics.enabled)
    return RunControllerDiagnostics(controller_diagnostics);
  if (config.game_root.empty())
    config.game_root =
        ReadDefaultGame(config.user_directory, executable_directory);
  if (config.game_root.empty()) {
    std::cerr << "no game configured; use --game once or create "
              << (config.user_directory / "default-game.txt") << '\n';
    Usage();
    return 2;
  }

  auto frontend_config =
      moderngekko::frontend::LoadConfig(config.user_directory, true);
  if (!frontend_config) {
    std::cerr << "invalid config.ini: " << frontend_config.error << '\n';
    return 2;
  }
  config.graphics.internal_resolution_scale = frontend_config.dolphin_scale;
  if (config.graphics.backend.empty())
    config.graphics.backend = frontend_config.graphics_backend;
  config.fullscreen = frontend_config.fullscreen;
  config.show_fps_in_title = frontend_config.show_fps_in_title;
  if (use_default_mods) {
    config.mod_directories.push_back(executable_directory / "Mods");
    config.mod_directories.push_back(config.user_directory / "Mods");
  }

  if (!netplay_role && !frontend_config.controller.empty()) {
    std::string controller_message;
    if (!moderngekko::frontend::EnsureControllerConfig(
            config.user_directory, frontend_config.controller,
            &controller_message)) {
      std::cerr << "controller configuration: " << controller_message << '\n';
      return 2;
    }
    std::cout << "controller configuration: " << controller_message << '\n';
  }

#ifdef MODERNGEKKO_DOL_PATCH_MANIFEST
  bool dol_changed = false;
  std::string dol_patch_error;
  if (!moderngekko::frontend::ApplyDolPatchManifest(
          config.game_root / "sys" / "main.dol",
          executable_directory / MODERNGEKKO_DOL_PATCH_MANIFEST, &dol_changed,
          &dol_patch_error)) {
    std::cerr << "DOL patching failed: " << dol_patch_error << '\n';
    return 2;
  }
  if (dol_changed)
    std::cout << "Applied native DOL patches\n";
#endif
  const auto inspected = moderngekko::InspectGame(config.game_root);
  if (!inspected) {
    std::cerr << "invalid game: " << inspected.error << '\n';
    return 2;
  }

#ifdef MODERNGEKKO_REQUIRED_DISC_ID
  if (inspected.metadata->disc_id != MODERNGEKKO_REQUIRED_DISC_ID) {
    std::cerr << "unsupported disc ID: expected "
              << MODERNGEKKO_REQUIRED_DISC_ID << ", got "
              << inspected.metadata->disc_id << '\n';
    return 2;
  }
#endif
#ifdef MODERNGEKKO_REQUIRED_DOL_SHA256
  if (inspected.metadata->dol_sha256 != MODERNGEKKO_REQUIRED_DOL_SHA256) {
    std::cerr << "unsupported main DOL: this release requires its pinned game build\n";
    return 2;
  }
#endif
#ifdef MODERNGEKKO_REQUIRED_REL_SHA256
  if (inspected.metadata->rel_sha256 != MODERNGEKKO_REQUIRED_REL_SHA256) {
    std::cerr << "unsupported _Main.rel: this release requires its pinned game build\n";
    return 2;
  }
#endif
#ifdef MODERNGEKKO_REQUIRED_ASSETS_SHA256
  if (inspected.metadata->assets_sha256 !=
      MODERNGEKKO_REQUIRED_ASSETS_SHA256) {
    std::cerr << "unsupported game assets: this release requires its pinned game build\n";
    return 2;
  }
#endif

  // Compatibility discovery belongs to the runner, never the runtime library.
  if (module_path.empty()) {
    if (const char *env = std::getenv("STATICRECOMP_MODULE"))
      module_path = env;
    else {
      const std::string module_name =
          "g" + inspected.metadata->disc_id + "_recomp" + LibrarySuffix();
      const auto bundled = executable_directory / module_name;
      const auto user_module =
          config.user_directory / "StaticRecompModules" / module_name;
      if (std::filesystem::is_regular_file(bundled))
        module_path = bundled;
      else if (std::filesystem::is_regular_file(user_module))
        module_path = user_module;
    }
  }
  if (!module_path.empty())
    config.module =
        moderngekko::ModuleSource::DynamicPath(std::move(module_path));

#if defined(__linux__) || defined(_WIN32)
  if (!config.headless && config.graphics.backend.empty())
    config.graphics.backend = "Vulkan";
#endif

  if (netplay_role) {
    moderngekko::frontend::NetplayOptions options;
    options.role = *netplay_role;
    options.address = netplay_address.empty() ? frontend_config.netplay_address
                                              : netplay_address;
    options.port = netplay_port.value_or(frontend_config.netplay_port);
    options.nickname = netplay_nickname.empty()
                           ? frontend_config.netplay_nickname
                           : netplay_nickname;
    options.buffer = netplay_buffer.empty() ? frontend_config.netplay_buffer
                                            : netplay_buffer;
    const std::vector<std::string> configured_controllers =
        moderngekko::frontend::ReadConfiguredControllers(config.user_directory);
    options.controllers =
        netplay_controllers.empty()
            ? (configured_controllers.empty() ? frontend_config.controllers
                                              : configured_controllers)
            : netplay_controllers;
    if (options.controllers.empty() && !frontend_config.controller.empty())
      options.controllers.push_back(frontend_config.controller);
    if (options.controllers.empty()) {
      std::cerr << "netplay requires at least one selected controller\n";
      return 2;
    }
    frontend_config.netplay_address = options.address;
    frontend_config.netplay_port = options.port;
    frontend_config.netplay_nickname = options.nickname;
    frontend_config.netplay_buffer = options.buffer;
    frontend_config.controllers = options.controllers;
    frontend_config.controller = options.controllers.front();
    std::string controller_message;
    if (!moderngekko::frontend::EnsureControllerConfig(
            config.user_directory, options.controllers, &controller_message)) {
      std::cerr << "controller configuration: " << controller_message << '\n';
      return 2;
    }
    std::string save_error;
    if (!moderngekko::frontend::SaveConfig(config.user_directory,
                                           frontend_config, &save_error)) {
      std::cerr << "configuration: " << save_error << '\n';
      return 2;
    }
    std::cerr << "netplay: runner started\n";
    return moderngekko::frontend::RunNetplayLobby(
        std::move(config), std::move(frontend_config), std::move(options));
  }

  auto created = moderngekko::Runtime::Create(std::move(config));
  if (!created) {
    std::cerr << "initialization failed: " << created.error->message << '\n';
    return 1;
  }
  std::cout << "audio backend: " << created.runtime->GetConfig().audio.backend
            << '\n';

  std::signal(SIGINT, HandleStopSignal);
  std::signal(SIGTERM, HandleStopSignal);
  std::jthread signal_watcher([&](std::stop_token stop_token) {
    while (!stop_token.stop_requested()) {
      if (s_stop_requested) {
        s_stop_requested = 0;
        created.runtime->RequestStop();
        return;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  });
  const moderngekko::RuntimeRunResult result = created.runtime->Run();
  signal_watcher.request_stop();
  if (result.error) {
    std::cerr << "runtime failed: " << result.error->message << '\n';
    return 1;
  }
  return 0;
}

#if defined(_WIN32)
// Optionally confine the process to the cores that share the largest L3.
//
// The emulated CPU is a single serial instruction stream, so nothing here is
// about parallelism -- core usage stays at 1.00 either way. It is about cache
// residency. A recompiled module is one dispatch switch spanning the whole
// game, which lives or dies on staying in L3, and on a chip with 3D V-Cache on
// one CCD only, Windows migrating the thread between dies makes it repeatedly
// lose its working set. Measured on a 9950X3D: 55.41 -> 70.42 fps, +27%.
//
// This applies to every Dolphin thread, not only the emulated CPU thread, so it
// is opt-in. Set MODERNGEKKO_CACHE_AFFINITY=1 to enable it after benchmarking
// the target machine.
void PinProcessToLargestCache() {
  if (!moderngekko::frontend::AffinityEnabled(
          std::getenv("MODERNGEKKO_CACHE_AFFINITY")))
    return;

  DWORD length = 0;
  GetLogicalProcessorInformationEx(RelationCache, nullptr, &length);
  if (length == 0)
    return;
  std::vector<char> buffer(length);
  if (!GetLogicalProcessorInformationEx(
          RelationCache,
          reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()), &length))
    return;

  const moderngekko::frontend::CacheDomain domain =
      moderngekko::frontend::LargestSharedCache(buffer.data(), length);
  if (!domain)
    return;

  if (moderngekko::frontend::ApplyCacheDomain(GetCurrentProcess(), domain).affinity_set) {
    std::cout << "[perf] pinned to the cores sharing the largest L3 (" << (domain.size >> 20)
              << " MB), mask 0x" << std::hex << static_cast<unsigned long long>(domain.mask)
              << std::dec << '\n';
  }
}
#endif

int main(int argc, char **argv) {
  try {
#if defined(_WIN32)
    PinProcessToLargestCache();
#endif
    return RunMain(argc, argv);
  } catch (const std::exception &error) {
    std::cerr << "fatal error: " << error.what() << '\n';
  } catch (...) {
    std::cerr << "fatal error: unknown exception\n";
  }
  return 1;
}
