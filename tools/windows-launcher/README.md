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
