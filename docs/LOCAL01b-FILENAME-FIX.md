# OpenMUA2 LOCAL01b — Windows setup filename repair

September 22, 2026. Cumulative setup patch for LOCAL01 or LOCAL01a.
This does not upgrade the MG01 + FPC01 baseline to MR01.

## Apply

Extract **all contents** of `OpenMUA2_LOCAL01b_FilenameFix.zip` directly into
`D:\Programming\GitHub\OpenMUA2\`, merging folders and allowing replacement of
`FILE-MANIFEST.json`, the launcher/LOCAL01a support files, and the tinygltf
sample/test/packaging-record files included here. Then run **Setup.cmd** again.
There is no new repair command, registry change, tool installation or requirement
to recreate the repository. Keep the original source ZIP plus these patch files
as a recovery copy. Do not reapply LOCAL01a on top of LOCAL01b: its older manifest
would undo this manifest update.

The patch replaces known baseline files, so compare before extracting over
intentional edits to the included fixture, tester, launcher or packaging records.
Unrelated source files are not included and remain untouched. Setup still checks
every manifest entry; intentional unrelated edits will still be reported.

## Cause and scope

The original ZIP and manifest contain this tinygltf sample image:

```text
models/CubeImageUriSpaces/ 2x2 image  has multiple      spaces.png
```

There is an ASCII space immediately after the last slash. Microsoft's Windows
filename documentation describes removing leading/trailing ASCII spaces when
names are created. That behavior is consistent with the reported one-file setup
failure. The local Windows filesystem was not inspected remotely; the exact
extracted filename there has not been observed.

The new filename removes only that initial space:

```text
models/CubeImageUriSpaces/2x2 image  has multiple      spaces.png
```

The 79-byte PNG is byte-identical to the original (SHA-256
`00b9839320de4bbd342d092e5cedf05845ab4be733c0240d5dd5466a9565b217`).
All repeated internal spaces remain. `CubeImageUriMultipleSpaces.gltf` points to
the new name. The corresponding tinygltf test assertion and comment now match
this portable fixture; its remaining URI encoding/decoding checks are retained.
The on-disk fixture no longer tests an initial space in that image filename.
The library implementation and runtime build files are unchanged.

`locks/SOURCE_TRANSFORMATIONS.json` records the rename and exact before/after
hashes. The file manifest uses the new path and checks the original image bytes,
updated reference and test. Nothing is excluded from verification to hide an
error. The previously delivered Python-selection repair is included unchanged.

No WBFS, extracted game data, generated game translation, compiler/runtime
implementation, save file or Git history is supplied or changed by this overlay.
The normal Setup.cmd operation can still initialize/commit a new local source
repository after verification, as before. On hosts that preserved the old
leading-space file, that legacy extra file is not deleted by this ZIP overlay;
the new model/manifest use only the portable file.

## Verification

See `evidence/local01b/` for the reproduced original manifest failure and fresh
unit/native-fixture test logs. The external ZIP validation receipt covers final
archive integrity and full patched-tree checks. Tests run in Linux; native
Windows extraction and Setup.cmd execution are not claimed.

To rerun the Python tests in the local repo:

```bat
OpenMUA2.cmd test
```

Microsoft reference (KB 2829981):
https://learn.microsoft.com/en-us/troubleshoot/windows-client/shell-experience/file-folder-name-whitespace-characters
