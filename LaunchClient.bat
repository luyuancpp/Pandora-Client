@echo off
REM ============================================================
REM  Xuanming - Launch Client (Connect to local DS)
REM  本地启动客户端并连接 127.0.0.1:7777 的 DS
REM ============================================================
setlocal

set ENGINE_ROOT=F:\work\UnrealEngine
set PROJECT_FILE=%~dp0Xuanming.uproject

echo [Xuanming] Launching Client, connecting to 127.0.0.1:7777...
"%ENGINE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT_FILE%" 127.0.0.1:7777 -game -log -windowed -resx=1280 -resy=720 -nosteam

pause
