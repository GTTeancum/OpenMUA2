import importlib.util
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "tools" / "workspace.py"
SPEC = importlib.util.spec_from_file_location("openmua2_workspace_ci", MODULE_PATH)
assert SPEC and SPEC.loader
workspace = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(workspace)


class WorkspaceCIAllowlistTests(unittest.TestCase):
    def test_github_workflows_are_source_code(self) -> None:
        self.assertTrue(
            workspace.is_code_path(".github/workflows/tooling-ci.yml", set())
        )
        self.assertTrue(
            workspace.is_code_path(".github/workflows/moderngekko-ci.yml", set())
        )

    def test_github_private_files_remain_excluded(self) -> None:
        self.assertFalse(workspace.is_code_path(".github/.env", set()))
        self.assertFalse(workspace.is_code_path(".github/signing-key.pfx", set()))


if __name__ == "__main__":
    unittest.main()
