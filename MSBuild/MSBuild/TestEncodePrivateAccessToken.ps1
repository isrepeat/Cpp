# --- Helpers ---
$ThisFileDirectory = Join-Path $(Get-Location) ""   # woraround to save result of Get-Location to string var

function WriteDebug([string] $msg) {
    Write-Host "##[debug] $msg"
}

$PrivateAccessToken = "z3cyxtdaiveissrlsjwfdwiunvmjqr6fnxbtxgialsqj36orl52a"
Write-Host "PrivateAccessToken = ${PrivateAccessToken}"

$Base64Pat1 = [Convert]::ToBase64String([System.Text.Encoding]::UTF8.GetBytes("`:${PrivateAccessToken}"))
Write-Host "$Base64Pat1 = ${$Base64Pat1}"

$Base64Pat2 = [Convert]::ToBase64String([System.Text.Encoding]::UTF8.GetBytes("`:${PrivateAccessToken}"))
Write-Host "$Base64Pat2 = ${$Base64Pat2}"

cmd /c 'pause'
exit

PrivateAccessToken = z3cyxtdaiveissrlsjwfdwiunvmjqr6fnxbtxgialsqj36orl52a
OnozY3l4dGRhaXZlaXNzcmxzandmZHdpdW52bWpxcjZmbnhidHhnaWFsc3FqMzZvcmw1MmE= =
OnozY3l4dGRhaXZlaXNzcmxzandmZHdpdW52bWpxcjZmbnhidHhnaWFsc3FqMzZvcmw1MmE= =