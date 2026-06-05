@echo off
chcp 65001 > nul
REM ============================================================
REM  Xuanming - Launch Editor (workaround for ZenServer issue)
REM ------------------------------------------------------------
REM  双击 Xuanming.uproject 会启 ZenServer，目前在你机器上启不来。
REM  这个脚本用 -ddc=NoZenLocalFallback 禁用 Zen，回退到文件系统 DDC。
REM ============================================================
setlocal

call "%~dp0Tools\ResolveEngineRoot.bat" || (
    pause
    exit /b 1
)
set PROJECT_FILE=%~dp0Xuanming.uproject

echo [Xuanming] Launching Editor (Zen disabled)...
"%ENGINE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT_FILE%" -ddc=NoZenLocalFallback -log

if errorlevel 1 (
    echo.
    echo [Xuanming] Editor exited with error. Check Saved\Logs\Xuanming.log
    pause
)
