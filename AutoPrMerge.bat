@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

:: Navigate to the current repository directory
CD /D "%~dp0"

:: Check if we are in a Git repository
git rev-parse --is-inside-work-tree >nul 2>&1
IF ERRORLEVEL 1 (
    ECHO "Error: This is not a Git repository."
    PAUSE
    EXIT /B
)

:: Get repository URL (HTTPS only)
FOR /F "tokens=*" %%I IN ('git config --get remote.origin.url') DO SET RepoURL=%%I

:: Parse the owner and repo from the HTTPS URL
:: Example: https://github.com/owner/repo.git
SET RepoURL=%RepoURL:git@github.com=%
SET RepoURL=%RepoURL:https://github.com/=%
SET RepoURL=%RepoURL:.git=%

:: Split into owner and repo
FOR /F "tokens=1,2 delims=/" %%A IN ("%RepoURL%") DO (
    SET Owner=%%A
    SET RepoName=%%B
)

:: Check if values were retrieved
IF NOT DEFINED Owner (
    ECHO "Error: Unable to determine repository owner."
    PAUSE
    EXIT /B
)

IF NOT DEFINED RepoName (
    ECHO "Error: Unable to determine repository name."
    PAUSE
    EXIT /B
)

SET RepoPath=%Owner%/%RepoName%
ECHO "Working in repository: %RepoPath%"


:: Request parameters
SET /P Head=Enter Source Branch (head): 
SET /P Base=Enter Target Branch (base): 
SET /P Title=Enter Pull Request Title: 
::SET /P Body=Enter Pull Request Body: 

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

ECHO "Pull Request #%PR_Number% successfully merged."

:: Delete merged branch
ECHO Deleting source branch %Head%...
gh api repos/%RepoPath%/git/refs/heads/%Head% -X DELETE
IF ERRORLEVEL 1 (
    ECHO "Failed to delete branch %Head%."
    PAUSE
    EXIT /B
)

ECHO "Source branch %Head% successfully deleted."

:: Prune deleted branch from local repository
ECHO Cleaning up local references...
git fetch origin --prune
IF ERRORLEVEL 1 (
    ECHO "Failed to prune deleted branches."
    PAUSE
    EXIT /B
)

ECHO "Local references successfully pruned."
PAUSE
