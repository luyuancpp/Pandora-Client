@echo off
chcp 65001 > nul
REM ============================================================
REM  Pandora - Launch Client (Connect to local DS)
REM ------------------------------------------------------------
REM  Usage:
REM    LaunchClient.bat                  -> connect to 127.0.0.1:7777
REM    LaunchClient.bat 192.168.1.10     -> connect to that IP on 7777
REM    LaunchClient.bat 192.168.1.10 8000-> connect to that IP:port
REM ============================================================
setlocal

call "%~dp0Tools\ResolveEngineRoot.bat" || (
    pause
    exit /b 1
)
set PROJECT_FILE=%~dp0Pandora.uproject
set TARGET_IP=%1
if "%TARGET_IP%"=="" set TARGET_IP=127.0.0.1
set TARGET_PORT=%2
if "%TARGET_PORT%"=="" set TARGET_PORT=7777

echo [Pandora] Launching Client, connecting to %TARGET_IP%:%TARGET_PORT%...
"%ENGINE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT_FILE%" %TARGET_IP%:%TARGET_PORT% -game -log -windowed -resx=1280 -resy=720 -nosteam -ddc=NoZenLocalFallback

pause