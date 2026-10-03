#include "moderngekko/runtime.hpp"

#include "AudioCommon/AudioCommon.h"
#include "Common/Config/Config.h"
#include "Common/Crypto/SHA1.h"
#include "Core/HLE/HLE.h"
#include "mua2_action_bindings.hpp"
#include "managed_xbox_profile.hpp"
#include "mua2_interaction_context.hpp"
#include "moderngekko/gameplay/button_qte.hpp"
#include "mua2_fusion_buttons.hpp"
#include "mua2_hero_buttons.hpp"
#include "Common/HookableEvent.h"
#include "Common/IOFile.h"
#include "Common/StringUtil.h"
#include "Core/Boot/Boot.h"
#include "Core/Boot/BootManager.h"
#include "Core/Config/GraphicsSettings.h"
#include "Core/Config/MainSettings.h"
#include "Core/CoreTiming.h"
#include "Core/HW/SystemTimers.h"
#include "Core/Core.h"
#include "Common/RuntimeTiming.h"
#include "Core/HW/GBACore.h"
#include "Core/HW/Memmap.h"
#include "Core/HW/Wiimote.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"
#include "InputCommon/InputConfig.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "Core/Host.h"
#include "Core/PowerPC/JitCommon/JitIndirectProfile.h"
#include "Core/NetPlay/NetPlayClient.h"
#include "Core/PowerPC/JitInterface.h"
#include "Core/PowerPC/PowerPC.h"
#include "Core/PowerPC/StaticRecomp/StaticRecompModuleSource.h"
#include "Core/State.h"
#include "Core/System.h"
#include "DolphinNoGUI/Platform.h"
#include "InputCommon/ControllerEmu/ControllerEmu.h"
#include "InputCommon/ControllerInterface/Touch/InputOverrider.h"
#include "UICommon/UICommon.h"
#include "VideoCommon/FrameDumper.h"
#include "VideoCommon/PerformanceMetrics.h"
#include "VideoCommon/VideoEvents.h"
#include "VideoCommon/VideoConfig.h"
#include "automation_protocol.hpp"
#include "frame_timing.hpp"
#include "dolphin_runtime_internal.hpp"
#include "moderngekko/cpu_state.h"
#include "moderngekko/mod_loader.hpp"
#include "moderngekko/module_loader.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <fstream>
#include <fmt/format.h>
#include <mutex>
#include <stop_token>
#include <thread>
#include <utility>

namespace {
static_assert(sizeof(ModernGekkoModuleDesc) == sizeof(StaticRecompModuleDesc));
static_assert(offsetof(ModernGekkoModuleDesc, chunk_hashes) ==
              offsetof(StaticRecompModuleDesc, chunk_hashes));
bool s_fusion_buttons_enabled = false;
bool s_hero_buttons_enabled = false;
std::uint8_t s_control_ports=0, s_hero_ports=0, s_fusion_ports=0;

// Opt-in, read-only tracing at the menu action consumer (not merely the input
// producer). Captures the caller and its action set without modifying input.
void ObserveMenuActionQuery(const Core::CPUThreadGuard& guard)
{
  auto& system = guard.GetSystem();
  const auto& state = system.GetPPCState();
  const u32 action = state.gpr[4];
  if (action < 89 || action > 105) return;
  auto& memory = system.GetMemory();
  const auto* code = memory.GetPointerForRange(0x801d566c, 64);
  if (!code || Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code, 64)) !=
                   "3869ED9F484640D8E31FD8DAFB1AA0A7AEA7E863") return;
  const auto* bits = memory.GetPointerForRange(state.gpr[3], 20);
  if (!bits) return;
  static std::ofstream trace([] {
    const char* path = std::getenv("OPENMUA2_MENU_TRACE");
    return path ? path : "";
  }());
  if (!trace) return;
  const auto data = std::span<const u8>(bits, 20);
  trace << system.GetCoreTiming().GetTicks() << ',' << state.spr[8] << ','
        << state.gpr[3] << ',' << action << ','
        << ((moderngekko::controls::ReadBE(data, 4 * (action / 32)) >> (action % 32)) & 1)
        << ',' << state.gpr[29] << ',' << state.gpr[30] << ',' << state.gpr[31] << '\n';
}

void ObserveHeroCandidate(const Core::CPUThreadGuard& guard)
{
  if (!s_hero_buttons_enabled) return;
  auto& system = guard.GetSystem();
  auto& memory = system.GetMemory();
  const auto* code = memory.GetPointerForRange(0x80052fac, 256);
  if (!code || Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code, 256)) !=
                   "C60514A390882682A0BBC0896329C875B6A4AB0C") return;
  auto& state = system.GetPPCState();
  if (!moderngekko::controls::XboxPortEnabled(s_hero_ports,state.gpr[22])) return;
  if (std::uint64_t(state.gpr[1]) + 0x28 != state.gpr[5]) return;
  const auto read = [&](u32 address, std::size_t size) -> std::span<const u8> {
    const auto* bytes = memory.GetPointerForRange(address, size);
    return bytes ? std::span<const u8>(bytes, size) : std::span<const u8>{};
  };
  const auto index = moderngekko::controls::HeroButtonIndex(
      read, state.gpr[3], state.gpr[4], state.gpr[22], state.gpr[5]);
  if (index) {
    state.gpr[6] = *index;
    // A direct request gets one native attempt. Never fall through to another
    // hero if the selected hero is dead, controlled elsewhere, or ineligible.
    state.gpr[19] = 3;
  }
}

void ObserveFusionCandidate(const Core::CPUThreadGuard& guard, u32 entry,
                            std::string_view digest, unsigned owner_register,
                            unsigned input_register)
{
  if (!s_fusion_buttons_enabled) return;
  auto& system = guard.GetSystem();
  auto& memory = system.GetMemory();
  const auto* code = memory.GetPointerForRange(entry, 256);
  if (!code || Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code, 256)) != digest)
    return;
  const auto read = [&](u32 address, std::size_t size) -> std::span<const u8> {
    const auto* bytes = memory.GetPointerForRange(address, size);
    return bytes ? std::span<const u8>(bytes, size) : std::span<const u8>{};
  };
  auto& state = system.GetPPCState();
  if (!moderngekko::controls::XboxPortEnabled(s_fusion_ports,state.gpr[input_register])) return;
  const auto candidate = moderngekko::controls::FusionCandidate(
      read, state.gpr[owner_register], state.gpr[input_register]);
  if (candidate)
    state.gpr[3] = *candidate; // Original handle/type/eligibility processing follows.
}

void ObserveFusionScreenCandidate(const Core::CPUThreadGuard& guard)
{
  ObserveFusionCandidate(guard, 0x81068a60,
      "EDF55656FC8E4DEBCD9A0D75777E5D415A73D05C", 24, 26);
}
void ObserveFusionWorldCandidate(const Core::CPUThreadGuard& guard)
{
  ObserveFusionCandidate(guard, 0x810692c4,
      "74486FDF3DF1008C2A24A01D741F43DC4D9EAB06", 27, 28);
}

// The game repeatedly configures a four-minute Wii Remote idle timeout.
// Managed Xbox profiles use a persistent virtual remote. Ask the original
// setter to disable its idle timeout, preserving its critical section and
// all explicit disconnect/reconnect handling. Never write a guessed SDA slot.
void ObserveMua2AutoSleep(const Core::CPUThreadGuard& guard)
{
  if (!s_control_ports) return;
  auto& system = guard.GetSystem();
  auto& state = system.GetPPCState();
  if (state.gpr[3] == 0) return;
  const auto* code = system.GetMemory().GetPointerForRange(0x80403ae4, 52);
  if (!code || Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code, 52)) !=
                   "25A770A100B607D837FD07079F46956A62D7BB38") return;
  state.gpr[3] = 0;
  static bool reported = false;
  if (!reported)
  {
    reported = true;
    std::fprintf(stderr, "[openmua2] managed Xbox: disabled native remote idle timeout\n");
  }
}

// Direct cooperative QTE replacement for managed Xbox controls. No motion action is
// generated: button edges own progress; the existing animation/event functions
// consume that progress, including the existing mission completion callback.
struct DirectQteSession {
  moderngekko::controls::CoopInteraction context;
  moderngekko::game::ButtonQte progress;
  moderngekko::game::ButtonQte::Result result;
  bool down = false;
  double duration = 0.0;
  u64 last_update = 0;
};
bool s_direct_qte_enabled = false;
std::array<DirectQteSession, 4> s_direct_qtes;

void ResetDirectQtes() { s_direct_qtes = {}; }

bool DirectQteCode(const Core::CPUThreadGuard& guard, u32 address,
                   std::string_view expected) {
  const auto* code = guard.GetSystem().GetMemory().GetPointerForRange(address, 64);
  return code && Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code, 64)) == expected;
}

DirectQteSession* FindDirectQte(const Core::CPUThreadGuard& guard, u32 target,
                                u32 actor = 0) {
  if (!s_direct_qte_enabled) return nullptr;
  auto& memory = guard.GetSystem().GetMemory();
  const auto read = [&](u32 address, std::size_t size) -> std::span<const u8> {
    const auto* bytes = memory.GetPointerForRange(address, size);
    return bytes ? std::span<const u8>(bytes, size) : std::span<const u8>{};
  };
  for (unsigned port = 0; port < s_direct_qtes.size(); ++port) {
    auto& session = s_direct_qtes[port];
    if (!session.context.actor || session.context.target != target ||
        (actor && session.context.actor != actor)) continue;
    moderngekko::controls::CoopInteraction live;
    if (!moderngekko::controls::HasCoopInteraction(read, port, &live) ||
        live != session.context) { session = {}; continue; }
    const auto entity = read(target, 0x3b0);
    if (entity.size() != 0x3b0 ||
        moderngekko::controls::ReadBE(entity, 0x3ac) != live.actor) return nullptr;
    const float start = std::bit_cast<float>(moderngekko::controls::ReadBE(entity, 0x384));
    const float end = std::bit_cast<float>(moderngekko::controls::ReadBE(entity, 0x380));
    if (!std::isfinite(start) || !std::isfinite(end) || start < 0 ||
        start >= end || end > 1) return nullptr;
    return &session;
  }
  return nullptr;
}

