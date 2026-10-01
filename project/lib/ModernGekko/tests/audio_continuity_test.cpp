// Offline continuity test: no game, audio device, desktop input or capture.
#include "AudioCommon/Mixer.h"
#include "Common/Config/Config.h"
#include "Common/Swap.h"
#include "Core/Config/MainSettings.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

std::vector<s16> Render(bool streaming, bool interrupt_producer) {
  auto mixer = std::make_unique<Mixer>(48000);
  const unsigned input_rate = streaming ? 48000 : 32000;
  mixer->SetDMAInputSampleRateDivisor(Mixer::FIXED_SAMPLE_RATE_DIVIDEND / input_rate);
  mixer->SetStreamInputSampleRateDivisor(Mixer::FIXED_SAMPLE_RATE_DIVIDEND / input_rate);
  const unsigned input_frames = input_rate / 100;
  unsigned input_index = 0, pending = 0;
  std::vector<s16> input(input_frames * 2), result;
  std::array<s16, 480 * 2> output{};
  for (unsigned block = 0; block < 80; ++block) {
    ++pending;
    // Simulate a 20ms producer interruption, then its normal catch-up work.
    if (!interrupt_producer || (block != 20 && block != 21)) {
      while (pending) {
        --pending;
        for (unsigned i = 0; i < input_frames; ++i, ++input_index) {
          const double t = input_index * 48000.0 / input_rate;
          input[2*i] = Common::swap16(static_cast<s16>(std::lround(5000 * std::cos(t * .011))));
          input[2*i+1] = Common::swap16(static_cast<s16>(std::lround(6000 * std::sin(t * .071) + 4000 * std::sin(t * .023))));
        }
        if (streaming) mixer->PushStreamingSamples(input.data(), input_frames);
        else mixer->PushSamples(input.data(), input_frames);
      }
    }
    mixer->Mix(output.data(), 480);
    result.insert(result.end(), output.begin(), output.end());
  }
  return result;
}

int main() {
  Config::Init();
  Config::SetCurrent(Config::MAIN_AUDIO_BUFFER_SIZE, 80);
  // Recovery/repetition cannot disguise a gap in this test.
  Config::SetCurrent(Config::MAIN_AUDIO_FILL_GAPS, false);
  for (bool streaming : {false, true}) {
    const auto reference = Render(streaming, false);
    const auto interrupted = Render(streaming, true);
    const auto first = std::find_if(reference.begin(), reference.end(), [](s16 v) { return v != 0; });
    const double startup_ms = (first - reference.begin()) / 2.0 / 48.0;
    if (startup_ms < 20 || startup_ms > 65 || reference != interrupted) {
      std::cerr << "Continuity failure: streaming=" << streaming << " startup_ms=" << startup_ms << '\n';
      return 1;
    }
    std::cout << (streaming ? "48kHz music" : "32kHz DMA") << ": startup_ms=" << startup_ms
              << ", 20ms producer interruption is sample-identical to uninterrupted output\n";
  }
  Config::Shutdown();
}
