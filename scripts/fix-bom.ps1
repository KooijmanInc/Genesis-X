# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

# Remove UTF-8 BOM from tracked text files (qmake-safe)
$ErrorActionPreference = "Stop"

function Get-RepoRoot { git rev-parse --show-toplevel 2>$null }
$root = Get-RepoRoot
if (-not $root) { Write-Error "Run inside a Git repository."; exit 1 }
Set-Location $root

$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)

# File patterns to check (add more if needed)
$patterns = @("*.pro","*.pri","*.qml","*.qrc","*.ui","*.gradle","*.kts","*.groovy","*.sh","*.ps1","*.bat","*.cmd","*.yml","*.yaml","*.xml","*.md","*.txt","*.cpp","*.h","*.hpp")

# Get tracked files matching patterns
$files = @()
foreach ($pat in $patterns) {
  $files += (& git ls-files -- $pat)
}

foreach ($f in $files | Sort-Object -Unique) {
  $bytes = [System.IO.File]::ReadAllBytes($f)
  if ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF) {
    $text = [System.Text.Encoding]::UTF8.GetString($bytes, 3, $bytes.Length - 3)
    [System.IO.File]::WriteAllText($f, $text, $Utf8NoBom)
    Write-Host "BOM removed: $f"
  }
}
Write-Host "Done."