void ObserveDirectQteClock(const Core::CPUThreadGuard& guard) {
  if (!s_direct_qte_enabled || !DirectQteCode(guard, 0x80ed2c48,
      "ADC2B0FAD37A0D512B5521DBA79E9B0955504CE8")) return;
  auto& system = guard.GetSystem();
  auto& state = system.GetPPCState();
  auto* session = FindDirectQte(guard, state.gpr[22], state.gpr[23]);
  if (!session) return;
  const double duration = state.ps[1].PS0AsDouble();
  if (!std::isfinite(duration) || duration <= 0 || duration > 100000) return;
  session->duration = duration;
  const auto ticks = system.GetCoreTiming().GetTicks();
  const unsigned port = static_cast<unsigned>(session - s_direct_qtes.data());
  const u64 identity = (u64(session->context.actor_handle) << 32) | session->context.target_handle;
  // A discontinuity must not convert an old held sample into a new press.
  if (session->last_update && (ticks < session->last_update ||
      ticks - session->last_update > system.GetSystemTimers().GetTicksPerSecond() / 10))
    session->progress.Update(identity, port, session->down, false);
  session->last_update = ticks;
  session->result = session->progress.Update(identity, port, session->down, true);
  if (session->result.advanced)
    std::fprintf(stderr, "[openmua2] direct QTE target=%08x port=%u presses=%u/%u complete=%u\n",
        session->context.target_handle, port, session->result.presses,
        session->result.required, unsigned(session->result.completed_now));
}

void ObserveDirectQteStage(const Core::CPUThreadGuard& guard) {
  if (!s_direct_qte_enabled || !DirectQteCode(guard, 0x80ed2e6c,
      "C57FD5D0C09C4FAA4FCFD52B2C937B47550C79FF")) return;
  auto& system = guard.GetSystem();
  const auto& state = system.GetPPCState();
  auto* session = FindDirectQte(guard, state.gpr[22], state.gpr[23]);
  if (!session || !session->result.required || session->duration <= 0) return;
  auto* entity = system.GetMemory().GetPointerForRange(state.gpr[22], 0x3b0);
  const bool complete = session->result.presses == session->result.required;
  // Replace the motion-driven stage selection. Stage 2 retains the original
  // one-shot completion script and animation/participant cleanup path.
  entity[0x3a4] = entity[0x3a5] = entity[0x3a6] = 0;
  entity[0x3a7] = complete ? 2 : 1;
  if (complete) entity[0x375] |= 0x40;
  else entity[0x375] &= ~0x40;
}

void ObserveDirectQteAnimation(const Core::CPUThreadGuard& guard) {
  if (!s_direct_qte_enabled || !DirectQteCode(guard, 0x80ed3480,
      "9D12689345E251626614B19DA2E4F227BC8EF280")) return;
  auto& system = guard.GetSystem();
  auto& state = system.GetPPCState();
  auto* session = FindDirectQte(guard, state.gpr[24]);
  if (!session || !session->result.required || session->duration <= 0) return;
  const auto* entity = system.GetMemory().GetPointerForRange(state.gpr[24], 0x3b0);
  const auto bytes = std::span<const u8>(entity, 0x3b0);
  const float start = std::bit_cast<float>(moderngekko::controls::ReadBE(bytes, 0x384));
  const float end = std::bit_cast<float>(moderngekko::controls::ReadBE(bytes, 0x380));
  const double fraction = double(session->result.presses) / session->result.required;
  // Seek participants to button-owned progress and hold between presses.
  // Completion exits through the original stage-2 animation path instead.
  state.ps[29].SetPS0(0.0);
  state.ps[30].SetPS0((start + (end - start) * fraction) * session->duration);
}

// Experimental context-use rebind: observes the original evaluator, preserving
// its instructions, action thresholds, per-player queues and timing. Runs after
// evaluation so the experimental chord can retain digital use magnitude.
void ObserveMua2InputBindings(const Core::CPUThreadGuard& guard)
{
  auto& system = guard.GetSystem();
  auto& memory = system.GetMemory();
  constexpr u32 entry = 0x810f8544;
  const auto* code = memory.GetPointerForRange(entry, 256);
  if (!code || Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code, 256)) !=
                   "E312A14ED77641256F3D5BE04DC06C21878D6CFF")
    return;
  const u32 object = system.GetPPCState().gpr[15];
  if (!moderngekko::controls::XboxPortEnabled(s_control_ports,object)) return;
  if ((object & 3) || object < 0x80000000 || object > 0x817f4200)
    return;
  auto* data = memory.GetPointerForRange(object, 0xbe00);
  if (!data || moderngekko::controls::ReadBE(std::span<const u8>(data, 4), 0) != 0x811b4398)
    return;
  const auto result = moderngekko::controls::RebindContextUse(
      std::span<u8>(data + 4, moderngekko::controls::DescriptorTableSize));
  if (result == moderngekko::controls::RebindResult::Applied)
    std::fprintf(stderr, "[openmua2] experimental X-use descriptor applied to input object %08x\n", object);
  // Rebinding occurs after this evaluation. Its first output still uses the
  // stock descriptor; normalize only subsequent evaluations of our chord.
  if (result != moderngekko::controls::RebindResult::AlreadyApplied)
    return;
  const auto& state = system.GetPPCState();
  constexpr u32 value_bytes = 124 * 4;
  const u32 active_address = state.gpr[14];
  if (state.gpr[17] < value_bytes)
    return;
  const u32 values_address = state.gpr[17] - value_bytes;
  const auto ram_range = [](u32 address, u32 size) {
    return !(address & 3) &&
        ((address >= 0x80000000 && address <= 0x81800000 - size) ||
         (address >= 0x90000000 && address <= 0x94000000 - size));
  };
  const auto overlaps = [](u32 a, u32 n, u32 b, u32 m) {
    return std::uint64_t(a) < std::uint64_t(b) + m &&
           std::uint64_t(b) < std::uint64_t(a) + n;
  };
  if (!ram_range(active_address, 20) || !ram_range(values_address, value_bytes) ||
      overlaps(active_address, 20, values_address, value_bytes) ||
      overlaps(values_address, value_bytes, object, 4 + moderngekko::controls::DescriptorTableSize))
    return;
  auto* active = memory.GetPointerForRange(active_address, 20);
  auto* values = memory.GetPointerForRange(values_address, value_bytes);
  // Opt-in chronological samples avoid mistaking a later cleared action mask
  // for an input that never reached the game. Only the guarded title observer
  // writes this diagnostic; it never changes guest input or timing.
  static std::ofstream input_timing([] {
    const char* path = std::getenv("OPENMUA2_INPUT_TIMING");
    return path ? path : "";
  }());
  if (input_timing && active && values) {
    const auto a = std::span<const u8>(active, 20);
    input_timing << system.GetCoreTiming().GetTicks() << ','
        << Common::RuntimeTiming::Now() << ',' << object;
    for (std::size_t i = 0; i < 5; ++i)
      input_timing << ',' << moderngekko::controls::ReadBE(a, i * 4);
    input_timing << '\n';
  }
  if (!active || !values ||
      !moderngekko::controls::NormalizeChordUse(std::span<const u8>(active, 20),
                                               std::span<u8>(values, value_bytes)))
    return;
  constexpr u32 first_input = 0x81313274, input_stride = 0xbe00;
  if (object < first_input || object >= first_input + 4 * input_stride ||
      (object - first_input) % input_stride)
    return;
  const auto read = [&](u32 address, std::size_t size) -> std::span<const u8> {
    const auto* bytes = memory.GetPointerForRange(address, size);
    return bytes ? std::span<const u8>(bytes, size) : std::span<const u8>{};
  };
  if (moderngekko::controls::XboxPortEnabled(s_hero_ports,object))
    moderngekko::controls::MapHeroManagement(
        std::span<u8>(active, 20), std::span<u8>(values, value_bytes));
  if (moderngekko::controls::XboxPortEnabled(s_hero_ports,object))
    moderngekko::controls::ConsumeCameraMenuAliases(
        std::span<u8>(active, 20), std::span<u8>(values, value_bytes));
  if (moderngekko::controls::XboxPortEnabled(s_hero_ports,object))
    moderngekko::controls::MapStartButton(
        std::span<u8>(active, 20), std::span<u8>(values, value_bytes),
        moderngekko::controls::IsStartAcceptScreen(read));
  if (moderngekko::controls::XboxPortEnabled(s_fusion_ports,object) &&
      moderngekko::controls::ReadBE(std::span<const u8>(values, value_bytes), 14 * 4) == 0x3f800000)
  {
    bool request_context = false;
    const auto globals = read(0x80816a20, 24);
    if (globals.size() == 24 && moderngekko::controls::ReadBE(globals, 0) == 1)
    {
      const u32 owner = moderngekko::controls::ReadBE(globals, 16);
      if (!(owner & 3) && ((owner >= 0x80000000 && owner <= 0x817ff5c8) ||
                          (owner >= 0x90000000 && owner <= 0x93fff5c8)))
      {
        const auto actor = read(owner, 0xa38);
        request_context = actor.size() == 0xa38 &&
            moderngekko::controls::ReadBE(actor, 0x9c) == 0x8052a748 &&
            !(actor[0x4d0] & 0x80) && moderngekko::controls::ReadBE(actor, 0x45c) ==
                (object - first_input) / input_stride;
      }
    }
    moderngekko::controls::ConsumeFusionMarkers(
        std::span<u8>(active, 20), std::span<const u8>(values, value_bytes), request_context);
    return;
  }
  if (moderngekko::controls::XboxPortEnabled(s_hero_ports,object))
    moderngekko::controls::ConsumeHeroMarkers(
        std::span<u8>(active, 20), std::span<const u8>(values, value_bytes));
  const unsigned port = (object - first_input) / input_stride;
  moderngekko::controls::CoopInteraction live;
  const bool context = moderngekko::controls::HasCoopInteraction(read, port, &live);
  if (s_direct_qte_enabled) {
    auto& session = s_direct_qtes[port];
    if (!context) { session = {}; return; }
    const auto input = moderngekko::controls::ConsumeButtonQteInput(
        std::span<u8>(active, 20), std::span<u8>(values, value_bytes));
    if (!input) { session = {}; return; }
    const bool down = *input;
    if (session.context != live) {
      session = {};
      session.context = live;
      // Twelve separate presses. Holding never repeats.
      session.progress.Begin((u64(live.actor_handle) << 32) | live.target_handle, port, 12, down);
      std::fprintf(stderr, "[openmua2] direct QTE begin target=%08x port=%u\n", live.target_handle, port);
    }
    session.down = down;
  }

}

