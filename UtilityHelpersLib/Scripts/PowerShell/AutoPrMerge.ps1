param(
    [string]$BatDir = $null
)

# Назначаем рабочим каталогом текущий каталог скрипта
# (все относительные пути далее будут относительно этого каталога).
$callerLocation = Get-Location
Set-Location -Path $PSScriptRoot

# Добавляем в PSModulePath путь к кастомным модулям,
# чтобы подключить их по названию модуля (вместо абсолютного пути).
$modulePath = Join-Path $PSScriptRoot "Modules"
if (-not ($env:PSModulePath -split ';' | Where-Object { $_ -eq $modulePath })) {
    $env:PSModulePath = "$modulePath;$env:PSModulePath"
}

# --- Import ---
Import-Module -Name MessagingModule -Prefix m:: -ErrorAction Stop


# --- Определяем репозиторий, для которого запускался .bat ---
if ($BatDir) {
	# Нормализуем BatDir, убираем случайные кавычки, пробелы по краям.
	$BatDir = $BatDir.Trim().Trim('"')
    try {
        $workDir = (Resolve-Path -LiteralPath $BatDir).Path
    } catch {
        m::MessageError "Incorrect path BatDir: '$BatDir'"
        exit 1
    }
} else {
	# Иначе берём рабочий каталог, откуда запустили .ps1.
    $workDir = $callerLocation.Path
}

# Определяем корень git-репозитория для выбранного каталога.
$gitRoot = (& git -C "$workDir" rev-parse --show-toplevel 2>$null).Trim()
if (-not $gitRoot) {
    m::MessageError "Not a Git repository at: $workDir"
    exit 1
}

# Переходим в корень репозитория; указываем gh, с каким репо работать.
Push-Location $gitRoot
$env:GH_REPO = (& git config --get remote.origin.url 2>$null).Trim()

