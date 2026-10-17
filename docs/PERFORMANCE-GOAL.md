# TODO

## 1. Keyboard/mouse controls and PC options menus

- Adapt OpenXML1xbox's XML2 PC keyboard/mouse controls and options framework to OpenMUA2's existing gamepad actions, including powers, fusions, block and repeated-X QTEs.
- Reuse configurable primary/secondary bindings, presets, per-player profiles, mouse sensitivity, device assignment and saved settings.
- Adapt the actual XML2 Options/Advanced menu assets and mouse navigation, recoloring them red to match MUA2 while preserving native backgrounds and animations.
- Verify MUA2-specific menu integration, gameplay controls and rebinding; do not carry over XML1-specific addresses or camera behavior blindly.

## 2. Testing fixes

- **Blocker: Results screen cannot advance.** Start/continue mapping and Xbox prompt fix staged on October 17; awaiting user confirmation that progression works.
- Confirm the LT sound fix in user testing: repeated fusion activation was removed and the updated executable staged on October 17.
- Fix delayed right-stick camera response.
- Remove B.R.A.G. and Leaders from the Results screen.

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
