import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "tools" / "merge_mua2_generated.py"
SPEC = importlib.util.spec_from_file_location("merge_mua2_generated", MODULE_PATH)
assert SPEC and SPEC.loader
merge = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(merge)


def dol_header() -> str:
    return """// generated test header
// Function entry points
void func_80001000(CPUState* ctx);
void func_80001010(CPUState* ctx);
#define DOLRECOMP_ENTRY_POINT 0x80001000u
typedef void (*DolRecompFunction)(CPUState* ctx);
static inline DolRecompFunction dolrecomp_find_original(u32 address) {
    if (address >= 0x80001000u && address < 0x80001020u && ((address - 0x80001000u) & 3u) == 0u) return func_80001000;
    return NULL;
}
static inline int dolrecomp_call_original(CPUState* ctx, u32 address) {
    (void)ctx;
    (void)address;
    return 0;
}
"""


def rel_header(start: int = 0x80E4A164, end: int = 0x80E4A184) -> str:
    return f"""// REL generated test header
void func_{start:08X}(CPUState* ctx);
void func_{start + 0x10:08X}(CPUState* ctx);
static inline void test_range(u32 address) {{
    if (address >= 0x{start:08X}u && address < 0x{end:08X}u && ((address - 0x{start:08X}u) & 3u) == 0u) return;
}}
"""


class MergeDispatchTests(unittest.TestCase):
    def test_rel_metadata_requires_exact_live_audited_rel(self) -> None:
        rel_data = b"synthetic-rel"
        audit = {
            "status": "LIVE_TEXT_MATCH",
            "source_rel_sha256": hashlib.sha256(rel_data).hexdigest(),
        }
        with tempfile.TemporaryDirectory() as td:
            rel_path = Path(td) / "module.rel"
            rel_path.write_bytes(rel_data)
            self.assertEqual(merge.read_audited_rel(audit, rel_path), rel_data)

            rel_path.write_bytes(rel_data + b"-changed")
            with self.assertRaisesRegex(ValueError, "does not match audited source"):
                merge.read_audited_rel(audit, rel_path)

            audit["status"] = "LIVE_TEXT_MISMATCH"
            with self.assertRaisesRegex(ValueError, "does not report LIVE_TEXT_MATCH"):
                merge.read_audited_rel(audit, rel_path)

    def test_merged_dispatch_preserves_instruction_alignment(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            output = Path(td) / "generated.h"
            merge.emit_header(dol_header(), rel_header(), output)
            text = output.read_text(encoding="utf-8")

        guard = "if ((address & 3u) != 0u) return NULL;"
        self.assertIn(guard, text)
        self.assertLess(text.index(guard), text.index("u32 lo = 0;"))
        self.assertIn(
            "{0x80001000u, 0x80001010u, func_80001000}", text
        )
        self.assertIn(
            "{0x80E4A164u, 0x80E4A174u, func_80E4A164}", text
        )

    def test_merged_dispatch_rejects_overlapping_chunks(self) -> None:
        overlapping_rel = rel_header(0x80001008, 0x80001028)
        with tempfile.TemporaryDirectory() as td:
            output = Path(td) / "generated.h"
            with self.assertRaisesRegex(ValueError, "overlapping generated code chunks"):
                merge.emit_header(dol_header(), overlapping_rel, output)

    def test_merged_dispatch_rejects_unaligned_chunks(self) -> None:
        unaligned_rel = rel_header(0x80E4A166, 0x80E4A186)
        with tempfile.TemporaryDirectory() as td:
            output = Path(td) / "generated.h"
            with self.assertRaisesRegex(ValueError, "unaligned generated code chunk"):
                merge.emit_header(dol_header(), unaligned_rel, output)


if __name__ == "__main__":
    unittest.main()
