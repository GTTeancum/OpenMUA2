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
    def test_chunk_lookup_returns_linked_pc_and_rel_section(self) -> None:
        header = HEADER.read_text(encoding="utf-8")
        smc = SMC.read_text(encoding="utf-8")

        self.assertIn(
            "int ChunkIndexOf(u32 address, u32* linked_address = nullptr,",
            header,
        )
        self.assertIn(
            "u32* rel_section_index = nullptr);",
            header,
        )
        self.assertIn(
            "int StaticRecompCore::ChunkIndexOf(u32 address, u32* linked_address_out,",
            smc,
        )
        self.assertIn(
            "ResolveNativeAddress(address, &linked_address, &rel_section_index)",
            smc,
        )
        self.assertIn("*linked_address_out = linked_address;", smc)
        self.assertIn("*rel_section_index_out = rel_section_index;", smc)
        self.assertIn(
            "ChunkIndexOf(address, linked_address, rel_section_index)",
            smc,
        )

    def test_native_burst_reuses_resolution_and_section(self) -> None:
        run = RUN.read_text(encoding="utf-8")

        self.assertIn("u32 linked_dispatch_address = ppc.pc;", run)
        self.assertIn("u32 dispatch_rel_section = 0xffffffffu;", run)
        self.assertIn(
            "DispatchableAt(ppc.pc, &entry_chunk_index, &linked_dispatch_address,",
            run,
        )
        self.assertIn("&dispatch_rel_section)", run)
        self.assertIn("m_guest.pc = linked_dispatch_address;", run)
        self.assertIn(
            "fast_native_continue(m_guest.pc, &linked_dispatch_address,",
            run,
        )
        self.assertIn(
            "fast_dispatchable_at(address, &chunk_index, linked_address, rel_section_index)",
            run,
        )
        self.assertNotIn(
            "ResolveNativeAddress(runtime_dispatch_address, &linked_dispatch_address, nullptr)",
            run,
        )
        self.assertIn(
            "m_guest.pc = TranslateRelAddress(m_guest.pc, dispatch_rel_section);",
            run,
        )

    def test_post_dispatch_translation_fast_paths_origin_section(self) -> None:
        header = HEADER.read_text(encoding="utf-8")
        smc = SMC.read_text(encoding="utf-8")

        self.assertIn(
            "u32 TranslateRelAddress(u32 linked_address, u32 preferred_rel_section) const;",
            header,
        )
        start = smc.index(
            "u32 StaticRecompCore::TranslateRelAddress(u32 linked_address, "
            "u32 preferred_rel_section) const"
        )
        end = smc.index("int StaticRecompCore::GetAddressLookupIndex", start)
        body = smc[start:end]
        self.assertIn(
            "preferred_rel_section < m_active_rel_sections.size()",
            body,
        )
        self.assertIn(
            "const ActiveRelSection& section = "
            "m_active_rel_sections[preferred_rel_section];",
            body,
        )
        self.assertIn(
            "return section.runtime_start + (linked_address - section.linked_start);",
            body,
        )
        # Cross-section and DOL targets must retain the general lookup fallback.
        self.assertIn(
            "ResolveRuntimeAddress(linked_address, &runtime_address);",
            body,
        )


if __name__ == "__main__":
    unittest.main()
