import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest import mock

spec = importlib.util.spec_from_file_location('combat_runner', Path(__file__).parents[1]/'tools/run_combat_benchmark.py')
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)

class CommandPublicationTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        for name in ('commands', 'processed', 'failed'):
            (self.root/name).mkdir()

    def test_partial_file_never_enters_watched_directory(self):
        original = Path.write_text
        def observed_write(path, text, **kwargs):
            self.assertEqual(path.parent, self.root/'staging')
            self.assertEqual(list((self.root/'commands').iterdir()), [])
            return original(path, text, **kwargs)
        with mock.patch.object(Path, 'write_text', observed_write):
            mod.publish_command(self.root, '000001.txt', {'command':'pad_frames','frames':12})
        self.assertEqual((self.root/'commands/000001.txt').read_text(), 'command=pad_frames\nframes=12\n')
        self.assertEqual(list((self.root/'staging').iterdir()), [])

    def test_windows_sharing_violation_retries_publication_only(self):
        original = Path.rename
        attempts = []
        def rename(path, target):
            attempts.append(path.read_text())
            if len(attempts) == 1:
                error = OSError('sharing violation')
                error.winerror = 32
                raise error
            return original(path, target)
        with mock.patch.object(Path, 'rename', rename), mock.patch.object(mod.time, 'sleep'):
            mod.publish_command(self.root, '000001.txt', {'command':'stop'})
        self.assertEqual(attempts, ['command=stop\n']*2)

    def test_nonsharing_failure_is_not_hidden(self):
        with mock.patch.object(Path, 'rename', side_effect=FileNotFoundError('missing')):
            with self.assertRaises(FileNotFoundError):
                mod.publish_command(self.root, '000001.txt', {'command':'stop'})

    def test_receipts_reject_extra_temporary_commands_and_failures(self):
        (self.root/'processed/000001.txt').write_text('command=stop\n')
        mod.validate_command_receipts(self.root, 1)
        extra = self.root/'processed/000001.txt.tmp'
        extra.write_text('command=stop\n')
        with self.assertRaises(RuntimeError):mod.validate_command_receipts(self.root, 1)
        extra.unlink()
        (self.root/'failed/000002.txt.tmp').write_text('')
        with self.assertRaises(RuntimeError):mod.validate_command_receipts(self.root, 1)

class DiagnosticArgumentsTest(unittest.TestCase):
    def test_runtime_must_confirm_backend_and_single_core(self):
        video = "Effective video: dual_core=0 sync_gpu=0\nHost CPU affinity: logical_processors=1 mask=0x1\n"
        mod.validate_runtime_settings("CPU backend: JIT\n" + video, "jit")
        mod.validate_runtime_settings("CPU backend: StaticRecomp\n" + video, "staticrecomp")
        for log in ("", "CPU backend: JIT\n", "CPU backend: StaticRecomp\n" + video,
                    "CPU backend: JIT\n" + video.replace("dual_core=0", "dual_core=1"),
                    "CPU backend: JIT\n" + video + video.replace("dual_core=0", "dual_core=1")):
            with self.subTest(log=log), self.assertRaises(RuntimeError):
                mod.validate_runtime_settings(log, "jit")
        for affinity in ("", "Host CPU affinity: logical_processors=1 mask=0x0\n",
                         "Host CPU affinity: logical_processors=1 mask=0x3\n",
                         "Host CPU affinity: logical_processors=2 mask=0x3\n"):
            with self.subTest(affinity=affinity), self.assertRaises(RuntimeError):
                mod.validate_runtime_settings("CPU backend: JIT\nEffective video: dual_core=0 sync_gpu=0\n" + affinity, "jit")
        mod.validate_runtime_settings("CPU backend: JIT\n" + video.replace("mask=0x1", "mask=0x8000"), "jit")

    def test_block_profile_is_isolated_and_ordinary_runs_disable_inherited_diagnostics(self):
        with tempfile.TemporaryDirectory() as directory:
            user = Path(directory)
            (user / 'Config').mkdir()
            path = user / 'Config/Dolphin.ini'
            path.write_text('[Core]\nCPUThread = True\n[Interface]\nDebugModeEnabled = True\n[Debug]\nJitEnableProfiling = True\n')
            mod.configure_benchmark_profile(user, False)
            config = mod.configparser.ConfigParser()
            config.read(path)
            self.assertFalse(config.getboolean('Interface', 'DebugModeEnabled'))
            self.assertFalse(config.getboolean('Debug', 'JitEnableProfiling'))
            self.assertFalse(config.getboolean('Core', 'CPUThread'))
            mod.configure_benchmark_profile(user, True)
            config.read(path)
            self.assertTrue(config.getboolean('Debug', 'JitEnableProfiling'))
            self.assertTrue(config.getboolean('Interface', 'DebugModeEnabled'))
            self.assertFalse(config.getboolean('Core', 'CPUThread'))

    def test_failure_detail_is_tied_to_failed_receipt_not_stale_status(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'status.txt').write_text('last_error=unrelated earlier failure\n')
            (root / 'errors').mkdir()
            (root / 'errors/000128.txt').write_text('000128.txt: pad_frames requires a running emulated core\n')
            self.assertEqual(mod.command_failure_detail(root, '000128.txt'),
                             '000128.txt: pad_frames requires a running emulated core')
            self.assertIn('000129.txt failed', mod.command_failure_detail(root, '000129.txt'))
            self.assertNotIn('unrelated', mod.command_failure_detail(root, '000129.txt'))

    def test_invalid_ranges_and_conflicting_modes_fail_before_launch(self):
        required = [item for name in ("runner", "module", "game", "user", "state", "route", "output")
                    for item in ("--" + name, "unused")]
        cases = [["--jit-ranges", value] for value in
                 ("garbage", "80400000-80300000", "80300000-80300000", "100000000-100000004")]
        cases.append(["--jit-ranges", "80300000-80400000", "--jit-diagnostic"])
        cases.append(["--cpu", "staticrecomp", "--jit-block-profile"])
        cases.append(["--cpu", "staticrecomp", "--simple-format", "on"])
        cases.append(["--jit-profile-callers", "803c63bc"])
        for targets in ("803c63bd", "803c63bc,", "100000000", "junk", ",".join(["803c63bc"]*33)):
            cases.append(["--jit-block-profile", "--jit-profile-callers", targets])
        for extra in cases:
            with self.subTest(extra=extra), mock.patch("sys.argv", ["runner"] + required + extra), \
                 mock.patch.object(mod.shutil, "copytree") as copy, mock.patch("sys.stderr"):
                with self.assertRaises(SystemExit) as raised:
                    mod.main()
                self.assertEqual(raised.exception.code, 2)
                copy.assert_not_called()

if __name__ == '__main__':unittest.main()
