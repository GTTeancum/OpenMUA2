@echo off
rem Scoped to the calling Build.cmd SETLOCAL; no machine/user environment changes.
where cl >nul 2>&1
if not errorlevel 1 exit /b 0
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo MSVC environment not active. Run Build.cmd from an x64 Native Tools Command Prompt.
  exit /b 1
)
set "VSROOT="
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%I"
if not defined VSROOT (
  echo No installed x64 MSVC build environment found. No installer will be run.
  exit /b 1
)
call "%VSROOT%\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
if errorlevel 1 exit /b 1
where cl >nul 2>&1
exit /b %ERRORLEVEL%
