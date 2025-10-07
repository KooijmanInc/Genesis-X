@echo off
setlocal EnableExtensions EnableDelayedExpansion

set ROOT=%~dp0..
set PKG_DIR=%ROOT%\scripts\packages

if "%~1"=="" set TARGET=all
if not "%~1"=="" set TARGET=%~1

if /I "%TARGET%"=="list" (
  echo Available packages:
  for %%F in ("%PKG_DIR%\*.bat") do (
    set "name=%%~nF"
    echo !name!
  )
  goto :eof
)

if /I "%TARGET%"=="all" (
  for %%F in ("%PKG_DIR%\*.bat") do (
    echo ==> Running package: %%~nF
    call "%%F" || goto :error
  )
  goto :eof
)

:loop
if "%~1"=="" goto :eof
echo ==> Running package: %~1
if exist "%PKG_DIR%\%~1.bat" (
  call "%PKG_DIR%\%~1.bat" || goto :error
) else (
  echo No script for package: %~1
  exit /b 1
)
shift
goto :loop

:error
echo Package failed. Aborting.
exit /b 1

:eof
endlocal
