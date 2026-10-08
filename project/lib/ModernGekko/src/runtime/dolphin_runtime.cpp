#include "moderngekko/runtime.hpp"

#include "AudioCommon/AudioCommon.h"
#include "Common/Config/Config.h"
#include "Common/Crypto/SHA1.h"
#include "Core/HLE/HLE.h"
#include "mua2_action_bindings.hpp"
#include "mua2_gamepad_actions.hpp"
#include "mua2_gamepad_aim.hpp"
#include "mua2_gamepad_turret.hpp"
#include "gamepad_port_state.hpp"
#include "mua2_gamepad_provider.hpp"
#include "mua2_tutorial_ready.hpp"
#include "mua2_xbox_prompts.hpp"
#include "managed_xbox_profile.hpp"
#include "mua2_interaction_context.hpp"
#include "mua2_wave_qte_context.hpp"
#include "moderngekko/gameplay/button_qte.hpp"
#include "mua2_fusion_buttons.hpp"
#include "mua2_fusion_entry.hpp"
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

std::uint8_t s_direct_gamepad_ports = 0;
std::array<moderngekko::controls::GamepadActions,4> s_gamepad_actions;
std::array<moderngekko::controls::FusionEntryState,4> s_fusion_entry;
std::array<moderngekko::controls::GamepadPortState,4> s_gamepad_ports;
struct GamepadTutorialClock { u32 object=0; unsigned owner=4; float seconds=0; u64 ticks=0; };
GamepadTutorialClock s_gamepad_tutorial_clock;
moderngekko::controls::GamepadSample ReadGamepadSample(unsigned port) {
  moderngekko::controls::GamepadSample sample;
  if(port>=4) return sample;
  const auto lock=ControllerEmu::EmulatedController::GetStateLock();
  // Resolve stable physical ports directly; Wii profiles do not select or
  // transform the input source. Process-local test devices replace only their
  // own port and never send input outside this runtime.
  std::shared_ptr<ciface::Core::Device> device;
  for (const auto& candidate:g_controller_interface.GetAllDevices()) {
    const auto source=candidate->GetSource();
    const auto id=candidate->GetPreferredId();
    if (!id || *id!=static_cast<int>(port)) continue;
    if (source=="OpenMUA2Test") { device=candidate; break; }
    if (source=="XInput") device=candidate;
  }
  if (!device) { s_gamepad_ports[port].Update({}); return {}; }
  // Poll the selected gamepad directly, independent of Wii report generation.
  device->UpdateInput();
  const auto* connected=device->FindInput("Connected");
  if(!connected || connected->GetState()<=0.5) { s_gamepad_ports[port].Update({}); return {}; }
  for(unsigned i=0;i<sample.inputs.size();++i) {
    const auto* input=device->FindInput(moderngekko::automation::XboxInputNames[i]);
    if(!input) { s_gamepad_ports[port].Update({}); return {}; }
    sample.inputs[i]=input->GetState();
  }
  sample.connected=true;
  s_gamepad_ports[port].Update(sample);
  return s_gamepad_ports[port].sample;
}

bool s_gamepad_provider=false;
std::uint8_t s_xbox_prompt_ports = 0;
std::uint8_t s_tutorial_ready_ports = 0;
bool s_tutorial_ready_attempted = false;
bool s_tutorial_ready_experiment = false;
bool ConfirmTutorialReady(const Core::CPUThreadGuard& guard, bool single_icon) {
  auto& system=guard.GetSystem(); auto& state=system.GetPPCState();
  const unsigned port=state.gpr[single_icon?30:25];
  if (port>=4 || !(s_tutorial_ready_ports & (1u<<port))) return false;
  const u32 entry=single_icon?0x8024d2f4:0x8024d1fc;
  const char* expected=single_icon?"28D8438ED1C800DBD893700DC4D78BAB255B4BDD":
                                    "5E69D8E42E6B59B9D4193C946A2E54493BA309DD";
  auto& memory=system.GetMemory();
  const auto* code=memory.GetPointerForRange(entry,64);
  if (!code || Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code,64))!=expected) return false;
  auto* help=memory.GetPointerForRange(state.gpr[31],80);
  const u32 input=0x81313274+port*0xbe00;
  const auto* device=memory.GetPointerForRange(input,0xbe00);
  if (!help || !device || moderngekko::controls::ReadBE(std::span<const u8>(device,4),0)!=0x811b4398) return false;
  const auto* hud=memory.GetPointerForRange(moderngekko::controls::ReadBE(std::span<const u8>(help,80),0),4);
  if (!hud || moderngekko::controls::ReadBE(std::span<const u8>(hud,4),0)!=0x81190a08) return false;
  const u64 slot_address=single_icon?u64(state.gpr[31])+32:u64(state.gpr[28])+32;
  if (slot_address<state.gpr[31] || slot_address-u64(state.gpr[31])>56) return false;
  const auto slot=moderngekko::controls::TutorialReadySlot(
      std::span<const u8>(help,80),port,unsigned(slot_address-state.gpr[31]),single_icon);
  if (!slot) return false;
  // Replace pointer hit-testing with confirmation, without synthesizing pointer
  // coordinates. Original ready aggregation, delay, close and callbacks follow.
  state.npc=single_icon?0x8024d374:0x8024d294;
  if (!s_gamepad_actions[port].ready_confirm) return true;
  help[*slot]=0; help[*slot+1]=1;
  if (single_icon) state.gpr[27]=1;
  std::fprintf(stderr,"[openmua2] tutorial A confirmation port=%u type=%u slot=%u\n",port,help[67],*slot);
  return true;
}
bool ObserveTutorialReady(const Core::CPUThreadGuard& guard) { return ConfirmTutorialReady(guard,false); }
bool ObserveTutorialReadySingle(const Core::CPUThreadGuard& guard) { return ConfirmTutorialReady(guard,true); }

void ObserveGamepadPointerWarning(const Core::CPUThreadGuard& guard) {
  if (!s_tutorial_ready_ports) return;
  auto& system=guard.GetSystem(); auto& state=system.GetPPCState(); auto& memory=system.GetMemory();
  const auto* code=memory.GetPointerForRange(0x8024c82c,64);
  if (!code || Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code,64))!=
      "7A4730FD80D75B8484EFA7B51783143C867D34CB") return;
  const auto* object=memory.GetPointerForRange(state.gpr[3],36);
  const auto* manager=memory.GetPointerForRange(0x80669a88,12828);
  if (!object || !manager ||
      moderngekko::controls::ReadBE(std::span<const u8>(object,36),24)!=0x80564290 ||
      moderngekko::controls::ReadBE(std::span<const u8>(manager,12828),0)!=0x805404f0) return;
  const auto* hud=memory.GetPointerForRange(moderngekko::controls::ReadBE(std::span<const u8>(object,36),0),4);
  if (!hud || moderngekko::controls::ReadBE(std::span<const u8>(hud,4),0)!=0x81190a08) return;
  if (moderngekko::controls::GamepadOnlyProfiles(s_tutorial_ready_ports,
      std::span<const u8>(manager+12812,16))) state.gpr[4]=0;
}

void ObserveTutorialText(const Core::CPUThreadGuard& guard) {
  if (!s_tutorial_ready_ports || !s_xbox_prompt_ports) return;
  auto& memory=guard.GetSystem().GetMemory();
  const auto* code=memory.GetPointerForRange(0x8024d740,64);
  if (!code || Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code,64))!=
      "CCE872A2BC08E81DCB1844AEE58EBA812AEF7521") return;
  const auto replace=[&](u32 address,std::size_t size,const char* hash,std::string_view text) {
    auto* bytes=memory.GetPointerForRange(address,size);
    if (!bytes || text.size()>=size ||
        Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(bytes,size))!=hash) return;
    std::memset(bytes,0,size); std::memcpy(bytes,text.data(),text.size());
  };
  replace(0x80564038,178,"09F66445DAD15BA89FFA9EF54FAF36F53AE9559E",
          "Press $XA to confirm that you are ready.\n\nPress $XB to return to the instructions.");
  replace(0x80563bd8,344,"207D7AB1D4C0AF0F2666F20AE8784D70DC7E0403",
          "Combine Your Powers!\n4 Fusion Stars required.\nHold $XLT and press $XA/$XB/$XX/$XY to choose a Fusion partner.\n"
          "This also revives all downed heroes!\n\nDown But Not Out!\n%s required.\n"
          "Choose a downed hero for a Fusion Revival!\n\nPress $XA to continue...");
}


void ObserveXboxPrompt(const Core::CPUThreadGuard& guard) {
  auto& system = guard.GetSystem();
  auto& state = system.GetPPCState();
  if (!moderngekko::controls::XboxPromptPort(s_xbox_prompt_ports, state.gpr[25])) return;
  auto& memory = system.GetMemory();
  for (const auto& [address, hash] : std::array<std::pair<u32, const char*>, 2>{{
      {0x810f7b24, "9C7877005CC4815A52A29D60D59C129E2B1C9086"},
      {0x810f7d60, "E490F512E7FA8A30B3B841DFAA9B79ADDE24E501"}}}) {
    const auto* code = memory.GetPointerForRange(address, 64);
    if (!code || Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code,64)) != hash) return;
  }
  const auto* object = memory.GetPointerForRange(state.gpr[25],4);
  if (!object || moderngekko::controls::ReadBE(std::span<const u8>(object,4),0) != 0x811b4398) return;
  // At the common return, r25 still owns the controller and r26 the token.
  // Read at most 63 bytes, accepting only a complete ASCII C string in RAM.
  std::string token;
  for (u32 i=0; i<64; ++i) {
    const std::uint64_t address = std::uint64_t(state.gpr[26]) + i;
    if (!((address>=0x80000000 && address<0x81800000) ||
          (address>=0x90000000 && address<0x94000000))) return;
    const auto* byte = memory.GetPointerForRange(static_cast<u32>(address),1);
    if (!byte) return;
    if (!*byte) break;
    if (*byte<32 || *byte>126 || i==63) return;
    token.push_back(static_cast<char>(*byte));
  }
  bool start_accept_screen = false;
  if ((token == "MENU_OK" || token == "MenuExit") &&
      moderngekko::controls::XboxPromptPort(s_hero_ports,state.gpr[25])) {
    const auto read = [&](u32 address, std::size_t size) -> std::span<const u8> {
      const auto* bytes = memory.GetPointerForRange(address,size);
      return bytes ? std::span<const u8>(bytes,size) : std::span<const u8>{};
    };
    start_accept_screen = moderngekko::controls::IsStartAcceptScreen(read);
  }
  const auto replacement=moderngekko::controls::XboxActionPrompt(token,start_accept_screen,s_gamepad_provider);
  const auto original=state.gpr[3];
  if (replacement) state.gpr[3]=static_cast<u8>(*replacement);
  static std::ofstream trace([] {
    const char* path=std::getenv("OPENMUA2_PROMPT_TRACE"); return path?path:"";
  }());
  if (trace) {
    trace << std::hex << state.gpr[25] << std::dec << ','
          << token << ',' << original << ',' << state.gpr[3] << '\n';
    trace.flush();
  }
}

