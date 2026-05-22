@echo off
REM ============================================================
REM  Xuanming - Launch Dedicated Server (Local Test)
REM  本地启动 DS 进行联调
REM ============================================================
setlocal

set ENGINE_ROOT=F:\work\UnrealEngine
set PROJECT_FILE=%~dp0Xuanming.uproject

echo [Xuanming] Launching Dedicated Server on port 7777...
"%ENGINE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT_FILE%" -server -log -port=7777 -nosteam

pause
