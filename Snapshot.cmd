@echo off
setlocal
call "%~dp0OpenMUA2.cmd" snapshot %*
exit /b %ERRORLEVEL%
