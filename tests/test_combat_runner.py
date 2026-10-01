import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest import mock

spec = importlib.util.spec_from_file_location('combat_runner', Path(__file__).parents[1]/'tools/run_combat_benchmark.py')
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)

class CommandPublicationTest(unittest.TestCase):
    def test_guest_timed_input_requires_accurate_completed_receipt(self):
        command = {'command': 'xbox_time', 'milliseconds': 1000, 'path': 'hold.txt'}
        good = ('start_ticks=100\nend_ticks=729000110\nduration_ticks=729000000\n'
                'ticks_per_second=729000000\ncycles_late=10\ncompleted=1\nrelease=1\n')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for bad in ('', good.replace('completed=1', 'completed=0'),
                        good.replace('end_ticks=729000110', 'end_ticks=729000109'),
                        good.replace('duration_ticks=729000000', 'duration_ticks=728000000'),
                        good.replace('cycles_late=10', 'cycles_late=-1'),
                        good.replace('release=1', 'release=0'),
                        good.replace('end_ticks=729000110', 'end_ticks=730000100').replace(
                            'cycles_late=10', 'cycles_late=1000000')):
                (root/'hold.txt').write_text(bad)
                with self.subTest(bad=bad), self.assertRaises(RuntimeError):
                    mod.validate_timed_inputs(root, [command])
            (root/'hold.txt').write_text(good)
            mod.validate_timed_inputs(root, [command])
            with self.assertRaises(RuntimeError):
                mod.validate_timed_inputs(root, [command, command])

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

class QueuedRouteTest(unittest.TestCase):
    def test_whole_route_published_in_order_without_waiting_for_receipts(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'commands').mkdir()
            commands = [{'command': 'xbox_frames', 'port': 0, 'frames': 10, 'a': 1},
                        {'command': 'read_timing', 'path': 'end.txt'}]
            pending = mod.publish_route(root, commands, 8)
            self.assertEqual([name for name, item in pending], ['000009.txt', '000010.txt'])
            self.assertEqual([item for name, item in pending], commands)
            self.assertEqual((root / 'commands/000009.txt').read_text(),
                             'command=xbox_frames\nport=0\nframes=10\na=1\n')
            self.assertEqual((root / 'commands/000010.txt').read_text(),
                             'command=read_timing\npath=end.txt\n')
            self.assertFalse(list((root / 'staging').iterdir()))
            self.assertFalse((root / 'processed').exists())
            self.assertEqual(mod.publish_route(root, [], 10), [])


class DiagnosticArgumentsTest(unittest.TestCase):
    def test_audio_capture_is_opt_in_bounded_stereo_and_complete(self):
        import wave
        env = {'OPENMUA2_AUDIO_CAPTURE': 'inherited.wav', 'OTHER': 'keep'}
        mod.configure_audio_capture(env, False, Path('run'))
        self.assertEqual(env, {'OTHER': 'keep'})
        mod.configure_audio_capture(env, True, Path('run'))
        self.assertEqual(env['OPENMUA2_AUDIO_CAPTURE'], str(Path('run/mixed-audio.wav')))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            p = root / 'mixed-audio.wav'
            def write(channels, data):
                with wave.open(str(p), 'wb') as output:
                    output.setparams((channels, 2, 48000, 0, 'NONE', 'not compressed'))
                    output.writeframes(data)
            write(2, b'\0' * 16)
            mod.validate_audio_capture(root)
            p.write_bytes(p.read_bytes()[:-2])
            with self.assertRaises(RuntimeError): mod.validate_audio_capture(root)
            for channels, data in ((1, b'\0' * 16), (2, b'')):
                write(channels, data)
                with self.assertRaises(RuntimeError): mod.validate_audio_capture(root)

    def test_formatter_summary_must_prove_requested_mode_and_clean_comparisons(self):
        good = ('Simple formatter: mode=shadow eligible=12 replaced=0 compared=12 '
                'mismatches=0 fpscr_mismatches=0 abi_mismatches=0 abandoned=0 pending=0')
        mod.validate_formatter(good, 'shadow')
        mod.validate_formatter(good.replace('compared=12', 'compared=11').replace(
            'pending=0', 'pending=1'), 'shadow')
        mod.validate_formatter('', None)
        mod.validate_formatter(good.replace('mode=shadow', 'mode=on').replace(
            'replaced=0 compared=12', 'replaced=12 compared=0'), 'on')
        for bad in ('', good + '\n' + good, good.replace('mode=shadow', 'mode=on'),
                    good.replace('compared=12', 'compared=11'),
                    good.replace('eligible=12', 'eligible=0'),
                    good.replace('pending=0', 'pending=1'),
                    good.replace('abandoned=0', 'abandoned=1'),
                    good.replace('abi_mismatches=0', 'abi_mismatches=1'),
                    good.replace(' abi_mismatches=0', ''),
                    good.replace('mismatches=0', 'mismatches=1')):
            with self.subTest(bad=bad), self.assertRaises(RuntimeError):
                mod.validate_formatter(bad, 'shadow')
        with self.assertRaises(RuntimeError): mod.validate_formatter(good, None)

    def test_audio_profile_does_not_leak_into_ordinary_runs(self):
        env = {'OPENMUA2_AUDIO_PROFILE': 'inherited.csv', 'UNRELATED': 'keep'}
        mod.configure_audio_profile(env, False, Path('run'))
        self.assertEqual(env, {'UNRELATED': 'keep'})
        mod.configure_audio_profile(env, True, Path('run'))
        self.assertEqual(env['OPENMUA2_AUDIO_PROFILE'], str(Path('run/audio-profile.csv')))

    def test_audio_profile_requires_backend_and_completed_callback_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaises(RuntimeError):
                mod.validate_audio_profile(root, 'audio backend: No Audio Output\n')
            with self.assertRaises(RuntimeError):
                mod.validate_audio_profile(root, 'audio backend: Cubeb\n')
            p = root / 'audio-profile.csv'
            p.write_text('channel,calls\n0,100\n')
            with self.assertRaises(RuntimeError):
                mod.validate_audio_profile(root, 'audio backend: Cubeb\n')
            p.write_text('channel,calls\n11,0\n')
            with self.assertRaises(RuntimeError):
                mod.validate_audio_profile(root, 'audio backend: Cubeb\n')
            p.write_text('channel,calls\n11,100\n')
            mod.validate_audio_profile(root, 'audio backend: Cubeb\n')

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
        cases.append(["--profile-audio"])
        cases.append(["--audio", "Cubeb"])
        for targets in ("803c63bd", "803c63bc,", "100000000", "junk", ",".join(["803c63bc"]*33)):
            cases.append(["--jit-block-profile", "--jit-profile-callers", targets])
        for extra in cases:
            with self.subTest(extra=extra), mock.patch("sys.argv", ["runner"] + required + extra), \
                 mock.patch.object(mod.shutil, "copytree") as copy, mock.patch("sys.stderr"):
                with self.assertRaises(SystemExit) as raised:
                    mod.main()
                self.assertEqual(raised.exception.code, 2)
                copy.assert_not_called()

