@ECHO off
:: --- Global variables ---
SET NEW_LINE=ECHO.

SETLOCAL
:: Print script args
::for %%i in (%*) do ECHO arg = %%i
::%NEW_LINE%

:: --- Find msbuild exe ---
SET FIND_MSBUILD_SCRIPT="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe

FOR /F "tokens=*" %%g in ('%FIND_MSBUILD_SCRIPT%') do (SET MSBuildExe='%%g')
CALL Message.cmd "MSBuild.exe = '%MSBuildExe:'=%'"

CALL:FUNC_BuildProject %*

ENDLOCAL
GOTO :EOF


:: --- Functions ---
:FUNC_BuildProject 
:: SolutionTarget Configuration Platform
    SETLOCAL ENABLEDELAYEDEXPANSION
        SET functionName=FUNC_BuildProject

        CALL Message.cmd -c Yellow "%functionName%() [Enter]"
        SET paramSolutionFile=%~1
        SET paramSolutionTarget=%~2
        SET paramConfiguration=%~3
        SET paramPlatform=%~4
        CALL Message.cmd "paramSolutionFile = %paramSolutionFile%"
        CALL Message.cmd "paramSolutionTarget = %paramSolutionTarget%"
        CALL Message.cmd "paramConfiguration = %paramConfiguration%"
        CALL Message.cmd "paramPlatform = %paramPlatform%"
        %NEW_LINE%
        
        CALL Message.cmd "Build start '%paramSolutionTarget%' [%paramConfiguration% | %paramPlatform%] ..."
        %NEW_LINE%

        :: NOTE: replace "." to "_"
        %MSBuildExe:'="% "%paramSolutionFile%" ^
            /t:"%paramSolutionTarget:.=_%" ^
            /p:Configuration="%paramConfiguration%" ^
            /p:Platform="%paramPlatform%" ^
            -verbosity:minimal
        
        %NEW_LINE%
        CALL Message.cmd -c Green "Build done '%paramSolutionTarget%' [%paramConfiguration% | %paramPlatform%]"
        CALL Message.cmd -c Yellow "%functionName%() [Exit]"
    
        %NEW_LINE%
        %NEW_LINE%
    ENDLOCAL
EXIT /B 0


:: NOTE: If you need string variable with quotes use '' quotes. 
::       For example you SET someQuotedStr="Hello world":
::       - to unquote it use %someQuotedStr:'=% (to use it inside another string)
::       - to replace it with " use %someQuotedStr:'="%