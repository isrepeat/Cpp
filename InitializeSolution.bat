@echo off
SETLOCAL

:: --- Helpers ---
SET THIS_FILE_DIRECTORY=%~dp0
SET PROJECT_NAME=CppPrivate
SET NEW_LINE=echo.

:: --- Find msbuild exe ---
SET FIND_MSBUILD_SCRIPT="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe

FOR /F "tokens=*" %%g IN ('%FIND_MSBUILD_SCRIPT%') do (SET MSBuildExe="%%g")
echo MSBuild.exe = %MSBuildExe%
%NEW_LINE%

:: --- Confirmation ---
SET /P CONFIRMATION=[%PROJECT_NAME%]: Initialize solution (Y/[N])?
IF /I "%CONFIRMATION%" NEQ "Y" GOTO END


:: --- Initializing ---
echo [%PROJECT_NAME%]: Update git submodule ...
git submodule update --init --recursive
%NEW_LINE%
%NEW_LINE%

echo [%PROJECT_NAME%]: Checking out "UtilityHelpersLib" submodule develop branch ...
cd UtilityHelpersLib
git checkout develop
git pull
%NEW_LINE%
%NEW_LINE%

echo [%PROJECT_NAME%]: Restore "UtilityHelpersLib" nugets ...
%MSBuildExe% UtilityHelpersLib.sln /p:Configuration=Debug /p:Platform="x64" /p:RestorePackagesConfig=true /t:restore
%NEW_LINE%
%NEW_LINE%

echo [%PROJECT_NAME%]: Done
%NEW_LINE%

:END
ENDLOCAL
pause
