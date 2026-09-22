# Source provenance, notices and scope

This is a personal local-development handoff assembled from the user's returned dependency collection and retained MG01/FPC01 artifacts. The upstream project names do not imply endorsement. New workspace scripts/tests/documentation are provided under GPL-3.0-or-later; the root `LICENSE` contains the GPL text. This statement does not relicense third-party components or proprietary game material.

The full pinned source-node list, upstream URLs and transport hashes are in `locks/SOURCE_SNAPSHOT.json`. Key revisions are:

| Component | Commit |
|---|---|
| ModernGekko-Template | `c2553a99584b2045dbe46a70e1570994dae8beef` |
| DolRecomp | `1bec3554ecc4817cf78319ca3d8a0669477f29fa` |
| ModernGekko | `edd460adad0e82b56909af1d793642c83b6d31c8` |
| RecompCore | `10f7ac4fe48f74485fbc161ebf4d67242e11e671` |

Each upstream tree retains its own LICENSE/COPYING/NOTICE and per-file attribution. Components include separate dependency licenses and some upstream prebuilt Windows dependency libraries. Read their notices before redistributing a derived public repository or binaries. No blanket license to those components is substituted by the root GPL text.

## Packaging transformations

All recursive source nodes are ordinary files/directories, not embedded Git repositories. Nested `.gitignore`, `.gitattributes` and `.gitmodules` contents are preserved as `UPSTREAM.gitignore`, `UPSTREAM.gitattributes` and `UPSTREAM.gitmodules`. Ten internal file symlinks are materialized as ordinary files to avoid requiring Windows symlink privileges. 33 standalone font resources are omitted. Five recovered production files incorporate the MG01/FPC01 fixes. The complete path-by-path transformation record is in `locks/SOURCE_TRANSFORMATIONS.json`; all other delivered upstream file bytes are preserved.

The omitted optional OSD font is not silently downloaded. The pinned runtime checks whether the file exists and can use the built-in ImGui font. Example/demo font assets, GameCube font binaries and font atlas images are also omitted. Those optional assets are not regenerated or replaced in this package. Their notices remain where available. No standalone font binaries or original source transport archives containing fonts are bundled here.

## Exclusions

No MUA2 game image/executable/assets, raw captured RAM, MUA2 save state, complete generated game translation, compiled game module, compiler installer, private Linux SDK or WIT distribution is supplied in this repo ZIP. The user's original WBFS and installed toolchain remain necessary. The prior collector's WIT archive may be used locally; preserve its own accompanying notices.

Historical reports may contain addresses, hashes and measured descriptions of the user's game-derived work. They do not license proprietary game content and do not prove that the corresponding missing implementation is in this package. Publication to a remote repository is outside the automated setup/snapshot/backup operations.
