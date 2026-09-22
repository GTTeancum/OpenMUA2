# OpenMUA2 — LOCAL01 local source workspace

**Target folder:** `D:\Programming\GitHub\OpenMUA2\`  
**Input:** your Wii USA `RMSE52` `.wbfs` image, kept at that folder's root.  
**Delivery:** September 22, 2026. Offline source snapshot; no remote repository is created.

> **Important recovery boundary:** this contains the actually recoverable **MG01 + FPC01** source baseline, not MR01. The reported MR01 gameplay archive, cumulative 31-file overlay, native module and diagnostic states are unavailable. Their surviving reports are preserved as historical records, not substituted for missing code. LOCAL01 generates a DOL-native module and leaves REL execution to the retained runtime fallback. It is **not** the previously reported native-REL gameplay build or a verified Windows game.

The full vendored source tree is here, including DolRecomp, ModernGekko, RecompCore and all 42 pinned repository/submodule entries. The five recovered production file edits are already applied. Setup does not fetch repositories or install anything. There is no dependency on a ChatGPT download link after saving this package locally. See `docs/CURRENT-STATUS.md` before continuing development.

## Set up the local repository

Extract the **contents** of this ZIP directly into `D:\Programming\GitHub\OpenMUA2\`, so `Setup.cmd` is in that folder, not in a nested `OpenMUA2_LOCAL01` folder. Keep the WBFS beside it, with its existing filename. An existing folder containing only your game image is fine. **Do not extract over another source checkout or accept overwrite prompts for existing source files.** Use a new folder and compare first in that situation.

Open CMD and run:

```bat
cd /d D:\Programming\GitHub\OpenMUA2
OpenMUA2.cmd setup
```

Alternatively, double-click `Setup.cmd`. It verifies the package, initializes a local `main` branch and commits exactly the delivered files. It does not stage your WBFS, add an `origin`, push anything, change global Git settings, or alter an existing repository. An existing repository is left for you to merge/stage explicitly. The double-click launcher pauses at the end; the direct command does not.

Keep the original ZIP as a separate recovery copy. `OpenMUA2.code-workspace` opens the tree in VS Code.

## Build locally

Your existing Windows tools must include 64-bit Python 3.11+, Git, CMake, Ninja, the MSVC C/C++ toolchain and Windows SDK. Disc extraction also uses your existing **Wiimms ISO Tools (`wit.exe`)**. Nothing is downloaded or installed. `Build.cmd` activates an installed x64 MSVC environment when necessary; an x64 Native Tools Command Prompt also works.

```bat
cd /d D:\Programming\GitHub\OpenMUA2
Build.cmd --jobs 2
```

When WIT is not on PATH, supply its existing location:

```bat
Build.cmd --jobs 2 --wit "D:\Tools\Wiimms\bin\wit.exe"
```

The earlier dependency collection already contains `wit-v3.05a-r8638-cygwin64.zip`; its `bin\wit.exe` can be used in place without running the included installer. This local repository deliberately does not duplicate compiler installations or the collector's tool/SDK archives. The full source dependencies are included.

One root-level WBFS is selected automatically. To choose between several:

```bat
Build.cmd --image "Marvel - Ultimate Alliance 2 (USA).wbfs" --jobs 2
```

The script builds/tests DolRecomp, extracts the original image into `.local\game`, checks the exact original DOL/REL hashes, regenerates the DOL code, builds ModernGekko and the native module, then verifies the module ABI and DOL chunk hashes. It stops on a failed step and writes logs/receipts under `.local`. Original game files are not patched. The baseline module uses **O0 with IPO disabled**; higher optimization is an explicit development choice, not a correctness/performance promise.

**Windows execution of these launchers and a complete Windows build have not been tested here.** The Linux recompiler, regeneration, audit helper and local-repository tools were tested; see `evidence/local01/VALIDATION.json`. This is a source handoff with a concrete Windows build entry point, not a prebuilt or guaranteed-working Windows release.

After a successful local build:

```bat
Run.cmd
```

The runner uses `.local\user` for its own profile/configuration/save storage. Existing controller settings are retained. No physical input, audible audio, game-save round trip or gameplay is newly verified by this package. Do not enable the historical Wii shadow/lockstep verifier on this baseline; its MEM2 restoration repair is among the missing changes.

## Keep progress recoverable

```bat
Snapshot.cmd -m "Describe the source changes just completed"
Backup.cmd
```

`Snapshot.cmd` commits current source changes locally. `Backup.cmd` makes a verified ZIP under `.backups` containing the current source worktree **and a Git history bundle**, including uncommitted edits, new source files and tracked deletions. It does not implicitly commit or push. Pause editing/Git operations while making a backup.

For a separate drive, specify a new ZIP filename in an existing or new directory:

```bat
Backup.cmd --destination "E:\Backups\OpenMUA2-source-2026-09-22.zip"
```

To restore, extract a backup into an empty folder and run `Setup.cmd`. That restores Git history and the saved worktree without checking out over uncommitted changes. **Back up your original WBFS and `.local\user` separately with the game closed.** A source backup intentionally excludes both. Keeping `.backups` on the same disk is useful for mistakes, not disk-loss protection.

## Folder layout

```text
OpenMUA2\
  <your game>.wbfs              Your original input; never committed by these tools
  Setup.cmd / Build.cmd / Run.cmd
  Snapshot.cmd / Backup.cmd
  tools\                       Local workflow, image checks, module verifier
  project\                     Flat vendored upstream source; no nested Git repos
    lib\DolRecomp\
    lib\ModernGekko\
      vendor\dolphin\          RecompCore, GXRuntime and recursive dependencies
  locks\                       Exact source pins and packaging transformations
  patches\                     Cumulative recovered production patch
  docs\                        Build guide, current status, historical evidence
  recovery\                    Retained MG01 helpers and FPC01 repair/tests
  evidence\local01\            Fresh packaging/build/tool validation
  .local\                      Generated code, builds, extracted game, logs, user data
  .backups\                    Local source/history backups
```

`.local`, `.backups`, game images, generated game code and saves are excluded from normal Git operations. Upstream nested `.git*` control files were preserved under `UPSTREAM.*` names, preventing nested submodule/ignore/attribute behavior. The root attributes preserve source bytes. Standalone font files are omitted; the runtime's pinned OSD code checks for its optional external font and can use ImGui's built-in fallback.

## Development references

Read `AGENTS.md`, `docs/BUILD-AND-RUN.md`, `docs/RECOVERY-PLAN.md` and `docs/BACKUP-AND-GIT.md`. `FILE-MANIFEST.json` describes the delivered baseline. `OpenMUA2.cmd verify` compares against that baseline without undoing edits; it should report differences after intentional development.

No proprietary game image, generated game translation, raw RAM capture, local save state or game executable is included. Upstream licenses and notices remain in their source trees; see `THIRD-PARTY-NOTICES.md`. This package does not grant rights to publish game-derived code/assets. No remote or public publishing operation is performed.
