@echo off
REM ============================================================
REM  Xuanming - Build Editor (for opening project in editor)
REM  编译 Editor target，编辑器开发用
REM ============================================================
setlocal

set ENGINE_ROOT=F:\work\UnrealEngine
set PROJECT_FILE=%~dp0Xuanming.uproject

echo [Xuanming] Building XuanmingEditor (Development Win64)...
call "%ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" XuanmingEditor Win64 Development -project="%PROJECT_FILE%" -waitmutex

if errorlevel 1 (
    echo [Xuanming] ERROR: Build failed.
    pause
    exit /b 1
)

echo [Xuanming] Editor build complete.
pause
