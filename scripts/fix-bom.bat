:: SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
:: Copyright (c) 2025 Kooijman Incorporate Holding B.V.

@echo off
REM Remove UTF-8 BOM from tracked text files
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0\fix-bom.ps1" %*
