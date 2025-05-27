param (
    [Parameter(Mandatory = $true)]
    [string]$ProjectPath
)

Write-Host "Temporarily modifying '$ProjectPath'..."

if (-not (Test-Path $ProjectPath)) {
    Write-Host "File not found: $ProjectPath" -ForegroundColor Red
    exit 1
}

$lines = Get-Content $ProjectPath
$lines += '<!-- Force reload -->'
Set-Content $ProjectPath $lines

Start-Sleep -Milliseconds 300

$cleanLines = Get-Content $ProjectPath | Where-Object { $_ -notmatch 'Force reload' }
Set-Content $ProjectPath $cleanLines

Write-Host "Done." -ForegroundColor Green
