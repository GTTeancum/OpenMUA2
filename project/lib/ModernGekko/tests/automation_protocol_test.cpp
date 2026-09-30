#include "automation_protocol.hpp"

#include <filesystem>
#include <fstream>

#include "InputCommon/ControllerInterface/Touch/InputOverrider.h"

namespace
{
std::filesystem::path MakeTempDirectory()
{
  const auto path = std::filesystem::temp_directory_path() /
                    std::filesystem::path("moderngekko_automation_test");
  std::error_code ec;
  std::filesystem::remove_all(path, ec);
  std::filesystem::create_directories(path, ec);
  return path;
}
}

int main()
{
  namespace automation = moderngekko::automation;

  const std::filesystem::path root = MakeTempDirectory();
  const std::filesystem::path commands = root / "commands";
  std::filesystem::create_directories(commands);

  {
    std::ofstream output(commands / "002_pad.txt");
    output << "# comment\n"
           << "command=pad\n"
           << "port=1\n"
           << "a=1\n"
           << "main_x=0.5\n"
           << "main_y=-1\n";
  }
  {
    std::ofstream output(commands / "001_pause.txt");
    output << "command=pause\n";
  }

  const auto listed = automation::ListCommandFiles(commands);
  if (listed.size() != 2 || listed[0].filename() != "001_pause.txt" ||
      listed[1].filename() != "002_pad.txt")
  {
    return 1;
  }

  automation::Command command;
  std::string error;
  if (!automation::ParseCommandFile(commands / "002_pad.txt", &command, &error))
    return 2;
  if (command.type != automation::CommandType::Pad || command.pad.port != 1)
    return 3;
  if (command.pad.controls[0] != 1.0 || command.pad.controls[14] != 0.5 ||
      command.pad.controls[15] != -1.0)
  {
    return 4;
  }

  {
    std::ofstream output(commands / "003_bad.txt");
    output << "command=pad\nport=0\nunknown=1\n";
  }
  error.clear();
  if (automation::ParseCommandFile(commands / "003_bad.txt", &command, &error) ||
      error.find("unknown pad field") == std::string::npos)
  {
    return 5;
  }

  {
    std::ofstream output(commands / "010_read_timing.txt");
    output << "command=read_timing\npath=artifacts/timing.txt\n";
  }
  if (!automation::ParseCommandFile(commands / "010_read_timing.txt", &command, &error) ||
      command.type != automation::CommandType::ReadTiming ||
      command.path != std::filesystem::path("artifacts/timing.txt"))
    return 20;
  {
    std::ofstream output(commands / "011_bad_timing.txt");
    output << "command=read_timing\n";
  }
  if (automation::ParseCommandFile(commands / "011_bad_timing.txt", &command, &error))
    return 21;

  automation::Status status;
  {
    std::ofstream output(commands / "012_motion.txt");
    output << "command=pad_frames\nport=0\nframes=3\nnunchuk_z=1\n"
              "nunchuk_accel_dx=-30\nnunchuk_accel_dz=24\nwii_accel_dy=25\n"
              "nunchuk_shake_x=1\nrelease=0\n";
  }
  if (!automation::ParseCommandFile(commands / "012_motion.txt", &command, &error) ||
      command.pad.controls[ciface::Touch::NUNCHUK_ACCEL_DELTA_X] != -30 ||
      command.pad.controls[ciface::Touch::NUNCHUK_ACCEL_DELTA_Z] != 24 ||
      command.pad.controls[ciface::Touch::NUNCHUK_ACCEL_DELTA_Y] != 0 ||
      command.pad.controls[ciface::Touch::WIIMOTE_ACCEL_DELTA_Y] != 25 ||
      command.pad.controls[ciface::Touch::WIIMOTE_ACCEL_DELTA_Z] != 0 ||
      command.pad.controls[ciface::Touch::NUNCHUK_SHAKE_X] != 1 ||
      command.pad.controls[ciface::Touch::NUNCHUK_SHAKE_Y] != 0 || command.release_pad)
    return 22;
  {
    std::ofstream output(commands / "013_bad_motion.txt");
    output << "command=pad\nport=0\nnunchuk_accel_dx=31\n";
  }
  if (automation::ParseCommandFile(commands / "013_bad_motion.txt", &command, &error))
    return 23;
  {
    std::ofstream output(commands / "014_release.txt");
    output << "command=pad_frames\nport=0\nframes=2\n";
  }
  if (!automation::ParseCommandFile(commands / "014_release.txt", &command, &error) ||
      !command.release_pad)
    return 24;
  {
    std::ofstream output(commands / "015_bad_release.txt");
    output << "command=pad_frames\nport=0\nframes=2\nrelease=2\n";
  }
  if (automation::ParseCommandFile(commands / "015_bad_release.txt", &command, &error))
    return 25;
  status.state = "running";
  status.booted = true;
  status.fps = 60.0;
  status.frame_count = 120;
  status.present_count = 121;
  status.navigation_mode = 2;
  status.navigation_valid = true;
  status.navigation_room_id = 3;
  status.navigation_player_x = -13.0f;
  status.navigation_player_z = 168.0f;
  status.navigation_collision_valid = true;
  status.navigation_collision_base = 0x809f58c0;
  status.navigation_collision_segments = 184;
  status.navigation_npc_count = 2;
  status.navigation_archive = "M1_stadium_1F";
  status.navigation_location = "Phenac Stadium 1F";
  status.last_command = "002_pad.txt";
  const std::string formatted = automation::FormatStatus(status);
  if (!formatted.contains("state=running\n") ||
      !formatted.contains("booted=1\n") ||
      !formatted.contains("frame_count=120\n") ||
      !formatted.contains("present_count=121\n") ||
      !formatted.contains("navigation_mode=2\n") ||
      !formatted.contains("navigation_valid=1\n") ||
      !formatted.contains("navigation_room_id=3\n") ||
      !formatted.contains("navigation_player_x=-13\n") ||
      !formatted.contains("navigation_collision_valid=1\n") ||
      !formatted.contains("navigation_collision_base=2157926592\n") ||
      !formatted.contains("navigation_collision_segments=184\n") ||
      !formatted.contains("navigation_npc_count=2\n") ||
      !formatted.contains("navigation_archive=M1_stadium_1F\n") ||
      !formatted.contains("navigation_location=Phenac Stadium 1F\n") ||
      !formatted.contains("last_command=002_pad.txt\n"))
  {
    return 6;
  }

  {
    std::ofstream output(commands / "004_read_memory.txt");
    output << "command=read_memory\n"
           << "address=0x809e52b0\n"
           << "size=32\n"
           << "path=artifacts/player.bin\n";
  }
  error.clear();
  if (!automation::ParseCommandFile(commands / "004_read_memory.txt", &command, &error))
    return 7;
  if (command.type != automation::CommandType::ReadMemory ||
      command.address != 0x809e52b0 || command.size != 32 ||
      command.path != std::filesystem::path("artifacts/player.bin"))
  {
    return 8;
  }

  {
    std::ofstream output(commands / "005_bad_memory.txt");
    output << "command=read_memory\n"
           << "address=0xfffffff0\n"
           << "size=32\n"
           << "path=artifacts/bad.bin\n";
  }
  error.clear();
  if (automation::ParseCommandFile(commands / "005_bad_memory.txt", &command, &error) ||
      error.find("exceed the guest address space") == std::string::npos)
  {
    return 9;
  }

  {
    std::ofstream output(commands / "006_write_memory.txt");
    output << "command=write_memory\n"
           << "address=0x809e52bc\n"
           << "data=42 f6 00 00\n";
  }
  error.clear();
  if (!automation::ParseCommandFile(commands / "006_write_memory.txt", &command, &error))
    return 10;
  if (command.type != automation::CommandType::WriteMemory ||
      command.address != 0x809e52bc || command.size != 4 ||
      command.data != std::vector<std::uint8_t>({0x42, 0xf6, 0x00, 0x00}))
  {
    return 11;
  }

  {
    std::ofstream output(commands / "007_pad_frames.txt");
    output << "command=pad_frames\n"
           << "port=0\n"
           << "frames=3\n"
           << "main_y=1\n";
  }
  error.clear();
  if (!automation::ParseCommandFile(commands / "007_pad_frames.txt", &command, &error))
    return 12;
  if (command.type != automation::CommandType::PadFrames || command.frames != 3 ||
      command.pad.port != 0 || command.pad.controls[15] != 1.0)
  {
    return 13;
  }

  {
    std::ofstream output(commands / "008_bad_pad_frames.txt");
    output << "command=pad_frames\nport=0\nframes=0\na=1\n";
  }
  error.clear();
  if (automation::ParseCommandFile(commands / "008_bad_pad_frames.txt", &command, &error) ||
      error.find("frames=1..36000") == std::string::npos)
  {
    return 14;
  }

  {
    std::ofstream output(commands / "009_jit_profile.txt");
    output << "command=jit_profile_dump\npath=private/blocks.tsv\n";
  }
  error.clear();
  if (!automation::ParseCommandFile(commands / "009_jit_profile.txt", &command, &error) ||
      command.type != automation::CommandType::JitProfileDump ||
      command.path != std::filesystem::path("private/blocks.tsv"))
    return 15;
  {
    std::ofstream output(commands / "009_jit_profile.txt");
    output << "command=jit_profile_dump\n";
  }
  error.clear();
  if (automation::ParseCommandFile(commands / "009_jit_profile.txt", &command, &error) ||
      error.find("path=<file>") == std::string::npos)
    return 16;
  {
    std::ofstream output(commands / "009_jit_profile.txt");
    output << "command=jit_profile_reset\n";
  }
  error.clear();
  if (!automation::ParseCommandFile(commands / "009_jit_profile.txt", &command, &error) ||
      command.type != automation::CommandType::JitProfileReset)
    return 17;

  {
    std::ofstream output(commands / "xbox.txt");
    output << "command=xbox_frames\nport=0\nframes=5\na=1\nright_left=0.6\nrelease=0\n";
  }
  if (!automation::ParseCommandFile(commands / "xbox.txt", &command, &error) ||
      command.type != automation::CommandType::XboxFrames || command.frames != 5 ||
      command.xbox[0] != 1 || command.xbox[19] != 0.6 || command.release_pad) return 30;
  for (const char* invalid : {"a=nan", "a=2", "a=-1", "a=0.2garbage", "unknown=1", "release=2"}) {
    {
      std::ofstream output(commands / "xbox.txt");
      output << "command=xbox_frames\nport=0\nframes=5\n" << invalid << '\n';
    }
    if (automation::ParseCommandFile(commands / "xbox.txt", &command, &error)) return 31;
  }
  {
    std::ofstream output(commands / "xbox.txt");
    output << "command=xbox_time\nport=0\nmilliseconds=1250\npath=hold.txt\nleft_right=0.5\n";
  }
  if (!automation::ParseCommandFile(commands / "xbox.txt", &command, &error) ||
      command.type != automation::CommandType::XboxTime || command.milliseconds != 1250 ||
      command.frames != 0 || command.path != "hold.txt" || !command.release_pad) return 32;
  for (const char* invalid : {"milliseconds=0\npath=hold.txt", "milliseconds=600001\npath=hold.txt",
                             "milliseconds=-1\npath=hold.txt", "milliseconds=5",
                             "milliseconds=5\npath=hold.txt\nframes=5"}) {
    {
      std::ofstream output(commands / "xbox.txt");
      output << "command=xbox_time\nport=0\n" << invalid << '\n';
    }
    if (automation::ParseCommandFile(commands / "xbox.txt", &command, &error)) return 33;
  }
  std::error_code ec;
  std::filesystem::remove_all(root, ec);
  return 0;
}
