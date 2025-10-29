# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

# Add SPDX + copyright headers to tracked files (Windows/PowerShell)
# Writes files as UTF-8 *without* BOM (qmake-safe) and skips missing paths.
$ErrorActionPreference = "Stop"

function Get-RepoRoot { git rev-parse --show-toplevel 2>$null }

$root = Get-RepoRoot
if (-not $root) { Write-Error "Run inside a Git repository."; exit 1 }
Set-Location $root

$COPYRIGHT_OWNER = $env:COPYRIGHT_OWNER; if (-not $COPYRIGHT_OWNER) { $COPYRIGHT_OWNER = "Kooijman Incorporate Holding B.V." }
$COPYRIGHT_YEAR  = $env:COPYRIGHT_YEAR;  if (-not $COPYRIGHT_YEAR)  { $COPYRIGHT_YEAR  = "2025" }
$LICENSE_EXPR    = $env:LICENSE_EXPR;    if (-not $LICENSE_EXPR)    { $LICENSE_EXPR    = "(LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)" }

$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)

function Comment-Style($path) {
  $name = [System.IO.Path]::GetFileName($path)
  $ext  = [System.IO.Path]::GetExtension($path).TrimStart('.').ToLowerInvariant()
  if ($name -eq "CMakeLists.txt") { return "hash" }
  switch -Regex ($ext) {
    '^(c|cc|cxx|cpp|h|hh|hpp|qml|js|ts|java|kt|m|mm|gradle|groovy|kts)$' { return "slash" }
    '^(sh|bash|ps1|py|rb|pl|pro|pri|yml|yaml|toml|ini|cfg|conf|properties|cmake)$' { return "hash" }
    '^(bat|cmd)$' { return "bat" }
    '^(xml|qrc|ui|plist|html|htm|md|markdown)$' { return "angle" }
    '^(json|jpg|jpeg|png|gif|svg|ico|pdf|zip|gz|7z|tar|rar|a|so|dll|dylib|bin)$' { return "none" }
    default { return "none" }
  }
}

function Make-Header($style) {
  switch ($style) {
    "slash" { return "// SPDX-License-Identifier: $LICENSE_EXPR`n// Copyright (c) $COPYRIGHT_YEAR $COPYRIGHT_OWNER`n`n" }
    "hash"  { return "# SPDX-License-Identifier: $LICENSE_EXPR`n# Copyright (c) $COPYRIGHT_YEAR $COPYRIGHT_OWNER`n`n" }
    "bat"   { return ":: SPDX-License-Identifier: $LICENSE_EXPR`n:: Copyright (c) $COPYRIGHT_YEAR $COPYRIGHT_OWNER`n`n" }
    "angle" { return "<!-- SPDX-License-Identifier: $LICENSE_EXPR -->`n<!-- Copyright (c) $COPYRIGHT_YEAR $COPYRIGHT_OWNER -->`n`n" }
  }
}

# Gather tracked files, excluding some
$files = & git ls-files -- `
  ':!LICENSE' ':!LICENSE.*' ':!NOTICE' ':!NOTICE.*' ':!docs' ':!docs.*' `
  ':!LICENSES/**' ':!**/*.json' ':!.gitignore' ':!.gitattributes' `
  ':!**/firebase_cpp_sdk/**' ':!**/node_modules/**' ':!**/build/**' ':!**/dist/**'

# Skip paths that no longer exist (deleted/renamed but not committed)
$files = $files | Where-Object { Test-Path -LiteralPath $_ }

foreach ($f in $files) {
  # Extra safety: skip if file disappeared between listing and processing
  if (-not (Test-Path -LiteralPath $f)) { continue }

  $style = Comment-Style $f
  if ($style -eq "none") { continue }

  # Skip if header already present
  $head = Get-Content -LiteralPath $f -TotalCount 5 -ErrorAction SilentlyContinue | Out-String
  if ($head -match "SPDX-License-Identifier") { continue }

  $header = Make-Header $style
  $contentBytes = [System.IO.File]::ReadAllBytes($f)
  # Detect UTF-8 BOM
  $hasBom = ($contentBytes.Length -ge 3 -and $contentBytes[0] -eq 0xEF -and $contentBytes[1] -eq 0xBB -and $contentBytes[2] -eq 0xBF)
  if ($hasBom) {
    $content = [System.Text.Encoding]::UTF8.GetString($contentBytes, 3, $contentBytes.Length - 3)
  } else {
    $content = [System.Text.Encoding]::UTF8.GetString($contentBytes)
  }

  if ($content.StartsWith("#!")) {
    $idx = $content.IndexOf("`n")
    if ($idx -ge 0) {
      $first = $content.Substring(0, $idx).TrimEnd("`r","`n")
      $rest  = $content.Substring($idx+1)
      [System.IO.File]::WriteAllText($f, ($first + "`n" + $header + $rest), $Utf8NoBom)
    } else {
      [System.IO.File]::WriteAllText($f, ($content + "`n" + $header), $Utf8NoBom)
    }
  } else {
    [System.IO.File]::WriteAllText($f, ($header + $content), $Utf8NoBom)
  }
  Write-Host "Header added: $f"
}

Write-Host "Done. Override: COPYRIGHT_OWNER, COPYRIGHT_YEAR, LICENSE_EXPR"
