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
$names = @('data/vv_tips.engb','data/vv_tips.itab','data/vv_tips.xmlb',
    'packages/generated/maps/package/permanent.fb','packages/generated/maps/package/permanent_rev.fb',
    'textures/fonts/rev_med_eng.igb','textures/fonts/rev_med_ws_eng.igb')
$lines = @('OpenMUA2-Xbox-UI-v2')
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
Check $valid ($manifest.Replace('UI-v2','UI-v1')) $false
Check $valid (($lines[0..6]) -join "`n") $false
Check $valid ($manifest + "`n" + $lines[1]) $false
Check $valid ($manifest.Replace('data/vv_tips.engb','../unknown')) $false
Check $valid ($manifest.Replace($lines[1].Substring(0,64),('z'*64))) $false
"PASS: $checks Xbox UI version/hash/preservation-gate checks; synthetic archives only; no game or host input."
