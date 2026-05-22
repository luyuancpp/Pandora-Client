@echo off
REM ============================================================
REM  Xuanming - Build Client (Win64 Development)
REM  编译客户端二进制
REM ============================================================
setlocal

set ENGINE_ROOT=F:\work\UnrealEngine
set PROJECT_FILE=%~dp0Xuanming.uproject

echo [Xuanming] Building XuanmingClient (Development Win64)...
call "%ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" XuanmingClient Win64 Development -project="%PROJECT_FILE%" -waitmutex

if errorlevel 1 (
    echo [Xuanming] ERROR: Client build failed.
    pause
    exit /b 1
)

echo [Xuanming] Client build complete.
pause
