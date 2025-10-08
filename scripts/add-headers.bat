:: SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
:: Copyright (c) 2025 Kooijman Incorporate Holding B.V.

@echo off
REM Add SPDX + copyright headers on Windows via PowerShell
REM Optional env overrides before running:
REM   set COPYRIGHT_OWNER=Kooijman Incorporate Holding B.V.
REM   set COPYRIGHT_YEAR=2025
REM   set LICENSE_EXPR=(LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0\add-headers.ps1" %*
