:: SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
:: Copyright (c) 2025 Kooijman Incorporate Holding B.V.

@echo off
setlocal

REM --- Resolve source (folder with wizard.json & icon.png) ---
set "SRC=%~dp0"
rem if not exist "%SRC%wizard.json" (
rem   if not "%~1"=="" (
rem     set "SRC=%~1"
rem   )
rem )

rem if not exist "%SRC%wizard.json" (
rem   echo [ERROR] wizard.json not found in:
rem   echo         %SRC%
rem   echo Place this script INSIDE the wizard folder or pass the folder path as the first argument.
rem   exit /b 2
rem )

REM --- Resolve user wizard target ---
set "USER_WIZ=%APPDATA%\QtProject\qtcreator\templates\wizards\"
if not exist "%USER_WIZ%" mkdir "%USER_WIZ%" >nul 2>&1

REM --- Install only if missing ---
echo [INFO] Installing wizard to:
echo        %USER_WIZ%

REM Fallback to xcopy with explicit wildcard
rem xcopy /S /I /Y "%SRC%\icon.png" "%USER_WIZ%\\" >nul
xcopy /S /I /Y "%SRC%\projects\*" "%USER_WIZ%\\" >nul

rem if not exist "%USER_WIZ%\wizard.json" (
rem   echo [ERROR] Copy failed; wizard.json still missing.
rem   exit /b 3
rem )
echo [OK] Wizard installed.


REM --- Find Qt Creator ---
set "QTC=%QT_CREATOR_EXE%"

if not exist "%QTC%" set "QTC=C:\Qt\Tools\QtCreator\bin\qtcreator.exe"
if not exist "%QTC%" set "QTC=C:\Program Files\Qt Creator\bin\qtcreator.exe"
if not exist "%QTC%" set "QTC=C:\Program Files (x86)\Qt Creator\bin\qtcreator.exe"
if not exist "%QTC%" set "QTC=D:\Qt\Tools\QtCreator\bin\qtcreator.exe"
if not exist "%QTC%" set "QTC=D:\Program Files\Qt Creator\bin\qtcreator.exe"
if not exist "%QTC%" set "QTC=D:\Program Files (x86)\Qt Creator\bin\qtcreator.exe"
if not exist "%QTC%" set "QTC=E:\Qt\Tools\QtCreator\bin\qtcreator.exe"
if not exist "%QTC%" set "QTC=E:\Program Files\Qt Creator\bin\qtcreator.exe"
if not exist "%QTC%" set "QTC=E:\Program Files (x86)\Qt Creator\bin\qtcreator.exe"

if not exist "%QTC%" (
  for %%I in (qtcreator.exe) do if not "%%~$PATH:I"=="" set "QTC=%%~$PATH:I"
)

if exist "%QTC%" (
  echo [RUN] "%QTC%"
  start "" "%QTC%"
) else (
  echo [WARN] Could not find Qt Creator automatically.
  echo        Set QT_CREATOR_EXE to qtcreator.exe and rerun, or start it manually.
)

endlocal