// Opt-in, read-only tracing at the menu action consumer (not merely the input
// producer). Captures the caller and its action set without modifying input.
void ObserveMenuActionQuery(const Core::CPUThreadGuard& guard)
{
  auto& system = guard.GetSystem();
  const auto& state = system.GetPPCState();
  const u32 action = state.gpr[4];
  const bool gameplay_trace=std::getenv("OPENMUA2_GAMEPLAY_TRACE")!=nullptr;
  if (gameplay_trace ? !(action==9 || (action>=15 && action<=18) || action==28 || action==33) :
                       (action < 89 || action > 105)) return;
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
  if(gameplay_trace) {
    static std::map<std::tuple<u32,u32,u32,u32>,unsigned> counts;
    const u32 bit=(moderngekko::controls::ReadBE(data,4*(action/32))>>(action%32))&1;
    if(++counts[{state.spr[8],state.gpr[3],action,bit}]>4)return;
  }
  trace << system.GetCoreTiming().GetTicks() << ',' << state.spr[8] << ','
        << state.gpr[3] << ',' << action << ','
        << ((moderngekko::controls::ReadBE(data, 4 * (action / 32)) >> (action % 32)) & 1)
        << ',' << state.gpr[29] << ',' << state.gpr[30] << ',' << state.gpr[31] << '\n';
  if(gameplay_trace) trace.flush();
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
      read, state.gpr[3], state.gpr[4], state.gpr[22], state.gpr[5],
      moderngekko::controls::XboxPortEnabled(s_direct_gamepad_ports,state.gpr[22]) ?
        std::optional<unsigned>(s_gamepad_actions[(state.gpr[22]-0x81313274)/0xbe00].hero_slot) :
        std::nullopt);
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
      read, state.gpr[owner_register], state.gpr[input_register],
      moderngekko::controls::XboxPortEnabled(s_direct_gamepad_ports,state.gpr[input_register]) ?
        std::optional<int>(s_gamepad_actions[(state.gpr[input_register]-0x81313274)/0xbe00].fusion_slot) :
        std::nullopt);
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
  u64 last_clock = 0;
};

bool s_direct_qte_enabled = false;
std::array<DirectQteSession, 4> s_direct_qtes;

struct WaveQteSession {
  moderngekko::controls::WaveQteActor context;
  moderngekko::game::ButtonQte progress;
  u32 data = 0, config = 0;
  bool down = false;
  u64 last_update = 0;
};
bool s_wave_qte_enabled = false;
std::array<WaveQteSession, 4> s_wave_qtes;
void ResetDirectQtes() { s_direct_qtes = {}; s_wave_qtes = {}; s_gamepad_actions = {}; s_fusion_entry = {}; s_gamepad_ports = {}; s_gamepad_tutorial_clock = {}; }


bool DirectQteCode(const Core::CPUThreadGuard& guard, u32 address,
                   std::string_view expected) {
  const auto* code = guard.GetSystem().GetMemory().GetPointerForRange(address, 64);
  return code && Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code, 64)) == expected;
}

// Experimental until native challenge lifecycle and UI acceptance pass.
// Select the shared completion path directly; never synthesize a wave action.
void ObserveWaveQte(const Core::CPUThreadGuard& guard) {
  if (!s_wave_qte_enabled ||
      !DirectQteCode(guard, 0x8105c4c0, "631BCA4FA399550451583406F513A6AF1F7AD28C") ||
      !DirectQteCode(guard, 0x8105cb64, "6902BD64FF297A26161E5E2A8FEC809594196C80") ||
      !DirectQteCode(guard, 0x8105cc9c, "D0CFCB928F937987DCFB326FA1673A3A92E9445D")) return;
  auto& system = guard.GetSystem();
  auto& state = system.GetPPCState();
  if (state.gpr[3] != 8) return;
  const auto read = [&](u32 address, std::size_t size) -> std::span<const u8> {
    const auto end = u64(address) + size;
    if ((address & 3) || !((address >= 0x80000000 && end <= 0x81800000) ||
                          (address >= 0x90000000 && end <= 0x94000000))) return {};
    const auto* bytes = system.GetMemory().GetPointerForRange(address, size);
    return bytes ? std::span<const u8>(bytes, size) : std::span<const u8>{};
  };
  // String pool offsets can be unaligned; FindWaveQteActor bounds them itself.
  const auto actor_read = [&](u32 address, std::size_t size) -> std::span<const u8> {
    const auto* bytes = system.GetMemory().GetPointerForRange(address, size);
    return bytes ? std::span<const u8>(bytes, size) : std::span<const u8>{};
  };
  for (unsigned port = 0; port < s_wave_qtes.size(); ++port) {
    auto& session = s_wave_qtes[port];
    if (!session.context.actor || session.context.actor != state.gpr[24]) continue;
    moderngekko::controls::WaveQteActor live;
    if (!moderngekko::controls::FindWaveQteActor(actor_read, port, &live) ||
        live != session.context) { session = {}; return; }
    const auto data = read(state.gpr[27], 48);
    if (data.size() != 48 || moderngekko::controls::ReadBE(data, 0) != live.handle) return;
    const auto config_address = moderngekko::controls::ReadBE(data, 44);
    const auto config = read(config_address, 64);
    if (config.size() != 64 || moderngekko::controls::ReadBE(config, 56) != 8) return;
    const u64 identity = (u64(live.handle) << 32) | state.gpr[27];
    if (session.data != state.gpr[27] || session.config != config_address) {
      session.data = state.gpr[27]; session.config = config_address;
      session.progress.Begin(identity, port, 12, session.down);
      session.last_update = 0;
    }
    const auto ticks = system.GetCoreTiming().GetTicks();
    if (session.last_update && (ticks < session.last_update || ticks - session.last_update >
        system.GetSystemTimers().GetTicksPerSecond() / 10))
      session.progress.Update(identity, port, session.down, false);
    session.last_update = ticks;
    const auto result = session.progress.Update(identity, port, session.down, true);
    if (result.advanced)
      std::fprintf(stderr, "[openmua2] direct wave QTE actor=%08x port=%u presses=%u/%u\n",
                   live.handle, port, result.presses, result.required);
    // Skip motion sampling. Block automatic animation-end success, then select
    // the original Win/participant-cleanup branch only after twelve fresh presses.
    state.gpr[3] = 0;
    state.gpr[26] = 1;
    state.gpr[28] = result.presses == result.required ? 1 : 0;
    return;
  }
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

void AdvanceDirectQteInput(const Core::CPUThreadGuard& guard, DirectQteSession* session) {
  auto& system=guard.GetSystem();
  const auto ticks = system.GetCoreTiming().GetTicks();
  const unsigned port = static_cast<unsigned>(session - s_direct_qtes.data());
  const u64 identity = (u64(session->context.actor_handle) << 32) | session->context.target_handle;
  // A discontinuity must not convert an old held sample into a new press.
  if (session->last_update && (ticks < session->last_update ||
      ticks - session->last_update > system.GetSystemTimers().GetTicksPerSecond() / 10))
    session->progress.Update(identity, port, session->down, false);
  session->last_update = ticks;
  // Native animation updates stop when gameplay is suspended. Input polling
  // continues for menus/status, but must not advance the suspended QTE.
  const bool accepting=!s_gamepad_provider || (session->last_clock &&
      ticks>=session->last_clock &&
      ticks-session->last_clock<=system.GetSystemTimers().GetTicksPerSecond()/10);
  session->result = session->progress.Update(identity, port, session->down, accepting);
  if (session->result.advanced)
    std::fprintf(stderr, "[openmua2] direct QTE target=%08x port=%u presses=%u/%u complete=%u\n",
        session->context.target_handle, port, session->result.presses,
        session->result.required, unsigned(session->result.completed_now));
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
  session->last_clock=system.GetCoreTiming().GetTicks();
  if(!s_gamepad_provider) AdvanceDirectQteInput(guard,session);
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

void UpdateButtonQteInput(const Core::CPUThreadGuard& guard,unsigned port,u8* active,u8* values) {
  constexpr u32 value_bytes=124*4;
  const auto read=[&](u32 address,std::size_t size)->std::span<const u8> {
    const auto* bytes=guard.GetSystem().GetMemory().GetPointerForRange(address,size);
    return bytes?std::span<const u8>(bytes,size):std::span<const u8>{};
  };
  if (s_wave_qte_enabled) {
    moderngekko::controls::WaveQteActor wave;
    auto& session = s_wave_qtes[port];
    if (moderngekko::controls::FindWaveQteActor(read, port, &wave)) {
      const auto down = moderngekko::controls::ConsumeButtonQteInput(
          std::span<u8>(active, 20), std::span<u8>(values, value_bytes));
      if (!down) { session = {}; return; }
      if (session.context != wave) { session = {}; session.context = wave; }
      session.down = *down;
      s_direct_qtes[port] = {};
      return;
    }
    session = {};
  }
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
    // Animation callbacks may run less frequently than controller samples.
    // Count fresh presses here so a complete press/release cannot disappear.
    if(s_gamepad_provider && session.duration>0 &&
       FindDirectQte(guard,live.target,live.actor)==&session)
      AdvanceDirectQteInput(guard,&session);
  }
}

// Only a live button-QTE session may animate the paired tilted-X HUD cells.
// This is presentation only: press edges and native completion stay independent.
std::array<bool,2> s_gamepad_power_prompt_hooks{};
bool ReplaceGamepadPowerPrompt(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !s_xbox_prompt_ports || !s_direct_qte_enabled)return false;
  auto& state=guard.GetSystem().GetPPCState();
  const auto sprite=moderngekko::controls::XboxPowerHudSprite(
      state.gpr[3],state.gpr[4]!=0,state.gpr[5]!=0);
  if(!sprite)return false;
  if(std::getenv("OPENMUA2_POINTER_DRAW_TRACE")) {
    static unsigned seen=0;
    const unsigned bit=1u<<(state.gpr[3]-29);
    if(!(seen&bit)) {
      seen|=bit;
      std::fprintf(stderr,"[openmua2] PS2 power prompt action=%u alternate=%u static=%u sprite=%u\n",
                   state.gpr[3],state.gpr[4],state.gpr[5],*sprite);
    }
  }
  state.gpr[3]=*sprite;state.npc=state.pc+4;return true;
}
bool s_rapid_tap_prompt_installed=false;
bool s_gamepad_lockon_active=false;
std::array<bool,7> s_gamepad_lockon_hooks{};
void ObserveRapidTapPrompt(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !s_xbox_prompt_ports || !s_direct_qte_enabled)return;
  auto& system=guard.GetSystem();auto& state=system.GetPPCState();
  const auto sprite=state.gpr[6];
  // The restored PS2 grab target uses X. Keep shared block/LB sprite40 intact.
  // Sprite99 is the paired pack's static X; do not apply rapid-tap animation.
  if(s_gamepad_lockon_active && state.spr[8]==0x8024e7e8 && sprite==40) {
    state.gpr[6]=99;return;
  }
  if(std::getenv("OPENMUA2_POINTER_DRAW_TRACE")) {
    static std::array<u64,64> seen{};
    static unsigned count=0;
    const u64 key=(u64(state.spr[8])<<32)|sprite;
    if(count<seen.size() && std::find(seen.begin(),seen.begin()+count,key)==seen.begin()+count) {
      seen[count++]=key;
      std::fprintf(stderr,"[openmua2] HUD draw caller=%08x sprite=%u object=%08x slot=%u\n",state.spr[8],sprite,state.gpr[3],state.gpr[7]);
    }
  }
  if(!((sprite>=95 && sprite<=99) || sprite==104 || sprite==108))return;
  const auto* code=system.GetMemory().GetPointerForRange(0x80fa770c,16);
  const auto* hud=system.GetMemory().GetPointerForRange(state.gpr[3],4);
  static unsigned diagnostic_calls=0;
  if(std::getenv("OPENMUA2_RAPID_TAP_TRACE") && diagnostic_calls++<24)
    std::fprintf(stderr,"[openmua2] rapid cue sprite=%u object=%08x vtable=%08x target=%08x clock=%llu ticks=%llu\n",sprite,state.gpr[3],hud?moderngekko::controls::ReadBE(std::span<const u8>(hud,4),0):0,s_direct_qtes[0].context.target,static_cast<unsigned long long>(s_direct_qtes[0].last_clock),static_cast<unsigned long long>(system.GetCoreTiming().GetTicks()));
  if(!code || !hud || moderngekko::controls::ReadBE(std::span<const u8>(hud,4),0)!=0x81190a08 ||
     Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code,16))!="2B58B32A4737B01C8D8D74A363EA16C6F699F32A")return;
  for(auto& session:s_direct_qtes) {
    if(!session.context.target || FindDirectQte(guard,session.context.target)!=&session)continue;
    const auto ticks=system.GetCoreTiming().GetTicks();
    const auto frequency=system.GetSystemTimers().GetTicksPerSecond();
    if(!frequency || !session.last_clock || ticks<session.last_clock || ticks-session.last_clock>frequency/10)return;
    // Four complete presses per second, using game time (pauses stop the cue).
    state.gpr[6]=95+((ticks/(frequency/8))&1);
    return;
  }
}

