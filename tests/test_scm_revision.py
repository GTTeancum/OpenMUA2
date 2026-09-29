"""Exercise version generation in real Git repositories without game data."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
DOLPHIN = ROOT / "project/lib/ModernGekko/vendor/dolphin"


@unittest.skipUnless(shutil.which("cmake") and shutil.which("git"), "CMake and Git required")
class ScmRevisionTests(unittest.TestCase):
    def test_branch_and_detached_revision_counts(self):
        for branch in ("main", "master", "feature"):
            with self.subTest(branch=branch), tempfile.TemporaryDirectory() as temp:
                source = Path(temp) / "source"
                output = Path(temp) / "output"
                template = source / "Source/Core/Common/scmrev.h.in"
                template.parent.mkdir(parents=True)
                shutil.copyfile(DOLPHIN / "Source/Core/Common/scmrev.h.in", template)

                def git(*args):
                    return subprocess.run(["git", "-C", str(source), *args],
                                          check=True, capture_output=True, text=True)

                git("init", "-b", branch)
                git("add", ".")
                git("-c", "user.name=Test", "-c", "user.email=test@example.invalid",
                    "commit", "-m", "fixture")
                git("checkout", "--detach")
                git("-c", "user.name=Test", "-c", "user.email=test@example.invalid",
                    "commit", "--allow-empty", "-m", "ahead")
                result = subprocess.run([
                    "cmake", f"-DPROJECT_SOURCE_DIR={source.as_posix()}",
                    f"-DPROJECT_BINARY_DIR={output.as_posix()}", "-DGIT_FOUND=TRUE",
                    f"-DGIT_EXECUTABLE={Path(shutil.which('git')).as_posix()}",
                    "-P", str(DOLPHIN / "CMake/ScmRevGen.cmake")],
                    check=True, capture_output=True, text=True)
                self.assertEqual(result.stderr, "")
                header = (output / "Source/Core/Common/scmrev.h").read_text()
                expected = 0 if branch == "feature" else 1
                self.assertIn(f"#define SCM_COMMITS_AHEAD_MASTER {expected}\n", header)


if __name__ == "__main__":
    unittest.main()