std::mutex s_runtime_mutex;
bool s_runtime_active = false;
Platform *s_platform = nullptr;
std::string s_window_title;
bool s_show_fps_in_title = true;
bool s_external_ui_common = false;
std::unique_ptr<BootSessionData> s_boot_session_data;
u64 s_previous_net_wait_ns = 0;
double s_net_wait_ms_per_second = 0.0;
std::chrono::steady_clock::time_point s_previous_net_wait_sample;

std::string FormatWindowTitle(const std::string &title, double fps) {
  if (!std::isfinite(fps) || fps < 0.0)
    fps = 0.0;
  const auto now = std::chrono::steady_clock::now();
  std::string formatted_title = fmt::format("{} | {:.1f} FPS", title, fps);
  const NetPlay::InputWaitTelemetry telemetry =
      NetPlay::NetPlayClient::GetInputWaitTelemetry();
  if (!telemetry.active) {
    s_previous_net_wait_ns = 0;
    s_net_wait_ms_per_second = 0.0;
    s_previous_net_wait_sample = {};
    return formatted_title;
  }
  if (s_previous_net_wait_sample.time_since_epoch().count() == 0) {
    s_previous_net_wait_sample = now;
    s_previous_net_wait_ns = telemetry.total_wait_ns;
  } else if (telemetry.total_wait_ns < s_previous_net_wait_ns) {
    s_previous_net_wait_sample = now;
    s_previous_net_wait_ns = telemetry.total_wait_ns;
    s_net_wait_ms_per_second = 0.0;
  } else if (now - s_previous_net_wait_sample >=
             std::chrono::milliseconds(500)) {
    const double seconds =
        std::chrono::duration<double>(now - s_previous_net_wait_sample).count();
    s_net_wait_ms_per_second =
        static_cast<double>(telemetry.total_wait_ns - s_previous_net_wait_ns) /
        1000000.0 / seconds;
    s_previous_net_wait_sample = now;
    s_previous_net_wait_ns = telemetry.total_wait_ns;
  }
  return fmt::format("{} | Net wait {:.1f} ms/s | Buffer {}", formatted_title,
                     s_net_wait_ms_per_second, telemetry.buffer_size);
}

PowerPC::CPUCore SelectCPUCore(moderngekko::CPUBackend backend) {
  const char *v = std::getenv("MODERNGEKKO_STATICRECOMP");
  const bool static_recomp = backend == moderngekko::CPUBackend::StaticRecomp ||
      (backend == moderngekko::CPUBackend::Default && (!v || !*v || *v != '0'));
  if (static_recomp)
    return PowerPC::CPUCore::StaticRecomp;
#ifdef _M_ARM_64
  return PowerPC::CPUCore::JITARM64;
#else
  return PowerPC::CPUCore::JIT64;
#endif
}
} // namespace

std::vector<std::string> Host_GetPreferredLocales() { return {}; }
void Host_PPCSymbolsChanged() {}
void Host_PPCBreakpointsChanged() {}
bool Host_UIBlocksControllerState() { return false; }
void Host_Message(HostMessageID id) {
  if (id == HostMessageID::WMUserStop && s_platform)
    s_platform->Stop();
}
void Host_UpdateTitle(const std::string &) {
  if (!s_platform)
    return;

  std::string title = s_window_title;
  if (s_show_fps_in_title &&
      s_platform->GetWindowSystemInfo().type != WindowSystemType::Headless)
    title = FormatWindowTitle(
        title, Core::System::GetInstance().GetPerfMetrics().GetFPS());
  s_platform->SetTitle(title);
}
void Host_UpdateDisasmDialog() {}
void Host_JitCacheInvalidation() {}
void Host_JitProfileDataWiped() {}
void Host_RequestRenderWindowSize(int, int) {}
bool Host_RendererHasFocus() {
  return !s_platform || s_platform->IsWindowFocused();
}
bool Host_RendererHasFullFocus() { return Host_RendererHasFocus(); }
bool Host_RendererIsFullscreen() {
  return s_platform && s_platform->IsWindowFullscreen();
}
bool Host_TASInputHasFocus() { return false; }
void Host_YieldToUI() {}
void Host_TitleChanged() {}
void Host_UpdateDiscordClientID(const std::string &) {}
bool Host_UpdateDiscordPresenceRaw(const std::string &, const std::string &,
                                   const std::string &, const std::string &,
                                   const std::string &, const std::string &,
                                   std::int64_t, std::int64_t, int, int) {
  return false;
}
std::unique_ptr<GBAHostInterface>
Host_CreateGBAHost(std::weak_ptr<HW::GBA::Core>) {
  return nullptr;
}

