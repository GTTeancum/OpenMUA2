# TODO

## Tracked Items

1. Hero Management and pause-menu overlays.
3. Costume switching.

These are deferred items, not authorization to resume the blocked controls goal. Gameplay acceptance and edge-case testing belong to the beta testers. Four-port testing, a fresh maze-win replay, Nullifier research, a broad interaction inventory, packaging, and installed-build regressions are outside the current pass.

## 1. Hero Management and Pause-Menu Overlays

Deferred at the user's request. Check earlier hooks and test setup before changing rendering behavior; the cause is not established.

- [ ] Fix retained menu graphics over gameplay after View ÃƒÂ¢Ã¢â‚¬Â Ã¢â‚¬â„¢ Hero Management ÃƒÂ¢Ã¢â‚¬Â Ã¢â‚¬â„¢ Back. Start ÃƒÂ¢Ã¢â‚¬Â Ã¢â‚¬â„¢ Hero Details ÃƒÂ¢Ã¢â‚¬Â Ã¢â‚¬â„¢ Back ÃƒÂ¢Ã¢â‚¬Â Ã¢â‚¬â„¢ Back restored the world in the recorded test.
- [ ] Review gameplay elements appearing over menu/instruction panels, including the signal marker over hacking tutorial text. Do not assume every instance has the same cause.

Evidence: [Hero Management graphics audit](../evidence/windows-20261007/HERO-MENU-GRAPHICS-AUDIT.json).

## 3. Costume Switching

- [ ] On the active characters menu, X cycles the selected character through available costumes and Y opens Details (replacing the current X Details binding). Update the visible button prompts to match. Use the local XML1 XboxRecomp repository implementation as the reference. Default is `skin`; additional costumes use `skin_02` through `skin_10`. Skip missing costumes, cycle in numerical order, and wrap back to the default after the last available costume. Example: `skin` -> `skin_03` -> `skin_04` -> `skin_07` -> `skin`. Keep these bindings scoped to the active characters menu; preserve X/Y actions on other menus.

## Xbox / XInput Quick Reference

Agreed gameplay mapping. Context-specific menu differences and unresolved prompts are tracked above; this table is not a claim that every installed UI label has been replaced.

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
