@echo off
rem SPDX-License-Identifier: GPL-3.0-or-later
rem LOCAL01a: test an interpreter, not just the presence of py.exe.
setlocal EnableExtensions DisableDelayedExpansion
set "ERRORLEVEL="
pushd "%~dp0" || exit /b 1
set "_OPENMUA2_PY="
set "_OPENMUA2_SELECTOR="
rem Process-local isolation only. No registry, PATH or global settings are changed.
set "PYTHONHOME="
set "PYTHONPATH="
set "PYTHONINSPECT="
set "PYTHONSTARTUP="
set "PYLAUNCHER_ALLOW_INSTALL="
set "PYLAUNCHER_DRYRUN="
set "PYTHON_MANAGER_AUTOMATIC_INSTALL=false"

rem An explicit override must be a real executable path, without extra arguments.
if defined OPENMUA2_PYTHON (
  call :probe "%OPENMUA2_PYTHON%" ""
  if not defined _OPENMUA2_PY goto :bad_override
  goto :run
)

rem Prefer working interpreter executables over a possibly stale py registration.
for /f "delims=" %%P in ('where.exe python.exe 2^>nul') do call :probe "%%P" ""
for /f "delims=" %%P in ('where.exe python3.exe 2^>nul') do call :probe "%%P" ""
if defined _OPENMUA2_PY goto :run

rem Also look in the normal per-user and machine-wide install directories.
if defined LOCALAPPDATA (
  for /d %%D in ("%LOCALAPPDATA%\Programs\Python\Python*") do call :probe "%%~fD\python.exe" ""
  for /d %%D in ("%LOCALAPPDATA%\Python\pythoncore-*") do call :probe "%%~fD\python.exe" ""
)
if defined ProgramFiles (
  for /d %%D in ("%ProgramFiles%\Python*") do call :probe "%%~fD\python.exe" ""
)
if defined _OPENMUA2_PY goto :run

rem A broken default must not hide a working registered 3.11+ interpreter.
rem Broad -3 also covers future versions. Every selector is actually tested.
for /f "delims=" %%P in ('where.exe py.exe 2^>nul') do (
  for %%V in (-3 -3.14 -3.13 -3.12 -3.11) do call :probe "%%P" "%%V"
)
if defined _OPENMUA2_PY goto :run

echo ERROR: No working 64-bit Python 3.11 or newer was found.
echo A stale py.exe entry will be skipped; no Python is installed by this script.
echo Set OPENMUA2_PYTHON to the full path of an existing python.exe, then retry.
echo Example only - replace this path with your actual Python location:
echo   set "OPENMUA2_PYTHON=D:\Tools\Python313\python.exe"
echo   Setup.cmd
popd
exit /b 1

:bad_override
echo ERROR: OPENMUA2_PYTHON does not start a compatible 64-bit Python 3.11+:
echo   "%OPENMUA2_PYTHON%"
echo Correct the executable path, or clear the override to use automatic detection:
echo   set "OPENMUA2_PYTHON="
popd
exit /b 1

:run
echo Using Python: "%_OPENMUA2_PY%" %_OPENMUA2_SELECTOR%
rem Run the requested action exactly once; its failure is NOT a detection failure.
"%_OPENMUA2_PY%" %_OPENMUA2_SELECTOR% -E -s "%~dp0tools\workspace.py" %*
set "_OPENMUA2_RC=%ERRORLEVEL%"
popd
exit /b %_OPENMUA2_RC%

:probe
if defined _OPENMUA2_PY exit /b 0
if not exist "%~1" exit /b 1
rem Avoid the WindowsApps python Store alias, which can open the Store rather
rem than run Python. Installed manager runtimes and py.exe remain eligible.
set "_OPENMUA2_CANDIDATE=%~1"
if /i "%~nx1"=="python.exe" if not "%_OPENMUA2_CANDIDATE:\Microsoft\WindowsApps\=%"=="%_OPENMUA2_CANDIDATE%" exit /b 1
if /i "%~nx1"=="python3.exe" if not "%_OPENMUA2_CANDIDATE:\Microsoft\WindowsApps\=%"=="%_OPENMUA2_CANDIDATE%" exit /b 1
rem 42 is a deliberate success sentinel: a shim returning zero is not Python.
"%~1" %~2 -E -s -c "import sys, struct, argparse, hashlib, json, pathlib, zipfile; sys.exit(42 if sys.version_info >= (3, 11) and struct.calcsize('P') == 8 else 1)" <nul >nul 2>&1
if not "%ERRORLEVEL%"=="42" exit /b 1
set "_OPENMUA2_PY=%~1"
set "_OPENMUA2_SELECTOR=%~2"
exit /b 0