try {
    m::Message "Working repo: $gitRoot"
    if ($env:GH_REPO) {
		m::Message "Remote: $env:GH_REPO"
	}
	m::NewLine

	<#
	m::MessageAction "Detecting gh..."
	if (-not (Get-Command gh -ErrorAction SilentlyContinue)) {
		m::MessageError "gh not found in PATH"
		exit 1
	}
	m::Message "PS version: $($PSVersionTable.PSVersion)"
	m::Message "gh path: $((Get-Command gh).Source)"
	m::Message "gh version: $(gh --version)"
	#>


    # --- Проверка, что мы внутри репозитория ---
    git rev-parse --is-inside-work-tree 2>$null | Out-Null
    if ($LASTEXITCODE -ne 0) {
		m::MessageError "This is not a Git repository."
        exit 1
    }


	# --- Ввод ---
	$HeadRaw = Read-Host "Enter Source Branch (head)"
	$BaseRaw = Read-Host "Enter Target Branch (base)"
	$Title = Read-Host "Enter Pull Request Title"
	$NewBranchRaw = Read-Host "Enter optional new branch name (leave empty to skip)"
	$Body = "Auto-generated PR from script"

    # Нормализация ввода
    $Head = if ([string]::IsNullOrWhiteSpace($HeadRaw)) { $null } else { $HeadRaw.Trim() }
    $Base = if ([string]::IsNullOrWhiteSpace($BaseRaw)) { $null } else { $BaseRaw.Trim() }
    $NewBranch = if ([string]::IsNullOrWhiteSpace($NewBranchRaw)) { $null } else { $NewBranchRaw.Trim() }

	if ([string]::IsNullOrWhiteSpace($Head)) { 
		m::MessageError "Head branch is required."
		exit 1
	}
	if ([string]::IsNullOrWhiteSpace($Base)) {
		m::MessageError "Base branch is required."
		exit 1
	}
	if ([string]::IsNullOrWhiteSpace($Title)) {
		m::MessageError "PR Title is required."
		exit 1
	}


	# --- Push head ---
	m::MessageAction "Pushing '$Head' to origin..."
	git push origin "$Head"
	if ($LASTEXITCODE -ne 0) { 
		m::MessageError "Failed to push '$Head' to origin."
		exit 1
	}
	m::Message "Pushed."


	# --- Create PR ---
	m::MessageAction "Creating Pull Request..."
	$PR_Number = $null

	# 1) создаем pr и сохраняем вывод
	$prCreationOutput = gh pr create `
		--title "$Title" `
		--body "$Body" `
		--base "$Base" `
		--head "$Head" `
		2>&1
	if ($LASTEXITCODE -ne 0) {
		m::MessageError "Failed to create Pull Request.`n$prCreationOutput"
		exit 1
	}

	# 2) пробуем вытащить номер прямо из выведенного URL: .../pull/123
	if ($prCreationOutput -match '/pull/(\d+)\b') {
		$PR_Number = $matches[1]
	}

	# 3) Если всё ещё нет номера — даём запрос на list (иногда PR появляется с задержкой)
	if (-not $PR_Number) {
		$attempts = 5
		for ($i = 1; $i -le $attempts -and -not $PR_Number; $i++) {
			m::MessageAction "Request pr number..."
			Start-Sleep -Milliseconds 400
			$PR_Number = gh pr list `
				--state open `
				--head "$Head" `
				--json number `
				--jq ".[0].number" `
				2>$null
		}
	}

	if ([string]::IsNullOrWhiteSpace($PR_Number)) {
		m::MessageError "Unable to obtain Pull Request number after creation."
		exit 1
	}
	m::Message "Pull Request created: #$PR_Number"


	# --- Merge PR ---
	m::MessageAction "Merging PR #$PR_Number..."
	gh pr merge $PR_Number --merge --subject "Merge PR #$PR_Number $Title" #| Out-Null
	if ($LASTEXITCODE -ne 0) { 
		m::MessageError "Failed to merge Pull Request."
		exit 1
	}
	m::Message "Pull Request #$PR_Number successfully merged."


	# --- Resolve repo path ---
	$RepoPath = gh repo view --json nameWithOwner --jq ".nameWithOwner"
	if ([string]::IsNullOrWhiteSpace($RepoPath)) { 
		m::MessageError "Unable to resolve repo path (nameWithOwner)."
		exit 1
	}


	# --- Delete source branch on remote ---
	m::MessageAction "Deleting remote branch '$Head'..."
	gh api "repos/$RepoPath/git/refs/heads/$Head" -X DELETE #| Out-Null
	if ($LASTEXITCODE -ne 0) { 
		m::MessageError "Failed to delete remote branch '$Head'."
		exit 1
	}
	m::Message "Remote branch '$Head' deleted."


	# --- Prune locals ---
	m::MessageAction "Pruning local references..."
	git fetch origin --prune
	if ($LASTEXITCODE -ne 0) { 
		m::MessageError "Failed to prune local references."
		exit 1
	}
	m::Message "Local references pruned."


	# --- Optional: create new branch logic (handles empty/same-name cases) ---
	$CurrentBranch = (git branch --show-current).Trim()
	$shouldKeepHead =
		(-not $NewBranch) -or
		($NewBranch -ieq $Head) -or
		($NewBranch -ieq $CurrentBranch)

	if ($shouldKeepHead) {
		m::MessageAction "Keeping the same branch name '$Head'. Rebasing it onto 'origin/$Base'..."

		# Обновим remote ссылки
		git fetch origin #| Out-Null

		# Переключимся на Head, если внезапно не на ней
		$current = (git branch --show-current).Trim()
		if ($current -ne $Head) {
			git checkout "$Head"
			if ($LASTEXITCODE -ne 0) {
				m::MessageError "Failed to checkout '$Head'."
				exit 1
			}
		}

		# Rebase на свежую origin/Base
		git rebase "origin/$Base"
		if ($LASTEXITCODE -ne 0) {
			m::MessageError "Rebase failed. Resolve conflicts and run 'git rebase --continue' (or '--abort')."
			exit 1
		}

		m::Message "Done. '$Head' is now rebased onto 'origin/$Base'."
	}
	else {
		# --- обычный сценарий: создаём новую ветку от origin/Base ---
		m::MessageAction "Refreshing 'origin/$Base'..."
		git fetch origin #| Out-Null

		m::MessageAction "Creating new branch '$NewBranch' from 'origin/$Base'..."
		git checkout --no-track -b "$NewBranch" "origin/$Base"
		if ($LASTEXITCODE -ne 0) {
			m::MessageError "Failed to create '$NewBranch' from origin/$Base."
			exit 1
		}

		# После переключения можно удалить локальную Head
		$isHeadExists = git branch --list "$Head"
		if ($isHeadExists) {
			m::MessageAction "Deleting local branch '$Head'..."
			git branch -D "$Head"
			if ($LASTEXITCODE -ne 0) { 
				m::MessageError "Failed to delete local branch '$Head'."
				exit 1
			}
			m::Message "Local branch '$Head' deleted."
		}

		m::Message "Done. New branch '$NewBranch' is created from 'origin/$Base'."
	}
}
finally {
    Pop-Location
}