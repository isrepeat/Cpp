param (
    [Parameter(Mandatory = $true)]
    [string]$ProjectPath
)

Write-Host "Temporarily modifying '$ProjectPath'..."

if (-not (Test-Path $ProjectPath)) {
    Write-Host "File not found: $ProjectPath" -ForegroundColor Red
    exit 1
}

$content = Get-Content $ProjectPath -Raw
$contentWithMarker = "$content`n<!-- Force reload -->"

# Сохраняем с BOM вручную
$bom = [System.Text.Encoding]::UTF8.GetPreamble()
$data = [System.Text.Encoding]::UTF8.GetBytes($contentWithMarker)
[System.IO.File]::WriteAllBytes($ProjectPath, $bom + $data)

Start-Sleep -Milliseconds 300

# Удаляем маркер
$restoredContent = $contentWithMarker -replace '(\r?\n)?<!-- Force reload -->$', ''
$dataRestored = [System.Text.Encoding]::UTF8.GetBytes($restoredContent)
[System.IO.File]::WriteAllBytes($ProjectPath, $bom + $dataRestored)

Write-Host "Done." -ForegroundColor Green