// Shared CInputManager boundary, deliberately separate from the compatibility
// evaluator. Both polling callers can skip that evaluator when KPad is absent.
bool s_provider_update_installed=false, s_provider_query_installed=false;
bool s_provider_connected_installed=false, s_provider_ready_installed=false;
bool s_fusion_query_installed=false, s_fusion_selector_installed=false;
bool s_fusion_prompt_panel_installed=false;
bool s_fusion_prompt_background_installed=false, s_fusion_prompt_color_installed=false, s_gamepad_fusion_panel_show_installed=false;
bool s_provider_capabilities_installed=false;
bool s_fusion_screen_gate_installed=false, s_fusion_world_gate_installed=false;
bool s_gamepad_tutorial_clock_installed=false, s_gamepad_tutorial_accept_installed=false;
std::array<int,8> s_provider_status_last{-1,-1,-1,-1,-1,-1,-1,-1};
std::array<u64,4> s_provider_status_queries{};
std::array<u64,4> s_provider_updates{}, s_provider_queries{};
bool ProviderCode(const Core::CPUThreadGuard& guard,u32 address,std::size_t size,
                  const char* expected) {
  const auto* code=guard.GetSystem().GetMemory().GetPointerForRange(address,size);
  return code && Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(code,size))==expected;
}
// Candidate shared local-gamepad aiming. No CNetPlayManager flag is modified.
bool s_gamepad_aim_requested=false, s_gamepad_aim_active=false;
std::array<bool,6> s_gamepad_aim_hooks{};
bool s_gamepad_turret_installed=false, s_gamepad_turret_prompt_installed=false;
bool s_gamepad_nullifier_prompt_installed=false;
bool ReplaceGamepadAimInstruction(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !s_gamepad_aim_requested)return false;
  auto& state=guard.GetSystem().GetPPCState();
  // Select the shipped stick wording only within each original tutorial's
  // own branch. Do not replace tutorial identity, owner, or NetPlay state.
  if(state.pc==0x8024dbbc && s_gamepad_turret_installed) {
    state.npc=0x8024dbd0;return true;
  }
  if(state.pc==0x8024dd08 && s_gamepad_aim_active) {
    state.npc=0x8024dd1c;return true;
  }
  return false;
}
bool ReplaceGamepadTurretRotation(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !s_gamepad_aim_requested || !s_gamepad_turret_installed)return false;
  auto& system=guard.GetSystem();auto& state=system.GetPPCState();
  auto& memory=system.GetMemory();
  const auto* actor=memory.GetPointerForRange(state.gpr[26],1748);
  const auto* origin=memory.GetPointerForRange(state.gpr[31],100);
  const auto* clock=memory.GetPointerForRange(0x80629490,1472);
  if(!actor || !origin || !clock || (actor[1232]&0x80))return false;
  const auto a=std::span<const u8>(actor,1748), c=std::span<const u8>(clock,1472);
  if(moderngekko::controls::ReadBE(a,156)!=0x8052a748 ||
     moderngekko::controls::ReadBE(c,256)!=0x80534c90)return false;
  const unsigned port=moderngekko::controls::ReadBE(a,1116);
  if(port>=4 || !(s_direct_gamepad_ports&(1u<<port)))return false;
  const auto read_float=[](std::span<const u8> bytes,unsigned offset) {
    return std::bit_cast<float>(moderngekko::controls::ReadBE(bytes,offset));
  };
  const auto o=std::span<const u8>(origin,100);
  const auto rotation=moderngekko::controls::RotateGamepadTurret(
    {read_float(a,92),read_float(a,96)}, {read_float(o,92),read_float(o,96)},
    read_float(a,1740),read_float(a,1744),read_float(c,1468),ReadGamepadSample(port));
  if(!rotation)return false;
  state.ps[30].SetPS0(rotation->pitch);state.ps[31].SetPS0(rotation->yaw);
  // Preserve the original rotation setter and cleanup. No cursor validity,
  // world ray, target hit, damage, fire, or exit state is synthesized.
  state.npc=0x810cead4;
  return true;
}

