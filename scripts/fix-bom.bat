@echo off
REM Remove UTF-8 BOM from tracked text files
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0\fix-bom.ps1" %*
