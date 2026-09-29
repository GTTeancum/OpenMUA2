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

if __name__ == '__main__':unittest.main()
