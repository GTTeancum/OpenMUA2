param([Parameter(Mandatory=$true)][string]$WorkDirectory)
$ErrorActionPreference = 'Stop'
$testDirectory = Join-Path ([IO.Path]::GetFullPath($WorkDirectory)) ('launcher-profile-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testDirectory | Out-Null
$dll = Join-Path $testDirectory 'ProfileTests.dll'
& "$env:WINDIR\Microsoft.NET\Framework64\v4.0.30319\csc.exe" /nologo /target:library "/out:$dll" /reference:System.IO.Compression.dll /reference:System.IO.Compression.FileSystem.dll /reference:System.Windows.Forms.dll (Join-Path $PSScriptRoot 'OpenMUA2.cs')
if ($LASTEXITCODE -ne 0) { throw 'Launcher test compilation failed' }
$type = [Reflection.Assembly]::LoadFile($dll).GetType('OpenMUA2')
$flags = [Reflection.BindingFlags]'Static,NonPublic'
$managed = $type.GetMethod('IsManagedProfile',$flags)
$replace = $type.GetMethod('ReplaceManagedDevice',$flags)
[string]$body = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'Xbox-v5-profile.txt'))
[string]$current = "[Wiimote1]`nDevice = SDL/0/Xbox One Controller`n" + $body
if (!$managed.Invoke($null,@($current,$body))) { throw 'Managed profile rejected' }
[string]$custom = $current.Replace('Start | `Back`','Start')
if ($managed.Invoke($null,@($custom,$body))) { throw 'Custom profile accepted' }
if ($managed.Invoke($null,@(($current + "`nUnknown = 1"),$body))) { throw 'Unknown setting accepted' }
if ($managed.Invoke($null,@(($current.Replace('[Wiimote1]',"[Wiimote1]`nDevice = Duplicate")),$body))) { throw 'Duplicate key accepted' }
if ($managed.Invoke($null,@(($current + "`n[Wiimote1]`n"),$body))) { throw 'Duplicate section accepted' }
[string]$crlf = $current.Replace("`r",'').Replace("`n","`r`n")
[string]$next = $replace.Invoke($null,@($crlf,'XInput/0/Gamepad'))
if ($next -ne $crlf.Replace('Device = SDL/0/Xbox One Controller','Device = XInput/0/Gamepad')) { throw 'CRLF migration changed another field' }
[string]$extra = $crlf + "`n[Wiimote2]`nDevice = Custom/1/Gamepad`nButtons/A = Custom`n"
$next = $replace.Invoke($null,@($extra,'XInput/0/Gamepad'))
if ($next -ne $extra.Replace('Device = SDL/0/Xbox One Controller','Device = XInput/0/Gamepad')) { throw 'Mixed-newline migration changed another port' }
if (!$managed.Invoke($null,@($next,$body))) { throw 'Migrated profile rejected' }
if ($managed.Invoke($null,@("[Wiimote1]`nMalformed",$body))) { throw 'Malformed profile accepted' }
'PASS: 9 profile parsing/migration checks; no hardware or host input.'
