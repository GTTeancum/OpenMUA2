import importlib.util
from pathlib import Path
import re
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "tools" / "merge_mua2_generated.py"
SPEC = importlib.util.spec_from_file_location("merge_mua2_generated_holes", MODULE_PATH)
assert SPEC and SPEC.loader
merge = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(merge)


DOL_HEADER = """// generated test header
// Function entry points
void func_80001000(CPUState* ctx);
#define DOLRECOMP_ENTRY_POINT 0x80001000u
typedef void (*DolRecompFunction)(CPUState* ctx);
static inline DolRecompFunction dolrecomp_find_original(u32 address) {
    if (address >= 0x80001000u && address < 0x80001010u && ((address - 0x80001000u) & 3u) == 0u) return func_80001000;
    return NULL;
}
static inline int dolrecomp_call_original(CPUState* ctx, u32 address) {
    (void)ctx;
    (void)address;
    return 0;
}
"""

REL_HEADER = """// REL generated test header
void func_80002000(CPUState* ctx);
static inline void test_range(u32 address) {
    if (address >= 0x80002000u && address < 0x80002010u && ((address - 0x80002000u) & 3u) == 0u) return;
}
"""

ENTRY_RE = re.compile(
    r"\{(0x[0-9A-F]+)u, (0x[0-9A-F]+)u, func_[0-9A-F]+\}"
)


class MergeDispatchHoleTests(unittest.TestCase):
    def test_merged_dispatch_does_not_bridge_uncovered_guest_code(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            output = Path(td) / "generated.h"
            merge.emit_header(DOL_HEADER, REL_HEADER, output)
            text = output.read_text(encoding="utf-8")

        ranges = [
            (int(start, 16), int(end, 16))
            for start, end in ENTRY_RE.findall(text)
        ]
        self.assertEqual(
            ranges,
            [(0x80001000, 0x80001010), (0x80002000, 0x80002010)],
        )

        uncovered_address = 0x80001800
        self.assertFalse(
            any(start <= uncovered_address < end for start, end in ranges),
            "merged dispatch must not make holes between generated ranges native-eligible",
        )


if __name__ == "__main__":
    unittest.main()
