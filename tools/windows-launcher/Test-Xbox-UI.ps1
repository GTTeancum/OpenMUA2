param([Parameter(Mandatory=$true)][string]$WorkDirectory)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression
$testRoot = Join-Path ([IO.Path]::GetFullPath($WorkDirectory)) ('xbox-ui-gate-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testRoot | Out-Null
$dll = Join-Path $testRoot 'UiTests.dll'
& "$env:WINDIR\Microsoft.NET\Framework64\v4.0.30319\csc.exe" /nologo /target:library "/out:$dll" /reference:System.IO.Compression.dll /reference:System.IO.Compression.FileSystem.dll /reference:System.Windows.Forms.dll (Join-Path $PSScriptRoot 'OpenMUA2.cs')
if ($LASTEXITCODE -ne 0) { throw 'Xbox UI test compilation failed' }
$type = [Reflection.Assembly]::LoadFile($dll).GetType('OpenMUA2')
$method = $type.GetMethod('VerifyXboxUiAssets', [Reflection.BindingFlags]'Static,NonPublic')
$names = @('data/strings.engb','data/strings.itab','data/strings.xmlb',
    'data/vv_tips.engb','data/vv_tips.itab','data/vv_tips.xmlb',
    'packages/generated/maps/package/permanent.fb','packages/generated/maps/package/permanent_rev.fb',
    'textures/fonts/rev_med_eng.igb','textures/fonts/rev_med_ws_eng.igb')
$lines = @('OpenMUA2-Xbox-UI-v3')
foreach ($name in $names) {
    $sha = [Security.Cryptography.SHA256]::Create()
    try { $hash = [BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::ASCII.GetBytes($name))).Replace('-','').ToLowerInvariant() }
    finally { $sha.Dispose() }
    $lines += "$hash`t$name"
}
function Write-Fixture([string]$mode) {
    $path = Join-Path $testRoot ($mode + '.wad')
    $stream = [IO.File]::Create($path)
    $zip = [IO.Compression.ZipArchive]::new($stream,[IO.Compression.ZipArchiveMode]::Create)
    try {
        foreach ($name in $names) {
            if ($mode -eq 'missing' -and $name -eq $names[0]) { continue }
            $entry = $zip.CreateEntry($name); $s = $entry.Open()
            try {
                $text = if ($mode -eq 'changed' -and $name -eq $names[0]) { 'changed' } else { $name }
                $b = [Text.Encoding]::ASCII.GetBytes($text); $s.Write($b,0,$b.Length)
            } finally { $s.Dispose() }
        }
        if ($mode -eq 'duplicate') { $zip.CreateEntry($names[0]) | Out-Null }
    } finally { $zip.Dispose(); $stream.Dispose() }
    return $path
}
$script:checks = 0
function Check([string]$path,[string]$manifest,[bool]$accepted) {
    $reader = [IO.StringReader]::new($manifest)
    $passed = $true
    try { $method.Invoke($null,@($path,$reader)) | Out-Null }
    catch { if ($_.Exception.InnerException -isnot [IO.InvalidDataException]) { throw }; $passed = $false }
    finally { $reader.Dispose() }
    if ($passed -ne $accepted) { throw "Unexpected Xbox UI gate result for $path" }
    $script:checks++
}
$valid = Write-Fixture 'valid'
$manifest = $lines -join "`n"
Check $valid $manifest $true
Check (Write-Fixture 'changed') $manifest $false
Check (Write-Fixture 'missing') $manifest $false
Check (Write-Fixture 'duplicate') $manifest $false
Check $valid ($manifest.Replace('UI-v3','UI-v2')) $false
Check $valid (($lines[0..6]) -join "`n") $false
Check $valid ($manifest + "`n" + $lines[1]) $false
Check $valid ($manifest.Replace('data/vv_tips.engb','../unknown')) $false
Check $valid ($manifest.Replace($lines[1].Substring(0,64),('z'*64))) $false
# Shared-metric packs must include both coordinate tables, and rapid-tap packs
# must verify the same complete set before enabling their paired runtime path.
$legacyNames = $names
$legacyLines = $lines
$names += @('ui/fonts/rev_med.xmlb','ui/fonts/rev_med_ws.xmlb')
foreach ($name in $names[10..11]) {
    $sha = [Security.Cryptography.SHA256]::Create()
    try { $hash = [BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::ASCII.GetBytes($name))).Replace('-','').ToLowerInvariant() }
    finally { $sha.Dispose() }
    $lines += "$hash`t$name"
}
$shared = Write-Fixture 'shared'
foreach ($version in @(4,5,6)) {
    $current = ($lines -join "`n").Replace('UI-v3',"UI-v$version")
    Check $shared $current $true
    Check $valid $current $false
    Check $shared (($legacyLines -join "`n").Replace('UI-v3',"UI-v$version")) $false
    Check $shared ($current.Replace($lines[10].Substring(0,64),('0'*64))) $false
    Check $shared ($current.Replace($lines[11].Substring(0,64),('0'*64))) $false
    Check $shared ($current + "`n" + $lines[10]) $false
}
Check $shared ($lines -join "`n") $false
Check $shared (($lines -join "`n").Replace('UI-v3','UI-v9')) $false
# Version 7 requires the native Options layout in the paired archive.
$oldLines = $lines
$name = 'packages/generated/maps/package/menus/options_rev.fb'
$names += $name
$sha = [Security.Cryptography.SHA256]::Create()
try { $hash = [BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::ASCII.GetBytes($name))).Replace('-','').ToLowerInvariant() }
finally { $sha.Dispose() }
$lines += "$hash`t$name"
$menu = Write-Fixture 'menu'
$current = ($lines -join "`n").Replace('UI-v3','UI-v7')
Check $menu $current $true
Check $shared $current $false
Check $menu (($oldLines -join "`n").Replace('UI-v3','UI-v7')) $false
Check $menu ($current.Replace($hash,('0'*64))) $false
# Version 8 additionally requires the pause-menu copy of the controls layout.
$oldLines = $lines
$name = 'packages/generated/maps/package/menus/cw_pda_rev.fb'
$names += $name
$sha = [Security.Cryptography.SHA256]::Create()
try { $hash = [BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::ASCII.GetBytes($name))).Replace('-','').ToLowerInvariant() }
finally { $sha.Dispose() }
$lines += "$hash`t$name"
$menu = Write-Fixture 'pause-menu'
$current = ($lines -join "`n").Replace('UI-v3','UI-v8')
Check $menu $current $true
Check $shared $current $false
Check $menu (($oldLines -join "`n").Replace('UI-v3','UI-v8')) $false
Check $menu ($current.Replace($hash,('0'*64))) $false
# Verify package versions activate the corresponding runtime features only.
$configure = $type.GetMethod('ConfigureXboxUiVersion', [Reflection.BindingFlags]'Static,NonPublic')
foreach ($version in @(3,4,5,6,7,8)) {
    $info = [Diagnostics.ProcessStartInfo]::new()
    foreach ($key in @('OPENMUA2_GAMEPAD_PROVIDER','OPENMUA2_DIRECT_GAMEPAD','OPENMUA2_RAPID_TAP_GLYPHS','OPENMUA2_GAMEPAD_AIM','OPENMUA2_GAMEPAD_LOCKON')) {
        $info.EnvironmentVariables.Remove($key)
    }
    $configure.Invoke($null,@($info,$version)) | Out-Null
    foreach ($key in @('OPENMUA2_GAMEPAD_PROVIDER','OPENMUA2_DIRECT_GAMEPAD','OPENMUA2_RAPID_TAP_GLYPHS')) {
        if (($info.EnvironmentVariables[$key] -eq '1') -ne ($version -ge 5)) { throw "Wrong v$version switch: $key" }
        $script:checks++
    }
    foreach ($key in @('OPENMUA2_GAMEPAD_AIM','OPENMUA2_GAMEPAD_LOCKON')) {
        if (($info.EnvironmentVariables[$key] -eq '1') -ne ($version -ge 6)) { throw "Wrong v$version switch: $key" }
        $script:checks++
    }
}
"PASS: $checks Xbox UI version/hash/preservation-gate checks; synthetic archives only; no game or host input."
