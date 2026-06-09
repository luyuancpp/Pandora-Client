@echo off
chcp 65001 > nul
REM ============================================================
REM  Pandora - Launch Editor (workaround for ZenServer issue)
REM ------------------------------------------------------------
REM  Double-clicking Pandora.uproject starts ZenServer, which is
REM  currently failing on this machine.
REM  This script disables Zen and falls back to filesystem DDC.
REM ============================================================
setlocal

call "%~dp0Tools\ResolveEngineRoot.bat" || (
    pause
    exit /b 1
)
set PROJECT_FILE=%~dp0Pandora.uproject

echo [Pandora] Launching Editor (Zen disabled)...
"%ENGINE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT_FILE%" -ddc=NoZenLocalFallback -log

if errorlevel 1 (
    echo.
    echo [Pandora] Editor exited with error. Check Saved\Logs\Pandora.log
    pause
)
