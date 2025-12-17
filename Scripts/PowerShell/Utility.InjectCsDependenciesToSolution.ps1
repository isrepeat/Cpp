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

$SourceSolutionPath = '"..\UtilityHelpersLib.sln"'

$TargetSolutionPath = Read-Host "Enter path to the target .sln file"
$TargetSolutionPath = m::WrapInQuotesIfNeeded $TargetSolutionPath
if ([string]::IsNullOrWhiteSpace($TargetSolutionPath) -or $TargetSolutionPath -eq '""') {
    m::MessageError "no path provided"
	exit 1
}

$FoldersToInsertArgs = @(
    '-f', 'CodeAnalyzer',
    '-f', 'HelpersCs'
)

$ProjectsToInsertArgs = @(
)

$SolutionInjectorExtraArgs = @()

#$RootNameOpt = Read-Host "Enter root name (root solution folder for injected items)"
$RootNameOpt = "UtilityHelpersLib [submodule]"
$RootNameOpt = m::WrapInQuotesIfNeeded $RootNameOpt
if (-not [string]::IsNullOrWhiteSpace($RootNameOpt) -and
	$RootNameOpt -ne '""'
) {
	$SolutionInjectorExtraArgs += @('--rootName', $RootNameOpt)
}

$SolutionConfigsToRemoveArgs = @(
    "-rmCfg", "Debug|ARM",
	"-rmCfg", "Debug|ARM64",
	"-rmCfg", "Release|ARM",
	"-rmCfg", "Release|ARM64"
)
$SolutionInjectorExtraArgs += $SolutionConfigsToRemoveArgs

# --- Show args ---
m::Message -color Blue -Text "Source solution: $SourceSolutionPath"
m::Message -color Blue -Text "Target solution: $TargetSolutionPath"
m::Message -color Blue -Text "Folders to insert: $($FoldersToInsertArgs -join ' ')"
m::Message -color Blue -Text "Projects to insert: $($ProjectsToInsertArgs -join ' ')"
m::Message -color Blue -Text "Extra args: $($SolutionInjectorExtraArgs -join ' ')"

# --- Run tool ---
m::MessageAction "Launching `"$solutionInjectorExe`" ..."
& "$solutionInjectorExe" `
    $SourceSolutionPath `
    $TargetSolutionPath `
    $FoldersToInsertArgs `
    $ProjectsToInsertArgs `
    $SolutionInjectorExtraArgs