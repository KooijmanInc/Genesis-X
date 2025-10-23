# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

param(
  [string[]]$Remote = @("origin","gitlab"),
  [string]$Base = "staging",
  [string]$Prefix = "^chore/",
  [switch]$Delete,
  [switch]$Help
)

if ($Help) {
@"
Usage: cleanup-merged-branches.ps1 [-Remote origin,gitlab] [-Base staging] [-Prefix '^chore/'] [-Delete]
"@ | Write-Host
  exit 0
}

$root = (& git rev-parse --show-toplevel 2>$null)
if (-not $root) { Write-Error "Run inside a Git repository."; exit 1 }
Set-Location $root

$protected = @("main","master","staging",$Base)
$configuredRemotes = (& git remote) | ForEach-Object { $_.Trim() }

function Cleanup-Remote([string]$r) {
  Write-Host "== Remote: $r =="
  if (-not ($configuredRemotes -contains $r)) {
    Write-Host "Skipping: remote not configured in this repo."
    return
  }

  git fetch $r --prune | Out-Null

  $merged = (& git branch -r --merged "$r/$Base") -replace '^[ *]*','' | Where-Object { $_ -like "$r/*" }

  $targets = @()
  foreach ($line in $merged) {
    if (-not $line) { continue }
    $name = $line.Substring($r.Length + 1) # strip 'remote/'
    if ($name -notmatch $Prefix) { continue }
    if ($protected -contains $name) { continue }
    $targets += $name
  }

  if (-not $targets -or $targets.Count -eq 0) {
    Write-Host "No candidates on ${r} (merged into $Base & matching $Prefix)."
    return
  }

  Write-Host "Candidates on ${r}:"
  $targets | ForEach-Object { Write-Host "  - $_" }

  if ($Delete) {
    foreach ($b in $targets) {
      Write-Host "Deleting ${r}/$b"
      git push $r --delete $b | Out-Null
    }
  } else {
    Write-Host "(dry-run) Pass -Delete to remove them."
  }
}

$Remote | ForEach-Object { Cleanup-Remote $_ }
