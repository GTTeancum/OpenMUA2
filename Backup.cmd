@echo off
setlocal
call "%~dp0OpenMUA2.cmd" backup %*
exit /b %ERRORLEVEL%
