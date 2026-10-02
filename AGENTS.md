# OpenMUA2 local agent handoff

Read `docs/CURRENT-STATUS.md` and `README.md` first. This repo is LOCAL01, **MG01 + FPC01 recovered source**, not MR01. Do not infer implementation from historical reports or claim a missing checkpoint is delivered. Never ask the user to upload MG02/MR01; those were assistant-generated archives that were not retained.

The intended root is `D:\Programming\GitHub\OpenMUA2\`; the original Wii USA RMSE52 WBFS is at root. Discover the actual filename. Never alter, overwrite, stage or publish it. Extraction, generated game C, build products, logs and saves belong in `.local`, not tracked source. Never replace the game's real `sys/main.dol` with a code-generation composite.

All 42 source nodes are pinned and flattened under `project`. No implicit branch updates, submodule initialization or dependency downloads. Original upstream Git metadata is kept as `UPSTREAM.*`; do not rename it back casually. `locks/SOURCE_TRANSFORMATIONS.json` records the five recovered production edits and packaging transformations. Preserve upstream notices and the absence of standalone fonts.

Use `tools/workspace.py` and the Windows entry points. Assume build tools are already installed. Do not run installers or change global Git, environment, SDK or system settings. Check real tool results; a generated command is not a passing Windows build. Failed steps must stop and keep logs.

Before editing, inspect `git status`; do not reset, clean, overwrite or discard user changes. Commit focused source changes with `Snapshot.cmd` or reviewed Git commands; save source/history with `Backup.cmd`. Never push or add a remote without explicit authorization. Keep original WBFS and `.local/user` backed up separately.

Do not disable runtime fallback/hash protection or alter timing just to reach a later screen. The missing MEM2/loop/FMA/REL fixes require implementation and regression tests before live Wii shadow verification is safe. Preserve failures as failures; tests are scoped, not full-game guarantees. Update `docs/CURRENT-STATUS.md` and add new evidence for each checkpoint, then take a local source snapshot and backup before handing off.

`FILE-MANIFEST.json` records the delivered baseline, not current editable source. Intentional edits make `verify` report differences. Do not regenerate the manifest just to hide unreviewed changes. A verified source backup has its own independent worktree manifest.

## Mandatory disk hygiene

- Check repository size and free disk space before large builds, diagnostic batches and backups, and clean up at every completed checkpoint. Do not let old experiments accumulate between turns.
- Keep the current supported build, one useful rollback baseline, compact test results/logs and the specific states needed to reproduce unresolved issues. Remove superseded build intermediates, duplicate test-profile disk images, obsolete generated translations and redundant runtime copies after checking that no active process needs them. Historical receipts remain historical evidence, not proof that a pruned binary is still present.
- Retain only the two newest verified routine source/history backups. Verify a replacement before pruning older backups; retain any explicitly designated recovery archive. Put backup temporary files on the destination drive, not an almost-full source drive.
- Never delete the original WBFS, authoritative extracted game data, actual user saves, uncommitted source, pinned dependencies or active-process files. Do not blanket-delete `.local` or run `git clean`. Preserve unique diagnostic data until its purpose and replacement are established; deduplicate verified identical copies first.
- Before deletion, inspect candidates and resolve every absolute path inside the intended artifact directory. Record what was removed, bytes reclaimed and remaining repository size. Keep compact failure evidence even when large failed-run artifacts are pruned.
- If free space falls below 20 GiB, stop creating large artifacts and clean obsolete data first. Cleanup is part of the authorized development workflow, not an optional follow-up for the user.
