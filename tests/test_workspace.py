# SPDX-License-Identifier: GPL-3.0-or-later
from __future__ import annotations
import argparse
from contextlib import redirect_stdout
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile

MODULE = Path(__file__).resolve().parents[1] / 'tools/workspace.py'
spec = importlib.util.spec_from_file_location('workspace', MODULE)
w = importlib.util.module_from_spec(spec); spec.loader.exec_module(w)


def write_manifest(root, names):
    rows = [{'path': n, 'size': (root/n).stat().st_size, 'sha256': w.sha256(root/n)} for n in names]
    (root/'FILE-MANIFEST.json').write_text(json.dumps({'schema': 1, 'files': rows}))


class WorkspaceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='openmua2 fixture ')
        self.root = Path(self.temp.name) / 'repo with spaces'
        self.root.mkdir()
        (self.root/'project').mkdir()
        (self.root/'project/example.c').write_text('int sample = 1;\n')
        (self.root/'README.md').write_text('local source\n')
        (self.root/'.gitignore').write_text('/.local/\n/.backups/\n*.wbfs\n*.WBFS\n.env*\n/.backup-*\n/.recovery-history.bundle\n')
        (self.root/'.gitattributes').write_text('* -text\n')
        write_manifest(self.root, ['project/example.c', 'README.md', '.gitignore', '.gitattributes'])
    def tearDown(self):
        self.temp.cleanup()
    def init(self):
        with redirect_stdout(io.StringIO()): w.setup(self.root)
    def options(self, **kw):
        return argparse.Namespace(cc=sys.executable, cxx=sys.executable, module_cc=sys.executable,
            config='Release', jobs=2, module_opt=0, wit=sys.executable, image=kw.get('image'), sdk=None)
    def image(self, name='Marvel Ultimate Alliance 2.wbfs'):
        p=self.root/name;p.write_bytes(b'WBFS'+bytes(512));return p
    def test_manifest_passes(self):
        self.assertEqual(w.verify_manifest(self.root),4)
    def test_manifest_detects_corruption_without_repair(self):
        p=self.root/'project/example.c';p.write_text('my code edit')
        with self.assertRaises(ValueError):w.verify_manifest(self.root)
        self.assertEqual(p.read_text(),'my code edit')
    def test_manifest_missing_file(self):
        (self.root/'README.md').unlink()
        with self.assertRaises(ValueError):w.verify_manifest(self.root)
    def test_manifest_traversal_rejected(self):
        doc={'schema':1,'files':[{'path':'../escape','size':0,'sha256':'0'*64}]}
        (self.root/'FILE-MANIFEST.json').write_text(json.dumps(doc))
        with self.assertRaises(ValueError):w.manifest_rows(self.root)
    def test_manifest_case_duplicate_rejected(self):
        doc=json.loads((self.root/'FILE-MANIFEST.json').read_text())
        doc['files'].append({**doc['files'][0],'path':'PROJECT/example.c'})
        (self.root/'FILE-MANIFEST.json').write_text(json.dumps(doc))
        with self.assertRaises(ValueError):w.manifest_rows(self.root)
    def test_manifest_invalid_hash_rejected(self):
        doc=json.loads((self.root/'FILE-MANIFEST.json').read_text());doc['files'][0]['sha256']='no'
        (self.root/'FILE-MANIFEST.json').write_text(json.dumps(doc))
        with self.assertRaises(ValueError):w.manifest_rows(self.root)
    def test_path_validation(self):
        for name in ('../x','/x','D:/x','x\\y','x/../y','./x','x//y','x\0y',''):
            with self.subTest(name=name),self.assertRaises(ValueError):w.safe_rel(name)
        self.assertEqual(w.safe_rel('project/a file.c'),Path('project/a file.c'))
    @unittest.skipIf(os.name=='nt','POSIX symlink fixture')
    def test_reject_symlink_destination(self):
        (self.root/'.local').symlink_to(self.root/'project',target_is_directory=True)
        with self.assertRaises(ValueError):w.within(self.root,'.local/file')
    def test_image_unique_and_case_insensitive(self):
        p=self.image('game.WBFS');self.assertEqual(w.discover_image(self.root),p.resolve())
    def test_image_ambiguity(self):
        self.image('one.wbfs');self.image('two.wbfs')
        with self.assertRaises(ValueError):w.discover_image(self.root)
        self.assertEqual(w.discover_image(self.root,'one.wbfs').name,'one.wbfs')
    def test_image_missing(self):
        with self.assertRaises(ValueError):w.discover_image(self.root)
    def test_image_outside_root(self):
        p=Path(self.temp.name)/'outside.wbfs';p.write_bytes(b'WBFS')
        with self.assertRaises(ValueError):w.discover_image(self.root,str(p))
    def test_image_bad_magic(self):
        p=self.image();p.write_bytes(b'nope')
        with self.assertRaises(ValueError):w.image_record(p)
    def test_image_readonly_hashing(self):
        p=self.image();before=p.read_bytes();record=w.image_record(p)
        self.assertEqual(p.read_bytes(),before);self.assertEqual(record[0]['sha256'],hashlib.sha256(before).hexdigest())
    def test_image_split_gaps_rejected(self):
        p=self.image();p.with_suffix('.wbf2').write_bytes(b'split')
        with self.assertRaises(ValueError):w.image_record(p)
    def test_image_split_parts_hashed(self):
        p=self.image();p.with_suffix('.wbf1').write_bytes(b'split')
        self.assertEqual(len(w.image_record(p)),2)
    def test_code_allowlist_excludes_private(self):
        for n in ['game.wbfs','project/test.wbfs','project/test.WbFs','project/test.wbf13',
                  'project/test.dol','project/key.pem','project/.env','project/generated/game.c',
                  '.local/user/save.dat','random-private.txt']:
            with self.subTest(name=n):self.assertFalse(w.is_code_path(n,set()))
        self.assertTrue(w.is_code_path('project/src/changed.c',set()))
        self.assertTrue(w.is_code_path('docs/notes.md',set()))
    def test_setup_git_preserves_wbfs_and_no_remote(self):
        p=self.image();before=w.sha256(p);self.init()
        self.assertEqual(w.sha256(p),before)
        tracked=w.git_text(self.root,'ls-files')
        self.assertNotIn('.wbfs',tracked);self.assertIn('project/example.c',tracked)
        self.assertEqual(w.git_text(self.root,'remote'),'')
    def test_setup_existing_repo_is_idempotent(self):
        self.init();before=w.git_text(self.root,'rev-parse','HEAD');self.init()
        self.assertEqual(w.git_text(self.root,'rev-parse','HEAD'),before)
    def interrupted_init(self):
        w.git(self.root,'init','-b','main')
        w.write_json(self.root/'.git/openmua2-initializing.json',
                     {'schema':1,'manifest_sha256':w.sha256(self.root/'FILE-MANIFEST.json')})
    def test_setup_resumes_own_interrupted_initial_commit(self):
        self.interrupted_init();self.init()
        self.assertTrue(w.git_text(self.root,'rev-parse','HEAD'))
        self.assertFalse((self.root/'.git/openmua2-initializing.json').exists())
    def test_setup_resume_rejects_unrelated_staged_files(self):
        self.interrupted_init();p=self.image();w.git(self.root,'add','-f',p.name)
        with self.assertRaises(ValueError):w.setup(self.root)
        self.assertNotEqual(w.git(self.root,'rev-parse','--verify','HEAD',check=False).returncode,0)
    def test_setup_resume_rejects_changed_manifest(self):
        self.interrupted_init();p=self.root/'FILE-MANIFEST.json';p.write_text(p.read_text()+'\n')
        with self.assertRaises(ValueError):w.setup(self.root)
    def test_setup_uses_literal_index_input_not_quadratic_pathspec(self):
        calls=[];real=w.git
        def wrapped(*args,**kwargs):
            calls.append(args[1:]);return real(*args,**kwargs)
        with patch.object(w,'git',side_effect=wrapped):self.init()
        self.assertTrue(any(c and c[0]=='update-index' and '--stdin' in c for c in calls))
        self.assertFalse(any('--pathspec-from-file=-' in c for c in calls))
    def test_setup_stops_before_git_on_corruption(self):
        (self.root/'README.md').write_text('edited')
        with self.assertRaises(ValueError):w.setup(self.root)
        self.assertFalse((self.root/'.git').exists())
    def test_snapshot_tracks_edits_and_excludes_game(self):
        self.init();self.image();(self.root/'project/example.c').write_text('int changed=2;\n')
        (self.root/'project/new.c').write_text('int new_source=3;\n')
        (self.root/'.env').write_text('not tracked')
        with redirect_stdout(io.StringIO()):w.snapshot(self.root,'fixture snapshot')
        self.assertEqual(w.git_text(self.root,'log','-1','--format=%s'),'fixture snapshot')
        tracked=w.git_text(self.root,'ls-files');self.assertIn('project/new.c',tracked)
        self.assertNotIn('.env',tracked);self.assertNotIn('.wbfs',tracked)
    def test_snapshot_records_deletion(self):
        self.init();(self.root/'project/example.c').unlink()
        with redirect_stdout(io.StringIO()):w.snapshot(self.root)
        self.assertNotIn('project/example.c',w.git_text(self.root,'ls-files'))
    def test_snapshot_rejects_forced_private_staging(self):
        self.init();p=self.image();w.git(self.root,'add','-f',p.name)
        with self.assertRaises(ValueError):w.snapshot(self.root)
    def test_snapshot_noop_preserves_head(self):
        self.init();before=w.git_text(self.root,'rev-parse','HEAD')
        with redirect_stdout(io.StringIO()):w.snapshot(self.root)
        self.assertEqual(w.git_text(self.root,'rev-parse','HEAD'),before)
    def test_backup_restores_history_and_uncommitted_work(self):
        self.init();head=w.git_text(self.root,'rev-parse','HEAD')
        self.image();(self.root/'.local/user').mkdir(parents=True)
        (self.root/'.local/user/save.bin').write_bytes(b'private save')
        (self.root/'README.md').unlink()
        (self.root/'project/example.c').write_text('uncommitted edit\n')
        (self.root/'project/new.c').write_text('uncommitted new file\n')
        out=Path(self.temp.name)/'source backup.zip'
        with redirect_stdout(io.StringIO()):w.backup(self.root,str(out))
        restored=Path(self.temp.name)/'restored source';restored.mkdir()
        with zipfile.ZipFile(out) as z:
            self.assertFalse(any(n.endswith('.wbfs') or n.startswith('.local/') for n in z.namelist()))
            z.extractall(restored)
        with redirect_stdout(io.StringIO()):w.setup(restored)
        self.assertEqual(w.git_text(restored,'rev-parse','HEAD'),head)
        self.assertFalse((restored/'README.md').exists())
        self.assertEqual((restored/'project/example.c').read_text(),'uncommitted edit\n')
        self.assertTrue((restored/'project/new.c').exists())
        status=w.git_text(restored,'status','--porcelain')
        self.assertIn('D README.md',status);self.assertIn('M project/example.c',status)
        self.assertIn('?? project/new.c',status)
    def test_backup_destination_not_overwritten(self):
        self.init();out=Path(self.temp.name)/'exists.zip';out.write_bytes(b'keep')
        with self.assertRaises(ValueError):w.backup(self.root,str(out))
        self.assertEqual(out.read_bytes(),b'keep')
    def test_backup_refuses_index_lock(self):
        self.init();(self.root/'.git/index.lock').write_bytes(b'locked')
        with self.assertRaises(ValueError):w.backup(self.root)
    def test_build_configure_argument_quoting_and_offline(self):
        calls=[]
        with patch.object(w,'logged',side_effect=lambda *args:calls.append(args)):
            w.cmake_configure(self.root,self.root/'project/source with spaces',self.root/'.local/build',self.options())
        args=calls[0][2]
        self.assertIn('-DFETCHCONTENT_FULLY_DISCONNECTED=ON',args)
        self.assertIn(self.root/'project/source with spaces',args)
        self.assertNotIn('shell=True',args)
    def test_module_configure_uses_explicit_compiler(self):
        calls=[]
        with patch.object(w,'logged',side_effect=lambda *args:calls.append(args)):
            w.cmake_configure(self.root,self.root/'project',self.root/'.local/module',self.options(),module=True)
        args=calls[0][2]
        self.assertTrue(any(str(s).startswith('-DCMAKE_C_COMPILER=') for s in args))
        self.assertFalse(any(str(s).startswith('-DCMAKE_CXX_COMPILER=') for s in args))
    def test_failed_child_is_not_success(self):
        with self.assertRaises(RuntimeError):
            w.logged(self.root,'fail',[sys.executable,'-c','import sys; print("failure fixture");sys.exit(7)'])
        record=list((self.root/'.local/logs').glob('*.json'))[0]
        self.assertEqual(json.loads(record.read_text())['returncode'],7)
    def test_extraction_is_atomic_and_readonly(self):
        p=self.image();before=w.sha256(p)
        def fake_log(root,label,args,*_):
            stage=Path(args[-1]);stage.mkdir(parents=True);(stage/'extracted.bin').write_bytes(b'extracted')
        with patch.object(w,'logged',side_effect=fake_log),patch.object(w,'game_audit',return_value={'disc_id':'RMSE52'}):
            out=w.extract_game(self.root,self.options(),Path(sys.executable))
        self.assertEqual(w.sha256(p),before);self.assertTrue((out/'extracted.bin').exists())
        self.assertTrue((self.root/'.local/receipts/extraction.json').is_file())
    def test_failed_extraction_does_not_publish_game(self):
        self.image()
        def fake_log(root,label,args,*_):
            stage=Path(args[-1]);stage.mkdir(parents=True);(stage/'bad.bin').write_bytes(b'bad')
        with patch.object(w,'logged',side_effect=fake_log),patch.object(w,'game_audit',side_effect=ValueError('bad hash')):
            with self.assertRaises(ValueError):w.extract_game(self.root,self.options(),Path(sys.executable))
        self.assertFalse((self.root/'.local/game').exists())
    def test_existing_unreceipted_game_not_overwritten(self):
        self.image();game=self.root/'.local/game';game.mkdir(parents=True);(game/'keep').write_text('keep')
        with self.assertRaises(ValueError):w.extract_game(self.root,self.options(),Path(sys.executable))
        self.assertEqual((game/'keep').read_text(),'keep')
    def test_run_requires_verified_build_receipt(self):
        opts=argparse.Namespace(graphics=None,audio=None)
        with self.assertRaises(ValueError):w.run_game(self.root,opts)
    def test_parser_defaults_conservative(self):
        o=w.make_parser().parse_args(['build'])
        self.assertEqual(o.jobs,2);self.assertEqual(o.module_opt,0)
    def test_write_json_replaces_only_destination(self):
        p=self.root/'new.json';w.write_json(p,{'a':1});w.write_json(p,{'a':2})
        self.assertEqual(json.loads(p.read_text()),{'a':2})
        self.assertFalse(list(self.root.glob('new.json.*.tmp')))

if __name__=='__main__':unittest.main()
