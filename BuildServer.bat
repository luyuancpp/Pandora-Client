@echo off
chcp 65001 > nul
REM ============================================================
REM  Pandora - Build Dedicated Server (Win64 Development)
REM ============================================================
setlocal

call "%~dp0Tools\ResolveEngineRoot.bat" || (
    pause
    exit /b 1
)
set PROJECT_FILE=%~dp0Pandora.uproject

echo [Pandora] Building PandoraServer (Development Win64)...
call "%ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" PandoraServer Win64 Development -project="%PROJECT_FILE%" -waitmutex

if errorlevel 1 (
    echo [Pandora] ERROR: Server build failed.
    pause
    exit /b 1
)

echo [Pandora] Server build complete.
echo Binary: %ENGINE_ROOT%\Engine\Binaries\Win64\PandoraServer.exe (or in project Binaries)
pause