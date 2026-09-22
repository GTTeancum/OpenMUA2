# Local Git, snapshots and recovery

## Initial setup

`Setup.cmd` stages the package manifest's exact file list, not `git add .`. All nested source repositories are flattened. No Git submodule network operations are required. The first commit includes source dependencies, recovered edits, tests, notes, receipts and the manifest. No `.git` directory is supplied in the initial ZIP; setup creates it locally.

Local `core.autocrlf=false`, `core.longpaths=true` and Windows `core.filemode=false` avoid unintended source transformations. No global setting is modified. Your configured Git identity is used. If none exists, the initial/snapshot commit uses a command-local descriptive fallback identity; it does not assign you a fake global identity or create a remote account.

## Snapshot and backup are different

`Snapshot.cmd -m "message"` commits allowed source changes. It notices tracked deletions and new files in code directories. Unknown new files in the root are not silently committed; place development files in a documented source directory or review/stage them explicitly with Git. It refuses currently tracked game/private files instead of silently including them.

`Backup.cmd` does **not** make a commit. It writes a ZIP containing all local Git references/history plus the source worktree as it exists, including uncommitted edits, untracked allowed source files and tracked deletions. It verifies ZIP CRCs, records per-file hashes, rechecks source bytes and detects changes to Git status/HEAD during creation. Save files and the WBFS are excluded.

Do not edit files or run another Git operation while backing up. A failed backup leaves no final named ZIP. Destination collisions are rejected. Source backup can be large because the complete vendored dependencies and their Git history are retained. `.backups` is ignored and is not recursively packed into itself.

## Restore testable local history

1. Extract a source backup ZIP into a new empty folder.
2. Run `Setup.cmd` there. It verifies `.backup-manifest.json`, verifies the embedded history bundle, recreates branches/tags and restores the index at the saved HEAD.
3. Inspect `git status`. Your previously uncommitted edits/deletions/new files remain as worktree changes; setup does not check them out of existence.
4. Copy your separately backed-up WBFS to the root and, with the game closed, your runtime user data to `.local/user`.

No network access is used for restoration. A local fetch from `.recovery-history.bundle` is not a fetch from a remote service. Detached-HEAD work is restored at the recorded commit on the local restore branch; branch naming may differ, but recorded history and worktree content are preserved. Ignored global Git configuration is not archived.

## What must be backed up separately

The original `.wbfs`/`.wbfN` files and `.local/user` are deliberately excluded from source backups. Back up both to a private external location with the game closed. Rebuildable generated C, extracted assets, build products and logs are also excluded. Deliberately copy a diagnostic log under `evidence/` only after checking it for private/game-derived content; `.log` files are ignored by default, so use a reviewed text filename or stage intentionally.

Git ignore rules do not retroactively remove files someone force-added to older commits. The supplied setup begins with a source-only history, but do not force-add game images or secrets later. A backup's Git bundle includes existing history. These tools do not claim to scrub proprietary data or secrets already inserted into past commits.

No remote is configured and nothing is published. Before creating a public repository, review upstream redistribution notices, included third-party prebuilt dependencies, recovery evidence and any future game-derived code. The `GitHub` name in your local folder is not publishing authorization.
