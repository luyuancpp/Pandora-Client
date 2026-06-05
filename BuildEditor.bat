@echo off
chcp 65001 > nul
REM ============================================================
REM  Xuanming - Build Editor (for opening project in editor)
REM ============================================================
setlocal

call "%~dp0Tools\ResolveEngineRoot.bat" || (
    pause
    exit /b 1
)
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
