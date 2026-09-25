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
            "int ChunkIndexOf(u32 address, u32* linked_address = nullptr,",
            header,
        )
        self.assertIn(
            "int StaticRecompCore::ChunkIndexOf(u32 address, u32* linked_address_out,",
            smc,
        )
        self.assertIn(
            "ResolveNativeAddress(address, &linked_address, rel_section_index, true,",
            smc,
        )
        self.assertIn("*linked_address_out = linked_address;", smc)
        self.assertIn(
            "ChunkIndexOf(address, linked_address, rel_section_index, rel_section_hint)",
            smc,
        )

    def test_native_burst_reuses_resolution_from_dispatchability(self) -> None:
        run = RUN.read_text(encoding="utf-8")

        self.assertIn("u32 linked_dispatch_address = ppc.pc;", run)
        self.assertIn(
            "DispatchableAt(ppc.pc, &entry_chunk_index, &linked_dispatch_address,",
            run,
        )
        self.assertIn("m_guest.pc = linked_dispatch_address;", run)
        self.assertIn(
            "fast_native_continue(m_guest.pc, linked_result_address,",
            run,
        )
        self.assertIn(
            "fast_dispatchable_at(address, &chunk_index, linked_address, rel_section_index,",
            run,
        )
        self.assertNotIn(
            "ResolveNativeAddress(runtime_dispatch_address, &linked_dispatch_address, nullptr)",
            run,
        )

        # Linked->runtime translation after dispatch remains intact; this change
        # only removes the duplicate runtime->linked lookup.
        self.assertIn(
            "m_guest.pc = TranslateRelAddress(linked_result_address, dispatch_rel_section_index,",
            run,
        )

    def test_same_section_return_translation_uses_hint_before_full_scan(self) -> None:
        header = HEADER.read_text(encoding="utf-8")
        smc = SMC.read_text(encoding="utf-8")
        run = RUN.read_text(encoding="utf-8")

        self.assertIn(
            "u32 TranslateRelAddress(u32 linked_address, u32 rel_section_hint = 0xffffffffu,",
            header,
        )
        self.assertIn(
            "if (rel_section_hint < m_active_rel_sections.size() &&",
            smc,
        )
        self.assertIn(
            "translate_section(rel_section_hint, &runtime_address)",
            smc,
        )
        self.assertIn(
            "*resolved_rel_section_index = i;",
            smc,
        )
        self.assertIn(
            "*resolved_rel_section_index = 0xffffffffu;",
            smc,
        )
        self.assertIn("u32 dispatch_rel_section_index = 0xffffffffu;", run)
        self.assertIn("&dispatch_rel_section_index", run)

    def test_same_section_native_resolution_uses_hint_before_full_scan(self) -> None:
        header = HEADER.read_text(encoding="utf-8")
        smc = SMC.read_text(encoding="utf-8")
        run = RUN.read_text(encoding="utf-8")

        self.assertIn(
            "u32 rel_section_hint = 0xffffffffu);",
            header,
        )
        self.assertIn(
            "if (rel_section_hint < m_active_rel_sections.size() && "
            "resolve_section(rel_section_hint))",
            smc,
        )
        self.assertIn("if (i == rel_section_hint)", smc)
        self.assertIn(
            "ResolveNativeAddress(address, &linked_address, rel_section_index, true,",
            smc,
        )
        self.assertIn(
            "const u32 rel_section_hint = rel_section_index ? *rel_section_index : 0xffffffffu;",
            run,
        )
        self.assertIn(
            "rel_section_hint) &&",
            run,
        )
        # Hint miss still retains the old refresh fallback.
        self.assertIn("RefreshRelSections();", smc)
        self.assertIn("return resolve_active();", smc)

    def test_dispatchability_skips_forced_fallback_helper_when_ranges_are_empty(self) -> None:
        smc = SMC.read_text(encoding="utf-8")

        guard = (
            "if (!m_forced_fallback_ranges.empty() && "
            "IsForcedFallbackAddress(address))"
        )
        self.assertEqual(smc.count(guard), 2)
        self.assertIn(
            "for (const StaticRecompRange& range : m_forced_fallback_ranges)",
            smc,
        )
        self.assertIn(
            "if (address >= range.start && address < range.end)",
            smc,
        )

    def test_interpreter_fallback_skips_empty_forced_range_scan(self) -> None:
        run = RUN.read_text(encoding="utf-8").replace("\r\n", "\n")

        self.assertIn(
            "if (m_module_active && !m_forced_fallback_ranges.empty() &&\n"
            "            IsForcedFallbackAddress(ppc.pc))",
            run,
        )
        self.assertNotIn(
            "if (m_module_active && IsForcedFallbackAddress(ppc.pc))",
            run,
        )

    def test_interpreter_fallback_checks_termination_before_dispatch_probes(self) -> None:
        run = RUN.read_text(encoding="utf-8").replace("\r\n", "\n")

        self.assertIn(
            "} while (ppc.downcount > 0 && *state_ptr == CPU::State::Running &&\n"
            "                   !(m_module_active && DispatchableAt(ppc.pc)) &&\n"
            "                   !IsHostCallAddress(ppc.pc));",
            run,
        )
        self.assertNotIn(
            "} while (!(m_module_active && DispatchableAt(ppc.pc)) &&\n"
            "                   !IsHostCallAddress(ppc.pc) && ppc.downcount > 0 &&",
            run,
        )

    def test_burst_backedge_checks_termination_before_continuation_probe(self) -> None:
        run = RUN.read_text(encoding="utf-8").replace("\r\n", "\n")

        self.assertIn(
            "} while (ppc.downcount > 0 && *state_ptr == CPU::State::Running &&\n"
            "                 fast_native_continue(m_guest.pc, linked_result_address,\n"
            "                                      linked_result_reusable, &linked_dispatch_address,\n"
            "                                      &dispatch_rel_section_index));",
            run,
        )
        self.assertNotIn(
            "} while (fast_native_continue(m_guest.pc, linked_result_address,",
            run,
        )

    def test_burst_backedge_does_not_duplicate_module_active_check(self) -> None:
        run = RUN.read_text(encoding="utf-8").replace("\r\n", "\n")
        smc = SMC.read_text(encoding="utf-8")

        self.assertNotIn(
            "} while (m_module_active &&",
            run,
        )
        # Every fast continuation path already rejects an inactive module.
        self.assertIn(
            "if (!m_module_active || m_chunk_lookup_table.empty())",
            run,
        )
        self.assertIn(
            "if (!m_module_active || m_chunk_lookup_table.empty())",
            smc,
        )

    def test_continuation_reuses_linked_result_with_safe_fallbacks(self) -> None:
        header = HEADER.read_text(encoding="utf-8")
        smc = SMC.read_text(encoding="utf-8")
        run = RUN.read_text(encoding="utf-8")

        self.assertIn("FastDispatchableLinkedAt(u32 runtime_address, u32 linked_address,", header)
        self.assertIn(
            "FastDispatchableLinkedAt(address, preserved_linked_address, *rel_section_index,",
            run,
        )
        self.assertIn("linked_result_address = m_guest.pc;", run)
        self.assertIn("linked_result_reusable = !do_ls;", run)
        self.assertIn("linked_result_reusable = false;", run)
        self.assertIn(
            "m_guest.pc = TranslateRelAddress(linked_result_address, dispatch_rel_section_index,",
            run,
        )
        self.assertIn("&dispatch_rel_section_index);", run)
        # A direct linked lookup must still prove that REL chunks belong to an
        # active section, and DOL chunks require the non-REL sentinel.
        self.assertIn("if (m_chunk_rel_sections[chunk] >= 0)", smc)
        self.assertIn("if (rel_section_index >= m_active_rel_sections.size())", smc)
        self.assertIn("else if (rel_section_index != 0xffffffffu)", smc)
        # If the preserved result cannot be trusted, continuation retains the
        # old runtime->linked resolver and its refresh fallback.
        self.assertIn(
            "return fast_dispatchable_at(address, &chunk_index, linked_address, rel_section_index,",
            run,
        )
        self.assertIn("RefreshRelSections();", smc)


if __name__ == "__main__":
    unittest.main()
