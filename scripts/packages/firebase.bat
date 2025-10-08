:: SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
:: Copyright (c) 2025 Kooijman Incorporate Holding B.V.

@echo off
setlocal EnableExtensions EnableDelayedExpansion

REM Requirements: PowerShell 5+ (Invoke-WebRequest, Expand-Archive)
REM Optional: for checksum, PowerShell's Get-FileHash

set ROOT=%~dp0..\..
set DEST=%ROOT%\3rdparty\firebase_cpp_sdk
set DEPS=%ROOT%\config\deps.json

if exist "%DEST%" (
  echo ✅ firebase_cpp_sdk already present at: %DEST%
  exit /b 0
)

for /f "delims=" %%v in ('powershell -NoP -C "(Get-Content '%DEPS%'|Out-String|ConvertFrom-Json).firebase.version"') do set VER=%%v
for /f "delims=" %%u in ('powershell -NoP -C "(Get-Content '%DEPS%'|Out-String|ConvertFrom-Json).firebase.url"') do set URL=%%u
for /f "delims=" %%s in ('powershell -NoP -C "($cfg=(Get-Content '%DEPS%'|Out-String|ConvertFrom-Json)).firebase.sha256; if($cfg){$cfg}else{''}"') do set EXPECTED_SHA=%%s

if not exist "%ROOT%\3rdparty" mkdir "%ROOT%\3rdparty"

set TMP=%TEMP%\firebase_tmp_%RANDOM%
if exist "%TMP%" rmdir /s /q "%TMP%"
mkdir "%TMP%"

set ZIP=%TMP%\firebase_cpp_sdk_%VER%.zip
echo ⬇️  Downloading Firebase C++ SDK v%VER% ...

REM Simple retry loop (3 attempts)
set /a tries=0
:download_try
set /a tries+=1
powershell -NoP -C "Invoke-WebRequest -Uri '%URL%' -OutFile '%ZIP%' -UseBasicParsing" && goto :download_ok
if %tries% GEQ 3 (
  echo ❌ Download failed after %tries% attempts.
  exit /b 1
)
echo ⚠️  Download failed. Retrying...
timeout /t 2 >nul
goto :download_try

:download_ok

if not "%EXPECTED_SHA%"=="" (
  echo 🔐 Verifying checksum...
  for /f "delims=" %%h in ('powershell -NoP -C "(Get-FileHash -Algorithm SHA256 '%ZIP%').Hash.ToLower()"') do set ACTUAL_SHA=%%h
  set EXPECTED_SHA=%EXPECTED_SHA: =%
  set ACTUAL_SHA=%ACTUAL_SHA: =%

  if /I not "%ACTUAL_SHA%"=="%EXPECTED_SHA%" (
    echo ❌ SHA-256 mismatch!
    echo Expected: %EXPECTED_SHA%
    echo Actual:   %ACTUAL_SHA%
    exit /b 1
  )
  echo ✅ Checksum OK.
) else (
  echo ⚠️  No SHA-256 provided in deps.json. Skipping verification.
)

echo 📦 Unpacking...
powershell -NoP -C "Expand-Archive -Path '%ZIP%' -DestinationPath '%TMP%' -Force"

if not exist "%TMP%\firebase_cpp_sdk" (
  echo ❌ Expected folder 'firebase_cpp_sdk' not found after unzip.
  exit /b 1
)

move "%TMP%\firebase_cpp_sdk" "%DEST%" >nul
echo ✅ Firebase C++ SDK v%VER% ready at: %DEST%

endlocal

