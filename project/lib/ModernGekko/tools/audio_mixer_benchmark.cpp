// Synthetic, offline CPU-cost diagnostic. No audio device, game or OS input.
#include "AudioCommon/Mixer.h"
#include "AudioCommon/PerformanceDiagnostics.h"
#include "Common/Config/Config.h"
#include "Common/Swap.h"
#include "Core/Config/MainSettings.h"
#include "process_affinity.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>

namespace {
double CpuSeconds() {
#ifdef _WIN32
  FILETIME created{}, exited{}, kernel{}, user{};
  if (!GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user))
    return -1;
  const auto ticks = [](FILETIME v) {
    return (std::uint64_t(v.dwHighDateTime) << 32) | v.dwLowDateTime;
  };
  return (ticks(kernel) + ticks(user)) * 1e-7;
#else
  return -1;
#endif
}
}

int main() {
#ifdef _WIN32
  const auto affinity = moderngekko::frontend::PinProcessToOneProcessor();
  if (affinity.error != ERROR_SUCCESS) return 1;
  std::cerr << "One logical processor; mask=" << affinity.mask << '\n';
#endif
  Config::Init();
  Config::SetCurrent(Config::MAIN_AUDIO_FILL_GAPS, false);
  constexpr std::size_t frames = 512;
  constexpr int iterations = 4096;
  std::array<s16, frames * 2> input{}, output{};
  for (std::size_t i = 0; i < frames; ++i) {
    const s16 tone = static_cast<s16>((static_cast<int>(i % 96) - 48) * 256);
    input[i * 2] = Common::swap16(tone);
    input[i * 2 + 1] = Common::swap16(static_cast<s16>(-tone));
  }
  for (int repeat = 0; repeat < 3; ++repeat) {
    // Routing here only changes the mixer's channel inclusion. No streams or
    // device discovery are initialized, and no production config is written.
    for (int mode = 0; mode < 5; ++mode) {
      const bool seven = mode == 1 || mode == 4;
      const bool single = mode == 2;
      const bool feed = mode >= 3;
      Config::SetCurrent(Config::MAIN_WIIMOTE_AUDIO_ROUTING_ENABLED, seven);
      for (const auto& setting : Config::MAIN_WIIMOTE_AUDIO_OUTPUT_ENABLED)
        Config::SetCurrent(setting, seven);
      auto mixer = std::make_unique<Mixer>(48000);
      mixer->SetDMAInputSampleRateDivisor(Mixer::FIXED_SAMPLE_RATE_DIVIDEND / 48000);
      const auto mix = [&] {
        if (feed) mixer->PushSamples(input.data(), frames);
        if (single) mixer->MixWiimoteSpeaker(0, output.data(), frames);
        else mixer->Mix(output.data(), frames);
      };
      for (int i = 0; i < 128; ++i) mix();
      const auto start = std::chrono::steady_clock::now();
      const double cpu_start = CpuSeconds();
      for (int i = 0; i < iterations; ++i) mix();
      const double cpu_end = CpuSeconds();
      const double cpu = cpu_start < 0 || cpu_end < 0 ? -1 : cpu_end - cpu_start;
      const double wall = std::chrono::duration<double>(
          std::chrono::steady_clock::now() - start).count();
      const bool silent = std::all_of(output.begin(), output.end(), [](s16 v) { return v == 0; });
      if (silent == feed) return 2;
      std::int64_t checksum = 0;
      for (std::size_t i = 0; i < output.size(); ++i)
        checksum += std::int64_t(output[i]) * (i + 1);
      std::cout << "{\"repeat\":" << repeat << ",\"mode\":" << mode
                << ",\"channels\":" << (single ? 1 : seven ? 7 : 11)
                << ",\"dma_tone\":" << (feed ? "true" : "false")
                << ",\"output_frames\":" << frames * iterations
                << ",\"represented_audio_seconds\":" << (frames * iterations / 48000.0)
                << ",\"wall_seconds\":" << wall << ",\"cpu_seconds\":" << cpu
                << ",\"last_block_checksum\":" << checksum << "}\n";
    }
  }
  Config::Shutdown();
  AudioCommon::Performance::Get().Flush();
}
