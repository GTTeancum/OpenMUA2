# OpenMUA2 LOCAL01a — Python launcher repair

Applies to the delivered OpenMUA2 LOCAL01 source package. The game/compiler/runtime
source baseline remains MG01 + FPC01, not MR01. This is not a new game build.

## The reported failure

The original `OpenMUA2.cmd` ran `where py`, then used `py -3` whenever py.exe was
present. It never checked whether that launcher could start Python. The supplied
error shows the selected interpreter at a missing Daytona-project location, so
`tools\workspace.py setup` did not start. This is interpreter discovery, not a
WBFS or compiler failure. The error alone does not identify whether the stale
selection originated in registration, launcher configuration or environment.

## Apply

Extract the CONTENTS of `OpenMUA2_LOCAL01a_LauncherFix.zip` directly into:

    D:\Programming\GitHub\OpenMUA2\

Allow replacement of `OpenMUA2.cmd` and `FILE-MANIFEST.json`. Merge the included
`docs`, `tests` and `evidence` directories. Do not replace or delete whole folders.
The replacement manifest is for the supplied LOCAL01 source bytes plus this fix;
it does not hide other source edits. Keep the original LOCAL01 ZIP for recovery.

Then double-click `Setup.cmd`, or run from CMD:

    cd /d D:\Programming\GitHub\OpenMUA2
    Setup.cmd

Apply the WHOLE hotfix: replacing just the CMD file leaves the old integrity
manifest expecting its old hash. No game image, game asset, compiled game code,
upstream source file, existing Git history, save, or system installation is part
of this overlay. No rebuild or deletion of `.local` is needed for this fix.

This overlay targets the failed initial LOCAL01 setup. Do not apply it on top of
a later package or an edited launcher/manifest without comparing your changes.
If restoring a source backup (`.backup-manifest.json` present), restore its
original bytes first and perform this change as a source update afterward;
this hotfix does not replace a backup's separate integrity manifest.

## Interpreter selection

The shared launcher is used by Setup, Build, Run, Snapshot and Backup. It:

1. Honors an explicit `OPENMUA2_PYTHON` executable path, and fails clearly when
   that override is invalid rather than silently switching environments.
2. Tests all `python.exe` / `python3.exe` PATH candidates, then conventional
   per-user and machine-wide Python install directories.
3. Tests registered `py -3` and explicit `-3.14`, `-3.13`, `-3.12`, `-3.11`
   selectors, so a stale default need not hide an older working installation.

Each candidate must actually execute a standard-library probe as 64-bit Python
3.11 or newer. The probe exits with the distinctive status 42 on success;
a shim returning zero is not accepted. The requested workspace operation is
then run exactly once, and its exit code is returned. A failure inside setup or
build is not retried with another interpreter.

The launcher locally clears inherited PYTHONHOME/PYTHONPATH/interactive-startup
variables, invokes Python with `-E -s`, and disables the official launchers'
automatic-install modes. It avoids the WindowsApps `python.exe` Store alias.
These are process-local settings only: no registry edits, global PATH edits,
installer commands, remote uploads or system-wide configuration changes occur.
Third-party executable shims are external programs; their own behavior cannot
be guaranteed by this script.

The search is not a recursive disk scan. A portable or nonstandard Python that
is neither on PATH nor in the searched directories needs the override below.
The launcher will print the selected executable (and py selector when used).

## Choose an existing Python explicitly

The following is an EXAMPLE PATH, not an assumed installation on your PC.
Replace it with the full path to a working 64-bit Python 3.11+ executable:

    set "OPENMUA2_PYTHON=D:\Tools\Python313\python.exe"
    Setup.cmd

Use an executable path only, without extra command-line arguments. This CMD
setting applies to that terminal session. To resume automatic discovery:

    set "OPENMUA2_PYTHON="

When no compatible installation is found, the launcher stops with those
instructions; it does not download or install Python. Do not restore the old
Daytona folder merely to satisfy a stale launcher selection.

## Validation and limits

Fresh Linux checks: 40 existing workspace tests plus 13 portable launcher/probe
checks passed. Five native-Windows test cases are included but were skipped
because native Windows CMD was unavailable. The portable cases execute the
production Python probe, check minimum version/word size and polluted-environment
handling, and inspect launcher dispatch/exit-code contracts. They do not emulate
CMD or establish a completed Windows setup/build.

The native cases test an explicit working interpreter, a missing override,
repository paths and argument strings with spaces, workspace nonzero-exit
propagation without duplicate execution, and stale inherited PYTHONHOME/PATH.
They can be run by `OpenMUA2.cmd test` once Python has been selected. No Windows
runtime test, new gameplay test or upstream compiler change is claimed here.

`evidence/local01a/VALIDATION.json` and `tests.txt` record the completed tests.
The updated full manifest was checked against the original LOCAL01 archive and
the replacement files. The external ZIP validation receipt identifies the final
hotfix bytes.

## References

Official Python documentation consulted for launcher selectors, installation
controls, and isolation switches (not needed to apply the offline patch):

- https://docs.python.org/3/using/windows.html
- https://docs.python.org/3.13/using/windows.html
- https://docs.python.org/3/using/cmdline.html

License: GPL-3.0-or-later for this launcher and tests, as in the local tooling.
