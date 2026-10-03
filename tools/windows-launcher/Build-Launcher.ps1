param(
    [Parameter(Mandatory=$true)][string]$RuntimeDirectory,
    [Parameter(Mandatory=$true)][string]$OutputExe,
    [Parameter(Mandatory=$true)][string]$IconPath,
    [string]$XboxUiManifest
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$runtimeRoot = (Resolve-Path -LiteralPath $RuntimeDirectory).Path.TrimEnd('\')
foreach ($required in @('moderngekko-run.exe', 'Sys', 'LICENSE', 'THIRD-PARTY-NOTICES.md')) {
    if (!(Test-Path -LiteralPath (Join-Path $runtimeRoot $required))) { throw "Missing $required" }
}
$buildDirectory = Join-Path $env:TEMP ('OpenMUA2-package-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $buildDirectory | Out-Null
$zipPath = Join-Path $buildDirectory 'payload.zip'
$manifestPath = Join-Path $buildDirectory 'payload.manifest'
$zipFile = [IO.File]::Create($zipPath)
$zip = [IO.Compression.ZipArchive]::new($zipFile, [IO.Compression.ZipArchiveMode]::Create)
$manifest = [Collections.Generic.List[string]]::new()
try {
    foreach ($file in Get-ChildItem -LiteralPath $runtimeRoot -Recurse -File) {
        if ($file.FullName -eq (Join-Path $runtimeRoot 'OpenMUA2.ico')) { continue }
        $relative = $file.FullName.Substring($runtimeRoot.Length + 1).Replace('\','/')
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $file.FullName, $relative, [IO.Compression.CompressionLevel]::Optimal) | Out-Null
        $manifest.Add((Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant() + "`t" + $relative)
    }
    $windowIcon = (Resolve-Path -LiteralPath $IconPath).Path
    [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $windowIcon, 'OpenMUA2.ico', [IO.Compression.CompressionLevel]::Optimal) | Out-Null
    $manifest.Add((Get-FileHash -LiteralPath $windowIcon -Algorithm SHA256).Hash.ToLowerInvariant() + "`tOpenMUA2.ico")
} finally { $zip.Dispose(); $zipFile.Dispose() }
[IO.File]::WriteAllLines($manifestPath, $manifest, [Text.UTF8Encoding]::new($false))
$uiResource = @()
if ($XboxUiManifest) {
    $uiManifestPath = (Resolve-Path -LiteralPath $XboxUiManifest).Path
    if ([IO.File]::ReadAllLines($uiManifestPath)[0] -ne 'OpenMUA2-Xbox-UI-v3') { throw 'Unsupported Xbox UI manifest version.' }
    $uiResource = @("/resource:$uiManifestPath,xbox-ui.manifest")
}
& "$env:WINDIR\Microsoft.NET\Framework64\v4.0.30319\csc.exe" /nologo /target:winexe /platform:x64 /optimize+ "/win32icon:$windowIcon" "/out:$OutputExe" /reference:System.IO.Compression.dll /reference:System.IO.Compression.FileSystem.dll /reference:System.Windows.Forms.dll "/resource:$zipPath,payload.zip" "/resource:$manifestPath,payload.manifest" "/resource:$PSScriptRoot\Xbox-v5-profile.txt,Xbox-v5-profile.txt" @uiResource (Join-Path $PSScriptRoot 'OpenMUA2.cs')
if ($LASTEXITCODE -ne 0) { throw 'Launcher compilation failed.' }
Write-Output "Build intermediates retained at $buildDirectory"
Get-FileHash -LiteralPath $OutputExe -Algorithm SHA256
