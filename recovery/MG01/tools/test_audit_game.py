#!/usr/bin/env python3
import unittest,struct,json,copy,os
from pathlib import Path
from audit_game import be32,checked,dol_report,rel_report,audit
ROOT=Path(os.environ.get('MUA2_GAME_ROOT', str(Path(__file__).resolve().parents[1]/'game')))
class AuditTests(unittest.TestCase):
    def test_bounds(self):
        for off,size in [(-1,1),(0,-1),(4,1),(2,3)]:
            with self.assertRaises(ValueError): checked(b'abcd',off,size)
    def test_truncated_u32(self):
        with self.assertRaises(ValueError): be32(b'abc',0)
    def test_real_hashes_and_counts(self):
        a=audit(ROOT); self.assertEqual(a['disc_id'],'RMSE52'); self.assertEqual(a['asset_files'],385)
        self.assertEqual(a['dol']['text_bytes'],5316160); self.assertEqual(a['rel']['text_bytes'],3255744)
        self.assertEqual(sum(i['relocations'] for i in a['rel']['imports']),95772)
    def test_no_invented_rel_base(self):
        self.assertIsNone(rel_report((ROOT/'files/Marvel-rev-fin-plf2.rel').read_bytes())['runtime_load_address'])
    def test_dol_bad_entry(self):
        d=bytearray((ROOT/'sys/main.dol').read_bytes()); struct.pack_into('>I',d,0xe0,0)
        with self.assertRaises(ValueError): dol_report(d)
    def test_dol_truncated(self):
        with self.assertRaises(ValueError): dol_report((ROOT/'sys/main.dol').read_bytes()[:-1])
    def test_rel_bad_section_table(self):
        d=bytearray((ROOT/'files/Marvel-rev-fin-plf2.rel').read_bytes()); struct.pack_into('>I',d,0x10,0xfffffff0)
        with self.assertRaises(ValueError): rel_report(d)
    def test_rel_bad_import_table(self):
        d=bytearray((ROOT/'files/Marvel-rev-fin-plf2.rel').read_bytes()); struct.pack_into('>I',d,0x2c,15)
        with self.assertRaises(ValueError): rel_report(d)
    def test_rel_unknown_relocation(self):
        d=bytearray((ROOT/'files/Marvel-rev-fin-plf2.rel').read_bytes()); imp=be32(d,0x28); stream=be32(d,imp+4); d[stream+8+2]=200
        with self.assertRaises(ValueError): rel_report(d)
    def test_rel_missing_end(self):
        d=(ROOT/'files/Marvel-rev-fin-plf2.rel').read_bytes()
        with self.assertRaises(ValueError): rel_report(d[:-8])
if __name__=='__main__': unittest.main(verbosity=2)
