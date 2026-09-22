@echo off
setlocal
call "%~dp0OpenMUA2.cmd" setup %*
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" echo Setup failed. Read the error above; no game image was overwritten.
if "%RC%"=="0" echo Local source setup complete. This is LOCAL01, not the missing MR01 gameplay build.
echo.
pause
exit /b %RC%
