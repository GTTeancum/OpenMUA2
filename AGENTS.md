# OpenMUA2 local agent handoff

Read `docs/CURRENT-STATUS.md` and `README.md` first. This repo is LOCAL01, **MG01 + FPC01 recovered source**, not MR01. Do not infer implementation from historical reports or claim a missing checkpoint is delivered. Never ask the user to upload MG02/MR01; those were assistant-generated archives that were not retained.

The intended root is `D:\Programming\GitHub\OpenMUA2\`; the original Wii USA RMSE52 WBFS is at root. Discover the actual filename. Never alter, overwrite, stage or publish it. Extraction, generated game C, build products, logs and saves belong in `.local`, not tracked source. Never replace the game's real `sys/main.dol` with a code-generation composite.

All 42 source nodes are pinned and flattened under `project`. No implicit branch updates, submodule initialization or dependency downloads. Original upstream Git metadata is kept as `UPSTREAM.*`; do not rename it back casually. `locks/SOURCE_TRANSFORMATIONS.json` records the five recovered production edits and packaging transformations. Preserve upstream notices and the absence of standalone fonts.

Use `tools/workspace.py` and the Windows entry points. Assume build tools are already installed. Do not run installers or change global Git, environment, SDK or system settings. Check real tool results; a generated command is not a passing Windows build. Failed steps must stop and keep logs.

Before editing, inspect `git status`; do not reset, clean, overwrite or discard user changes. Commit focused source changes with `Snapshot.cmd` or reviewed Git commands; save source/history with `Backup.cmd`. Never push or add a remote without explicit authorization. Keep original WBFS and `.local/user` backed up separately.

Do not disable runtime fallback/hash protection or alter timing just to reach a later screen. The missing MEM2/loop/FMA/REL fixes require implementation and regression tests before live Wii shadow verification is safe. Preserve failures as failures; tests are scoped, not full-game guarantees. Update `docs/CURRENT-STATUS.md` and add new evidence for each checkpoint, then take a local source snapshot and backup before handing off.

`FILE-MANIFEST.json` records the delivered baseline, not current editable source. Intentional edits make `verify` report differences. Do not regenerate the manifest just to hide unreviewed changes. A verified source backup has its own independent worktree manifest.
