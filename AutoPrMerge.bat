@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION
REM ===============================================================
REM AutoPrMerge.bat
REM - Searches for %_SUBMODULE_NAME%.sln (independent of submodule folder name)
REM - Inside %_SUBMODULE_DIR% searches for AutoPrMerge.ps1
REM - Runs the PowerShell script, forwarding all arguments
REM ===============================================================

REM Determine search root: repo root if available, otherwise batch file folder.
SET "_BAT_DIR=%~dp0"
FOR /F "usebackq delims=" %%R IN (`git rev-parse --show-toplevel 2^>NUL`) DO SET "_REPO_ROOT=%%R"
IF NOT DEFINED _REPO_ROOT (
    SET "_REPO_ROOT=%_BAT_DIR%"
)
IF "%_REPO_ROOT:~-1%"=="\" SET "_REPO_ROOT=%_REPO_ROOT:~0,-1%"

SET "_SUBMODULE_NAME=UtilityHelpersLib"

REM ---------------------------------------------------------------
REM 0) Fast-path candidates (nearest likely locations)
REM    - same dir as .bat
REM    - sibling subfolder named %_SUBMODULE_NAME%
REM    - repo root
REM    - subfolder under repo root
REM ---------------------------------------------------------------
SET "_SUBMODULE_SLN="
FOR %%C IN (
    "%_BAT_DIR%%_SUBMODULE_NAME%.sln"
    "%_BAT_DIR%%_SUBMODULE_NAME%\%_SUBMODULE_NAME%.sln"
    "%_REPO_ROOT%\%_SUBMODULE_NAME%.sln"
    "%_REPO_ROOT%\%_SUBMODULE_NAME%\%_SUBMODULE_NAME%.sln"
) DO (
    IF EXIST "%%~fC" (
        SET "_SUBMODULE_SLN=%%~fC"
        GOTO :FOUND_SUBMODULE_SLN
    )
)

REM ---------------------------------------------------------------
REM 1) Fallback: recursive search anywhere under repo root
REM Using DIR:
REM   /s     -- search recursively
REM   /b     -- bare format (only paths)
REM   /a:-d  -- files only (exclude directories)
REM   2^>nul -- suppress "File Not Found" message
REM ---------------------------------------------------------------
FOR /F "delims=" %%F IN ('
    DIR /s /b /a:-d "%_REPO_ROOT%\%_SUBMODULE_NAME%.sln" 2^>nul
') DO (
    SET "_SUBMODULE_SLN=%%~fF"
    GOTO :FOUND_SUBMODULE_SLN
)

ECHO [AutoPrMerge] Could not find "%_SUBMODULE_NAME%.sln" under "%_REPO_ROOT%".
ECHO [AutoPrMerge] Make sure the submodule "%_SUBMODULE_NAME%" is initialized:
ECHO     git submodule update --init --recursive
SET "_RC=1"
GOTO :EXIT_SCRIPT

:FOUND_SUBMODULE_SLN
REM Extract directory containing %_SUBMODULE_SLN%.
FOR %%I IN ("%_SUBMODULE_SLN%") DO SET "_SUBMODULE_DIR=%%~dpI"

REM Search for AutoPrMerge.ps1 inside the submodule directory (recursive).
SET "_PS1_PATH="
FOR /F "delims=" %%P IN ('
    DIR /s /b /a:-d "%_SUBMODULE_DIR%\AutoPrMerge.ps1" 2^>nul
') DO (
    SET "_PS1_PATH=%%~fP"
    GOTO :FOUND_PS1
)

ECHO [AutoPrMerge] Could not find AutoPrMerge.ps1 under "%_SUBMODULE_DIR%".
ECHO [AutoPrMerge] Please verify the script exists inside the submodule.
SET "_RC=1"
GOTO :EXIT_SCRIPT

:FOUND_PS1
REM Run PowerShell implementation, forwarding all arguments.
powershell -NoProfile -ExecutionPolicy Bypass -File "%_PS1_PATH%" -BatDir "%~dp0." %*
SET "_RC=%ERRORLEVEL%"
GOTO :EXIT_SCRIPT

:EXIT_SCRIPT
PAUSE
ENDLOCAL & EXIT /B %_RC%