@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

:: Request parameters
SET /P Head=Enter Source Branch (head): 
SET /P Base=Enter Target Branch (base): 
SET /P Title=Enter Pull Request Title: 
:: SET /P Body=Enter Pull Request Description: 

:: Check if GitHub CLI is installed
gh --version >nul 2>&1
IF ERRORLEVEL 1 (
    ECHO "GitHub CLI (gh) is not installed."
    PAUSE
    EXIT /B
)

:: Create Pull Request
ECHO Creating Pull Request...
gh pr create --title "%Title%" --body "%Body%" --base "%Base%" --head "%Head%"
IF ERRORLEVEL 1 (
    ECHO "Failed to create Pull Request."
    PAUSE
    EXIT /B
)

:: Get Pull Request number
FOR /F "delims=" %%I IN ('gh pr list --state open --head "%Head%" --json number --jq ".[0].number"') DO SET PR_Number=%%I

:: Check if Pull Request number was retrieved
IF NOT DEFINED PR_Number (
    ECHO "Error: Unable to find Pull Request number."
    PAUSE
    EXIT /B
)

ECHO Pull Request created with number #%PR_Number%

:: Merge Pull Request with custom message
ECHO Merging Pull Request...
gh pr merge %PR_Number% --merge --subject "Merge PR #%PR_Number% %Title%"

IF ERRORLEVEL 1 (
    ECHO "Failed to merge Pull Request."
    PAUSE
    EXIT /B
)

ECHO Pull Request #%PR_Number% successfully merged.
PAUSE