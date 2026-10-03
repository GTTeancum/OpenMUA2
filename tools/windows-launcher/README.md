# Windows user package

This source packages the supported Windows runner and its runtime resources into
one root OpenMUA2.exe. GameData and saves stay beside it; the runtime payload and
logs live under LocalAppData. Supply the user-provided icon separately.

```powershell
./tools/windows-launcher/Build-Launcher.ps1 -RuntimeDirectory <prepared-runtime> -OutputExe <output-exe> -IconPath <icon.ico>
```

The prepared runtime must contain moderngekko-run.exe, Sys, LICENSE and
THIRD-PARTY-NOTICES.md. Do not include game data, private diagnostics or saves.
`OpenMUA2.exe --verify-package <report.txt>` verifies every embedded file and runs
runner help without launching gameplay. Normal launch starts from the beginning
with one logical CPU (mask 4), JIT, Vulkan, Cubeb and the game-folder save profile.

Unmodified managed Xbox v5 profiles migrate to a connected Windows XInput gamepad.
Only the player-one Device entry changes; the original file is backed up in Config.
Custom profiles remain untouched. Other gamepads retain SDL discovery as fallback.

Run Test-Profile.ps1 with a writable scratch directory for profile parsing and
migration regression checks. These checks do not operate a controller or desktop.

For the matching Xbox UI, compile a private candidate with
`tools/build_qte_glyph_override.py --xbox-ui` and its required input/output paths.
It creates the WAD and a sibling `.xbox-ui.manifest`. Pass that manifest to
Build-Launcher.ps1 with `-XboxUiManifest`, and stage the resulting executable and
WAD together. The manifest is embedded in the executable; no extra installation
root file is needed. Normal launch checks the seven relevant archive members
before enabling the guarded Xbox prompt resolver. A missing or mismatched member
stops launch with an asset mismatch error, instead of enabling incompatible font
slots. Packages without the manifest retain their original UI behavior.

`Test-Xbox-UI.ps1 -WorkDirectory <scratch>` checks the version/hash gate using small
synthetic archives. The common UI pack is still a partial conversion: remaining
Wii-specific tutorials, menu operations and other QTE variants need validation.
