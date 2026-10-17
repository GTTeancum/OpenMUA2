# LT fusion correction

## LT fusion isolation and results fix staged - 2026-10-17

LT no longer emits Block (12). Normal character processing enters the original
fusion branch through a guarded adapter, preserving native mode, availability,
partner and resource checks. One activation attempt is consumed per LT hold;
rejected requests cannot replay activation sound every frame. LB remains Block.

A contained port-0 checkpoint check observed normal -> fusion selection (mode 1)
-> selected partner (mode 2), with Block action/flag zero. LB entered CCHBlock and
cleared on release. Holding LT through the chooser timeout did not re-enter it.
Native screenshots were inspected; this is not a complete fusion/playthrough or
physical-controller acceptance. Audio was not auditioned; user listening remains.
The fixture includes other joined players but only port 0 was exercised.

Windows Release build and five focused tests passed with no compiler/linker
warnings/errors observed. The installed self-contained EXE is
C:/Games/MUA2/OpenMUA2.exe, SHA256
`a6873ddbf4ac371f3bc1b4e9617a39609e6707515067ed2f75fa7d66aeabfcf5`.
Embedded package verification passed. All 400 game files and 55 saves remained
hash-identical. It also includes the mission-results Start/continue fix, whose
in-game acceptance remains with the user. Camera delay and Results tabs remain open.
Rollback: %LOCALAPPDATA%/OpenMUA2/staging-archive/pre-lt-fix-20261017/OpenMUA2.exe.

Implementation: CCHNormal::Process (0x810c19cc) delegates a single LT request to
CCHBlock's fusion-only branch (0x8107e278), with the original 0x60-byte ABI frame
and r24-r31 preservation. The Block preparation and Block action are never used.
The original fusion code and epilogue run; hashes protect both source ranges.
An epilogue observer clears the transient call record, also cleared on reset/load.
The native entry query at return PC 0x8107e30c consumes once, while unrelated
queries retain their existing behavior. Ownership changes, absent/dead actors,
release, menu input and disconnected pads cannot reuse a pending request.

Regression tests exercise 300 held samples after rejection, release/repress,
owner changes, absent actors, existing chooser, and independent LB behavior.
Private evidence: task work/lt-fix-20261017/checks.json and native captures;
work/lt-fix-build.log, lt-regressions.log, lt-package-verification.txt and
lt-staging-verification.json. No proprietary assets or captures are committed.
