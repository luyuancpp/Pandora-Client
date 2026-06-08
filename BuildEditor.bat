@echo off
chcp 65001 > nul
REM ============================================================
REM  Pandora - Build Editor (for opening project in editor)
REM ============================================================
setlocal

call "%~dp0Tools\ResolveEngineRoot.bat" || (
    pause
    exit /b 1
)
set PROJECT_FILE=%~dp0Pandora.uproject

echo [Pandora] Building PandoraEditor (Development Win64)...
call "%ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" PandoraEditor Win64 Development -project="%PROJECT_FILE%" -waitmutex

if errorlevel 1 (
    echo [Pandora] ERROR: Build failed.
    pause
    exit /b 1
)

echo [Pandora] Editor build complete.
pause