u32 GamepadAimManager(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !s_gamepad_aim_active)return 0;
  auto& memory=guard.GetSystem().GetMemory();
  const auto* global=memory.GetPointerForRange(0x8081736c,4);
  if(!global)return 0;
  const u32 address=moderngekko::controls::ReadBE(std::span<const u8>(global,4),0);
  const auto* bytes=memory.GetPointerForRange(address,25864);
  return bytes && moderngekko::controls::GamepadAimContext({bytes,25864})?address:0;
}
void ObserveGamepadAimModeChange(const Core::CPUThreadGuard& guard) {
  auto& system=guard.GetSystem();const auto& state=system.GetPPCState();
  const u32 manager=GamepadAimManager(guard);
  if(!manager || state.gpr[3]!=0x80629490)return;
  auto& memory=system.GetMemory();
  const auto* game=memory.GetPointerForRange(0x80629490,36272);
  const auto* next=memory.GetPointerForRange(state.gpr[4],4);
  // Native initializer80101744..80101924 interns "mazehack" in this slot.
  const auto* maze=memory.GetPointerForRange(state.gpr[13]-22240,4);
  auto* bytes=memory.GetPointerForRange(manager,25864);
  if(!game || !next || !maze || !bytes ||
     moderngekko::controls::ReadBE({game,36272},256)!=0x80534c90)return;
  moderngekko::controls::CenterGamepadAimOnModeChange({bytes,25864},
    moderngekko::controls::ReadBE({game,36272},36268),
    moderngekko::controls::ReadBE({next,4},0),
    moderngekko::controls::ReadBE({maze,4},0));
}
bool ReplaceGamepadAimMode(const Core::CPUThreadGuard& guard) {
  auto& state=guard.GetSystem().GetPPCState();
  const u32 manager=GamepadAimManager(guard);
  if(!manager || state.gpr[3]!=manager)return false;
  state.gpr[3]=1;state.npc=LR(state);return true;
}
bool ReplaceGamepadAimSample(const Core::CPUThreadGuard& guard) {
  auto& system=guard.GetSystem();auto& state=system.GetPPCState();
  const u32 manager=GamepadAimManager(guard);const unsigned port=state.gpr[28];
  if(!manager || port>=4 || state.gpr[31]!=manager+4904+72*port)return false;
  auto& memory=system.GetMemory();
  auto* record=memory.GetPointerForRange(state.gpr[31],72);
  const auto* inputs=memory.GetPointerForRange(0x81313238,60);
  const auto* players=memory.GetPointerForRange(0x80635648,128);
  if(!record || !inputs || !players)return false;
  const bool joined=moderngekko::controls::GamepadHasJoinedPlayer({inputs,60},{players,128},port);
  // Native HUD getter80FB9030 returns singleton+0x6ec0 (CHudPointerError).
  // Its visible flag is the PS2 producer's additional centering predicate.
  const auto* help=memory.GetPointerForRange(0x812d9ea0,80);
  const auto* hud=memory.GetPointerForRange(0x812d2fe0,4);
  if(!help || !hud || moderngekko::controls::ReadBE({hud,4},0)!=0x81190a08 ||
     moderngekko::controls::ReadBE({help,80},0)!=0x812d2fe0 ||
     moderngekko::controls::ReadBE({help,80},24)!=0x80564248)return false;
  if(!moderngekko::controls::WriteGamepadAim({record,72},ReadGamepadSample(port),joined,help[20]!=0))return false;
  state.npc=0x80ffbd5c;return true;
}
bool ReplaceGamepadAimProjection(const Core::CPUThreadGuard& guard) {
  if(!GamepadAimManager(guard))return false;
  auto& state=guard.GetSystem().GetPPCState();
  // PS2 00523154/0052329C/0052342C/00523460 use display aspect (0).
  // Wii mode1 instead selects a fixed aspect in 80017260 and related getters.
  // Enter each native zero-selector instruction; preserve all projection math.
  switch(state.pc) {
  case 0x80ffb38c:state.npc=0x80ffb3c4;return true;
  case 0x80ffb51c:state.npc=0x80ffb554;return true;
  case 0x80ffb748:state.npc=0x80ffb780;return true;
  default:return false;
  }
}
std::array<bool,3> s_profile_dispatch_hooks{};
std::array<unsigned,3> s_profile_dispatch_counts{};
void ObserveProfileDispatch(const Core::CPUThreadGuard& guard) {
  const auto& state=guard.GetSystem().GetPPCState();
  auto& memory=guard.GetSystem().GetMemory();
  const auto* global=memory.GetPointerForRange(0x8081736c,4);
  if(!global)return;
  const u32 manager=moderngekko::controls::ReadBE({global,4},0);
  const auto* data=memory.GetPointerForRange(manager,27456);if(!data)return;
  const u32 active=moderngekko::controls::ReadBE({data,27456},25860);
  const auto* menu=memory.GetPointerForRange(active,10488);
  if(!menu || moderngekko::controls::ReadBE({menu,10488},10404)!=0x8118f008)return;
  const unsigned site=state.pc==0x81000a90?0:state.pc==0x80f94214?1:2;
  const auto pad=ReadGamepadSample(0);
  const bool a=pad.Down(moderngekko::controls::GamepadInput::A);
  const bool start=pad.Down(moderngekko::controls::GamepadInput::Start);
  if(s_profile_dispatch_counts[site]>=64 ||
     (s_profile_dispatch_counts[site]>=8 && !a && !start))return;
  ++s_profile_dispatch_counts[site];
  std::fprintf(stderr,"[openmua2] profile-dispatch pc=%08x lr=%08x menu=%08x r3=%08x flags=%02x mode=%u ready=%u a=%u start=%u\n",
    state.pc,LR(state),active,state.gpr[3],menu[0],
    moderngekko::controls::ReadBE({data,27456},27452),menu[10408],unsigned(a),unsigned(start));
}
void InstallProfileDispatchTrace(const Core::CPUThreadGuard& guard) {
  const char* trace=std::getenv("OPENMUA2_PROFILE_DISPATCH_TRACE");
  // Diagnostic-only. Full aiming uses the remaining observer capacity.
  if(s_gamepad_aim_requested || !trace || std::strcmp(trace,"1")!=0)return;
  constexpr std::array<u32,3> sites{0x81000a90,0x80f94214,0x80fc8738};
  constexpr std::array<const char*,3> hashes{
    "711BF556F3D0B8A9A42EE56B61EEC2FD30BD1186","24FEFB1AD0D10BBCB2EDE37A8DFE4EE400D036D3",
    "AA623108F9FDF6E6BE31DF12BC52C5F180542453"};
  for(unsigned i=0;i<3;++i)if(!s_profile_dispatch_hooks[i] && ProviderCode(guard,sites[i],32,hashes[i])) {
    s_profile_dispatch_hooks[i]=HLE::SetExternalStartObserver(guard,sites[i],ObserveProfileDispatch);
    if(s_profile_dispatch_hooks[i])std::fprintf(stderr,"[openmua2] profile-dispatch installed site=%08x\n",sites[i]);
  }
}
void InstallGamepadAim(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_aim_requested || s_gamepad_aim_active)return;
  constexpr std::array<u32,6> sites{0x81015b08,0x80ffbb58,0x80ffb38c,0x80ffb51c,0x80ffb748,0x80100454};
  constexpr std::array<unsigned,6> sizes{80,104,60,60,60,16};
  constexpr std::array<const char*,6> hashes{
    "ACA962EA4C7DC74CDF5F929914DA94966E56D0B1","B2845CF93C6BD62BA4AB83FC116F294C86AFDF97",
    "2A00CD46DAAE28AE46BEDEF221E4EA3605E22A27","46284EF5A981DD2297A5142774DAC6B3D64908A9",
    "2DBE55B4236F0D53E33755273B35850BD9D882E9","BD6EC3E04F9C6E96F9765DAF12A3B7DA2C7673F7"};
  for(unsigned i=0;i<6;++i)if(!ProviderCode(guard,sites[i],sizes[i],hashes[i]))return;
  if(!s_gamepad_aim_hooks[0])s_gamepad_aim_hooks[0]=HLE::SetExternalFunctionReplacement(guard,sites[0],ReplaceGamepadAimMode);
  if(!s_gamepad_aim_hooks[1])s_gamepad_aim_hooks[1]=HLE::SetExternalBranchReplacement(guard,sites[1],ReplaceGamepadAimSample);
  for(unsigned i=2;i<5;++i)if(!s_gamepad_aim_hooks[i])
    s_gamepad_aim_hooks[i]=HLE::SetExternalBranchReplacement(guard,sites[i],ReplaceGamepadAimProjection);
  if(!s_gamepad_aim_hooks[5])s_gamepad_aim_hooks[5]=HLE::SetExternalStartObserver(guard,sites[5],ObserveGamepadAimModeChange);
  s_gamepad_aim_active=std::all_of(s_gamepad_aim_hooks.begin(),s_gamepad_aim_hooks.end(),[](bool value){return value;});
  if(s_gamepad_aim_active)std::fprintf(stderr,"[openmua2] shared local gamepad aiming installed\n");
}
// Complete opt-in CHudTargetPoints path, preserving native timers, callbacks,
// random selection and all four input owners. Never changes NetPlay state.
bool ReplaceGamepadLockonGate(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_lockon_active)return false;
  auto& state=guard.GetSystem().GetPPCState();
  switch(state.pc) {
    case 0x8024fed0: state.npc=0x8024fee4;return true; // assign target button
    case 0x8024f43c: state.npc=0x8024f450;return true; // consume assigned action
    case 0x8024e72c: state.npc=0x8024e740;return true; // draw assigned button
    case 0x8024de54: state.npc=0x8024de68;return true; // instruction4 gamepad text
    default:return false;
  }
}
void ObserveGamepadLockonAction(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_lockon_active)return;
  auto& state=guard.GetSystem().GetPPCState();
  // Translate the Wii table's block entry to PS2 grab when the target is born.
  // The render comparison alone aliases grab back to its original sprite arm;
  // target storage stays action11, so LB cannot satisfy the X target.
  if(state.pc==0x8024ff2c && state.gpr[0]==12)state.gpr[0]=11;
  else if(state.pc==0x8024e7b4 && state.gpr[0]==11)state.gpr[0]=12;
  // The separate native star model overlays the face glyph. Keep its lifecycle
  // and the circle model, but give the star zero scale for every gamepad target.
  else if(state.pc==0x8024ea98)state.ps[1].SetPS0(0.0);
}
void InstallGamepadLockon(const Core::CPUThreadGuard& guard) {
  const auto* requested=std::getenv("OPENMUA2_GAMEPAD_LOCKON");
  if(s_gamepad_lockon_active || !requested || std::strcmp(requested,"1") ||
     !s_gamepad_provider || s_direct_gamepad_ports!=15 || !s_xbox_prompt_ports ||
     !s_rapid_tap_prompt_installed || !s_direct_qte_enabled)return;
  constexpr std::array<u32,7> sites{0x8024fed0,0x8024f43c,0x8024e72c,0x8024de54,0x8024ff2c,0x8024e7b4,0x8024ea98};
  constexpr std::array<const char*,7> hashes{
    "497903D9559A70FE50D584F5D6599100883EAA6B","B0E99921F95949A967A3FC8427A4356FCDB3E344",
    "30FE7119B906903967C316449B74A04EBC717CEF","1AFE29EE3A954E99A3173F214A03EC7983BDEB31",
    "9D8EA19A9A977EA5A24546111E548A7B5EAEE672","42BEC3A15C7BADFDF896CDAE87F1FE7A512962CB","A7780F517910EC6E31F2214A39243220D9AF419D"};
  for(unsigned i=0;i<sites.size();++i)if(!ProviderCode(guard,sites[i],16,hashes[i]))return;
  for(unsigned i=0;i<sites.size();++i)if(!s_gamepad_lockon_hooks[i])
    s_gamepad_lockon_hooks[i]=i<4 ? HLE::SetExternalBranchReplacement(guard,sites[i],ReplaceGamepadLockonGate) :
      HLE::SetExternalStartObserver(guard,sites[i],ObserveGamepadLockonAction);
  // Partially registered hooks remain inert; never enable only the consumer.
  s_gamepad_lockon_active=std::all_of(s_gamepad_lockon_hooks.begin(),s_gamepad_lockon_hooks.end(),[](bool b){return b;});
  if(s_gamepad_lockon_active)std::fprintf(stderr,"[openmua2] complete gamepad lock-on candidate installed; paired Xbox HUD pack required\n");
}
void TraceProvider(const char* kind,unsigned port,u64& count) {
  ++count;
  if(count==1 || (std::getenv("OPENMUA2_PROVIDER_TRACE") && (count&(count-1))==0))
    std::fprintf(stderr,"[openmua2] gamepad provider %s port=%u calls=%llu\n",
                 kind,port,static_cast<unsigned long long>(count));
}
// The original queue and release-mask bookkeeping follows physical evaluation.
// Keep it even when no controller is connected: script-injected actions are a
// separate source, and stale physical values must still become zero.
bool PublishProvider(const Core::CPUThreadGuard& guard,unsigned port,u32 bits_address,
                     u32 values_address) {
  if(port>=4 || !(s_direct_gamepad_ports&(1u<<port))) return false;
  auto& memory=guard.GetSystem().GetMemory();
  const u32 object=0x81313274+port*0xbe00;
  auto* input=memory.GetPointerForRange(object,0xbe00);
  const auto valid=[](u32 address,u32 size) {
    return !(address&3) && ((address>=0x80000000 && address<=0x81800000-size) ||
                           (address>=0x90000000 && address<=0x94000000-size));
  };
  if(!input || moderngekko::controls::ReadBE(std::span<const u8>(input,4),0)!=0x811b4398 ||
     !valid(bits_address,20) || !valid(values_address,496)) return false;
  // Never let corrupt caller pointers overwrite code, descriptors or each other.
  const auto overlap=[](u32 a,u32 n,u32 b,u32 m) {
    return u64(a)<u64(b)+m && u64(b)<u64(a)+n;
  };
  if(overlap(bits_address,20,values_address,496) ||
     overlap(bits_address,20,object,4+moderngekko::controls::DescriptorTableSize) ||
     overlap(values_address,496,object,4+moderngekko::controls::DescriptorTableSize)) return false;
  auto* active=memory.GetPointerForRange(bits_address,20);
  auto* values=memory.GetPointerForRange(values_address,496);
  if(!active || !values) return false;
  const auto read=[&](u32 address,std::size_t size)->std::span<const u8> {
    const auto* bytes=memory.GetPointerForRange(address,size);
    return bytes?std::span<const u8>(bytes,size):std::span<const u8>{};
  };
  bool request=false;
  const auto globals=read(0x80816a20,24);
  if(globals.size()==24 && moderngekko::controls::ReadBE(globals,0)==1) {
    const auto actor=read(moderngekko::controls::ReadBE(globals,16),0xa38);
    request=actor.size()==0xa38 && moderngekko::controls::ReadBE(actor,0x9c)==0x8052a748 &&
      !(actor[0x4d0]&0x80) && moderngekko::controls::ReadBE(actor,0x45c)==port;
  }
  auto& actions=s_gamepad_actions[port];
  actions=moderngekko::controls::BuildGamepadActions(ReadGamepadSample(port),
      moderngekko::controls::IsStartAcceptScreen(read),request,
      moderngekko::controls::GamepadHasJoinedPlayer(read(0x81313238,60),read(0x80635648,128),port),
      moderngekko::controls::ActiveMenuType(read)!=0);
  s_fusion_entry[port].Apply(actions,moderngekko::controls::ReadFusionActor(read,port));
  std::copy(actions.active.begin(),actions.active.end(),active);
  std::copy(actions.values.begin(),actions.values.end(),values);
  UpdateButtonQteInput(guard,port,active,values);
  moderngekko::controls::MergeNativeActionQueue(std::span<u8>(input,0xbe00),
                                               std::span<u8>(active,20));
  return true;
}
bool ReplaceProviderUpdate(const Core::CPUThreadGuard& guard) {
  auto& state=guard.GetSystem().GetPPCState();
  const unsigned port=state.gpr[23];
  if(!s_gamepad_provider || port>=4 || state.gpr[22]!=0x81313238 ||
     state.gpr[26]!=0x81313274+port*0xbe00 ||
     state.gpr[30]!=state.gpr[26]+0x5f08 || state.gpr[29]!=state.gpr[26]+0x5f1c ||
     state.gpr[27]!=state.gpr[26]+0xbb20 ||
     !ProviderCode(guard,0x810fc770,0x4e4,"05F032E3A4922AAD405C03013D1734C2115F6DBB")) return false;
  auto& memory=guard.GetSystem().GetMemory();
  auto* current=memory.GetPointerForRange(state.gpr[30],20);
  auto* previous=memory.GetPointerForRange(state.gpr[29],20);
  if(!current || !previous) return false;
  std::array<u8,20> old;
  std::copy_n(current,20,old.begin());
  if(!PublishProvider(guard,port,state.gpr[30],state.gpr[27])) return false;
  std::copy(old.begin(),old.end(),previous);
  // Remain inside the native update loop after the input-disabled gate. Keep
  // its activity timing and four-port iteration; omit KPad availability gates.
  state.npc=0x810fcb80;
  TraceProvider("update",port,s_provider_updates[port]);
  return true;
}
bool ReplaceProviderQuery(const Core::CPUThreadGuard& guard) {
  auto& state=guard.GetSystem().GetPPCState();
  if(!s_gamepad_provider || state.gpr[31]!=0x81313238 || state.gpr[28]>=4 ||
     !ProviderCode(guard,0x810fc5f8,0x124,"3A12236A46CB9C76365A7AE203979DE4A7C89639")) return false;
  auto& memory=guard.GetSystem().GetMemory();
  const auto* manager=memory.GetPointerForRange(state.gpr[31],60);
  if(!manager) return false;
  const unsigned port=moderngekko::controls::ReadBE(std::span<const u8>(manager,60),4+state.gpr[28]*4);
  if(port>=4 || manager[20+port]) {
    // Preserve native ownership/disabled-port rejection, including return value.
    state.gpr[3]=0;state.npc=0x810fc6fc;return true;
  }
  if(!PublishProvider(guard,port,state.gpr[30],state.gpr[29])) return false;
  // Join the original activity bookkeeping with the offset register it expects.
  state.gpr[30]=port*0xbe00;
  state.npc=0x810fc6c0;
  TraceProvider("query",port,s_provider_queries[port]);
  return true;
}
bool ReplaceGamepadStatus(const Core::CPUThreadGuard& guard, bool ready) {
  auto& system=guard.GetSystem();auto& state=system.GetPPCState();
  if(!s_gamepad_provider || state.gpr[3]!=0x81313238 || state.gpr[4]>=4 ||
     !ProviderCode(guard,ready?0x810fd1ec:0x810fd174,ready?0x118:0x78,
       ready?"CC09B94F59745A481FE2245D2ECF3C71BB9311A0":"C9A5AA566837F05C9DB7D66FADE23D5A3305951D"))return false;
  const auto* data=system.GetMemory().GetPointerForRange(state.gpr[3],60);
  if(!data || moderngekko::controls::ReadBE(std::span<const u8>(data,60),0)!=0x811b4298)return false;
  const unsigned logical=state.gpr[4];
  const unsigned physical=moderngekko::controls::ReadBE(std::span<const u8>(data,60),4+logical*4);
  std::array<bool,4> connected{};
  if(physical<4)connected[physical]=ReadGamepadSample(physical).connected;
  const auto status=moderngekko::controls::GamepadConnectionStatus(std::span<const u8>(data,60),logical,connected);
  if(!status)return false;
  const unsigned trace_index=(ready?4:0)+logical;
  if(s_provider_status_last[trace_index]!=int(*status) && std::getenv("OPENMUA2_PROVIDER_TRACE"))
    std::fprintf(stderr,"[openmua2] gamepad status %s logical=%u physical=%u available=%u\n",
                 ready?"ready":"connected",logical,physical,unsigned(*status));
  s_provider_status_last[trace_index]=int(*status);
  state.gpr[3]=*status?1:0;
  state.npc=LR(state);
  TraceProvider(ready?"ready":"connected",logical,s_provider_status_queries[logical]);
  return true;
}
// PS2 CCHBlock queries FusionPower as held (CInput +20, 0x3d3a78..a8).
// The Wii caller queries a transient edge instead. Supply the gamepad action's
// held semantics at the shared input boundary; other actions retain native edges.
// Development-only consumer inventory. Observe reads without changing actions,
// guest registers, ownership, deadlines, or gameplay. Bounded per process.
bool s_action_held_trace_installed=false, s_action_scalar_trace_installed=false;
std::array<u64,1024> s_action_consumer_seen{};
unsigned s_action_consumer_count=0;
void TraceActionConsumer(const Core::CPUThreadGuard& guard, unsigned kind) {
  const char* enabled=std::getenv("OPENMUA2_ACTION_CONSUMER_TRACE");
  if(!enabled || std::strcmp(enabled,"1")!=0) return;
  const auto& state=guard.GetSystem().GetPPCState();
  const u32 input=state.gpr[3], action=state.gpr[4];
  if(input<0x81313274 || input>=0x81313274+4*0xbe00 ||
     (input-0x81313274)%0xbe00 || action>=124) return;
  const unsigned port=(input-0x81313274)/0xbe00;
  const u64 key=(u64(LR(state))<<32)|(kind<<16)|(port<<8)|action;
  for(unsigned i=0;i<s_action_consumer_count;++i)
    if(s_action_consumer_seen[i]==key) return;
  if(s_action_consumer_count==s_action_consumer_seen.size()) return;
  s_action_consumer_seen[s_action_consumer_count++]=key;
  std::fprintf(stderr,"[openmua2] action consumer kind=%u port=%u action=%u caller=%08x\n",
               kind,port,action,LR(state));
}
void ObserveHeldActionConsumer(const Core::CPUThreadGuard& guard) {TraceActionConsumer(guard,0);}
void ObserveScalarActionConsumer(const Core::CPUThreadGuard& guard) {TraceActionConsumer(guard,1);}
bool ReplaceGamepadFusionQuery(const Core::CPUThreadGuard& guard) {
  TraceActionConsumer(guard,2);
  auto& state=guard.GetSystem().GetPPCState();
  const u32 input=state.gpr[3];
  if(!s_gamepad_provider || state.gpr[4]!=33 || input<0x81313274 ||
     input>=0x81313274+4*0xbe00 || (input-0x81313274)%0xbe00 ||
     !ProviderCode(guard,0x810f92bc,0xf4,"46E64FD6009BC2D2271DA7DD2624AF772B144FCA"))return false;
  const unsigned port=(input-0x81313274)/0xbe00;
  if(!(s_direct_gamepad_ports&(1u<<port)))return false;
  const auto* data=guard.GetSystem().GetMemory().GetPointerForRange(input,0xbe00);
  if(!data || moderngekko::controls::ReadBE(std::span<const u8>(data,4),0)!=0x811b4398)return false;
  state.gpr[3]=(data[0x5f08+7]&2)?1:0;
  state.npc=LR(state);
  return true;
}
bool ReplaceGamepadConnected(const Core::CPUThreadGuard& guard) {return ReplaceGamepadStatus(guard,false);}
bool ReplaceGamepadReady(const Core::CPUThreadGuard& guard) {return ReplaceGamepadStatus(guard,true);}