class ProcessorLaunchTest(unittest.TestCase):
    def api(self):
        api = mock.Mock()
        api.get_mask.return_value = 0xffff
        return api

    def test_default_does_not_change_affinity(self):
        api = self.api()
        with mock.patch.object(mod.subprocess, "Popen") as popen:
            self.assertIs(mod.launch_on_processor(["runner"], affinity=api), popen.return_value)
        api.get_mask.assert_not_called()
        api.set_mask.assert_not_called()

    def test_child_inherits_selected_mask_then_parent_restored(self):
        api = self.api()
        child = mock.Mock()
        def launch(command, **kwargs):
            self.assertEqual(api.set_mask.call_args_list, [mock.call(4)])
            self.assertEqual(kwargs, {"creationflags": 123})
            return child
        with mock.patch.object(mod.subprocess, "Popen", side_effect=launch):
            self.assertIs(mod.launch_on_processor(["runner"], 2, affinity=api,
                                                 creationflags=123), child)
        self.assertEqual(api.set_mask.call_args_list, [mock.call(4), mock.call(0xffff)])
        child.terminate.assert_not_called()

    def test_failed_launch_still_restores_parent(self):
        api = self.api()
        with mock.patch.object(mod.subprocess, "Popen", side_effect=OSError("launch failed")):
            with self.assertRaisesRegex(OSError, "launch failed"):
                mod.launch_on_processor(["runner"], 2, affinity=api)
        self.assertEqual(api.set_mask.call_args_list, [mock.call(4), mock.call(0xffff)])

    def test_failed_restore_stops_child(self):
        api = self.api()
        api.set_mask.side_effect = [None, OSError("restore failed")]
        with mock.patch.object(mod.subprocess, "Popen") as popen:
            with self.assertRaisesRegex(OSError, "restore failed"):
                mod.launch_on_processor(["runner"], 2, affinity=api)
            popen.return_value.terminate.assert_called_once_with()
            popen.return_value.wait.assert_called_once_with(timeout=10)

    def test_invalid_or_disallowed_processor_never_launches(self):
        for index in (-1, 64, True, "2", 16):
            api = self.api()
            with self.subTest(index=index), mock.patch.object(mod.subprocess, "Popen") as popen:
                with self.assertRaises(ValueError):
                    mod.launch_on_processor(["runner"], index, affinity=api)
                popen.assert_not_called()
                api.set_mask.assert_not_called()

    def test_failed_affinity_setup_never_launches(self):
        api = self.api()
        api.set_mask.side_effect = OSError("selection failed")
        with mock.patch.object(mod.subprocess, "Popen") as popen:
            with self.assertRaisesRegex(OSError, "selection failed"):
                mod.launch_on_processor(["runner"], 2, affinity=api)
            popen.assert_not_called()

    def test_runtime_must_confirm_selected_processor(self):
        log = ("CPU backend: JIT\nEffective video: dual_core=0 sync_gpu=0\n"
               "Host CPU affinity: logical_processors=1 mask=0x4\n")
        mod.validate_runtime_settings(log, "jit", 2)
        with self.assertRaisesRegex(RuntimeError, "requested logical processor"):
            mod.validate_runtime_settings(log, "jit", 0)


if __name__ == '__main__':unittest.main()
