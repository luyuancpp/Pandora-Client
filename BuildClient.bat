@echo off
chcp 65001 > nul
REM ============================================================
REM  Pandora - Build Client (Win64 Development)
REM ============================================================
setlocal

call "%~dp0Tools\ResolveEngineRoot.bat" || (
    pause
    exit /b 1
)
set PROJECT_FILE=%~dp0Pandora.uproject

echo [Pandora] Building PandoraClient (Development Win64)...
call "%ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" PandoraClient Win64 Development -project="%PROJECT_FILE%" -waitmutex

if errorlevel 1 (
    echo [Pandora] ERROR: Client build failed.
    pause
    exit /b 1
)

echo [Pandora] Client build complete.
pause