:: SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
:: Copyright (c) 2025 Kooijman Incorporate Holding B.V.

@echo off

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0\check-spdx-and-bom.ps1" %*
