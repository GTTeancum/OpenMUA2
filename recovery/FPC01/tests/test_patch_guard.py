"""Guard/idempotence tests; never touches the original project."""
from __future__ import annotations
import hashlib
import importlib.util
from pathlib import Path
import os
import tempfile
import unittest
ROOT=Path(__file__).resolve().parent.parent
spec=importlib.util.spec_from_file_location('patch_fctiw',ROOT/'tools/patch_fctiw.py')
patch=importlib.util.module_from_spec(spec)
spec.loader.exec_module(patch)
BODY=(ROOT/'tests/fixtures/fctiw-original.txt').read_bytes()
SOURCE=b'// Previous unrelated source\n'+BODY+b'\nvoid ppc_fcmp(dummy) {}\n// Following unrelated source\n'

class GuardTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.root=Path(self.temp.name)
        self.p=self.root/'cpu.c'
        self.p.write_bytes(SOURCE)
    def tearDown(self): self.temp.cleanup()
    def test_fixture_hash(self):
        self.assertEqual(hashlib.sha256(BODY).hexdigest(),patch.ORIGINAL_FUNCTION_SHA256)
    def test_dry_run(self):
        result=patch.patch_files([self.p])
        self.assertFalse(result[0]['applied'])
        self.assertEqual(self.p.read_bytes(),SOURCE)
    def test_three_changes_only(self):
        patch.patch_files([self.p],True)
        expected=SOURCE.replace(b'~0x00006000u',b'~0x00060000u').replace(b'|= 0x00004000u',b'|= 0x00020000u').replace(b'|= 0x00002000u',b'|= 0x00040000u')
        self.assertEqual(self.p.read_bytes(),expected)
    def test_idempotent(self):
        patch.patch_files([self.p],True)
        before=self.p.read_bytes()
        result=patch.patch_files([self.p],True)
        self.assertEqual(result[0]['state'],'already_fixed')
        self.assertFalse(result[0]['applied'])
        self.assertEqual(before,self.p.read_bytes())
    def test_unrelated_changes_preserved(self):
        extra=b'// MG02 unrelated changes\nconst int unrelated = 0x00006000u;\n'
        self.p.write_bytes(extra+SOURCE+extra)
        patch.patch_files([self.p],True)
        self.assertTrue(self.p.read_bytes().startswith(extra))
        self.assertTrue(self.p.read_bytes().endswith(extra))
        self.assertEqual(self.p.read_bytes().count(b'const int unrelated = 0x00006000u;'),2)
    def test_crlf(self):
        self.p.write_bytes(SOURCE.replace(b'\n',b'\r\n'))
        patch.patch_files([self.p],True)
        self.assertNotIn(b'\n',self.p.read_bytes().replace(b'\r\n',b''))
        self.assertEqual(patch.analyze(self.p)[1]['state'],'already_fixed')
    def test_conflict_refused(self):
        before=SOURCE.replace(b'result = 0x7FFFFFFFu',b'result = 42u')
        self.p.write_bytes(before)
        with self.assertRaises(ValueError): patch.patch_files([self.p],True)
        self.assertEqual(before,self.p.read_bytes())
    def test_duplicate_definition_refused(self):
        self.p.write_bytes(SOURCE+BODY)
        with self.assertRaises(ValueError): patch.patch_files([self.p],True)
    def test_missing_definition_refused(self):
        self.p.write_bytes(b'// no function\n')
        with self.assertRaises(ValueError): patch.patch_files([self.p],True)
    def test_missing_boundary_refused(self):
        self.p.write_bytes(BODY)
        with self.assertRaises(ValueError): patch.patch_files([self.p],True)
    def test_mixed_newlines_refused(self):
        self.p.write_bytes(SOURCE.replace(b'\n',b'\r\n',1))
        with self.assertRaises(ValueError): patch.patch_files([self.p],True)
    def test_symlink_refused(self):
        q=self.root/'link.c'; q.symlink_to(self.p)
        with self.assertRaises(ValueError): patch.patch_files([q],True)
        self.assertEqual(SOURCE,self.p.read_bytes())
    def test_duplicate_destination_refused(self):
        with self.assertRaises(ValueError): patch.patch_files([self.p,self.p],True)
        self.assertEqual(SOURCE,self.p.read_bytes())
    def test_all_preflighted(self):
        q=self.root/'second.c'; q.write_bytes(b'conflict')
        with self.assertRaises(ValueError): patch.patch_files([self.p,q],True)
        self.assertEqual(SOURCE,self.p.read_bytes())
    def test_two_copies(self):
        q=self.root/'second.c'; q.write_bytes(SOURCE)
        records=patch.patch_files([self.p,q],True)
        self.assertEqual([r['applied'] for r in records],[True,True])
        self.assertEqual(self.p.read_bytes(),q.read_bytes())
    def test_mode_preserved(self):
        self.p.chmod(0o640)
        patch.patch_files([self.p],True)
        self.assertEqual(self.p.stat().st_mode&0o777,0o640)

if __name__=='__main__': unittest.main(verbosity=2)
