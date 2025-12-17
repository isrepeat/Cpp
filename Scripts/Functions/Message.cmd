@ECHO off
:: --- Colors --- 
:: Sets the ESC character (https://stackoverflow.com/questions/2048509/how-to-echo-with-different-colors-in-the-windows-command-line)
FOR /F %%a in ('ECHO prompt $E ^| cmd') do SET "ESC=%%a"

:: --- Global variables ---
SET NEW_LINE=ECHO.

SETLOCAL
:: Print script args
::FOR %%i in (%*) do ECHO arg = %%i
::%NEW_LINE%

CALL:Message %*

ENDLOCAL
GOTO :EOF


:: --- Functions ---
:Message 
:: [flag Prefix] [Color] Msg
    SETLOCAL ENABLEDELAYEDEXPANSION
        ::FOR %%i in (%*) do ECHO Message_arg = %%i
        ::%NEW_LINE%

        SET argMsg=
        SET argColor=0m
        SET argPrefix=
        
        :Message_ArgsParsing
        IF "%~1" NEQ "" (
            IF "%~1" == "-s" (
                SET argPrefix=[%SOLUTION_NAME%]: 
            ) ELSE IF "%~1" == "-c" (
                SET argColor=%~2
                SHIFT
            ) ELSE IF "%~1" NEQ "" (
                :: Check again at last, if it's not empty use it as msg with keeping quotes
                SET argMsg=%1
            )
            SHIFT
            GOTO :Message_ArgsParsing
        )

        :: Handle parsed args
        SET paramMsg=%argMsg%
        SET paramColor=%argColor%
        SET paramPrefix=%argPrefix%

        IF "%argColor%" == "Red" (
            SET paramColor=91m
        ) ELSE IF "%argColor%" == "Green" (
            SET paramColor=92m
        ) ELSE IF "%argColor%" == "Blue" (
            SET paramColor=94m
        ) ELSE IF "%argColor%" == "Yellow" (
            SET paramColor=93m
        )

        ECHO %ESC%[%paramColor%%paramPrefix%%paramMsg%%ESC%[0m 
    ENDLOCAL
EXIT /B 0