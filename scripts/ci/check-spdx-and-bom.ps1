# SPDX & UTF-8 BOM checks (Windows/PowerShell local runner)
$ErrorActionPreference = "Stop"

function Get-RepoRoot { git rev-parse --show-toplevel 2>$null }
$root = Get-RepoRoot
if (-not $root) { Write-Error "Run inside a Git repository."; exit 1 }
Set-Location $root

# Gather tracked files
$files = & git ls-files

$missing = New-Object System.Collections.Generic.List[string]
$bom     = New-Object System.Collections.Generic.List[string]

$exclude = [regex]'(^LICENSE(S)?/)|(\.gitignore$)|(\.gitattributes$)|(.*firebase_cpp_sdk/)|(.*node_modules/)|(.*build/)|(.*dist/)'
$spdx = [regex]'^(c|cc|cxx|cpp|h|hh|hpp|qml|js|ts|java|kt|m|mm|gradle|groovy|kts|sh|bash|ps1|py|rb|pl|pro|pri|yml|yaml|toml|ini|cfg|conf|properties|cmake|xml|qrc|ui|plist|html|htm|md|markdown|bat|cmd)$'
$bomSens = [regex]'^(pro|pri|qml|qrc|ui|gradle|kts|groovy|sh|ps1|bat|cmd|yml|yaml|xml|md|txt)$'

foreach ($f in $files) {
  if ($exclude.IsMatch($f)) { continue }
  # test "textness" by trying to read; if binary, skip
  try {
    $head = Get-Content -LiteralPath $f -TotalCount 5 -Encoding UTF8 -ErrorAction Stop
  } catch {
    continue
  }

  $name = [System.IO.Path]::GetFileName($f)
  $ext = [System.IO.Path]::GetExtension($f).TrimStart('.').ToLowerInvariant()

  if ($name -eq "CMakeLists.txt" -or $spdx.IsMatch($ext)) {
    $headStr = ($head | Out-String)
    if (-not ($headStr -match "SPDX-License-Identifier")) {
      $missing.Add($f) | Out-Null
    }
  }

  if ($bomSens.IsMatch($ext)) {
    $bytes = [System.IO.File]::ReadAllBytes($f)
    if ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF) {
      $bom.Add($f) | Out-Null
    }
  }
}

$rc = 0
if ($missing.Count -gt 0) {
  Write-Host "❌ Files missing SPDX header:"
  $missing | ForEach-Object { Write-Host " - $_" }
  $rc = 1
}
if ($bom.Count -gt 0) {
  Write-Host "❌ Files contain UTF-8 BOM (not allowed):"
  $bom | ForEach-Object { Write-Host " - $_" }
  $rc = 1
}

if ($rc -eq 0) { Write-Host "✅ SPDX & BOM checks passed." }
exit $rc