// Shared physical-device capability query (vtable +124). Gameplay uses its
// two outputs even after IsConnected/IsReady succeed. An Xbox pad provides
// complete controls on its own; no extension identity or motion is synthesized.
bool ReplaceGamepadCapabilities(const Core::CPUThreadGuard& guard) {
  auto& system=guard.GetSystem();auto& state=system.GetPPCState();
  if(!s_gamepad_provider || state.gpr[3]!=0x81313238 ||
     !ProviderCode(guard,0x810fd7b4,0x1f0,"FA4F3CA473B1E98B04C68DB89BD8E8F49BF12ADD"))return false;
  auto& memory=system.GetMemory();
  const auto* manager=memory.GetPointerForRange(state.gpr[3],60);
  if(!manager || moderngekko::controls::ReadBE(std::span<const u8>(manager,60),0)!=0x811b4298)return false;
  const auto ram=[](u32 a) {return (a>=0x80000000 && a<0x81800000) || (a>=0x90000000 && a<0x94000000);};
  if(!ram(state.gpr[5]) || !ram(state.gpr[6]))return false;
  auto* connected=memory.GetPointerForRange(state.gpr[5],1);
  auto* complete=memory.GetPointerForRange(state.gpr[6],1);
  if(!connected || !complete)return false;
  // This method takes a physical port, unlike the logical status methods.
  const unsigned physical=state.gpr[4];
  const bool available=physical<4 && ReadGamepadSample(physical).connected;
  *connected=*complete=available?1:0;
  state.npc=LR(state);
  static std::array<int,4> last{-1,-1,-1,-1};
  if(physical<4 && last[physical]!=int(available) && std::getenv("OPENMUA2_PROVIDER_TRACE")) {
    std::fprintf(stderr,"[openmua2] gamepad capabilities physical=%u connected=%u complete=%u\n",physical,unsigned(*connected),unsigned(*complete));
    last[physical]=int(available);
  }
  return true;
}

