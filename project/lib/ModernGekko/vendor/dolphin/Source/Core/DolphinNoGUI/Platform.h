// Copyright 2018 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>
#include <functional>
#include <utility>
#include <string>

#include "Common/Flag.h"
#include "Common/WindowSystemInfo.h"

class Platform
{
public:
  struct InputEvent
  {
    enum class Type { Key, Focus, Pointer, PointerLeave, Wheel } type;
    unsigned key = 0;
    bool down = false, repeat = false;
    double x = 0, y = 0;
  };
  // Installed before Init; invoked only by this application's window events.
  // Returning true consumes an event before emulator keyboard shortcuts.
  void SetInputHandler(std::function<bool(const InputEvent&)> handler)
  {
    m_input_handler = std::move(handler);
  }
  virtual ~Platform();

  bool IsRunning() const { return m_running.IsSet(); }
  bool IsWindowFocused() const { return m_window_focus; }
  bool IsWindowFullscreen() const { return m_window_fullscreen; }

  virtual bool Init();
  virtual void SetTitle(const std::string& title);
  virtual void MainLoop() = 0;
  virtual void SaveWindowGeometry() {}

  virtual WindowSystemInfo GetWindowSystemInfo() const = 0;

  // Requests a graceful shutdown, from SIGINT/SIGTERM.
  void RequestShutdown();

  // Requests a slot-1 savestate save/load, from SIGUSR1/SIGUSR2 (nogui has no
  // hotkey scheduler, so signals are the external savestate trigger).
  void RequestSaveState();
  void RequestLoadState();

  // Request an immediate shutdown.
  void Stop();

  static std::unique_ptr<Platform> CreateHeadlessPlatform();
#ifdef HAVE_WAYLAND
  static std::unique_ptr<Platform> CreateWaylandPlatform();
#endif
#ifdef HAVE_X11
  static std::unique_ptr<Platform> CreateX11Platform();
#endif

#ifdef __linux__
  static std::unique_ptr<Platform> CreateFBDevPlatform();
#endif

#ifdef _WIN32
  static std::unique_ptr<Platform> CreateWin32Platform();
#endif

#ifdef __APPLE__
  static std::unique_ptr<Platform> CreateMacOSPlatform();
#endif

protected:
  bool HandleInput(const InputEvent& event)
  {
    return m_input_handler && m_input_handler(event);
  }
  std::function<bool(const InputEvent&)> m_input_handler;
  void UpdateRunningFlag();

  Common::Flag m_running{true};
  Common::Flag m_shutdown_requested{false};
  Common::Flag m_tried_graceful_shutdown{false};
  Common::Flag m_save_state_requested{false};
  Common::Flag m_load_state_requested{false};

  bool m_window_focus = true;  // Should be made atomic if actually implemented
  bool m_window_fullscreen = false;
};
