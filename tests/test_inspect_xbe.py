# SPDX-License-Identifier: GPL-3.0-or-later
import importlib.util
import struct
import sys
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location(
    "inspect_xbe", Path(__file__).parents[1] / "tools/inspect_xbe.py")
mod = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = mod  # dataclasses resolve annotations via sys.modules
spec.loader.exec_module(mod)

BASE = 0x10000


def synthetic_xbe(*, kind="retail", text_size=0x3000, bss_extra=0x2000,
                  pad_to=0, libraries=(("XAPILIB", 1, 0, 5849),)):
    """Build a minimal, structurally valid XBE (headers + two sections)."""
    hdr = bytearray(0x1000)
    hdr[:4] = b"XBEH"
    cert, sects, names, libs = 0x200, 0x400, 0x500, 0x600
    text_raw, data_raw = 0x1000, 0x1000 + text_size
    data_size = 0x800
    image = 0x1000 + text_size + data_size + bss_extra
    text_va, data_va = BASE + 0x1000, BASE + 0x1000 + text_size
    struct.pack_into("<I", hdr, 0x104, BASE)
    struct.pack_into("<I", hdr, 0x108, 0x1000)       # size of headers
    struct.pack_into("<I", hdr, 0x10C, image)        # size of image
    struct.pack_into("<I", hdr, 0x114, 0x5F000000)   # timestamp
    struct.pack_into("<I", hdr, 0x118, BASE + cert)
    struct.pack_into("<I", hdr, 0x11C, 2)
    struct.pack_into("<I", hdr, 0x120, BASE + sects)
    struct.pack_into("<I", hdr, 0x128, (text_va + 0x10) ^ mod.ENTRY_KEYS[kind])
    struct.pack_into("<I", hdr, 0x158, (data_va + 0x20) ^ mod.THUNK_KEYS[kind])
    struct.pack_into("<III", hdr, 0x130, 0x10000, 0x100000, 0x1000)
    struct.pack_into("<I", hdr, 0x160, len(libraries))
    struct.pack_into("<I", hdr, 0x164, BASE + libs)
    struct.pack_into("<I", hdr, cert + 0x8, 0x4D550001)
    hdr[cert + 0xC:cert + 0xC + 80] = "MUA test".encode("utf-16-le").ljust(80, b"\0")
    for i, (name, va, vsize, raw, rsize, flags) in enumerate([
            (".text", text_va, text_size, text_raw, text_size, 0x04 | 0x02),
            (".data", data_va, data_size + bss_extra, data_raw, data_size, 0x01 | 0x02)]):
        h = sects + i * 0x38
        struct.pack_into("<IIIIII", hdr, h, flags, va, vsize, raw, rsize, BASE + names + i * 16)
        hdr[names + i * 16:names + i * 16 + len(name)] = name.encode()
    for i, (name, ma, mi, bu) in enumerate(libraries):
        o = libs + i * 16
        hdr[o:o + 8] = name.encode().ljust(8, b"\0")
        struct.pack_into("<4H", hdr, o + 8, ma, mi, bu, 0)
    body = bytearray(text_size + data_size)
    data = bytes(hdr) + bytes(body)
    return data + b"\0" * max(0, pad_to - len(data))


class InspectXbeTest(unittest.TestCase):
    def test_retail_layout(self):
        info = mod.parse(synthetic_xbe())
        self.assertEqual(info.build_kind, "retail")
        self.assertEqual(info.entry_point, BASE + 0x1010)
        self.assertEqual(info.title_name, "MUA test")
        self.assertEqual(info.title_id, 0x4D550001)
        self.assertEqual([s.name for s in info.sections], [".text", ".data"])
        self.assertEqual(info.sections[0].flags, ["preload", "executable"])
        self.assertEqual(info.totals["game_text"], 0x3000)
        self.assertEqual(info.totals["rwdata"], 0x2800)
        self.assertEqual(info.totals["xdk_libs"], 0)
        self.assertEqual(info.totals["zero_fill"], 0x2000)
        self.assertEqual(info.libraries[0].name, "XAPILIB")
        self.assertEqual(info.libraries[0].version, "1.0.5849")

    def test_roles_ignore_executable_flag(self):
        # XDK builds mark .data executable; it must still count as data.
        data = bytearray(synthetic_xbe())
        flags_off = 0x400 + 0x38  # second section header
        struct.pack_into("<I", data, flags_off, 0x01 | 0x02 | 0x04)
        info = mod.parse(bytes(data))
        self.assertEqual(info.totals["game_text"], 0x3000)
        self.assertEqual(info.totals["rwdata"], 0x2800)

    def test_debug_key_detected(self):
        self.assertEqual(mod.parse(synthetic_xbe(kind="debug")).build_kind, "debug")

    def test_rejects_garbage(self):
        with self.assertRaises(mod.XbeError):
            mod.parse(b"MZ" + b"\0" * 0x400)
        bad = bytearray(synthetic_xbe())
        struct.pack_into("<I", bad, 0x400 + 0x10, 0x7FFFFFFF)  # raw size past EOF
        with self.assertRaises(mod.XbeError):
            mod.parse(bytes(bad))

    def test_budget_exit_codes(self):
        with tempfile.TemporaryDirectory() as d:
            small = Path(d, "small.xbe")
            small.write_bytes(synthetic_xbe())
            self.assertEqual(mod.main([str(small)]), 0)
            self.assertEqual(mod.main([str(small), "--budget-bytes", "100"]), 1)
            broken = Path(d, "broken.xbe")
            broken.write_bytes(b"nope")
            self.assertEqual(mod.main([str(broken)]), 2)

    def test_default_budget_is_ten_megabytes(self):
        self.assertEqual(mod.DEFAULT_BUDGET_BYTES, 10_000_000)


if __name__ == "__main__":
    unittest.main()
