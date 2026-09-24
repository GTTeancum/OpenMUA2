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


class StaticRecompRelReturnFastPathPerfTests(unittest.TestCase):
    def test_dispatchability_carries_rel_section_identity(self) -> None:
        header = HEADER.read_text(encoding="utf-8")
        smc = SMC.read_text(encoding="utf-8")

        self.assertIn("u32* rel_section_index = nullptr", header)
        self.assertIn("u32 rel_section_index = 0xffffffffu;", smc)
        self.assertIn(
            "ResolveNativeAddress(address, &linked_address, &rel_section_index)",
            smc,
        )
        self.assertIn("*rel_section_index_out = rel_section_index;", smc)

    def test_same_section_return_skips_generic_section_scan(self) -> None:
        smc = SMC.read_text(encoding="utf-8")
        run = RUN.read_text(encoding="utf-8")

        self.assertIn(
            "u32 StaticRecompCore::TranslateRelAddressFromSection("
            "u32 linked_address, u32 rel_section_index)",
            smc,
        )
        self.assertIn(
            "if (rel_section_index < m_active_rel_sections.size())",
            smc,
        )
        self.assertIn(
            "return section.runtime_start + (linked_address - section.linked_start);",
            smc,
        )
        self.assertIn("return TranslateRelAddress(linked_address);", smc)

        self.assertIn(
            "u32 dispatch_rel_section_index = 0xffffffffu;",
            run,
        )
        self.assertIn(
            "&dispatch_rel_section_index",
            run,
        )
        self.assertIn(
            "TranslateRelAddressFromSection(m_guest.pc, dispatch_rel_section_index)",
            run,
        )
        self.assertIn(
            "fast_native_continue(m_guest.pc, &linked_dispatch_address,",
            run,
        )


if __name__ == "__main__":
    unittest.main()
