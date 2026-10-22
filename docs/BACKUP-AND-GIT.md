# Local Git, snapshots and recovery

## Initial setup

`Setup.cmd` stages the package manifest's exact file list, not `git add .`. All nested source repositories are flattened. No Git submodule network operations are required. The first commit includes source dependencies, recovered edits, tests, notes, receipts and the manifest. No `.git` directory is supplied in the initial ZIP; setup creates it locally.

Local `core.autocrlf=false`, `core.longpaths=true` and Windows `core.filemode=false` avoid unintended source transformations. No global setting is modified. Your configured Git identity is used. If none exists, the initial/snapshot commit uses a command-local descriptive fallback identity; it does not assign you a fake global identity or create a remote account.

## Source checkpoints: Git only

User instruction, 2026-10-22: no more full source backups. Use Git commits for
source checkpoints. Do not run `Backup.cmd` or create equivalent source/history
ZIPs or bundles during development or handoff. This supersedes older backup
instructions elsewhere in the repository.

`Snapshot.cmd -m "message"` commits allowed source changes. Alternatively, review
and stage focused paths with Git. Preserve unrelated user changes and never
commit proprietary game data. Pushing still requires user authorization.

The restoration instructions below apply only to existing legacy archives.

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
