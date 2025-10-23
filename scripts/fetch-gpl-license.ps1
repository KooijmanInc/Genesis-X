# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

# Fetch GPL-3.0-only full license text into LICENSES/GPL-3.0-only.txt
$ErrorActionPreference = "Stop"
$dest = Join-Path -Path (Get-Location) -ChildPath "LICENSES"
if (-not (Test-Path $dest)) { New-Item -ItemType Directory -Path $dest | Out-Null }
$uri = "https://www.gnu.org/licenses/gpl-3.0.txt"
$target = Join-Path $dest "GPL-3.0-only.txt"
Invoke-WebRequest -Uri $uri -OutFile $target -UseBasicParsing
Write-Host "Downloaded GPL-3.0-only license text to $target"