// Both native fusion pickers used to reject missing IR before the late Xbox
// candidate hook. Replace their shared selection boundary, retaining the native
// actor/type/eligibility/resource and request lifecycle code after selection.
void ObserveGamepadTutorialClock(const Core::CPUThreadGuard& guard) {
  s_gamepad_tutorial_clock={};
  if(!s_gamepad_provider || !ProviderCode(guard,0x8024ced8,64,"B2D520DA2D2678F5EF1EE88FF81283B63C800D57"))return;
  auto& system=guard.GetSystem();const auto& state=system.GetPPCState();
  const auto* help=system.GetMemory().GetPointerForRange(state.gpr[31],80);
  const float now=static_cast<float>(state.ps[1].PS0AsDouble());
  if(!help || state.gpr[30]>=4 || help[20]!=1 || help[64] || help[65] ||
     moderngekko::controls::ReadBE(std::span<const u8>(help,80),24)!=0x80564248 ||
     !std::isfinite(now) || now<0)return;
  s_gamepad_tutorial_clock={state.gpr[31],state.gpr[30],now,system.GetCoreTiming().GetTicks()};
}
void ObserveGamepadTutorialAccept(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !ProviderCode(guard,0x8024d054,64,"E8536E72613B812C2BBFFDDFFA3FC31DF79BE89A"))return;
  auto& system=guard.GetSystem();const auto& state=system.GetPPCState();
  const auto clock=s_gamepad_tutorial_clock;s_gamepad_tutorial_clock={};
  const auto ticks=system.GetCoreTiming().GetTicks();
  if(clock.object!=state.gpr[31] || clock.owner!=state.gpr[30] || ticks<clock.ticks ||
     ticks-clock.ticks>system.GetSystemTimers().GetTicksPerSecond()/60)return;
  auto* help=system.GetMemory().GetPointerForRange(clock.object,80);
  if(!help || !moderngekko::controls::AcceptGamepadTutorial(std::span<u8>(help,80),clock.owner,clock.seconds))return;
  // The original manager call and action-history cleanup execute next. Native
  // deadline handling performs hide, unpause and callbacks on a later update.
  std::fprintf(stderr,"[openmua2] gamepad tutorial accepted kind=%u owner=%u clock=%.6f\n",help[67],clock.owner,clock.seconds);
}

