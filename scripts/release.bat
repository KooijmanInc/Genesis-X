:: SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
:: Copyright (c) 2025 Kooijman Incorporate Holding B.V.

@echo off
set ver=%1
set msg=%2
if "%ver%"=="" goto :usage
if "%msg%"=="" goto :usage
git diff --quiet || ( echo Working tree not clean & exit /b 1 )
git fetch --tags
git tag -a %ver% -m "%msg%"
git push origin %ver%
echo Tag %ver% pushed to origin; mirror will sync to GitHub.
goto :eof
:usage
echo usage: scripts\release.bat vX.Y.Z "message"
exit /b 1

