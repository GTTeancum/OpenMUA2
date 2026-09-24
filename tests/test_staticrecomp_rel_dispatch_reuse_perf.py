from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
STATIC_RECOMP = (
    ROOT
    / "project"
    / "lib"
    / "ModernGekko"
    / "vendor"
    / "dolphin"
    / "Source"
    / "Core"
    / "Core"
    / "PowerPC"
    / "StaticRecomp"
)
HEADER = STATIC_RECOMP / "StaticRecompCore.h"
SMC = STATIC_RECOMP / "StaticRecompCore_SMC.cpp"
RUN = STATIC_RECOMP / "StaticRecompCore_Run.cpp"


class StaticRecompRelDispatchReusePerfTests(unittest.TestCase):
    def test_chunk_lookup_can_return_resolved_linked_pc(self) -> None:
        header = HEADER.read_text(encoding="utf-8")
        smc = SMC.read_text(encoding="utf-8")

        self.assertIn(
            "int ChunkIndexOf(u32 address, u32* linked_address = nullptr);",
            header,
        )
        self.assertIn(
            "int StaticRecompCore::ChunkIndexOf(u32 address, u32* linked_address_out)",
            smc,
        )
        self.assertIn(
            "ResolveNativeAddress(address, &linked_address, nullptr)",
            smc,
        )
        self.assertIn("*linked_address_out = linked_address;", smc)
        self.assertIn("ChunkIndexOf(address, linked_address)", smc)

    def test_native_burst_reuses_resolution_from_dispatchability(self) -> None:
        run = RUN.read_text(encoding="utf-8")

        self.assertIn("u32 linked_dispatch_address = ppc.pc;", run)
        self.assertIn(
            "DispatchableAt(ppc.pc, &entry_chunk_index, &linked_dispatch_address)",
            run,
        )
        self.assertIn("m_guest.pc = linked_dispatch_address;", run)
        self.assertIn(
            "fast_native_continue(m_guest.pc, &linked_dispatch_address)",
            run,
        )
        self.assertIn(
            "fast_dispatchable_at(address, &chunk_index, linked_address)",
            run,
        )
        self.assertNotIn(
            "ResolveNativeAddress(runtime_dispatch_address, &linked_dispatch_address, nullptr)",
            run,
        )

        # Linked->runtime translation after dispatch remains intact; this change
        # only removes the duplicate runtime->linked lookup.
        self.assertIn("m_guest.pc = TranslateRelAddress(m_guest.pc);", run)


if __name__ == "__main__":
    unittest.main()
