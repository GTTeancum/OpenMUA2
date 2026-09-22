@echo off
setlocal
call "%~dp0OpenMUA2.cmd" run %*
exit /b %ERRORLEVEL%
