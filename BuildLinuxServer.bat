@echo off
chcp 65001 > nul
REM ============================================================
REM  Xuanming - Build and Cook Linux Dedicated Server
REM  Build Linux DS, can be deployed to Linux servers directly
REM  Prerequisite: install Linux cross-compile toolchain
REM  https://dev.epicgames.com/documentation/zh-cn/unreal-engine/linux-development-requirements-for-unreal-engine
REM ============================================================
setlocal

call "%~dp0Tools\ResolveEngineRoot.bat" || (
    pause
    exit /b 1
)
set PROJECT_FILE=%~dp0Xuanming.uproject
set OUTPUT_DIR=%~dp0Build\LinuxServer
set CONFIG=Development

if "%LINUX_MULTIARCH_ROOT%"=="" (
    echo [Xuanming] WARNING: LINUX_MULTIARCH_ROOT not set.
    echo Please install the Linux Toolchain from:
    echo https://dev.epicgames.com/documentation/zh-cn/unreal-engine/linux-development-requirements-for-unreal-engine
    echo.
    echo If installed, restart your terminal and try again.
    pause
    exit /b 1
)

echo [Xuanming] LINUX_MULTIARCH_ROOT=%LINUX_MULTIARCH_ROOT%
echo [Xuanming] Building Linux Dedicated Server (%CONFIG%)...
echo [Xuanming] Output: %OUTPUT_DIR%
echo.

call "%ENGINE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" ^
    BuildCookRun ^
    -project="%PROJECT_FILE%" ^
    -noP4 ^
    -platform=Linux ^
    -serverplatform=Linux ^
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
    echo [Xuanming] ERROR: Linux server build failed.
    pause
    exit /b 1
)

echo.
echo [Xuanming] Linux Server build complete.
echo Output: %OUTPUT_DIR%\LinuxServer\
echo.
echo Deploy to Linux:
echo   scp -r %OUTPUT_DIR%\LinuxServer user@host:/opt/xuanming/
echo   ssh user@host
echo   cd /opt/xuanming/LinuxServer
echo   chmod +x XuanmingServer.sh
echo   ./XuanmingServer.sh -log -port=7777
echo.
pause
