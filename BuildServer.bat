@echo off
chcp 65001 > nul
REM ============================================================
REM  Xuanming - Build Dedicated Server (Win64 Development)
REM ============================================================
setlocal

call "%~dp0Tools\ResolveEngineRoot.bat" || (
    pause
    exit /b 1
)
set PROJECT_FILE=%~dp0Xuanming.uproject

echo [Xuanming] Building XuanmingServer (Development Win64)...
call "%ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" XuanmingServer Win64 Development -project="%PROJECT_FILE%" -waitmutex

if errorlevel 1 (
    echo [Xuanming] ERROR: Server build failed.
    pause
    exit /b 1
)

echo [Xuanming] Server build complete.
echo Binary: %ENGINE_ROOT%\Engine\Binaries\Win64\XuanmingServer.exe (or in project Binaries)
pause