// Select a roster partner before either platform-specific pointer picker.
// Continue at the common roster validation; native pair eligibility, resources,
// animation and completion remain owned by CCHPowFusion_Choose.
bool ReplaceGamepadFusionSelector(const Core::CPUThreadGuard& guard) {
  auto& state=guard.GetSystem().GetPPCState();auto& memory=guard.GetSystem().GetMemory();
  if(!s_gamepad_provider || !ProviderCode(guard,0x810679b4,0xc4,"46C2174D2C1D47ACFF1685E16BDEA8FD7128A9AF"))return false;
  const auto read=[&](u32 address,std::size_t size)->std::span<const u8> {
    const auto* data=memory.GetPointerForRange(address,size);
    return data?std::span<const u8>(data,size):std::span<const u8>{};
  };
  const u32 owner=state.gpr[19];
  const auto actor=read(owner,0xa38);
  if(actor.size()!=0xa38)return false;
  const unsigned port=moderngekko::controls::ReadBE(actor,0x45c);
  if(port>=4 || !(s_direct_gamepad_ports&(1u<<port)))return false;
  const auto candidate=moderngekko::controls::FusionCandidate(read,owner,0x81313274+port*0xbe00,s_gamepad_actions[port].fusion_slot);
  if(!candidate)return false;
  state.gpr[18]=*candidate;
  state.npc=0x81067a78;
  if(*candidate && std::getenv("OPENMUA2_PROVIDER_TRACE"))
    std::fprintf(stderr,"[openmua2] gamepad fusion partner port=%u owner=%08x candidate=%08x\n",port,owner,*candidate);
  return true;
}
// The native pointer-warning panel becomes the gamepad fusion instruction panel.
// Its visibility follows fusion selection, never physical pointer availability.
bool GamepadFusionPromptActive(const Core::CPUThreadGuard& guard) {
  auto& memory=guard.GetSystem().GetMemory();
  const auto* globals=memory.GetPointerForRange(0x80816a20,24);
  if(!s_gamepad_provider || !s_xbox_prompt_ports || !globals ||
     moderngekko::controls::ReadBE(std::span<const u8>(globals,24),0)!=1)return false;
  const auto* actor=memory.GetPointerForRange(moderngekko::controls::ReadBE(std::span<const u8>(globals,24),16),0xa38);
  if(!actor || moderngekko::controls::ReadBE(std::span<const u8>(actor,0xa38),0x9c)!=0x8052a748 || (actor[0x4d0]&128))return false;
  const unsigned port=moderngekko::controls::ReadBE(std::span<const u8>(actor,0xa38),0x45c);
  return port<4 && (s_direct_gamepad_ports&(1u<<port));
}
void ObserveGamepadFusionPromptPanel(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !s_xbox_prompt_ports ||
     !ProviderCode(guard,0x8024c82c,64,"7A4730FD80D75B8484EFA7B51783143C867D34CB"))return;
  auto& state=guard.GetSystem().GetPPCState();
  const auto* panel=guard.GetSystem().GetMemory().GetPointerForRange(state.gpr[3],36);
  if(!panel || moderngekko::controls::ReadBE(std::span<const u8>(panel,36),24)!=0x80564290)return;
  const bool active=GamepadFusionPromptActive(guard);
  state.gpr[4]=active?1:0;

}
// Native Xbox glyph pixels must retain their RGB channels. The old red warning
// tint erased the blue X. A neutral tint preserves all channels on the white panel.
void ObserveGamepadFusionPromptColor(const Core::CPUThreadGuard& guard) {
  if(!GamepadFusionPromptActive(guard) || !ProviderCode(guard,0x8024c604,0x228,"64E2F7F541C94F3CE2558BBF964C79B06E9E37E1"))return;
  auto& state=guard.GetSystem().GetPPCState();
  const bool background=state.pc==0x8024c77c;
  const u32 address=state.gpr[1]+(background?32:16);
  auto* color=guard.GetSystem().GetMemory().GetPointerForRange(address,16);
  if(!color)return;
  const std::array<float,4> rgba=background?std::array<float,4>{1,1,1,1}:std::array<float,4>{0.4f,0.4f,0.4f,1.0f};
  for(unsigned i=0;i<4;++i) {
    const u32 word=std::bit_cast<u32>(rgba[i]);
    for(unsigned j=0;j<4;++j)color[i*4+j]=u8(word>>(24-j*8));
  }
}
bool ReplaceGamepadFusionPanelShow(const Core::CPUThreadGuard& guard) {
  if(!GamepadFusionPromptActive(guard) || !ProviderCode(guard,0x8024c82c,64,"7A4730FD80D75B8484EFA7B51783143C867D34CB"))return false;
  auto& state=guard.GetSystem().GetPPCState();
  auto* panel=guard.GetSystem().GetMemory().GetPointerForRange(state.gpr[31],36);
  if(!panel || moderngekko::controls::ReadBE(std::span<const u8>(panel,36),24)!=0x80564290)return false;
  panel[20]=1;state.npc=0x8024c988;return true;
}
// Shared native labels for the gamepad provider. One source serves all players.
// Exact original hashes reject other executable layouts; restored states reapply safely.
void ApplyGamepadPromptText(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider || !s_xbox_prompt_ports)return;
  struct Label { u32 address, allocation; const char* digest; const char* text; bool requires_aim=false; };
  static constexpr std::array<Label,11> labels{{
    {0x8118cd00,24,"BD79D08488A368D5CC1174546A808785D9A58EE7","$MenuAccept Assign"},
    // Both native tutorial branches and the persistent mounted HUD must agree.
    // The scripted Storm Castle encounter can select the pointer-text branch.
    {0x80563d38,140,"4D34988B785D69A53833BBA33ADBA1620A079B22",
     "Use the left stick to aim the turret.\nDefend Captain America from enemy turrets\nand incoming Doombots!\n\n\n\nPress $ATTACK to continue...",true},
    {0x80563dc4,140,"99E42FA9AE35E9E21DAA679053674ABCBCDC1B5A",
     "Use the left stick to aim the turret.\nDefend Captain America from enemy turrets\nand incoming Doombots!\n\n\n\nPress $ATTACK to continue...",true},
    {0x8053eadc,49,"1C39818079EB3169797ED344280F1557D35099F1",
     "Left stick: Aim   $ABUTTON or $BBUTTON: Fire",true},
    {0x805636b4,32,"4B29FACF1A44BBFC58D915ABBB9C09875850A9C6","Fusion: Hold \xe1 + \xa4/\xa5/\xea/\xa6"},
    {0x80563bd8,344,"207D7AB1D4C0AF0F2666F20AE8784D70DC7E0403","Combine Your Powers!\n4 Fusion Stars required.\nHold $XLT and press $XA/$XB/$XX/$XY\nto choose a Fusion partner.\nThis also revives all downed heroes!\n\n\nDown But Not Out!\n%s required.\nFuse with a downed hero to perform a\nFusion Revival and get them back into the action!\n\n\nPress     to continue..."},
    {0x8118e590,27,"0DA37C975401238F0F7FD36445245BD617DCE7BE","Connect gamepad to join"},
    {0x805390b8,296,"362D67E815D15607B115085C9D87E00D3FC9ED0D",
     "Hack the Computer!\n\nMove the left stick to steer your signal. Avoid walls and obstacles. The first player to gain access earns 1000 EXP.",true},
    {0x80563708,404,"618E3F98BEBCC94A3F38F22079702DEA3E0098F9",
     "Hack the Computer!\nSteer your signal with the left stick.\nAvoid walls and obstacles.\nBe the first player to gain access!\n\nCollect coins for extra points.\nTouch power-ups to change speed or grow your signal.\nPausing resets your hacking attempt.",true},
    {0x8056389c,427,"8E98A933ADF651BDE57F4E9B0DF3B740C26D5567",
     "Hack the Computer!\nSteer your signal with the left stick.\nAvoid walls and obstacles.\nBe the first player to gain access!\n\nCollect coins for extra points.\nTouch $HACK2 and $HACK3 to change speed.\nTouch $HACK4 to grow your signal.\nPausing resets your hacking attempt.\n\nPress $ATTACK to continue...",true},
    {0x80563a48,363,"3302BB2CCACCE36A13BBF2F36C7CDF4FBC70BB3D",
     "Hack the Computer!\n\nSteer your signal with the left stick.\nAvoid walls and obstacles to gain access.\n\nCollect coins for extra points.\nTouch $HACK2 and $HACK3 to change speed.\nTouch $HACK4 to grow your signal.",true},
  }};
  for(const auto& label:labels) {
    if(label.requires_aim && !s_gamepad_aim_active)continue;
    auto* destination=guard.GetSystem().GetMemory().GetPointerForRange(label.address,label.allocation);
    const auto size=std::strlen(label.text)+1;
    if(!destination || size>label.allocation || !std::memcmp(destination,label.text,size))continue;
    if(Common::SHA1::DigestToString(Common::SHA1::CalculateDigest(destination,label.allocation))!=label.digest)continue;
    std::memset(destination,0,label.allocation);std::memcpy(destination,label.text,size);
  }
}
void ObserveProviderEntry(const Core::CPUThreadGuard& guard) {
  if(!s_gamepad_provider) return;
  InstallGamepadAim(guard);
  InstallProfileDispatchTrace(guard);
  InstallGamepadLockon(guard);
  if(s_gamepad_aim_requested && !s_gamepad_turret_installed &&
     ProviderCode(guard,0x810ce0b4,44,"41C617B68DCDCA7A98209485EBEF8FB7FA1B7AE1")) {
    s_gamepad_turret_installed=HLE::SetExternalBranchReplacement(guard,0x810ce0c0,ReplaceGamepadTurretRotation);
    if(s_gamepad_turret_installed)std::fprintf(stderr,"[openmua2] direct gamepad turret rotation installed\n");
  }
  if(s_gamepad_turret_installed && !s_gamepad_turret_prompt_installed &&
     ProviderCode(guard,0x8024dbbc,56,"0218E6EA8CD35A6C792F2A27FDC8EAF6836EFEDA")) {
    s_gamepad_turret_prompt_installed=HLE::SetExternalBranchReplacement(guard,0x8024dbbc,ReplaceGamepadAimInstruction);
    if(s_gamepad_turret_prompt_installed)std::fprintf(stderr,"[openmua2] native analog turret instruction installed\n");
  }
  if(s_gamepad_aim_active && !s_gamepad_nullifier_prompt_installed &&
     ProviderCode(guard,0x8024dd08,56,"4C2D412B50CB147F69053565F10A450534F3AC59")) {
    s_gamepad_nullifier_prompt_installed=HLE::SetExternalBranchReplacement(guard,0x8024dd08,ReplaceGamepadAimInstruction);
    if(s_gamepad_nullifier_prompt_installed)std::fprintf(stderr,"[openmua2] native analog Nullifier instruction installed\n");
  }
  ApplyGamepadPromptText(guard);
  const char* consumers=std::getenv("OPENMUA2_ACTION_CONSUMER_TRACE");
  if(consumers && std::strcmp(consumers,"1")==0) {
    if(!s_action_held_trace_installed && ProviderCode(guard,0x810f9218,8,"C198DADE616B555716E99769E3AAB984E017A523"))
      s_action_held_trace_installed=HLE::SetExternalStartObserver(guard,0x810f9218,ObserveHeldActionConsumer);
    if(!s_action_scalar_trace_installed && ProviderCode(guard,0x810f9294,20,"C53FD7832029A6CA543C0525EE9A66309CE46504"))
      s_action_scalar_trace_installed=HLE::SetExternalStartObserver(guard,0x810f9294,ObserveScalarActionConsumer);
  }

  const char* rapid_tap=std::getenv("OPENMUA2_RAPID_TAP_GLYPHS");
  if(rapid_tap && std::strcmp(rapid_tap,"1")==0 &&
     ProviderCode(guard,0x80f5d938,0xe0,"6EF1900036030E466B4B4A7C64630D87A20DF087")) {
    constexpr std::array<u32,2> sites{0x80f5daf0,0x80f5db4c};
    for(unsigned i=0;i<sites.size();++i)
      if(!s_gamepad_power_prompt_hooks[i])
        s_gamepad_power_prompt_hooks[i]=HLE::SetExternalBranchReplacement(guard,sites[i],ReplaceGamepadPowerPrompt);
  }
  if(!s_rapid_tap_prompt_installed && rapid_tap && std::strcmp(rapid_tap,"1")==0 &&
     ProviderCode(guard,0x80fa770c,16,"2B58B32A4737B01C8D8D74A363EA16C6F699F32A"))
    s_rapid_tap_prompt_installed=HLE::SetExternalStartObserver(guard,0x80fa770c,ObserveRapidTapPrompt);

  if(ProviderCode(guard,0x8024c604,0x228,"64E2F7F541C94F3CE2558BBF964C79B06E9E37E1")) {
    if(!s_fusion_prompt_background_installed)s_fusion_prompt_background_installed=HLE::SetExternalStartObserver(guard,0x8024c77c,ObserveGamepadFusionPromptColor);
    if(!s_fusion_prompt_color_installed)s_fusion_prompt_color_installed=HLE::SetExternalStartObserver(guard,0x8024c800,ObserveGamepadFusionPromptColor);
  }
  if(!s_gamepad_fusion_panel_show_installed && ProviderCode(guard,0x8024c82c,64,"7A4730FD80D75B8484EFA7B51783143C867D34CB"))
    s_gamepad_fusion_panel_show_installed=HLE::SetExternalBranchReplacement(guard,0x8024c870,ReplaceGamepadFusionPanelShow);
  if(!s_fusion_prompt_panel_installed && ProviderCode(guard,0x8024c82c,64,"7A4730FD80D75B8484EFA7B51783143C867D34CB"))
    s_fusion_prompt_panel_installed=HLE::SetExternalStartObserver(guard,0x8024c82c,ObserveGamepadFusionPromptPanel);
  if(!s_fusion_query_installed && ProviderCode(guard,0x810f92bc,0xf4,"46E64FD6009BC2D2271DA7DD2624AF772B144FCA"))
    s_fusion_query_installed=HLE::SetExternalFunctionReplacement(guard,0x810f92bc,ReplaceGamepadFusionQuery);
  if(!s_provider_connected_installed && ProviderCode(guard,0x810fd174,0x78,"C9A5AA566837F05C9DB7D66FADE23D5A3305951D")) {
    s_provider_connected_installed=HLE::SetExternalFunctionReplacement(guard,0x810fd174,ReplaceGamepadConnected);
    std::fprintf(stderr,"[openmua2] shared gamepad connection method %s\n",s_provider_connected_installed?"installed":"rejected");
  }
  if(!s_provider_ready_installed && ProviderCode(guard,0x810fd1ec,0x118,"CC09B94F59745A481FE2245D2ECF3C71BB9311A0")) {
    s_provider_ready_installed=HLE::SetExternalFunctionReplacement(guard,0x810fd1ec,ReplaceGamepadReady);
    std::fprintf(stderr,"[openmua2] shared gamepad readiness method %s\n",s_provider_ready_installed?"installed":"rejected");
  }
  if(!s_provider_capabilities_installed && ProviderCode(guard,0x810fd7b4,0x1f0,"FA4F3CA473B1E98B04C68DB89BD8E8F49BF12ADD")) {
    s_provider_capabilities_installed=HLE::SetExternalFunctionReplacement(guard,0x810fd7b4,ReplaceGamepadCapabilities);
    std::fprintf(stderr,"[openmua2] shared gamepad capabilities method %s\n",s_provider_capabilities_installed?"installed":"rejected");
  }
  if(!s_fusion_selector_installed && ProviderCode(guard,0x810679b4,0xc4,"46C2174D2C1D47ACFF1685E16BDEA8FD7128A9AF"))
    s_fusion_selector_installed=HLE::SetExternalBranchReplacement(guard,0x81067a2c,ReplaceGamepadFusionSelector);
  if(!s_gamepad_tutorial_clock_installed && ProviderCode(guard,0x8024ced8,64,"B2D520DA2D2678F5EF1EE88FF81283B63C800D57"))
    s_gamepad_tutorial_clock_installed=HLE::SetExternalStartObserver(guard,0x8024ced8,ObserveGamepadTutorialClock);
  if(!s_gamepad_tutorial_accept_installed && ProviderCode(guard,0x8024d054,64,"E8536E72613B812C2BBFFDDFFA3FC31DF79BE89A"))
    s_gamepad_tutorial_accept_installed=HLE::SetExternalStartObserver(guard,0x8024d054,ObserveGamepadTutorialAccept);
  static u64 entries=0;
  TraceProvider("entry",0,entries);
  // REL bytes are available here. Registration during asynchronous BootCore
  // would inspect uninitialized memory and cannot validate a BL instruction.
  if(!s_provider_update_installed &&
     ProviderCode(guard,0x810fc770,0x4e4,"05F032E3A4922AAD405C03013D1734C2115F6DBB")) {
    s_provider_update_installed=HLE::SetExternalBranchReplacement(guard,0x810fcaec,ReplaceProviderUpdate);
    std::fprintf(stderr,"[openmua2] shared gamepad update boundary %s\n",s_provider_update_installed?"installed":"rejected");
  }
  if(!s_provider_query_installed &&
     ProviderCode(guard,0x810fc5f8,0x124,"3A12236A46CB9C76365A7AE203979DE4A7C89639")) {
    s_provider_query_installed=HLE::SetExternalBranchReplacement(guard,0x810fc628,ReplaceProviderQuery);
    std::fprintf(stderr,"[openmua2] shared gamepad query boundary %s\n",s_provider_query_installed?"installed":"rejected");
  }
}

