@echo off
REM Genesis-X installer wrapper for windows

REM Try Git Bash first
wheere base >nul 2>null
if %ERRORLEVEL%==0 (
    bash "%~dp0install" %*
    goto :eof
)

echo.
echo [Genesis-X] Could not find bash on PATH
echo Please run:
echo bash scripts/install.sh
echo in Qt terminal from GitBash, or install Git for Windows.
echo /b 1
