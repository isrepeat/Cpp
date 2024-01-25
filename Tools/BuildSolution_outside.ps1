# --- Helpers ---
$ThisFileDirectory = Join-Path $(Get-Location) ""   # woraround to save result of Get-Location to string var
$SolutionName = "TeamRemoteDesktop"
$SourcesFolder = "D:\WORK\Workspaces\${SolutionName}"
$BuildSolution = "${SourcesFolder}\${SolutionName}.sln"
$BuildConfiguration = "Release"
$BuildAppTarget = "Client\TeamRemoteDesktop"
$BuildPackageTarget = "Client\Package\TeamRemoteDesktopPackage"
$QtMsBuildDir = "${SourcesFolder}\QtMsBuild_Local" # C:\Users\isrep\AppData\Local\QtMsBuild
$PublishPath = "${SourcesFolder}\Client\TeamRemoteDesktopPackage\AppPackages"
$AppxBuildMode = "StoreUpload"
$AppxBundle = "Never"

$QtVersion = "6.4.3"
$QtInstallDir = "D:\Qt\${QtVersion}\msvc2019_64"

function WriteTitle([string] $msg) {
    Write-Host "[$SolutionName]: $msg" -ForegroundColor DarkGray
}
function WriteDone([string] $msg) {
    Write-Host "[$SolutionName]: $msg" -ForegroundColor Green
}
function WriteOut([string] $msg) {
    Write-Host "[$SolutionName]: $msg"
}
function NewLine() {
    Write-Host ""
}

function WriteDebug([string] $msg) {
    Write-Host "##[debug] $msg"
}


$ThirdPartyTestExtractedDir = "${SourcesFolder}\3rdParty\VCruntime"
$ThirdPartyExtracted = Test-Path -Path ${ThirdPartyTestExtractedDir}

Write-Host "Check if 3rdParty.7z extracted"
#if (-not $ThirdPartyExtracted) {
if ((Test-Path -Path ${ThirdPartyTestExtractedDir})) {
    Write-Host "##[debug] Extracted"
} else {
    Write-Host "##[warning] 3rdParty not fully extracted, please run `"Initialize solution task`" once"
}



WriteOut "SourcesFolder = ${SourcesFolder}"
WriteOut "SolutionName = ${SolutionName}"
WriteOut "BuildSolution = ${BuildSolution}"
WriteOut "BuildAppTarget = ${BuildAppTarget}"
WriteOut "QtMsBuild_Local = ${QtMsBuild_Local}"
WriteOut "BuildPackageTarget = ${BuildPackageTarget}"
NewLine
WriteOut "QtVersion = ${QtVersion}"
WriteOut "QtInstallDir = ${QtInstallDir}"

# First build app to generate neccessary files that will be included in package
msbuild ${BuildSolution} /t:${BuildAppTarget}  /p:Configuration=${BuildConfiguration}  /p:Platform=x64  /p:PlatformToolSet=v143   /p:QtMsBuild=${QtMsBuildDir}  /p:QtVersion_p=${QtVersion}  /p:QtInstallDir_p=${QtInstallDir}  /p:RemoveGeneratedFiles_p=true  /p:AppxPackageDir=${PublishPath}

# Build/Publish package
msbuild ${BuildSolution} /t:${BuildPackageTarget}  /p:Configuration=${BuildConfiguration}  /p:Platform=x64  /p:PlatformToolSet=v143   /p:QtMsBuild=${QtMsBuildDir}  /p:QtVersion_p=${QtVersion}  /p:QtInstallDir_p=${QtInstallDir}  /p:RemoveGeneratedFiles_p=true  /p:AppxPackageDir=${PublishPath}  /p:UapAppxPackageBuildMode=${AppxBuildMode}  /p:AppxBundle=${AppxBundle}


## Find .cer file and extract version number from name
$packageCert = @(Get-ChildItem -r *.cer -Path ${PublishPath})[0].Name

WriteOut "packageCert = $packageCert"

## Update pipeline build number ...
$hasVersion = $packageCert -match '(?<=_)\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}(?=_)'
if ($hasVersion) {
  $version = $matches[0]
  WriteOut "version = $version"
}

WriteOut "end"

cmd /c 'pause'
exit