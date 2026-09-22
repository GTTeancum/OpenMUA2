@echo off
setlocal
call "%~dp0tools\msvc-env.cmd"
if errorlevel 1 exit /b 1
call "%~dp0OpenMUA2.cmd" build %*
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" echo Build stopped. See .local\logs. Source, WBFS and saves were retained.
exit /b %RC%
