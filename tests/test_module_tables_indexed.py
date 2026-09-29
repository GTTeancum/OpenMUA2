"""Validate module ABI coverage and hashes from synthetic indexed output."""
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest


SCRIPT = Path(__file__).resolve().parents[1] / (
    "project/lib/ModernGekko/vendor/dolphin/module-template/gen_module_tables.py")
PROTOTYPES = "void func_80001000(CPUState* ctx);\nvoid func_80001020(CPUState* ctx);\n"


def indexed(starts="0x80001000u, 0x80001020u", ends="0x80001010u, 0x80001030u"):
    return (PROTOTYPES +
            f"static const u32 dolrecomp_run_start[2] DOLRECOMP_UNUSED = {{{starts}}};\n" +
            f"static const u32 dolrecomp_run_end[2] DOLRECOMP_UNUSED = {{{ends}}};\n")


class ModuleTablesIndexedTests(unittest.TestCase):
    def generate(self, header):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            dol = bytearray(0x140)
            struct.pack_into(">I", dol, 0, 0x100)
            struct.pack_into(">I", dol, 0x48, 0x80001000)
            struct.pack_into(">I", dol, 0x90, 0x40)
            dol[0x100:] = bytes(range(0x40))
            (root / "main.dol").write_bytes(dol)
            (root / "generated.h").write_text(header)
            (root / "generated_smc.txt").write_text("")
            result = subprocess.run([sys.executable, str(SCRIPT),
                str(root / "generated.h"), str(root / "generated_smc.txt"),
                str(root / "main.dol"), str(root / "module_tables.inc")],
                capture_output=True, text=True)
            output = root / "module_tables.inc"
            return result, output.read_text() if output.exists() else ""

    def test_indexed_matches_linear_hashes_and_preserves_hole(self):
        result, output = self.generate(indexed())
        self.assertEqual(result.returncode, 0, result.stderr)
        linear = PROTOTYPES + (
            "if (address >= 0x80001000u && address < 0x80001010u) {}\n"
            "if (address >= 0x80001020u && address < 0x80001030u) {}\n")
        old_result, old_output = self.generate(linear)
        self.assertEqual(old_result.returncode, 0, old_result.stderr)
        self.assertEqual(output, old_output)
        self.assertIn("MODULE_CHUNK_RANGE_COUNT 2u", output)
        self.assertNotIn("{0x80001000u, 0x80001030u}", output)

    def test_rejects_malformed_indexed_ranges(self):
        for header in (indexed(ends="0x80001010u"),
                       indexed(ends="0x80001000u, 0x80001030u"),
                       indexed(ends="0x80001011u, 0x80001030u"),
                       indexed(ends="0x80001024u, 0x80001030u"),
                       PROTOTYPES + "static const u32 dolrecomp_run_start[1] = {0x80001000u};"):
            with self.subTest(header=header):
                result, output = self.generate(header)
                self.assertNotEqual(result.returncode, 0)
                self.assertEqual(output, "")

    def test_merged_table_takes_precedence_over_retained_arrays(self):
        merged = indexed(ends="0x80001020u, 0x80001030u") + (
            "{0x80001000u, 0x80001010u, func_80001000},\n"
            "{0x80001020u, 0x80001030u, func_80001020},\n")
        result, output = self.generate(merged)
        expected_result, expected = self.generate(indexed())
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(expected_result.returncode, 0, expected_result.stderr)
        self.assertEqual(output, expected)


if __name__ == "__main__":
    unittest.main()
