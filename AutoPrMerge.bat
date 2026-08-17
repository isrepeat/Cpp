@ECHO OFF
SETLOCAL

SET "_REPO_ROOT=%~dp0."
SET "_SCRIPT=%~dp0UtilityHelpersLib\Scripts\PowerShell\AutoPrMerge.ps1"

IF NOT EXIST "%_SCRIPT%" (
    ECHO [AutoPrMerge] Script not found: "%_SCRIPT%"
    ECHO [AutoPrMerge] Initialize the UtilityHelpersLib submodule first.
    SET "_RC=1"
    GOTO :EXIT_SCRIPT
)

powershell -NoProfile -ExecutionPolicy Bypass -File "%_SCRIPT%" -BatDir "%_REPO_ROOT%" %*
SET "_RC=%ERRORLEVEL%"

:EXIT_SCRIPT
PAUSE
ENDLOCAL & EXIT /B %_RC%
