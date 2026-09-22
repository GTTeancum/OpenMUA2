# Current status — LOCAL01

September 22, 2026. **Recovered source workspace, not a restored MR01 gameplay checkpoint.**

## Actually included

All 42 pinned repository/submodule entries from the returned dependency collector are materialized under `project/`. Their 41 unique transport archives and all three original upload parts were hash-verified before restoration. Recursive submodules are now ordinary vendored directories in a single local Git repository. `locks/SOURCE_SNAPSHOT.json` preserves original revisions, URLs and archive hashes; `locks/SOURCE_TRANSFORMATIONS.json` records all packaging changes.

Five recovered production files differ from the pristine source: the three MG01 Linux include/EGL CMake fixes and FPC01's corrected `fctiw` FI/FR masks in both DolRecomp copies. `patches/LOCAL01-recovered-production.patch` is the corresponding cumulative diff. No REL, MEM2, verifier-boundary, FPCC or FMA behavior was recreated from prose and presented as previously tested code.

The DOL-only generator is retained from MG01 and expects 325 chunks. Its code hashes, ABI and extracted input hashes remain checked. Fallback remains enabled for the REL and modified/uncovered DOL code. The real extracted `sys/main.dol` must never be replaced by a merged code-generation reference.

## What is missing

The MG02 and MR01 ZIPs were not found in the accessible workspace or searched Library. Their reports, screenshots and validation receipts survived; those files cannot reconstruct source bytes, native modules or diagnostic states. **Do not request either missing assistant-generated ZIP from the user or claim it is included here.**

MR01 described a cumulative 31-file overlay, native REL integration, shadow MEM2 isolation, local-loop boundary alignment, a reference floating-compare correction and a multiply-add helper repair. Those changes must be recovered as actual files or implemented and tested anew. The reported 11,832 passing MR01 live comparisons and gameplay sessions are historical, not tests of LOCAL01.

## Validation scope

Fresh LOCAL01 checks are in `../evidence/local01/VALIDATION.json`. They cover local workspace/Git/backup behavior, the retained patch guard, Linux DolRecomp compilation/CTests, regeneration of all 325 DOL chunks and a newly built module audit against the actual retained MG01 binary. That audit is not a newly built game module or a boot test. Generated chunks were compared byte-for-byte with retained MG01 output.

No fresh full runtime build, new full native module compilation, Windows build/execution, WBFS extraction, hardware controller, audio, save/reload or gameplay test was performed as part of this source packaging task. Extraction publication/error tests use fixtures; real game generation uses the retained original DOL. The original gameplay reports must not override these boundaries.

## Immediate priorities

First establish and commit a local build log without changing the pinned baseline. Then restore/reimplement native REL support with the exact observed linked layout and relocation replay. Reconstruct the missing verifier repairs before using Wii live differential checks. Repair and test the FMA helper independently, regenerate/rebuild, and only then repeat actual boot, movement/combat and save/reload tests. See `RECOVERY-PLAN.md`.

This source handoff is designed to stop remote workspace loss from also losing future edits: all delivered source is local and editable, and snapshot/backup commands do not depend on a service or remote repository.
