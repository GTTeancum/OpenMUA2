#include "../vendor/dolphin/Source/Core/AudioCommon/DiagnosticCapture.h"
#include <array>
#include <sstream>

int main()
{
  using AudioCommon::DiagnosticCapture::StereoRing;
  StereoRing ring(3);
  const std::array<std::int16_t, 4> a{1, -1, 2, -2};
  const std::array<std::int16_t, 4> b{3, -3, 32767, -32768};
  ring.Append(a); ring.Append(b);
  std::ostringstream wav;
  if (!ring.WriteWav(wav, 48000) || ring.TotalFrames() != 4 || ring.RetainedFrames() != 3) return 1;
  const auto text = wav.str();
  const std::array<unsigned char, 12> expected{2,0,254,255,3,0,253,255,255,127,0,128};
  if (text.size() != 56 || text.substr(0,4) != "RIFF" || text.substr(8,8) != "WAVEfmt " ||
      text.substr(36,4) != "data" || static_cast<unsigned char>(text[4]) != 48 ||
      static_cast<unsigned char>(text[24]) != 128 || static_cast<unsigned char>(text[25]) != 187 ||
      static_cast<unsigned char>(text[40]) != 12) return 2;
  for (std::size_t i = 0; i < expected.size(); ++i)
    if (static_cast<unsigned char>(text[44+i]) != expected[i]) return 3;
  const std::array<std::int16_t, 10> large{0,0,1,1,2,2,3,3,4,4};
  ring.Append(large);
  const std::array<std::int16_t, 1> odd{99};
  ring.Append(odd);
  std::ostringstream wrapped;
  if (!ring.WriteWav(wrapped, 48000) || ring.TotalFrames() != 9 || ring.RetainedFrames() != 3) return 4;
  const auto tail = wrapped.str();
  for (int i = 0; i < 6; ++i)
    if (tail[44 + i*2] != 2 + i/2 || tail[45 + i*2] != 0) return 5;
  StereoRing empty(0);
  empty.Append(a);
  std::ostringstream silence;
  if (!empty.WriteWav(silence, 48000) || silence.str().size() != 44 || empty.TotalFrames()) return 6;
  return 0;
}
