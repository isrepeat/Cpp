# Ќазначаем рабочим каталогом текущий каталог скрипта 
# (все относительные пути далее будут относительно этого каталога).
$callerLocation = Get-Location
Set-Location -Path $PSScriptRoot

# ƒобавл€ем в PSModulePath путь к кастомным модул€м,
# чтобы подключить их по названию модул€ (вместо абсолютного пути).
$modulePath = Join-Path $PSScriptRoot "Modules"
if (-not ($env:PSModulePath -split ';' | Where-Object { $_ -eq $modulePath })) {
    $env:PSModulePath = "$modulePath;$env:PSModulePath"
}

# --- Import ---
Import-Module -Name MessagingModule -Prefix m:: -ErrorAction Stop

# --- Main ---
$solutionInjectorExe = Join-Path $PSScriptRoot "..\..\Utilities\bin\SolutionInjector.exe"
$solutionInjectorExe = (Resolve-Path $solutionInjectorExe -ErrorAction Stop).Path

if (-not (Test-Path $solutionInjectorExe)) {
    m::MessageError "Executable not found: $solutionInjectorExe"
    exit 1
}

# Ќазначаем рабочим каталогом каталог caller'а чтобы все относительные пути 
# далее в скрипте и в solutionInjectorExe были относительна caller'а.
Set-Location -Path $callerLocation

$sourceSolutionPath = '"..\UtilityHelpersLib.sln"'

$targetSolutionPath = Read-Host "Enter path to the target .sln file"
$targetSolutionPath = m::WrapInQuotesIfNeeded $targetSolutionPath
if ([string]::IsNullOrWhiteSpace($targetSolutionPath) -or $targetSolutionPath -eq '""') {
    m::MessageError "no path provided"
	exit 1
}

# --- Show args ---
m::Message -color Blue -Text "Target solution: $targetSolutionPath"

# --- Run tool ---
m::MessageAction "Launching `"$solutionInjectorExe`" ..."
& "$solutionInjectorExe" `
	$sourceSolutionPath `
	$targetSolutionPath `
	"--normalize"