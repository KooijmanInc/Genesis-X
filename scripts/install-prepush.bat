@echo off
REM Install the pre-push hook into a repo-local hooks directory (.githooks)
for /f "delims=" %%i in ('git rev-parse --show-toplevel') do set ROOT=%%i
set HOOKS=%ROOT%\.githooks
set SRC=%~dp0pre-push

if not exist "%HOOKS%" mkdir "%HOOKS%"
copy /Y "%SRC%" "%HOOKS%\pre-push" >nul

git config core.hooksPath .githooks

echo Installed pre-push hook to .githooks\pre-push
echo Protected branches: main staging
echo To override once: set ALLOW_PROTECTED_PUSH=1 && git push <remote> <branch>
