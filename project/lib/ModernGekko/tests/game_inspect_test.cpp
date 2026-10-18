#include "moderngekko/game.hpp"
#include "moderngekko/sha256.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

int main()
{
  const fs::path root = fs::temp_directory_path() / "moderngekko-game-inspect-test";
  fs::remove_all(root);
  fs::create_directories(root / "sys");
  fs::create_directories(root / "files");

  std::array<unsigned char, 0x60> boot{};
  const char id[] = "TEST01";
  std::copy_n(id, 6, boot.begin());
  boot[0x18] = 0x5d; boot[0x19] = 0x1c; boot[0x1a] = 0x9e; boot[0x1b] = 0xa3;
  const char name[] = "Synthetic Test Game";
  std::copy_n(name, sizeof(name), boot.begin() + 0x20);
  std::ofstream(root / "sys" / "boot.bin", std::ios::binary)
      .write(reinterpret_cast<const char*>(boot.data()), boot.size());

  std::array<unsigned char, 0x104> dol{};
  dol[0x02] = 0x01;  // Text section 0 file offset: 0x100.
  dol[0x48] = 0x80; dol[0x49] = 0x00; dol[0x4a] = 0x31; dol[0x4b] = 0x00;
  dol[0x93] = 0x04;
  dol[0xe0] = 0x80; dol[0xe1] = 0x00; dol[0xe2] = 0x31; dol[0xe3] = 0x00;
  std::ofstream(root / "sys" / "main.dol", std::ios::binary)
      .write(reinterpret_cast<const char*>(dol.data()), dol.size());

  const auto result = moderngekko::InspectGame(root);
  if (!result || result.metadata->disc_id != "TEST01" ||
      result.metadata->game_name != "Synthetic Test Game" ||
      result.metadata->platform != moderngekko::GamePlatform::Wii ||
      result.metadata->entry_point != 0x80003100u ||
      result.metadata->dol_sha256 !=
          "ee292f5fc3d0e5cfa32d951bd682a3cd2806c102e4a0a50300a2c480e21bcef6")
    return 1;

  const auto sample = root / "files" / "hash-sample.bin";
  const auto write = [&](const std::vector<std::uint8_t>& bytes) {
    std::ofstream(sample, std::ios::binary | std::ios::trunc)
        .write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
  };
  // Published SHA-256 vectors, including a file spanning many stream reads.
  write({});
  if (moderngekko::HashFileSha256(sample) !=
      "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")
    return 2;
  write({'a', 'b', 'c'});
  if (moderngekko::HashFileSha256(sample) !=
      "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")
    return 3;
  write(std::vector<std::uint8_t>(1000000, 'a'));
  if (moderngekko::HashFileSha256(sample) !=
      "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0")
    return 4;
  // Compare the independent portable implementation at SHA block/padding and
  // streamed-read boundaries, including a partial final read.
  for (const auto size : {55, 56, 63, 64, 65, 65535, 65536, 65537, 131113})
  {
    std::vector<std::uint8_t> bytes(size);
    for (std::size_t i = 0; i < bytes.size(); ++i)
      bytes[i] = static_cast<std::uint8_t>(i * 37 + i / 7);
    write(bytes);
    if (moderngekko::HashFileSha256(sample) != moderngekko::Sha256(bytes))
      return 5;
  }
  // No metadata-only cache: even same-size, same-timestamp edits must change
  // the directory fingerprint, and deleting/renaming an asset must too.
  const auto before = moderngekko::HashDirectorySha256(root / "files");
  const auto timestamp = fs::last_write_time(sample);
  std::fstream(sample, std::ios::binary | std::ios::in | std::ios::out).put('!');
  fs::last_write_time(sample, timestamp);
  const auto edited = moderngekko::HashDirectorySha256(root / "files");
  if (!before || !edited || before == edited)
    return 6;
  fs::create_directories(root / "files" / "nested");
  fs::rename(sample, root / "files" / "nested" / "renamed.bin");
  const auto renamed = moderngekko::HashDirectorySha256(root / "files");
  if (!renamed || renamed == edited || moderngekko::HashFileSha256(sample) ||
      moderngekko::HashFileSha256(root / "files"))
    return 7;
  fs::remove(root / "files" / "nested" / "renamed.bin");
  if (moderngekko::HashDirectorySha256(root / "files") != result.metadata->assets_sha256)
    return 8;
  // Fixed directory-format vector: create in reverse name order, including
  // a nested path. Resolving shared parents must not change manifest bytes.
  std::ofstream(root / "files" / "z.bin", std::ios::binary) << "ab";
  std::ofstream(root / "files" / "nested" / "a.bin", std::ios::binary) << "c";
  if (moderngekko::HashDirectorySha256(root / "files") !=
      "bdfd5736fea10b81e4941dceb2ca4cefdc22124b34ce2495be5498823fc62071")
    return 9;
  fs::remove_all(root);
  return 0;
}
