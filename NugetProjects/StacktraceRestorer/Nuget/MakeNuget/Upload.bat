@echo off

set PATH=%PATH%;%~dp0..\..\..\..\Tools\

setlocal
:PROMPT
SET /P AREYOUSURE=Upload nuget (Y/[N])?
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END

echo upload nuget ...
nuget push -Source http://nuget.dct.ua/v3/index.json Output\StacktraceRestorer.1.0.13.nupkg

:END
endlocal

pause
