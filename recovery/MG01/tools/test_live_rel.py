#!/usr/bin/env python3
"""Unit tests plus one live-capture replay. Supply MUA2_GAME_ROOT and MUA2_RAM_ROOT."""
import os, struct, tempfile, unittest
from pathlib import Path
from audit_live_rel import Memory, patch, audit

class RelocationTests(unittest.TestCase):
    def test_lo_writes_exact_halfword(self):
        b=bytearray.fromhex('386300004e800020'); patch(b,2,4,0x81234567,0x80000002)
        self.assertEqual(b.hex(),'386345674e800020')
    def test_ha_carry(self):
        b=bytearray.fromhex('3c600000'); patch(b,2,6,0x8123ffff,0x80000002)
        self.assertEqual(b.hex(),'3c608124')
    def test_halfword_at_section_end(self):
        b=bytearray(2); patch(b,0,4,0x4567,0x80000000); self.assertEqual(b,b'Eg')
    def test_branch_preserves_link(self):
        b=bytearray.fromhex('48000001'); patch(b,0,10,0x80000100,0x80000000)
        self.assertEqual(b.hex(),'48000101')
    def test_negative_branch(self):
        b=bytearray.fromhex('48000000'); patch(b,0,10,0x80000000,0x80000100)
        self.assertEqual(b.hex(),'4bffff00')
    def test_branch_range_rejected(self):
        with self.assertRaises(ValueError): patch(bytearray(4),0,10,0x90000000,0x80000000)
    def test_unknown_relocation_rejected(self):
        with self.assertRaises(ValueError): patch(bytearray(4),0,200,0,0)
    def test_memory_cross_chunk(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp); (p/'memory-80000000.bin').write_bytes(b'ab'); (p/'memory-80000002.bin').write_bytes(b'cd')
            m=Memory(p); self.assertEqual(m.read(0x80000001,3),b'bcd')
            with self.assertRaises(ValueError): m.read(0x80000003,2)
    def test_memory_overlap_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp); (p/'memory-80000000.bin').write_bytes(b'ab'); (p/'memory-80000001.bin').write_bytes(b'cd')
            with self.assertRaises(ValueError): Memory(p)
    def test_real_capture_exact_match_and_original_bug(self):
        game=Path(os.environ['MUA2_GAME_ROOT']); ram=Memory(Path(os.environ['MUA2_RAM_ROOT']))
        rel=(game/'files/Marvel-rev-fin-plf2.rel').read_bytes()
        result=audit(rel,ram,0x80e4a080)
        self.assertEqual(result['status'],'LIVE_TEXT_MATCH'); self.assertEqual(result['relocations'],95772)
        self.assertEqual(result['comparisons'][1]['mismatch_bytes'],121737)
        self.assertEqual(result['layout'][6]['address'],0x811bcac0)
        with self.assertRaises(ValueError): audit(rel,ram,0x80e4a084)
if __name__=='__main__': unittest.main(verbosity=2)