namespace moderngekko {
namespace {
struct TimedXboxHold
{
  std::atomic<bool> complete{false};
  std::shared_ptr<automation::XboxTestDevice> device;
  bool release = true;
  u64 end_ticks = 0;
  s64 cycles_late = 0;
};

struct XboxSequenceStep
{
  automation::Command command;
  std::vector<u8> snapshot;
  u64 start = 0, end = 0, scheduled_start = 0, scheduled_end = 0, idle_ticks = 0;
  bool visited = false, completed = false;
};

struct XboxSequenceRun
{
  std::vector<XboxSequenceStep> steps;
  std::shared_ptr<automation::XboxTestDevice> device;
  std::atomic<bool> complete{false};
  std::size_t next = 0;
  std::optional<std::size_t> active_hold;
  u64 deadline = 0;
  u32 frequency = 0;
  const char* error = nullptr;
};

struct RuntimeAutomationState
{
  mutable std::mutex mutex;
  std::atomic<std::uint64_t> frame_count{0};
  std::atomic<std::uint64_t> present_count{0};
  std::uint64_t processed_commands = 0;
  std::string last_command;
  std::string last_error;
  std::array<std::shared_ptr<automation::XboxTestDevice>, 4> xbox_devices;
  // Accessed only while holding the CPU guard or from the CPU event callback.
  CoreTiming::EventType* xbox_hold_event = nullptr;
  std::shared_ptr<TimedXboxHold> xbox_hold;
  CoreTiming::EventType* xbox_sequence_event = nullptr;
  std::shared_ptr<XboxSequenceRun> xbox_sequence;
};

void EnsureAutomationDirectories(const std::filesystem::path& root)
{
  if (root.empty())
    return;
  std::error_code ec;
  std::filesystem::create_directories(root / "commands", ec);
  std::filesystem::create_directories(root / "processed", ec);
  std::filesystem::create_directories(root / "failed", ec);
  std::filesystem::create_directories(root / "errors", ec);
}

void SetAutomationError(RuntimeAutomationState& state, std::string message)
{
  std::lock_guard lock(state.mutex);
  state.last_error = std::move(message);
}

void RecordAutomationFailure(RuntimeAutomationState& state, const std::filesystem::path& root,
                             const std::filesystem::path& command_path, std::string message)
{
  const std::string detail = command_path.filename().string() + ": " + message;
  SetAutomationError(state, detail);
  // Publish the reason before the failed receipt. The periodic status file can
  // still describe the preceding command and is cleared by a successful stop.
  std::ofstream output(root / "errors" / command_path.filename(),
                       std::ios::binary | std::ios::trunc);
  output << detail << '\n';
  output.close();
  std::fprintf(stderr, "Automation failure: %s\n", detail.c_str());
  if (!output)
    std::fprintf(stderr, "Unable to save automation failure detail\n");
}

void MarkAutomationCommand(RuntimeAutomationState& state, std::string command_name)
{
  std::lock_guard lock(state.mutex);
  ++state.processed_commands;
  state.last_command = std::move(command_name);
  state.last_error.clear();
}

void ClearAutomationPad(int port)
{
  const auto lock = ControllerEmu::EmulatedController::GetStateLock();
  for (int control = static_cast<int>(ciface::Touch::FIRST_GC_CONTROL);
       control <= static_cast<int>(ciface::Touch::LAST_WII_CONTROL); ++control)
  {
    ciface::Touch::SetControlState(
        port, static_cast<ciface::Touch::ControlID>(control), 0.0);
  }
}

void ApplyAutomationPad(const automation::PadState& pad)
{
  const auto lock = ControllerEmu::EmulatedController::GetStateLock();
  ciface::Touch::RegisterWiiInputOverrider(pad.port);
  ClearAutomationPad(pad.port);
  for (std::size_t index = 0; index < pad.controls.size(); ++index)
  {
    ciface::Touch::SetControlState(
        pad.port,
        static_cast<ciface::Touch::ControlID>(
            static_cast<int>(ciface::Touch::FIRST_GC_CONTROL) +
            static_cast<int>(index)),
        pad.controls[index]);
  }
}

std::filesystem::path NormalizeScreenshotPath(const std::filesystem::path& path)
{
  if (!path.has_extension())
    return path.string() + ".png";
  return path;
}

void SaveAutomationScreenshot(const std::filesystem::path& path)
{
  std::error_code ec;
  std::filesystem::create_directories(path.parent_path(), ec);
  const Core::CPUThreadGuard guard(Core::System::GetInstance());
  if (g_frame_dumper)
    g_frame_dumper->SaveScreenshot(path.string());
}

std::optional<RuntimeError> ReadAutomationTiming(const std::filesystem::path& path)
{
  u64 ticks;
  u64 idle_ticks;
  {
    auto& system = Core::System::GetInstance();
    const Core::CPUThreadGuard guard(system);
    ticks = system.GetCoreTiming().GetTicks();
    idle_ticks = system.GetCoreTiming().GetIdleTicks();
  }
  // File I/O occurs after releasing the CPU guard. This is an explicit
  // diagnostic snapshot, never part of the per-frame timing path.
  std::error_code ec;
  std::filesystem::create_directories(path.parent_path(), ec);
  if (ec)
    return RuntimeError{RuntimeErrorCode::InitializationFailed,
                        "could not create timing output directory: " + ec.message()};
  std::ofstream output(path);
  output << "ticks=" << ticks << "\nidle_ticks=" << idle_ticks << "\n";
  output.close();
  if (!output)
    return RuntimeError{RuntimeErrorCode::InitializationFailed,
                        "could not write timing output file"};
  return {};
}

std::optional<RuntimeError> RunAutomationJitProfile(const std::filesystem::path& path,
                                                    bool reset)
{
  auto& system = Core::System::GetInstance();
  const Core::CPUThreadGuard guard(system);
  auto& jit = system.GetJitInterface();
  if (!jit.IsProfilingEnabled())
    return RuntimeError{RuntimeErrorCode::InvalidState,
                        "JIT block profiling is disabled; use --jit-block-profile in the combat harness"};
  if (reset)
  {
    jit.WipeBlockProfilingData(guard);
    return {};
  }

  // Explicit, intrusive diagnostic: pause the CPU while serializing resident
  // block counters. Invalidated blocks are absent; this is not release FPS or
  // a complete accounting of all execution since the reset.
  std::error_code ec;
  std::filesystem::create_directories(path.parent_path(), ec);
  if (ec)
    return RuntimeError{RuntimeErrorCode::InitializationFailed,
                        "could not create JIT profile directory: " + ec.message()};
  File::IOFile output(path.string(), "wb");
  if (!output.IsOpen())
    return RuntimeError{RuntimeErrorCode::InitializationFailed,
                        "could not open JIT profile output"};
  jit.JitBlockLogDump(guard, output.GetHandle());
  const bool write_failed = std::ferror(output.GetHandle()) != 0;
  const bool closed = output.Close();
  if (write_failed || !closed)
    return RuntimeError{RuntimeErrorCode::InitializationFailed,
                        "could not write JIT profile output"};
  File::IOFile callers(path.string() + ".callers.tsv", "wb");
  if (!callers.IsOpen())
    return RuntimeError{RuntimeErrorCode::InitializationFailed,
                        "could not open JIT caller output"};
  jit.JitCallerLogDump(guard, callers.GetHandle());
  const bool caller_write_failed = std::ferror(callers.GetHandle()) != 0;
  const bool callers_closed = callers.Close();
  if (caller_write_failed || !callers_closed)
    return RuntimeError{RuntimeErrorCode::InitializationFailed,
                        "could not write JIT caller output"};
  return {};
}

std::optional<RuntimeError> ReadAutomationMemory(const std::filesystem::path& path, u32 address,
                                                 u32 size)
{
  auto& system = Core::System::GetInstance();
  const Core::CPUThreadGuard guard(system);
  const u8* source = system.GetMemory().GetPointerForRange(address, size);
  if (!source)
  {
    return RuntimeError{RuntimeErrorCode::InvalidState,
                        fmt::format("guest range {:#010x}+{} is not readable", address, size)};
  }

  std::error_code ec;
  std::filesystem::create_directories(path.parent_path(), ec);
  if (ec)
  {
    return RuntimeError{RuntimeErrorCode::InitializationFailed,
                        "could not create memory output directory: " + ec.message()};
  }

  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output)
  {
    return RuntimeError{RuntimeErrorCode::InitializationFailed,
                        "could not open memory output file"};
  }
  output.write(reinterpret_cast<const char*>(source), size);
  if (!output)
  {
    return RuntimeError{RuntimeErrorCode::InitializationFailed,
                        "could not write memory output file"};
  }
  return {};
}

std::optional<RuntimeError> WriteAutomationMemory(u32 address,
                                                  const std::vector<std::uint8_t>& data)
{
  auto& system = Core::System::GetInstance();
  const Core::CPUThreadGuard guard(system);
  if (!system.GetMemory().GetPointerForRange(address, data.size()))
  {
    return RuntimeError{
        RuntimeErrorCode::InvalidState,
        fmt::format("guest range {:#010x}+{} is not writable", address, data.size())};
  }
  system.GetMemory().CopyToEmu(address, data.data(), data.size());
  return {};
}

std::optional<RuntimeError> ApplyTimedXboxHold(
    RuntimeAutomationState& state,
    const std::shared_ptr<automation::XboxTestDevice>& device,
    const automation::Command& command, std::stop_token stop_token)
{
  auto hold = std::make_shared<TimedXboxHold>();
  hold->device = device;
  hold->release = command.release_pad;
  auto& system = Core::System::GetInstance();
  auto& timing = system.GetCoreTiming();
  u64 start_ticks, duration_ticks;
  u32 ticks_per_second;
  {
    const Core::CPUThreadGuard guard(system);
    // Register once per core lifetime; re-registering the same name does not
    // replace CoreTiming's callback. No pointers are serialized as userdata.
    if (!state.xbox_hold_event)
      state.xbox_hold_event = timing.RegisterEvent("OpenMUA2XboxTimedHold",
        [&state](Core::System& callback_system, u64, s64 late) {
          const auto active = state.xbox_hold;
          if (!active) return;
          if (active->release) {
            const auto lock = ControllerEmu::EmulatedController::GetStateLock();
            active->device->values = {};
          }
          active->end_ticks = callback_system.GetCoreTiming().GetTicks();
          active->cycles_late = late;
          active->complete.store(true, std::memory_order_release);
        });
    state.xbox_hold = hold;
    ticks_per_second = system.GetSystemTimers().GetTicksPerSecond();
    duration_ticks = u64{ticks_per_second} * command.milliseconds / 1000;
    const auto lock = ControllerEmu::EmulatedController::GetStateLock();
    ciface::Touch::UnregisterWiiInputOverrider(command.pad.port);
    device->values = command.xbox;
    start_ticks = timing.GetTicks();
    timing.ScheduleEvent(static_cast<s64>(duration_ticks), state.xbox_hold_event);
  }
  // Host polling observes completion only. Input release occurs in the guest
  // scheduler, independent of render frequency, host sleep and file I/O.
  while (!hold->complete.load(std::memory_order_acquire) && !stop_token.stop_requested())
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  bool completed;
  {
    const Core::CPUThreadGuard guard(system);
    timing.RemoveEvent(state.xbox_hold_event);
    state.xbox_hold.reset();
    completed = hold->complete.load(std::memory_order_acquire);
    if (!completed) hold->end_ticks = timing.GetTicks();
    if (!completed || stop_token.stop_requested()) {
      const auto lock = ControllerEmu::EmulatedController::GetStateLock();
      device->values = {};
    }
  }
  std::error_code ec;
  std::filesystem::create_directories(command.path.parent_path(), ec);
  if (ec)
    return RuntimeError{RuntimeErrorCode::InitializationFailed, "could not create Xbox timing receipt directory"};
  std::ofstream output(command.path);
  output << "start_ticks=" << start_ticks << "\nend_ticks=" << hold->end_ticks
         << "\nduration_ticks=" << duration_ticks << "\nticks_per_second=" << ticks_per_second
         << "\ncycles_late=" << hold->cycles_late << "\ncompleted=" << completed
         << "\nrelease=" << command.release_pad << '\n';
  output.close();
  if (!output)
    return RuntimeError{RuntimeErrorCode::InitializationFailed, "could not write Xbox timing receipt"};
  if (!completed)
    return RuntimeError{RuntimeErrorCode::InvalidState, "Xbox timed hold interrupted"};
  return {};
}

// Runs exclusively on the guest CPU scheduler. No file I/O, probe allocation, host
// polling or guest memory writes occur between sequence steps.
void AdvanceXboxSequence(RuntimeAutomationState& state, Core::System& system)
{
  const auto run = state.xbox_sequence;
  if (!run) return;
  auto& timing = system.GetCoreTiming();
  const auto lock = ControllerEmu::EmulatedController::GetStateLock();
  if (run->active_hold) {
    auto& previous = run->steps[*run->active_hold];
    previous.end = timing.GetTicks();
    previous.completed = true;
    if (previous.command.release_pad) run->device->values = {};
    run->active_hold.reset();
  }
  while (run->next < run->steps.size()) {
    auto& step = run->steps[run->next++];
    const auto& command = step.command;
    step.visited = true;
    step.start = timing.GetTicks();
    step.scheduled_start = run->deadline;
    if (command.type == automation::CommandType::XboxTime) {
      run->device->values = command.xbox;
      run->active_hold = run->next - 1;
      run->deadline += u64{run->frequency} * command.milliseconds / 1000;
      step.scheduled_end = run->deadline;
      // Anchor every boundary to the original guest clock. Never accumulate
      // callback lateness or host receipt-writing delays into the input movie.
      if (run->deadline <= timing.GetTicks()) {
        run->error = "sequence missed an entire input interval";
        break;
      }
      timing.ScheduleEvent(static_cast<s64>(run->deadline - timing.GetTicks()), state.xbox_sequence_event);
      return;
    }
    if (command.type == automation::CommandType::ReadTiming) {
      step.idle_ticks = timing.GetIdleTicks();
    } else {
      const u8* source = system.GetMemory().GetPointerForRange(command.address, command.size);
      if (!source) { run->error = "sequence memory range is not readable"; break; }
      if (command.type == automation::CommandType::CheckMemory) {
        if (std::memcmp(source, command.data.data(), command.size) != 0) {
          run->error = "sequence memory guard mismatch; remaining input cancelled";
          break;
        }
      } else {
        std::memcpy(step.snapshot.data(), source, command.size);
      }
    }
    step.end = step.start;
    step.completed = true;
  }
  // Always release on completion/failure, even if the last hold requested
  // retention. A sequence cannot leave synthetic input active after it ends.
  run->device->values = {};
  run->complete.store(true, std::memory_order_release);
}

std::optional<RuntimeError> ApplyXboxSequence(
    RuntimeAutomationState& state, const std::shared_ptr<automation::XboxTestDevice>& device,
    const automation::Command& command, const std::vector<automation::Command>& commands,
    std::stop_token stop_token)
{
  auto run = std::make_shared<XboxSequenceRun>();
  run->device = device;
  run->steps.reserve(commands.size());
  for (const auto& item : commands) {
    XboxSequenceStep step;
    step.command = item;
    if (item.type == automation::CommandType::ReadMemory) step.snapshot.resize(item.size);
    run->steps.push_back(std::move(step));
  }
  auto& system = Core::System::GetInstance();
  auto& timing = system.GetCoreTiming();
  {
    const Core::CPUThreadGuard guard(system);
    run->frequency = system.GetSystemTimers().GetTicksPerSecond();
    const u64 now = timing.GetTicks();
    run->deadline = command.start_ticks ? command.start_ticks : now;
    if (run->deadline < now || run->deadline - now > u64{run->frequency} * 60)
      return RuntimeError{RuntimeErrorCode::InvalidState, "sequence start must be now or within the next 60 guest seconds"};
    if (run->deadline > std::numeric_limits<u64>::max() - u64{run->frequency} * 600)
      return RuntimeError{RuntimeErrorCode::InvalidState, "sequence guest tick overflow"};
    if (!state.xbox_sequence_event)
      state.xbox_sequence_event = timing.RegisterEvent("OpenMUA2XboxSequence",
        [&state](Core::System& callback_system, u64, s64) { AdvanceXboxSequence(state, callback_system); });
    state.xbox_sequence = run;
    {
      const auto lock = ControllerEmu::EmulatedController::GetStateLock();
      for (const auto& item : commands)
        if (item.type == automation::CommandType::XboxTime) {
          ciface::Touch::UnregisterWiiInputOverrider(item.pad.port);
          break;
        }
      device->values = {};
    }
    timing.ScheduleEvent(static_cast<s64>(run->deadline - now), state.xbox_sequence_event);
  }
  while (!run->complete.load(std::memory_order_acquire) && !stop_token.stop_requested())
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  {
    const Core::CPUThreadGuard guard(system);
    timing.RemoveEvent(state.xbox_sequence_event);
    state.xbox_sequence.reset();
    if (!run->complete.load(std::memory_order_acquire)) run->error = "sequence interrupted";
    const auto lock = ControllerEmu::EmulatedController::GetStateLock();
    device->values = {};
  }
  // All observations and receipts are buffered until input execution finishes.
  for (const auto& step : run->steps) {
    if (!step.visited || step.command.path.empty()) continue;
    std::error_code ec;
    std::filesystem::create_directories(step.command.path.parent_path(), ec);
    if (ec) return RuntimeError{RuntimeErrorCode::InitializationFailed, "could not create sequence output directory"};
    std::ofstream output(step.command.path, std::ios::binary);
    switch (step.command.type) {
    case automation::CommandType::ReadMemory:
      if (step.completed) output.write(reinterpret_cast<const char*>(step.snapshot.data()), step.snapshot.size());
      break;
    case automation::CommandType::ReadTiming:
      output << "ticks=" << step.start << "\nidle_ticks=" << step.idle_ticks << '\n';
      break;
    case automation::CommandType::XboxTime:
      output << "start_ticks=" << step.start << "\nend_ticks=" << step.end
             << "\nduration_ticks=" << u64{run->frequency} * step.command.milliseconds / 1000
             << "\nticks_per_second=" << run->frequency
             << "\ncycles_late=" << (step.completed ? static_cast<s64>(step.end - step.scheduled_end) : 0)
             << "\ncompleted=" << step.completed << "\nrelease=" << step.command.release_pad
             << "\nscheduled_start_ticks=" << step.scheduled_start
             << "\nscheduled_end_ticks=" << step.scheduled_end << '\n';
      break;
    default: break;
    }
    output.close();
    if (!output) return RuntimeError{RuntimeErrorCode::InitializationFailed, "could not write sequence output"};
  }
  if (run->error) return RuntimeError{RuntimeErrorCode::InvalidState, run->error};
  return {};
}

std::optional<RuntimeError> ApplyAutomationCommand(Runtime& runtime,
                                                   RuntimeAutomationState& state,
                                                   const automation::Command& command,
                                                   std::stop_token stop_token)
{
  auto& system = Core::System::GetInstance();
  switch (command.type)
  {
  case automation::CommandType::XboxSequence:
  case automation::CommandType::XboxFrames:
  case automation::CommandType::XboxTime:
  {
    if (Core::GetState(system) != Core::State::Running)
      return RuntimeError{RuntimeErrorCode::InvalidState, "Xbox input requires a running core"};
    std::vector<automation::Command> sequence;
    int port = command.pad.port;
    if (command.type == automation::CommandType::XboxSequence) {
      std::string error;
      if (!automation::LoadXboxSequence(command.path, &sequence, &error))
        return RuntimeError{RuntimeErrorCode::InvalidState, error};
      for (const auto& item : sequence)
        if (item.type == automation::CommandType::XboxTime) { port = item.pad.port; break; }
    }
    auto& device = state.xbox_devices[port];
    if (!device) {
      device = std::make_shared<automation::XboxTestDevice>();
      if (!g_controller_interface.AddDevice(device)) {
        device.reset();
        return RuntimeError{RuntimeErrorCode::InvalidState, "could not create process-local Xbox test device"};
      }
      const auto lock = ControllerEmu::EmulatedController::GetStateLock();
      ciface::Touch::UnregisterWiiInputOverrider(port);
      auto* controller = Wiimote::GetConfig()->GetController(port);
      controller->SetDefaultDevice(device->GetQualifiedName());
      controller->UpdateReferences(g_controller_interface);
      // Only this process-local synthetic device is mapped to the tested port.
      Config::SetBase(Config::MAIN_INPUT_BACKGROUND_INPUT, true);
    }
    if (command.type == automation::CommandType::XboxSequence) {
      if (auto error = ApplyXboxSequence(state, device, command, sequence, stop_token)) return error;
      break;
    }
    if (command.type == automation::CommandType::XboxTime) {
      if (auto error = ApplyTimedXboxHold(state, device, command, stop_token)) return error;
      break;
    }
    {
      const auto lock = ControllerEmu::EmulatedController::GetStateLock();
      ciface::Touch::UnregisterWiiInputOverrider(port);
      device->values = command.xbox;
    }
    auto first = state.frame_count.load(std::memory_order_relaxed);
    while (!stop_token.stop_requested()) {
      const auto current = state.frame_count.load(std::memory_order_relaxed);
      if (current < first) first = current;
      else if (current - first >= command.frames) break;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (command.release_pad || stop_token.stop_requested()) {
      const auto lock = ControllerEmu::EmulatedController::GetStateLock();
      device->values = {};
    }
    break;
  }
  case automation::CommandType::Pad:
    ApplyAutomationPad(command.pad);
    break;
  case automation::CommandType::PadFrames:
  {
    if (Core::GetState(system) != Core::State::Running)
    {
      return RuntimeError{RuntimeErrorCode::InvalidState,
                          "pad_frames requires a running emulated core"};
    }
    std::uint64_t first_frame = state.frame_count.load(std::memory_order_relaxed);
    ApplyAutomationPad(command.pad);
    while (!stop_token.stop_requested())
    {
      const std::uint64_t current_frame =
          state.frame_count.load(std::memory_order_relaxed);
      if (current_frame < first_frame)
        first_frame = current_frame;
      else if (current_frame - first_frame >= command.frames)
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (command.release_pad || stop_token.stop_requested())
      ClearAutomationPad(command.pad.port);
    break;
  }
  case automation::CommandType::ClearPad:
    ClearAutomationPad(command.pad.port);
    break;
  case automation::CommandType::Pause:
    if (auto error = runtime.Pause())
      return error;
    break;
  case automation::CommandType::Resume:
    if (auto error = runtime.Resume())
      return error;
    break;
  case automation::CommandType::SaveState:
    std::filesystem::create_directories(command.path.parent_path());
    State::SaveAs(system, command.path.string());
    break;
  case automation::CommandType::LoadState:
  {
    // LoadAs queues work when called off the CPU thread. Acknowledging early
    // allows a following timed input event to be erased by the restored state.
    auto loaded = std::make_shared<std::atomic<bool>>(false);
    Core::RunOnCPUThread(system, [&system, path = command.path.string(), loaded] {
      ResetDirectQtes();
      State::LoadAs(system, path); // Runs synchronously on this CPU thread.
      loaded->store(true, std::memory_order_release);
    });
    while (!loaded->load(std::memory_order_acquire) && !stop_token.stop_requested())
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    if (!loaded->load(std::memory_order_acquire))
      return RuntimeError{RuntimeErrorCode::InvalidState, "state load interrupted"};
    break;
  }
  case automation::CommandType::Screenshot:
    SaveAutomationScreenshot(NormalizeScreenshotPath(command.path));
    break;
  case automation::CommandType::ReadTiming:
    if (auto error = ReadAutomationTiming(command.path))
      return error;
    break;
  case automation::CommandType::JitProfileReset:
  case automation::CommandType::JitProfileDump:
    if (auto error = RunAutomationJitProfile(
            command.path, command.type == automation::CommandType::JitProfileReset))
      return error;
    break;
  case automation::CommandType::ReadMemory:
    if (auto error = ReadAutomationMemory(command.path, command.address, command.size))
      return error;
    break;
  case automation::CommandType::CheckMemory:
  {
    const Core::CPUThreadGuard guard(system);
    const u8* source = system.GetMemory().GetPointerForRange(command.address, command.size);
    if (!source || std::memcmp(source, command.data.data(), command.size) != 0)
      return RuntimeError{RuntimeErrorCode::InvalidState, "memory guard mismatch"};
    break;
  }
  case automation::CommandType::WriteMemory:
    if (auto error = WriteAutomationMemory(command.address, command.data))
      return error;
    break;
  case automation::CommandType::Stop:
    runtime.RequestStop();
    break;
  }
  MarkAutomationCommand(state, command.source_name);
  return {};
}

bool AutomationCommandNeedsReadyCore(automation::CommandType type)
{
  switch (type)
  {
  case automation::CommandType::Pause:
  case automation::CommandType::Resume:
  case automation::CommandType::SaveState:
  case automation::CommandType::LoadState:
  case automation::CommandType::Screenshot:
  case automation::CommandType::ReadTiming:
  case automation::CommandType::JitProfileReset:
  case automation::CommandType::JitProfileDump:
  case automation::CommandType::ReadMemory:
  case automation::CommandType::WriteMemory:
  case automation::CommandType::PadFrames:
  case automation::CommandType::XboxFrames:
  case automation::CommandType::XboxTime:
  case automation::CommandType::XboxSequence:
  case automation::CommandType::CheckMemory:
    return true;
  case automation::CommandType::Pad:
  case automation::CommandType::ClearPad:
  case automation::CommandType::Stop:
    return false;
  }
  return true;
}

bool AutomationCoreIsReady()
{
  const auto state = Core::GetState(Core::System::GetInstance());
  return state == Core::State::Running || state == Core::State::Paused;
}

void MoveAutomationCommand(const std::filesystem::path& source, const std::filesystem::path& root,
                           const char* destination_directory)
{
  std::error_code ec;
  const std::filesystem::path destination =
      root / destination_directory / source.filename();
  std::filesystem::remove(destination, ec);
  ec.clear();
  std::filesystem::rename(source, destination, ec);
  if (ec)
  {
    ec.clear();
    std::filesystem::copy_file(source, destination,
                               std::filesystem::copy_options::overwrite_existing, ec);
    if (!ec)
      std::filesystem::remove(source, ec);
  }
}

template <typename ImplT>
void WriteAutomationStatus(const std::filesystem::path& root, ImplT& impl)
{
  automation::Status status;
  const auto core_state = impl.booted ? Core::GetState(Core::System::GetInstance())
                                      : Core::State::Uninitialized;
  status.state = core_state == Core::State::Paused
                     ? "paused"
                     : (core_state == Core::State::Running ? "running" : "stopped");
  status.booted = impl.booted;
  status.title = impl.title;
  status.game_id = impl.metadata.disc_id;
  status.game_name = impl.metadata.game_name;
  if (impl.booted)
  {
    const auto& perf = Core::System::GetInstance().GetPerfMetrics();
    status.fps = perf.GetFPS();
    status.vps = perf.GetVPS();
    status.speed = perf.GetSpeed();
  }
  status.frame_count =
      impl.automation_state.frame_count.load(std::memory_order_relaxed);
  status.present_count =
      impl.automation_state.present_count.load(std::memory_order_relaxed);
  {
    std::lock_guard lock(impl.automation_state.mutex);
    status.processed_commands = impl.automation_state.processed_commands;
    status.last_command = impl.automation_state.last_command;
    status.last_error = impl.automation_state.last_error;
  }

  const std::filesystem::path status_path = root / "status.txt";
  const std::filesystem::path temp_path = root / "status.tmp";
  std::ofstream output(temp_path, std::ios::binary | std::ios::trunc);
  output << automation::FormatStatus(status);
  output.close();
  std::error_code ec;
  std::filesystem::rename(temp_path, status_path, ec);
  if (ec)
  {
    ec.clear();
    std::filesystem::copy_file(temp_path, status_path,
                               std::filesystem::copy_options::overwrite_existing, ec);
    std::filesystem::remove(temp_path, ec);
  }
}

template <typename ImplT>
void AutomationLoop(Runtime& runtime, ImplT& impl, std::stop_token stop_token)
{
  const std::filesystem::path root = impl.config.automation.directory;
  if (root.empty())
    return;

  EnsureAutomationDirectories(root);
  auto next_status_write = std::chrono::steady_clock::now();
  while (!stop_token.stop_requested())
  {

    for (const auto& command_path : automation::ListCommandFiles(root / "commands"))
    {
      automation::Command command;
      std::string error;
      if (!automation::ParseCommandFile(command_path, &command, &error))
      {
        RecordAutomationFailure(impl.automation_state, root, command_path, error);
        MoveAutomationCommand(command_path, root, "failed");
        continue;
      }

      if (AutomationCommandNeedsReadyCore(command.type) && !AutomationCoreIsReady())
        break;

      if (!command.path.empty())
        command.path = automation::ResolveControlPath(root, command.path);

      if (auto runtime_error =
              ApplyAutomationCommand(runtime, impl.automation_state, command, stop_token))
      {
        RecordAutomationFailure(impl.automation_state, root, command_path,
                                runtime_error->message);
        MoveAutomationCommand(command_path, root, "failed");
        continue;
      }

      MoveAutomationCommand(command_path, root, "processed");
    }

    const auto now = std::chrono::steady_clock::now();
    if (now >= next_status_write)
    {
      WriteAutomationStatus(root, impl);
      next_status_write = now + std::chrono::milliseconds(200);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  WriteAutomationStatus(root, impl);
}

}  // namespace

struct Runtime::Impl {
  RuntimeConfig config;
  GameMetadata metadata;
  std::string title;
  std::unique_ptr<Platform> platform;
  std::unique_ptr<ModManager> mods;
  Common::EventHook state_hook;
  bool ui_initialized = false;
  bool controllers_initialized = false;
  bool booted = false;
  std::atomic<bool> running{false};
  RuntimeAutomationState automation_state;
  Common::EventHook present_hook;
  Common::EventHook before_present_hook;
  Common::EventHook xfb_copy_hook;
  std::unique_ptr<moderngekko::telemetry::FrameTiming> frame_timing;
  std::filesystem::path frame_timing_path;
  std::unique_ptr<moderngekko::telemetry::PresentationTiming> presentation_timing;
  std::filesystem::path presentation_timing_path;
  bool automation_registered = false;
  std::jthread automation_thread;
};

namespace detail {
void SetExternalUICommon(bool external) {
  std::lock_guard lock(s_runtime_mutex);
  s_external_ui_common = external;
}

void SetBootSessionData(std::unique_ptr<BootSessionData> boot_session_data) {
  std::lock_guard lock(s_runtime_mutex);
  s_boot_session_data = std::move(boot_session_data);
}
} // namespace detail

ModuleSource ModuleSource::DynamicPath(std::filesystem::path path) {
  ModuleSource source;
  source.kind = Kind::DynamicPath;
  source.path = std::move(path);
  return source;
}

ModuleSource
ModuleSource::AttachedDescriptor(const ModernGekkoModuleDesc *descriptor) {
  ModuleSource source;
  source.kind = Kind::AttachedDescriptor;
  source.descriptor = descriptor;
  return source;
}

Runtime::Runtime(std::unique_ptr<Impl> impl) : m_impl(std::move(impl)) {}

RuntimeCreateResult Runtime::Create(RuntimeConfig config) {
  std::lock_guard lock(s_runtime_mutex);
  if (s_runtime_active)
    return {
        {},
        RuntimeError{RuntimeErrorCode::AlreadyActive,
                     "only one ModernGekko runtime may be active per process"}};

  GameInspectResult inspected = InspectGame(config.game_root);
  if (!inspected)
    return {{}, RuntimeError{RuntimeErrorCode::InvalidGame, inspected.error}};

  const auto cpu_core = SelectCPUCore(config.cpu_backend);
  const bool uses_native_module = cpu_core == PowerPC::CPUCore::StaticRecomp;
  // JIT executes the audited game image and does not load a generated DLL.
  if (!uses_native_module)
    config.module = {};

  const ModernGekkoModuleRequirements requirements = {
      MODERNGEKKO_CPU_ABI_VERSION, static_cast<std::uint32_t>(sizeof(CPUState)),
      inspected.metadata->disc_id.c_str()};
  ModuleLibrary validation_library;
  ModuleLoadResult module_result{};
  if (config.module.kind == ModuleSource::Kind::DynamicPath)
    module_result =
        validation_library.Open(config.module.path.string(), requirements);
  else if (config.module.kind == ModuleSource::Kind::AttachedDescriptor)
    module_result =
        validation_library.Attach(config.module.descriptor, requirements);
  else if (uses_native_module && !config.allow_interpreter)
    return {
        {},
        RuntimeError{
            RuntimeErrorCode::ModuleRequired,
            "no native module was supplied; use allow_interpreter explicitly"}};

  if (config.module.kind != ModuleSource::Kind::None &&
      module_result.status != ModuleLoadStatus::Ok) {
    if (!config.allow_interpreter) {
      std::string message = "native module was rejected";
      if (module_result.status == ModuleLoadStatus::DescriptorRejected)
        message += ": " + std::string(moderngekko_module_status_string(
                              module_result.validation_status));
      return {
          {},
          RuntimeError{RuntimeErrorCode::ModuleRejected, std::move(message)}};
    }
    config.module = {};
  }
  validation_library.Close();

  auto impl = std::make_unique<Impl>();
  impl->config = std::move(config);
  impl->metadata = std::move(*inspected.metadata);
  impl->title = impl->config.window_title.value_or(
      "ModernGekko - " + impl->metadata.game_name + " [" +
      impl->metadata.disc_id + "]");
  impl->mods = std::make_unique<ModManager>();
  const ModLoadReport mod_report = impl->mods->LoadDirectories(
      impl->config.mod_directories, impl->metadata.disc_id);
  for (const ModLoadIssue &issue : mod_report.issues)
    std::fprintf(stderr, "mod rejected: %s: %s\n", issue.source.c_str(),
                 issue.message.c_str());
  for (const LoadedModInfo &mod : mod_report.loaded)
    std::fprintf(stderr, "mod loaded: %s %s\n", mod.id.c_str(),
                 mod.version.c_str());

  if (!s_external_ui_common) {
    UICommon::SetUserDirectory(impl->config.user_directory.string());
    // Config saves and shader-cache creation require their parent directories.
    UICommon::CreateDirectories();
    UICommon::Init();
    impl->ui_initialized = true;
  }
  Config::SetBase(Config::MAIN_FULLSCREEN, impl->config.fullscreen);

  if (impl->config.headless)
    impl->platform = Platform::CreateHeadlessPlatform();
#ifdef _WIN32
  else
    impl->platform = Platform::CreateWin32Platform();
#endif
#ifdef MODERNGEKKO_HAVE_COCOA
  else impl->platform = Platform::CreateMacOSPlatform();
#endif
#ifdef HAVE_X11
  else if (impl->config.window_system != WindowSystem::Wayland) impl->platform =
      Platform::CreateX11Platform();
#endif
#ifdef HAVE_WAYLAND
  else if (impl->config.window_system != WindowSystem::X11) impl->platform =
      Platform::CreateWaylandPlatform();
#endif
  if (!impl->platform || !impl->platform->Init()) {
    if (impl->ui_initialized)
      UICommon::Shutdown();
    return {{},
            RuntimeError{RuntimeErrorCode::PlatformUnavailable,
                         "the requested Dolphin host platform is unavailable"}};
  }

  const WindowSystemInfo wsi = impl->platform->GetWindowSystemInfo();
  UICommon::InitControllers(wsi);
  impl->controllers_initialized = true;
  EnsureAutomationDirectories(impl->config.automation.directory);
  if (!impl->config.automation.directory.empty()) {
    for (int port = 0; port < 4; ++port)
    {
      ciface::Touch::RegisterGameCubeInputOverrider(port);
      for (int id = ciface::Touch::FIRST_GC_CONTROL; id <= ciface::Touch::LAST_GC_CONTROL; ++id)
        ciface::Touch::SetControlState(port, static_cast<ciface::Touch::ControlID>(id), 0.0);
      ciface::Touch::RegisterWiiInputOverrider(port);
      for (int id = ciface::Touch::FIRST_WII_CONTROL; id <= ciface::Touch::LAST_WII_CONTROL; ++id)
        ciface::Touch::SetControlState(port, static_cast<ciface::Touch::ControlID>(id), 0.0);
    }
    impl->automation_registered = true;
  }
  impl->platform->SetTitle(impl->title);

  Config::SetBase(Config::MAIN_CPU_CORE, cpu_core);
  std::fprintf(stderr, "CPU backend: %s\n",
               uses_native_module ? "StaticRecomp" : "JIT");
  if (!impl->config.graphics.backend.empty())
    Config::SetBase(Config::MAIN_GFX_BACKEND, impl->config.graphics.backend);
  else if (impl->config.headless)
    Config::SetBase(Config::MAIN_GFX_BACKEND, std::string("Null"));
  if (impl->config.graphics.internal_resolution_scale)
    Config::SetBase(Config::GFX_EFB_SCALE,
                    *impl->config.graphics.internal_resolution_scale);
  Config::SetBase(Config::GFX_SHADER_CACHE, true);
  Config::SetBase(Config::GFX_SHADER_COMPILATION_MODE,
                  ShaderCompilationMode::AsynchronousUberShaders);
  Config::SetBase(Config::GFX_WAIT_FOR_SHADERS_BEFORE_STARTING, true);
  const std::vector<std::string> audio_backends =
      AudioCommon::GetSoundBackends();
  // Explicit numeric audio profiling may exercise Cubeb without opening a
  // game window. Ordinary headless execution remains silent.
  const char* audio_profile = std::getenv("OPENMUA2_AUDIO_PROFILE");
  if (impl->config.headless && !(audio_profile && *audio_profile)) {
    impl->config.audio.backend = BACKEND_NULLSOUND;
  } else if (impl->config.audio.backend.empty() ||
             !std::ranges::contains(audio_backends,
                                    impl->config.audio.backend)) {
    constexpr std::array preferred_backends = {
        BACKEND_CUBEB, BACKEND_PULSEAUDIO, BACKEND_ALSA};
    const auto preferred =
        std::ranges::find_if(preferred_backends, [&](const char *backend) {
          return std::ranges::contains(audio_backends, backend);
        });
    impl->config.audio.backend =
        preferred != preferred_backends.end() ? *preferred : BACKEND_NULLSOUND;
  }
  Config::SetBase(Config::MAIN_AUDIO_BACKEND, impl->config.audio.backend);
  if (impl->config.audio.mute)
    Config::SetBase(Config::MAIN_AUDIO_VOLUME, 0);
  Config::SetBase(Config::MAIN_INPUT_BACKGROUND_INPUT,
                  impl->config.input.background_input);

  auto &jit = Core::System::GetInstance().GetJitInterface();
  StaticRecompModuleSource recomp_source;
  if (impl->config.module.kind == ModuleSource::Kind::DynamicPath)
    recomp_source =
        StaticRecompModuleSource::Dynamic(impl->config.module.path.string());
  else if (impl->config.module.kind == ModuleSource::Kind::AttachedDescriptor)
    recomp_source = StaticRecompModuleSource::Attached(
        reinterpret_cast<const StaticRecompModuleDesc *>(
            impl->config.module.descriptor));
  recomp_source.host_call = &ModManager::HostCall;
  recomp_source.host_call_contains = &ModManager::HostCallContains;
  recomp_source.host_call_range_contains =
      &ModManager::HostCallRangeContains;
  recomp_source.host_call_user = impl->mods.get();
  jit.SetStaticRecompModuleSource(std::move(recomp_source));

  s_runtime_active = true;
  s_platform = impl->platform.get();
  s_window_title = impl->title;
  s_show_fps_in_title = impl->config.show_fps_in_title;
  return {std::unique_ptr<Runtime>(new Runtime(std::move(impl))), {}};
}

Runtime::~Runtime() {
  const bool shutdown_trace =
      std::getenv("MODERNGEKKO_RUNTIME_SHUTDOWN_TRACE") != nullptr;
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: destructor begin\n");
  RequestStop();
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: destructor stopping automation\n");
  StopAutomation();
  if (m_impl->booted) {
    if (shutdown_trace)
      std::fprintf(stderr, "[moderngekko] runtime: destructor stopping booted core\n");
    Core::Stop(Core::System::GetInstance());
    Core::Shutdown(Core::System::GetInstance());
  }
  if (m_impl->automation_registered) {
    if (shutdown_trace)
      std::fprintf(stderr, "[moderngekko] runtime: unregistering automation input\n");
    for (int port = 0; port < 4; ++port)
    {
      ciface::Touch::UnregisterGameCubeInputOverrider(port);
      ciface::Touch::UnregisterWiiInputOverrider(port);
    }
    m_impl->automation_registered = false;
  }
  m_impl->state_hook = {};
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: destroying platform\n");
  {
    std::lock_guard lock(s_runtime_mutex);
    s_platform = nullptr;
  }
  m_impl->platform.reset();
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: shutting down controllers\n");
  if (m_impl->controllers_initialized)
    UICommon::ShutdownControllers();
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: shutting down UICommon\n");
  if (m_impl->ui_initialized)
    UICommon::Shutdown();
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: clearing globals\n");
  std::lock_guard lock(s_runtime_mutex);
  s_window_title.clear();
  s_show_fps_in_title = true;
  s_runtime_active = false;
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: destructor complete\n");
}

void Runtime::StopAutomation() {
  if (m_impl->automation_thread.joinable()) {
    m_impl->automation_thread.request_stop();
    m_impl->automation_thread.join();
  }
  m_impl->present_hook = {};
  m_impl->before_present_hook = {};
  m_impl->xfb_copy_hook = {};
}

RuntimeRunResult Runtime::Run() {
  if (m_impl->running.exchange(true))
    return {RuntimeExitReason::BootFailed,
            RuntimeError{RuntimeErrorCode::InvalidState,
                         "runtime is already running"}};

  std::unique_ptr<BootParameters> boot;
  {
    std::lock_guard lock(s_runtime_mutex);
    if (s_boot_session_data)
      boot = BootParameters::GenerateFromFile(
          PathToString(m_impl->metadata.main_dol), std::move(*s_boot_session_data));
    else if (m_impl->config.load_state_path)
      boot = BootParameters::GenerateFromFile(
          PathToString(m_impl->metadata.main_dol),
          BootSessionData(PathToString(*m_impl->config.load_state_path),
                          DeleteSavestateAfterBoot::No));
    else
      boot =
          BootParameters::GenerateFromFile(PathToString(m_impl->metadata.main_dol));
    s_boot_session_data.reset();
  }
  if (!boot) {
    m_impl->running = false;
    return {RuntimeExitReason::BootFailed,
            RuntimeError{RuntimeErrorCode::BootFailed,
                         "Dolphin rejected the extracted disc"}};
  }
  m_impl->state_hook =
      Core::AddOnStateChangedCallback([this](Core::State state) {
        if (state == Core::State::Uninitialized && m_impl->platform)
          m_impl->platform->Stop();
      });
  // A run override also takes precedence over imported per-game settings.
  // Keep CPU/GPU emulation serial for the planned original Xbox port.
  Config::SetCurrent(Config::MAIN_CPU_THREAD, false);
  if (!BootManager::BootCore(Core::System::GetInstance(), std::move(boot),
                             m_impl->platform->GetWindowSystemInfo())) {
    m_impl->running = false;
    return {RuntimeExitReason::BootFailed,
            RuntimeError{RuntimeErrorCode::BootFailed,
                         "Dolphin could not boot sys/main.dol"}};
  }
  m_impl->booted = true;
  s_fusion_buttons_enabled = false;
  s_hero_buttons_enabled = false;
  s_control_ports=s_hero_ports=s_fusion_ports=0;
  s_direct_qte_enabled = false;
  ResetDirectQtes();
  std::ifstream profile(m_impl->config.user_directory / "Config" / "WiimoteNew.ini");
  const std::string profile_text{std::istreambuf_iterator<char>(profile),std::istreambuf_iterator<char>()};
  const auto managed_ports=profile.bad()?0:moderngekko::controls::ManagedXboxPorts(profile_text);
  const auto enabled=[](const char* name) {
    const char* value=std::getenv(name);return value && std::string_view(value)=="1";
  };
  const auto want_hero_ports=std::uint8_t(managed_ports | (enabled("OPENMUA2_HERO_BUTTONS")?15:0));
  const auto want_fusion_ports=std::uint8_t(managed_ports | (enabled("OPENMUA2_FUSION_BUTTONS")?15:0));
  const auto want_control_ports=std::uint8_t(want_hero_ports | want_fusion_ports |
      (enabled("OPENMUA2_CONTEXT_X")?15:0));
  if (want_control_ports) {
    std::ifstream input(m_impl->metadata.main_dol, std::ios::binary);
    const std::vector<char> bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (!input.bad() && Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(bytes)) ==
                            "21822B3930C04E47176A977404A719485CD7C492") {
      const Core::CPUThreadGuard guard(Core::System::GetInstance());
      if (const char* path = std::getenv("OPENMUA2_MENU_TRACE"); path && *path)
        HLE::SetExternalStartObserver(guard, 0x801d566c, ObserveMenuActionQuery);
      const bool installed = HLE::SetExternalStartObserver(guard, 0x810f8544, ObserveMua2InputBindings);
      std::fprintf(stderr, "[openmua2] experimental X-use observer %s\n", installed ? "installed" : "rejected");
      if (installed) {
        s_control_ports=want_control_ports;
        const bool idle_installed = HLE::SetExternalStartObserver(guard, 0x80403ae4, ObserveMua2AutoSleep);
        std::fprintf(stderr, "[openmua2] managed Xbox idle-timeout observer %s\n",
                     idle_installed ? "installed" : "rejected");
      }
      if (installed && (managed_ports || enabled("OPENMUA2_DIRECT_QTE"))) {
        const bool clock = HLE::SetExternalStartObserver(guard, 0x80ed2c48, ObserveDirectQteClock);
        const bool stage = HLE::SetExternalStartObserver(guard, 0x80ed2e6c, ObserveDirectQteStage);
        const bool animation = HLE::SetExternalStartObserver(guard, 0x80ed3480, ObserveDirectQteAnimation);
        s_direct_qte_enabled = clock && stage && animation;
        std::fprintf(stderr, "[openmua2] direct button QTE %s\n",
                     s_direct_qte_enabled ? "installed" : "rejected");
      }
      if (installed && want_hero_ports) {
        s_hero_buttons_enabled = HLE::SetExternalStartObserver(guard, 0x80052fac, ObserveHeroCandidate);
        if (s_hero_buttons_enabled) s_hero_ports=want_hero_ports;
        std::fprintf(stderr, "[openmua2] experimental hero-button observer %s\n",
                     s_hero_buttons_enabled ? "installed" : "rejected");
      }
      if (installed && want_fusion_ports) {
        const bool screen = HLE::SetExternalStartObserver(guard, 0x81068a60, ObserveFusionScreenCandidate);
        const bool world = HLE::SetExternalStartObserver(guard, 0x810692c4, ObserveFusionWorldCandidate);
        s_fusion_buttons_enabled = screen && world;
        if (s_fusion_buttons_enabled) s_fusion_ports=want_fusion_ports;
        std::fprintf(stderr, "[openmua2] experimental fusion-button observers %s\n",
                     s_fusion_buttons_enabled ? "installed" : "rejected");
      }
    } else {
      std::fprintf(stderr, "[openmua2] experimental X-use rejected: executable identity mismatch\n");
    }
  }
  if (const char* path = std::getenv("MODERNGEKKO_FRAME_TIMES"); path && *path) {
    m_impl->frame_timing_path = path;
    m_impl->frame_timing = std::make_unique<moderngekko::telemetry::FrameTiming>();
  }
  if (const char* path = std::getenv("MODERNGEKKO_PRESENT_TIMES"); path && *path) {
    m_impl->presentation_timing_path = path;
    m_impl->presentation_timing =
        std::make_unique<moderngekko::telemetry::PresentationTiming>();
    m_impl->xfb_copy_hook = GetVideoEvents().after_frame_event.Register(
        [this](Core::System& system) {
          const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
              Clock::now().time_since_epoch()).count();
          // Reading the CPU clock from a separate video thread is not safe.
          const auto ticks = system.IsDualCoreMode() ? 0 : system.GetCoreTiming().GetTicks();
          m_impl->presentation_timing->Record("copy", 0, 0, ticks, ns, 0, 0, -1, -1);
        });
  }
  m_impl->before_present_hook = GetVideoEvents().before_present_event.Register(
      [this, logged = false](const PresentInfo& info) mutable {
        if (!logged) {
          logged = true;
          std::fprintf(stderr,
              "Effective video: dual_core=%d sync_gpu=%d gpu_determinism=%s "
              "immediate_xfb=%d skip_duplicate_xfb=%d vsync=%d efb_scale=%d msaa=%u "
              "smooth_early=%d rush=%d emulation_speed=%.6f "
              "cpu_overclock_enabled=%d cpu_overclock=%.6f "
              "vi_overclock_enabled=%d vi_overclock=%.6f\n",
              Core::System::GetInstance().IsDualCoreMode(), Config::Get(Config::MAIN_SYNC_GPU),
              Config::Get(Config::MAIN_GPU_DETERMINISM_MODE).c_str(),
              g_ActiveConfig.bImmediateXFB, g_ActiveConfig.bSkipPresentingDuplicateXFBs,
              g_ActiveConfig.bVSyncActive, g_ActiveConfig.iEFBScale, g_ActiveConfig.iMultisamples,
              Config::Get(Config::MAIN_SMOOTH_EARLY_PRESENTATION),
              Config::Get(Config::MAIN_RUSH_FRAME_PRESENTATION),
              Config::Get(Config::MAIN_EMULATION_SPEED), Config::Get(Config::MAIN_OVERCLOCK_ENABLE),
              Config::Get(Config::MAIN_OVERCLOCK), Config::Get(Config::MAIN_VI_OVERCLOCK_ENABLE),
              Config::Get(Config::MAIN_VI_OVERCLOCK));
        }
        if (m_impl->presentation_timing) {
          const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
              Clock::now().time_since_epoch()).count();
          const auto intended = std::chrono::duration_cast<std::chrono::nanoseconds>(
              info.intended_present_time.time_since_epoch()).count();
          m_impl->presentation_timing->Record("before", info.frame_count, info.present_count,
              info.emulated_timestamp, ns, intended, 0, static_cast<int>(info.reason),
              static_cast<int>(info.present_time_accuracy));
        }
      });
  m_impl->present_hook =
      GetVideoEvents().after_present_event.Register([this](const PresentInfo &info) {
        if (m_impl->presentation_timing) {
          const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
              Clock::now().time_since_epoch()).count();
          const auto intended = std::chrono::duration_cast<std::chrono::nanoseconds>(
              info.intended_present_time.time_since_epoch()).count();
          const auto actual = std::chrono::duration_cast<std::chrono::nanoseconds>(
              info.actual_present_time.time_since_epoch()).count();
          m_impl->presentation_timing->Record("after", info.frame_count, info.present_count,
              info.emulated_timestamp, ns, intended, actual, static_cast<int>(info.reason),
              static_cast<int>(info.present_time_accuracy));
        }
        if (m_impl->frame_timing &&
            info.reason != PresentInfo::PresentReason::VideoInterfaceDuplicate) {
          const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::steady_clock::now().time_since_epoch()).count();
          m_impl->frame_timing->Record(info.frame_count, info.present_count,
                                     info.emulated_timestamp, ns, false,
                                     Common::RuntimeTiming::Get().trace ?
                                         moderngekko::telemetry::FrameCpuCounters::Read() :
                                         moderngekko::telemetry::FrameCpuCounters{});
        }
        m_impl->automation_state.frame_count.store(info.frame_count,
                                                   std::memory_order_relaxed);
        m_impl->automation_state.present_count.store(info.present_count,
                                                     std::memory_order_relaxed);
      });
  if (!m_impl->config.automation.directory.empty()) {
    // CoreTiming clears its registered event types between core lifetimes.
    m_impl->automation_state.xbox_hold_event = nullptr;
    m_impl->automation_state.xbox_hold.reset();
    m_impl->automation_state.xbox_sequence_event = nullptr;
    m_impl->automation_state.xbox_sequence.reset();
    m_impl->automation_thread =
        std::jthread([this](std::stop_token stop_token) {
          AutomationLoop(*this, *m_impl, std::move(stop_token));
        });
  }
  const bool shutdown_trace =
      std::getenv("MODERNGEKKO_RUNTIME_SHUTDOWN_TRACE") != nullptr;
  std::jthread title_thread;
  if (!m_impl->config.headless && m_impl->config.show_fps_in_title) {
    title_thread = std::jthread([](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        Host_UpdateTitle({});
        for (int i = 0; i < 10 && !stop_token.stop_requested(); ++i)
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    });
  }
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: entering platform main loop\n");
  m_impl->platform->MainLoop();
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: platform main loop exited\n");
  title_thread.request_stop();
  if (title_thread.joinable())
    title_thread.join();
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: saving window geometry\n");
  m_impl->platform->SaveWindowGeometry();
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: stopping automation\n");
  StopAutomation();
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: stopping core\n");
  Core::Stop(Core::System::GetInstance());
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: shutting down core\n");
  Core::Shutdown(Core::System::GetInstance());
  Common::RuntimeTiming::Get().Flush();
  JitCommon::GetIndirectProfile().Flush();
  if (m_impl->frame_timing) {
    std::ofstream output(m_impl->frame_timing_path);
    m_impl->frame_timing->Write(output);
    output.close();
    if (!output)
      std::fprintf(stderr, "[moderngekko] failed to write frame timing trace\n");
    m_impl->frame_timing.reset();
  }
  if (m_impl->presentation_timing) {
    std::ofstream output(m_impl->presentation_timing_path);
    m_impl->presentation_timing->Write(output);
    output.close();
    if (!output)
      std::fprintf(stderr, "[moderngekko] failed to write presentation timing trace\n");
    m_impl->presentation_timing.reset();
  }
  if (shutdown_trace)
    std::fprintf(stderr, "[moderngekko] runtime: core shutdown complete\n");
  m_impl->booted = false;
  m_impl->running = false;
  return {};
}

void Runtime::RequestStop() {
  if (std::getenv("MODERNGEKKO_RUNTIME_SHUTDOWN_TRACE") != nullptr)
    std::fprintf(stderr, "[moderngekko] runtime: stop requested\n");
  if (m_impl && m_impl->platform)
    m_impl->platform->Stop();
}

std::optional<RuntimeError> Runtime::Pause() {
  if (!m_impl->running)
    return RuntimeError{RuntimeErrorCode::InvalidState,
                        "runtime is not running"};
  Core::SetState(Core::System::GetInstance(), Core::State::Paused);
  return {};
}

std::optional<RuntimeError> Runtime::Resume() {
  if (!m_impl->running)
    return RuntimeError{RuntimeErrorCode::InvalidState,
                        "runtime is not running"};
  Core::SetState(Core::System::GetInstance(), Core::State::Running);
  return {};
}

const RuntimeConfig &Runtime::GetConfig() const { return m_impl->config; }
const GameMetadata &Runtime::GetGameMetadata() const {
  return m_impl->metadata;
}
const std::string &Runtime::GetWindowTitle() const { return m_impl->title; }
} // namespace moderngekko
