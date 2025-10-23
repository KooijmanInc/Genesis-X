@echo off
REM SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
REM Copyright (c) 2025 Kooijman Incorporate Holding B.V.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0\cleanup-merged-branches.ps1" %*
