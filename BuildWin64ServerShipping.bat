@echo off
chcp 65001 > nul
REM ============================================================
REM  Pandora - Build and Cook Win64 Dedicated Server (Shipping)
REM ============================================================
setlocal

call "%~dp0Tools\ResolveEngineRoot.bat" || (
    pause
    exit /b 1
)
set PROJECT_FILE=%~dp0Pandora.uproject
set OUTPUT_DIR=%~dp0Build\WindowsServer
set CONFIG=Shipping

echo [Pandora] Building Win64 Dedicated Server (%CONFIG%)...
echo [Pandora] Output: %OUTPUT_DIR%
echo.

call "%ENGINE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" ^
    BuildCookRun ^
    -project="%PROJECT_FILE%" ^
    -noP4 ^
    -platform=Win64 ^
    -serverplatform=Win64 ^
    -serverconfig=%CONFIG% ^
    -cook ^
    -allmaps ^
    -build ^
    -stage ^
    -pak ^
    -archive ^
    -archivedirectory="%OUTPUT_DIR%" ^
    -server ^
    -noclient ^
    -utf8output

if errorlevel 1 (
    echo [Pandora] ERROR: Win64 server build failed.
    pause
    exit /b 1
)

echo.
echo [Pandora] Win64 Server build complete.
echo Output: %OUTPUT_DIR%\WindowsServer\
echo Run:    PandoraServer.exe -log -port=7777
pause