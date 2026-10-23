# TODO

## 1. Keyboard/mouse controls and PC options menus

- Adapt OpenXML1xbox's XML2 PC keyboard/mouse controls and options framework to OpenMUA2's existing gamepad actions, including powers, fusions, block and repeated-X QTEs.
- Reuse configurable primary/secondary bindings, presets, per-player profiles, mouse sensitivity, device assignment and saved settings.
- Include XML1's resolution choices: 720p, 1080p, 1440p and 4K; verify saved selection, actual render/output dimensions and menu layout at higher resolutions.
- Adapt the actual XML2 Options/Advanced menu assets and mouse navigation, recoloring them red to match MUA2 while preserving native backgrounds and animations.
- Verify MUA2-specific menu integration, gameplay controls and rebinding; do not carry over XML1-specific addresses or camera behavior blindly.
- Show an in-game screenshot of the converted UI once it is rendering.

## 2. Testing fixes

- **Blocker: Results screen cannot advance.** Start/continue mapping and Xbox prompt fix staged on October 17; awaiting user confirmation that progression works.
- Confirm the LT sound fix in user testing: repeated fusion activation was removed and the updated executable staged on October 17.
- Fix delayed right-stick camera response.
- Remove B.R.A.G. and Leaders from the Results screen.

## 3. FB to PKGB conversion and loose-file loading

- Adapt OpenXML1xbox's conversion and runtime loading approach to MUA2's `assetsfb.wad`: extract FB resources into loose files and generate standard binary PKGB manifests.
- Preserve original resource filenames, package associations, declaration order and localized-resource conventions; no hash-renamed assets or custom path aliases.
- Add first-run conversion through the game EXE, with progress, cancellation and retry, preserving installed mods, saves and the source archive.
- Support configurable loose-file loading without silently falling back to archived resources; retain packaged mode as an option.

## 4. Platform terminology scan

- Manually review player-facing text and button glyphs for Wii, Nintendo, Wii system memory, Wii Remote, Nunchuk, pointer and console-specific system messages. Include the remaining Wii glyph references reported during user testing. Replace obsolete instructions with accurate PC/gamepad wording; review each match in context.

## 5. Replace the Wiimote screen

- Replace the Wiimote screen with appropriate Xbox/gamepad and keyboard/mouse instructions and artwork.

## Xbox / XInput Quick Reference

Agreed gameplay mapping. Context-specific menu controls may differ.

| Input | Action |
| --- | --- |
| Left stick | Move; steer the hacking signal |
| Right stick | Camera control |
| A | Light attack / menu confirm |
| B | Heavy attack / menu back |
| X | Grab / contextual use |
| Y | Jump / character traversal |
| LB | Block |
| RT + A/B/X/Y | Four power slots |
| D-pad Up/Right/Down/Left | Select roster slots 1/2/3/4; menu navigation |
| LT + A/B/X/Y | Select fusion partner in roster slots 1/2/3/4; self-selection rejected |
| Start/Menu | Title/profile continuation; pause/resume; hacking results continuation |
| Back/View | Hero Management |
| Repeated X presses | Motion-QTE replacement; holding X does not count as repeated presses |
