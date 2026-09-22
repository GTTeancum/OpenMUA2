# SPDX-License-Identifier: GPL-3.0-or-later
"""LOCAL01a launcher tests. Native CMD cases run only on Windows.

The portable cases execute the production Python probe and check launcher
contracts. They do not emulate CMD or claim Windows launcher execution.
"""
from __future__ import annotations
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
LAUNCHER = ROOT / 'OpenMUA2.cmd'
TEXT = LAUNCHER.read_text(encoding='utf-8')
PROBE = re.search(r' -c "([^"\r\n]+)" <nul >nul 2>&1', TEXT).group(1)


class PythonProbeTests(unittest.TestCase):
    def probe(self, prefix='', env=None):
        return subprocess.run([sys.executable, '-E', '-s', '-c', prefix + PROBE],
                              env=env, capture_output=True, timeout=15).returncode

    def test_installed_interpreter_runs_the_production_probe(self):
        import struct
        self.assertEqual(self.probe(), 42 if sys.version_info >= (3, 11)
                         and struct.calcsize('P') == 8 else 1)

    def test_minimum_and_later_versions(self):
        for version in [(3, 11), (3, 12), (3, 13), (3, 14), (3, 15)]:
            with self.subTest(version=version):
                prefix = f'import sys, struct; sys.version_info={version!r}; struct.calcsize=lambda _:8; '
                self.assertEqual(self.probe(prefix), 42)

    def test_old_version_is_rejected(self):
        prefix = 'import sys, struct; sys.version_info=(3, 10); struct.calcsize=lambda _:8; '
        self.assertEqual(self.probe(prefix), 1)

    def test_32_bit_is_rejected(self):
        prefix = 'import sys, struct; sys.version_info=(3, 13); struct.calcsize=lambda _:4; '
        self.assertEqual(self.probe(prefix), 1)

    def test_broken_pythonhome_is_ignored_by_probe(self):
        env = dict(os.environ, PYTHONHOME='/openmua2-missing-python-home',
                   PYTHONPATH='/openmua2-missing-python-path')
        self.assertEqual(self.probe(env=env), 42)

    def test_probe_success_is_not_generic_zero(self):
        self.assertIn('if not "%ERRORLEVEL%"=="42" exit /b 1', TEXT)


class LauncherContractTests(unittest.TestCase):
    def test_workspace_action_occurs_once_after_selection(self):
        self.assertEqual(TEXT.count('tools\\workspace.py'), 1)
        before, run_and_probe = TEXT.split('\n:run\n', 1)
        run, probe = run_and_probe.split('\n:probe\n', 1)
        self.assertNotIn('tools\\workspace.py', before)
        self.assertIn('tools\\workspace.py', run)
        self.assertNotIn('tools\\workspace.py', probe)
        self.assertIn('exit /b %_OPENMUA2_RC%', run)

    def test_explicit_override_is_checked_and_bad_override_stops(self):
        self.assertIn('call :probe "%OPENMUA2_PYTHON%" ""', TEXT)
        self.assertIn('if not defined _OPENMUA2_PY goto :bad_override', TEXT)
        self.assertIn('exit /b 1', TEXT.split('\n:bad_override\n', 1)[1].split('\n:run\n')[0])

    def test_fallback_checks_specific_versions(self):
        self.assertIn('(-3 -3.14 -3.13 -3.12 -3.11)', TEXT)
        self.assertIn('call :probe "%%P" "%%V"', TEXT)

    def test_all_path_candidates_are_tested(self):
        self.assertIn("where.exe python.exe 2^>nul", TEXT)
        self.assertIn("where.exe python3.exe 2^>nul", TEXT)
        self.assertIn("where.exe py.exe 2^>nul", TEXT)
        self.assertIn('if not exist "%~1" exit /b 1', TEXT)

    def test_installer_modes_disabled_only_for_this_process(self):
        self.assertIn('setlocal EnableExtensions DisableDelayedExpansion', TEXT)
        self.assertIn('set "PYLAUNCHER_ALLOW_INSTALL="', TEXT)
        self.assertIn('set "PYTHON_MANAGER_AUTOMATIC_INSTALL=false"', TEXT)
        self.assertNotIn('setx ', TEXT.lower())
        self.assertNotIn('reg add ', TEXT.lower())

    def test_child_arguments_and_exit_code_are_preserved(self):
        self.assertIn('-E -s "%~dp0tools\\workspace.py" %*', TEXT)
        self.assertIn('set "_OPENMUA2_RC=%ERRORLEVEL%"', TEXT)
        self.assertIn('exit /b %_OPENMUA2_RC%', TEXT)

    def test_cmd_file_has_crlf(self):
        data = LAUNCHER.read_bytes()
        self.assertIn(b'\r\n', data)
        self.assertNotIn(b'\n', data.replace(b'\r\n', b''))


@unittest.skipUnless(os.name == 'nt', 'Requires native Windows CMD; not simulated')
class NativeWindowsLauncherTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='OpenMUA2 launcher test ')
        self.root = Path(self.temp.name) / 'repo with spaces'
        (self.root / 'tools').mkdir(parents=True)
        shutil.copy2(LAUNCHER, self.root / LAUNCHER.name)
        (self.root / 'tools/workspace.py').write_text(
            'import json, os, pathlib, sys\n'
            'p=pathlib.Path(__file__).resolve().parents[1]/"calls.jsonl"\n'
            'with p.open("a") as f: f.write(json.dumps(sys.argv[1:])+"\\n")\n'
            'sys.exit(int(os.environ.get("LAUNCHER_TEST_EXIT", "0")))\n', encoding='utf-8')
        self.env = dict(os.environ, OPENMUA2_PYTHON=sys.executable)

    def tearDown(self):
        self.temp.cleanup()

    def call(self, args):
        # Windows native test paths are quoted; only fixed test arguments follow.
        comspec = os.environ.get('COMSPEC', 'cmd.exe')
        command = f'"{comspec}" /d /v:off /s /c ""{self.root / "OpenMUA2.cmd"}" {args}"'
        return subprocess.run(command, executable=comspec, env=self.env,
                              capture_output=True, text=True, timeout=30)

    def calls(self):
        p = self.root / 'calls.jsonl'
        return [json.loads(s) for s in p.read_text().splitlines()] if p.exists() else []

    def test_explicit_interpreter_and_repository_spaces(self):
        result = self.call('setup')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.calls(), [['setup']])

    def test_nonzero_workspace_result_does_not_retry(self):
        self.env['LAUNCHER_TEST_EXIT'] = '17'
        result = self.call('setup')
        self.assertEqual(result.returncode, 17, result.stdout + result.stderr)
        self.assertEqual(self.calls(), [['setup']])

    def test_invalid_explicit_path_does_not_run_workspace(self):
        self.env['OPENMUA2_PYTHON'] = str(self.root / 'missing Python/python.exe')
        result = self.call('setup')
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertEqual(self.calls(), [])

    def test_quoted_snapshot_message_is_one_argument(self):
        result = self.call('snapshot -m "A source snapshot with spaces"')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.calls(), [['snapshot', '-m', 'A source snapshot with spaces']])

    def test_stale_python_environment_does_not_poison_selected_interpreter(self):
        self.env['PYTHONHOME'] = str(self.root / 'missing Daytona Python')
        self.env['PYTHONPATH'] = str(self.root / 'missing Daytona libraries')
        result = self.call('setup')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.calls(), [['setup']])


if __name__ == '__main__':
    unittest.main()
