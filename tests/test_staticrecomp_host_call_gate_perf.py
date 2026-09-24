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


class StaticRecompHostCallGatePerfTests(unittest.TestCase):
    def test_dispatchability_returns_verified_chunk_index(self) -> None:
        header = HEADER.read_text(encoding="utf-8")
        smc = SMC.read_text(encoding="utf-8")

        self.assertIn(
            "bool DispatchableAt(u32 address, u32* chunk_index = nullptr,",
            header,
        )
        self.assertIn(
            "bool FastDispatchableAt(u32 address, u32* chunk_index = nullptr,",
            header,
        )
        self.assertIn("*chunk_index = static_cast<u32>(index);", smc)
        self.assertIn("m_chunk_state[index] != CHUNK_VERIFIED", smc)

    def test_native_burst_only_probes_host_calls_in_candidate_chunks(self) -> None:
        run = RUN.read_text(encoding="utf-8")

        self.assertIn(
            "m_guest.host_call && ChunkContainsHostCall(chunk_index) && "
            "IsHostCallAddress(address)",
            run,
        )
        self.assertIn(
            "DispatchableAt(ppc.pc, &entry_chunk_index, &linked_dispatch_address,", run
        )
        self.assertIn("!host_call_at(ppc.pc, entry_chunk_index)", run)
        self.assertIn(
            "fast_dispatchable_at(address, &chunk_index, linked_address, rel_section_index)", run
        )
        self.assertIn("!host_call_at(address, chunk_index)", run)
        self.assertIn(
            "fast_native_continue(m_guest.pc, &linked_dispatch_address,", run
        )

        native_entry = run.index(
            "DispatchableAt(ppc.pc, &entry_chunk_index, &linked_dispatch_address,"
        )
        sync_in = run.index("SyncIn();", native_entry)
        self.assertNotIn(
            "m_guest.host_call && IsHostCallAddress(ppc.pc)",
            run[native_entry:sync_in],
        )


if __name__ == "__main__":
    unittest.main()
