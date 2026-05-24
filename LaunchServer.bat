@echo off
chcp 65001 > nul
REM ============================================================
REM  Xuanming - Launch Dedicated Server (Local Test)
REM ------------------------------------------------------------
REM  Usage:
REM    LaunchServer.bat                  -> uses default map TestArena
REM    LaunchServer.bat MyMap            -> uses Content/Maps/MyMap.umap
REM ============================================================
setlocal

set ENGINE_ROOT=F:\work\UnrealEngine
set PROJECT_FILE=%~dp0Xuanming.uproject
set MAP_NAME=%1
if "%MAP_NAME%"=="" set MAP_NAME=L_Whitebox_01

set MAP_FILE=%~dp0Content\Maps\Whitebox\%MAP_NAME%.umap
if not exist "%MAP_FILE%" (
    REM Fallback: also look directly under Content/Maps/
    set MAP_FILE=%~dp0Content\Maps\%MAP_NAME%.umap
)
if not exist "%MAP_FILE%" (
    echo [Xuanming][ERROR] Map not found: %MAP_NAME%
    echo   Searched:
    echo     %~dp0Content\Maps\Whitebox\%MAP_NAME%.umap
    echo     %~dp0Content\Maps\%MAP_NAME%.umap
    echo.
    echo You need to create a map first:
    echo   1. Open Editor: double-click Xuanming.uproject
    echo   2. File ^> New Level ^> Basic
    echo   3. Save As: Content/Maps/Whitebox/L_Whitebox_01.umap
    echo.
    pause
    exit /b 1
)

echo [Xuanming] Launching Dedicated Server on port 7777, map=%MAP_NAME%...
"%ENGINE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT_FILE%" /Game/Maps/Whitebox/%MAP_NAME% -server -log -port=7777 -nosteam -ddc=NoZenLocalFallback

pause
