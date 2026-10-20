# Alchemy Batch Converter

Native Windows desktop app using igb-blender's real importers and exporters.
The executable embeds the add-on's tracked source, profile catalog and worker.
Blender 4.5 and .NET Framework 4.8.1 must be installed; Blender is detected at
its standard installation path or selected in the app. No Python installation
is needed to run the app.

## Use

Run `.local/AlchemyBatchConverter/AlchemyBatchConverter.exe` from this repository.
Choose separate source/output folders, scan, review the detected import profiles,
choose a target preset, select assets, and convert. Source and output cannot be
nested. Folder structure is preserved. Existing output files are never overwritten.
Source originals remain unchanged. Use a fresh output folder for another run.

A target preset configures a supported exporter combination, not an arbitrary
Alchemy engine version. The 15 source profiles come directly from igb-blender.
Detection is heuristic; choose a source profile explicitly if detection is wrong.
The GUI exposes only export combinations implemented by the bundled exporter.

| Asset type | Conversion |
| --- | --- |
| Skinned characters | Wii RGB5A3; MUA PC DXT/CLUT; XML2 PC and PS2 CLUT; GameCube CMPR |
| Simple static geometry scenes | MUA PC / XML2 PC scene exporter; game-specific validation required |
| Animations, UI/textures, effects, unknown assets | Explicit copy unchanged or skip |
| Wii format-21 input / Xbox 360 native geometry | Copy or skip; current importer limitations |
| Scenes containing animation/actor/effect objects | Copy or skip to avoid discarding these components |

This is not an archive unpacker, herostat/package editor, or universal scene-graph
roundtrip tool. It scans loose `.igb` files, not WAD/PKGB archives. Static scene
conversion uses upstream geometry/collision/light handling; arbitrary embedded
metadata is not guaranteed to survive. Target compatibility still needs testing
in the destination game. Additional maps are supported only by MUA PC DXT output.
The upstream MUA PC exporter requires CLUT for diffuse-only skin output.

Settings can be saved/loaded as JSON. Each conversion uses an isolated background
Blender process, with a ten-minute watchdog. Cancel terminates only that worker.
Output is first written into `.alchemy-staging` beneath the chosen output folder,
parsed, hashed, then published atomically. An interrupted worker can leave its
unpublished staging directory for inspection; no partial final file is published.
Do not run two copies against the same output folder.

Runtime files, reports, logs, and the bundled add-on extract beside the executable
under `.local/alchemy-batch`. Worker environment/config/temp locations are explicitly
redirected there. The executable directory must be writable. Review logs in `jobs/`;
`last-report.json` holds the latest batch receipt. Export report saves a copy.

## Build and validation

`python tools/alchemy_batch/build.py` uses installed Roslyn and .NET Framework
reference assemblies, without downloads or package restore. It bundles the pinned
igb-blender checkout at `.local/luke-pc-20261018/igb-blender-reference`, revision
4672c8d (Wii export PR). Build output is ignored under `.local/AlchemyBatchConverter`.

- `python tools/alchemy_batch/test_worker.py`: classification and safety checks.
- `python tools/alchemy_batch/verify.py <authorized-PC-actors-folder>`: four-model
  Wii batch, exact previously game-tested output hashes, overwrite rejection,
  copy of an unreadable asset, other target exports, source hashes unchanged.
- EXE `--verify-cancel <source> <output>`: contained worker cancellation test.
- `scene_fixture.py`: synthetic cube for static-scene import/export validation.
- EXE `--render <png>`: render the actual WPF content offscreen.

Verification modes use the application's own scan/conversion methods. They do not
open a visible window, send OS input, or operate the desktop. GUI rendering is
checked separately from conversion results; no new in-game run is claimed.

Backend: https://github.com/KaikoClanworth1/igb-blender
Wii exporter contribution: https://github.com/KaikoClanworth1/igb-blender/pull/5
The embedded source preserves the upstream notices. Blender itself is not bundled.
