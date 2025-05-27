REM TODO: Reloading project not always work what you expected, try write / use VSIX to manage solution / projects.
@echo off
setlocal

if "%~1"=="" (
    echo Usage: ForceProjectReload "Relative\Path\To\MyProject.vcxproj"
    exit /b 1
)

REM Путь к текущей папке, где находится этот .bat
set SCRIPT_DIR=%~dp0
set PROJECT_PATH=%SCRIPT_DIR%%~1

if not exist "%PROJECT_PATH%" (
    echo Project file "%PROJECT_PATH%" not found.
    exit /b 1
)

powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%ForceProjectReload.ps1" "%PROJECT_PATH%"

endlocal
