@echo off
setlocal enabledelayedexpansion

REM Set current working directory to the script's location
cd /d "%~dp0"

REM Read args: src and dst
set "src=%~1"
set "dst=%~2"

if "%src%"=="" (
    echo ERROR: Source folder not specified.
    echo Usage: CopyCsXaml.bat ".\Controls" ".\result"
    pause
    exit /b 1
)

if "%dst%"=="" (
    echo ERROR: Destination folder not specified.
    echo Usage: CopyCsXaml.bat ".\Controls" ".\result"
    pause
    exit /b 1
)

echo Source: %src%
echo Destination: %dst%
echo.

REM Ensure destination exists
if not exist "%dst%" (
    mkdir "%dst%"
)

REM Copy .cs files
for %%F in ("%src%\*.cs") do (
    echo [+] Copying %%~nxF
    copy /Y "%%F" "%dst%\%%~nxF" >nul
)

REM Copy and rename .xaml to .xaml.txt
for %%F in ("%src%\*.xaml") do (
    echo [+] Renaming %%~nxF to %%~nF.xaml.txt
    copy /Y "%%F" "%dst%\%%~nF.xaml.txt" >nul
)

echo.
echo Done.
pause
