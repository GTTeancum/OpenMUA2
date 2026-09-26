# SPDX-License-Identifier: GPL-3.0-or-later
from __future__ import annotations

import importlib.util
from pathlib import Path
import tempfile
import unittest

MODULE = Path(__file__).resolve().parents[1] / "tools/prepare_restored_rmse52.py"
spec = importlib.util.spec_from_file_location("prepare_restored_rmse52", MODULE)
p = importlib.util.module_from_spec(spec)
spec.loader.exec_module(p)


class PrepareRestoredRMSE52Tests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="rmse52-layout-")
        self.game = Path(self.temp.name) / "game"
        meta = self.game / "disc-meta"
        meta.mkdir(parents=True)
        for name, data in {
            "ticket.bin": b"ticket",
            "tmd.bin": b"tmd",
            "cert.bin": b"cert",
            "h3.bin": b"h3",
        }.items():
            (meta / name).write_bytes(data)
        raw = bytearray(p.REGION_END)
        raw[:p.DISC_HEADER_END] = bytes(range(256))
        raw[p.REGION_START:p.REGION_END] = bytes(range(32))
        (meta / "disc-header.bin").write_bytes(raw)

    def tearDown(self):
        self.temp.cleanup()

    def test_repair_creates_dolphin_compatibility_layout(self):
        result = p.repair_layout(self.game)
        self.assertTrue(all(state == "created" for state in result.values()))
        self.assertEqual((self.game / "ticket.bin").read_bytes(), b"ticket")
        self.assertEqual((self.game / "disc/header.bin").read_bytes(), bytes(range(256)))
        self.assertEqual((self.game / "disc/region.bin").read_bytes(), bytes(range(32)))

    def test_repair_is_idempotent(self):
        p.repair_layout(self.game)
        result = p.repair_layout(self.game)
        self.assertTrue(all(state == "verified" for state in result.values()))

    def test_repair_refuses_to_overwrite_different_target(self):
        (self.game / "ticket.bin").write_bytes(b"different")
        with self.assertRaisesRegex(ValueError, "refusing to overwrite"):
            p.repair_layout(self.game)
        self.assertEqual((self.game / "ticket.bin").read_bytes(), b"different")

    def test_repair_rejects_short_raw_header(self):
        (self.game / "disc-meta/disc-header.bin").write_bytes(b"short")
        with self.assertRaisesRegex(ValueError, "too short"):
            p.repair_layout(self.game)

    def test_repair_rejects_missing_source_metadata(self):
        (self.game / "disc-meta/tmd.bin").unlink()
        with self.assertRaisesRegex(ValueError, "metadata file is missing"):
            p.repair_layout(self.game)


if __name__ == "__main__":
    unittest.main()