// Experimental context-use rebind: observes the original evaluator, preserving
// its instructions, action thresholds, per-player queues and timing. Runs after
// evaluation so the experimental chord can retain digital use magnitude.
void ObserveMua2InputBindings(const Core::CPUThreadGuard& guard)
{
  if(s_gamepad_provider) return; // shared provider owns these action buffers
  // BootCore starts asynchronously. Validate/install DOL branch hooks only
  // once the real input evaluator is running and guest RAM is initialized.
  if (s_tutorial_ready_experiment && s_direct_gamepad_ports && !s_tutorial_ready_attempted) {
    s_tutorial_ready_attempted=true;
      if (s_direct_gamepad_ports) {
        const bool ready=HLE::SetExternalBranchReplacement(guard,0x8024d1fc,ObserveTutorialReady);
        const bool single=HLE::SetExternalBranchReplacement(guard,0x8024d2f4,ObserveTutorialReadySingle);
        if (ready && single) {
          s_tutorial_ready_ports=s_direct_gamepad_ports;
          HLE::SetExternalStartObserver(guard,0x8024d740,ObserveTutorialText);
          HLE::SetExternalStartObserver(guard,0x8024c82c,ObserveGamepadPointerWarning);
        }
        std::fprintf(stderr,"[openmua2] shared gamepad readiness %s\n",ready&&single?"installed":"rejected");
      }
  }

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
  const bool direct=moderngekko::controls::XboxPortEnabled(s_direct_gamepad_ports,object);
  if (!direct) {
  const auto result = moderngekko::controls::RebindContextUse(
      std::span<u8>(data + 4, moderngekko::controls::DescriptorTableSize));
  if (result == moderngekko::controls::RebindResult::Applied)
    std::fprintf(stderr, "[openmua2] experimental X-use descriptor applied to input object %08x\n", object);
  // Rebinding occurs after this evaluation. Its first output still uses the
  // stock descriptor; normalize only subsequent evaluations of our chord.
  if (result != moderngekko::controls::RebindResult::AlreadyApplied)
    return;
  }
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
      (!direct && !moderngekko::controls::NormalizeChordUse(std::span<const u8>(active, 20),
                                               std::span<u8>(values, value_bytes))))
    return;
  constexpr u32 first_input = 0x81313274, input_stride = 0xbe00;
  if (object < first_input || object >= first_input + 4 * input_stride ||
      (object - first_input) % input_stride)
    return;
  const auto read = [&](u32 address, std::size_t size) -> std::span<const u8> {
    const auto* bytes = memory.GetPointerForRange(address, size);
    return bytes ? std::span<const u8>(bytes, size) : std::span<const u8>{};
  };
  if(direct) {
    const unsigned port=(object-first_input)/input_stride;
    bool request=false;
    const auto globals=read(0x80816a20,24);
    if(globals.size()==24 && moderngekko::controls::ReadBE(globals,0)==1) {
      const auto owner=moderngekko::controls::ReadBE(globals,16);
      const auto actor=read(owner,0xa38);
      request=actor.size()==0xa38 && moderngekko::controls::ReadBE(actor,0x9c)==0x8052a748 &&
              !(actor[0x4d0]&0x80) && moderngekko::controls::ReadBE(actor,0x45c)==port;
    }
    auto& actions=s_gamepad_actions[port];
    actions=moderngekko::controls::BuildGamepadActions(ReadGamepadSample(port),
        moderngekko::controls::IsStartAcceptScreen(read),request);
    std::copy(actions.active.begin(),actions.active.end(),active);
    std::copy(actions.values.begin(),actions.values.end(),values);
  } else {
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
  }
  const unsigned port = (object - first_input) / input_stride;
  UpdateButtonQteInput(guard,port,active,values);

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
    device->connected = command.xbox_connected ? 1.0 : 0.0;
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
      run->device->connected = command.xbox_connected ? 1.0 : 0.0;
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
      device = std::make_shared<automation::XboxTestDevice>(port);
      if (!g_controller_interface.AddDevice(device)) {
        device.reset();
        return RuntimeError{RuntimeErrorCode::InvalidState, "could not create process-local Xbox test device"};
      }
      const auto lock = ControllerEmu::EmulatedController::GetStateLock();
      ciface::Touch::UnregisterWiiInputOverrider(port);
      if (!s_gamepad_provider) {
        auto* controller = Wiimote::GetConfig()->GetController(port);
        controller->SetDefaultDevice(device->GetQualifiedName());
        controller->UpdateReferences(g_controller_interface);
      }
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
      device->connected = command.xbox_connected ? 1.0 : 0.0;
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
      if(std::getenv("OPENMUA2_PROVIDER_TRACE"))
        std::fprintf(stderr,"[openmua2] after load hooks update=%u query=%u evaluator=%u provider=%u\n",
          HLE::GetHookByAddress(0x810fc770),HLE::GetHookByAddress(0x810fc5f8),
          HLE::GetHookByAddress(0x810f8544),unsigned(s_gamepad_provider));
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
  s_control_ports=s_hero_ports=s_fusion_ports=s_direct_gamepad_ports=0;
  s_xbox_prompt_ports=0;
  s_tutorial_ready_ports=0;
  s_tutorial_ready_attempted=false;
  s_direct_qte_enabled = false;
  s_wave_qte_enabled = false;
  ResetDirectQtes();
  std::ifstream profile(m_impl->config.user_directory / "Config" / "WiimoteNew.ini");
  const std::string profile_text{std::istreambuf_iterator<char>(profile),std::istreambuf_iterator<char>()};
  const auto managed_ports=profile.bad()?0:moderngekko::controls::ManagedXboxPorts(profile_text);
  const auto enabled=[](const char* name) {
    const char* value=std::getenv(name);return value && std::string_view(value)=="1";
  };
  s_gamepad_provider=enabled("OPENMUA2_GAMEPAD_PROVIDER");
  s_gamepad_aim_requested=s_gamepad_provider && enabled("OPENMUA2_GAMEPAD_AIM");
  s_profile_dispatch_hooks={};s_profile_dispatch_counts={};
  s_gamepad_aim_active=false;s_gamepad_aim_hooks={};s_gamepad_turret_installed=false;s_gamepad_turret_prompt_installed=false;
  s_gamepad_nullifier_prompt_installed=false;
  s_provider_update_installed=s_provider_query_installed=false;
  s_provider_connected_installed=s_provider_ready_installed=false;
  s_fusion_query_installed=s_fusion_selector_installed=false;
  s_fusion_prompt_panel_installed=false;
  s_rapid_tap_prompt_installed=false;s_gamepad_power_prompt_hooks={};
  s_gamepad_lockon_active=false;s_gamepad_lockon_hooks={};
  s_action_held_trace_installed=s_action_scalar_trace_installed=false;
  s_action_consumer_count=0;s_action_consumer_seen={};
  s_fusion_prompt_background_installed=s_fusion_prompt_color_installed=s_gamepad_fusion_panel_show_installed=false;
  s_provider_capabilities_installed=false;
  s_fusion_screen_gate_installed=s_fusion_world_gate_installed=false;
  s_provider_status_queries={};
  s_provider_status_last.fill(-1);
  s_gamepad_tutorial_clock_installed=s_gamepad_tutorial_accept_installed=false;
  s_provider_updates={};s_provider_queries={};
  s_tutorial_ready_experiment=enabled("OPENMUA2_TUTORIAL_READY_EXPERIMENT");
  const auto want_hero_ports=std::uint8_t((s_gamepad_provider?15:managed_ports) | (enabled("OPENMUA2_HERO_BUTTONS")?15:0));
  const auto want_fusion_ports=std::uint8_t((s_gamepad_provider?15:managed_ports) | (enabled("OPENMUA2_FUSION_BUTTONS")?15:0));
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
        if(s_gamepad_provider || enabled("OPENMUA2_DIRECT_GAMEPAD")) {
          s_direct_gamepad_ports=s_gamepad_provider?15:managed_ports;
          std::fprintf(stderr,"[openmua2] direct gamepad actions ports=%u (development)\n",s_direct_gamepad_ports);
        }
        const bool idle_installed = HLE::SetExternalStartObserver(guard, 0x80403ae4, ObserveMua2AutoSleep);
        std::fprintf(stderr, "[openmua2] managed Xbox idle-timeout observer %s\n",
                     idle_installed ? "installed" : "rejected");
      }
      if(installed && s_gamepad_provider) {
        const bool update=HLE::SetExternalStartObserver(guard,0x810fc770,ObserveProviderEntry);
        const bool query=HLE::SetExternalStartObserver(guard,0x810fc5f8,ObserveProviderEntry);
        std::fprintf(stderr,"[openmua2] four-port gamepad provider entry hooks %s\n",update&&query?"installed":"rejected");
      }
      // Development-only until the matching font pack and full prompt audit
      // are staged together. Legacy custom profiles remain excluded; the shared
      // gamepad provider owns all four ports independently of Wii bindings.
      if (installed && (s_gamepad_provider || managed_ports) && enabled("OPENMUA2_XBOX_GLYPHS")) {
        const bool prompts=HLE::SetExternalStartObserver(guard,0x810f7d60,ObserveXboxPrompt);
        if (prompts) s_xbox_prompt_ports=s_gamepad_provider?15:managed_ports;
        std::fprintf(stderr,"[openmua2] Xbox prompt observer %s\n",prompts?"installed":"rejected");
      }
      if (installed && (s_gamepad_provider || managed_ports || enabled("OPENMUA2_DIRECT_QTE"))) {
        const bool clock = HLE::SetExternalStartObserver(guard, 0x80ed2c48, ObserveDirectQteClock);
        const bool stage = HLE::SetExternalStartObserver(guard, 0x80ed2e6c, ObserveDirectQteStage);
        const bool animation = HLE::SetExternalStartObserver(guard, 0x80ed3480, ObserveDirectQteAnimation);
        s_direct_qte_enabled = clock && stage && animation;
        std::fprintf(stderr, "[openmua2] direct button QTE %s\n",
                     s_direct_qte_enabled ? "installed" : "rejected");
      }
      if (installed && managed_ports && enabled("OPENMUA2_WAVE_QTE")) {
        s_wave_qte_enabled = HLE::SetExternalStartObserver(guard, 0x8105c4c0, ObserveWaveQte);
        std::fprintf(stderr, "[openmua2] experimental direct wave QTE %s\n",
                     s_wave_qte_enabled ? "installed" : "rejected");
      }
      if (installed && want_hero_ports) {
        s_hero_buttons_enabled = HLE::SetExternalStartObserver(guard, 0x80052fac, ObserveHeroCandidate);
        if (s_hero_buttons_enabled) s_hero_ports=want_hero_ports;
        std::fprintf(stderr, "[openmua2] experimental hero-button observer %s\n",
                     s_hero_buttons_enabled ? "installed" : "rejected");
      }
      if (installed && s_gamepad_provider) s_fusion_ports=want_fusion_ports;
      if (installed && !s_gamepad_provider && want_fusion_ports) {
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
