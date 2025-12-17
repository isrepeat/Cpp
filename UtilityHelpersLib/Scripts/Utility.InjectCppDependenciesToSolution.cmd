@ECHO off
SETLOCAL

SET SCRIPT_DIR=%~dp0
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%\PowerShell\Utility.InjectCppDependenciesToSolution.ps1"
PAUSE

ENDLOCAL