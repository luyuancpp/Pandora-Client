@echo off
chcp 65001 > nul
REM ============================================================
REM  Xuanming - Generate Visual Studio Project Files
REM
REM  Self-contained: does NOT modify the engine repo.
REM  Engine purity is enforced by Tools/CheckEngineUntouched.bat.
REM
REM  .NET version is pinned via THIS project's global.json
REM  (not the engine's). dotnet always resolves global.json from
REM  the working directory upward, so we pushd into our project
REM  before invoking dotnet/UBT.
REM ============================================================
setlocal

set ENGINE_ROOT=F:\work\UnrealEngine
set PROJECT_ROOT=%~dp0
set PROJECT_FILE=%PROJECT_ROOT%Xuanming.uproject
set UBT_SRC=%ENGINE_ROOT%\Engine\Source\Programs\UnrealBuildTool
set UBT_DLL=%UBT_SRC%\bin\Development\UnrealBuildTool.dll

REM ---- Step 1: engine purity check ----
echo [Xuanming] Step 1/4: Checking that engine source is untouched...
call "%PROJECT_ROOT%Tools\CheckEngineUntouched.bat" || (
    echo [Xuanming] ABORT: engine has unexpected modifications.
    pause
    exit /b 1
)

REM ---- Step 2: locate .NET 8 SDK ----
echo [Xuanming] Step 2/4: Locating .NET 8 SDK...
where dotnet >nul 2>&1 || (
    echo [Xuanming] ERROR: dotnet not found in PATH.
    echo Install .NET 8 SDK: winget install Microsoft.DotNet.SDK.8
    pause
    exit /b 1
)

REM Probe from project root - this picks up our project-local global.json
pushd "%PROJECT_ROOT%"
for /f "tokens=*" %%v in ('dotnet --version 2^>nul') do set DOTNET_VERSION=%%v
popd

echo [Xuanming]   project-pinned .NET version: %DOTNET_VERSION%
REM Check version starts with "8." -- avoid findstr regex pitfalls in delayed expansion
if not "%DOTNET_VERSION:~0,2%"=="8." (
    echo [Xuanming] ERROR: project requires .NET 8, got %DOTNET_VERSION%.
    echo Install: winget install Microsoft.DotNet.SDK.8
    pause
    exit /b 1
)

REM ---- Step 3: build UBT if missing ----
echo [Xuanming] Step 3/4: Verifying UnrealBuildTool is built...
if not exist "%UBT_DLL%" (
    echo [Xuanming]   UBT dll missing, building from source with .NET 8...
    REM Build from project root so global.json is picked up correctly
    pushd "%PROJECT_ROOT%"
    dotnet build "%UBT_SRC%\UnrealBuildTool.csproj" -c Development --nologo -v minimal
    set BUILD_RC=%ERRORLEVEL%
    popd
    if not "%BUILD_RC%"=="0" (
        echo [Xuanming] ERROR: UBT build failed.
        pause
        exit /b 1
    )
)

REM ---- Step 4: generate VS project files ----
echo [Xuanming] Step 4/4: Generating Visual Studio project files...
pushd "%PROJECT_ROOT%"
dotnet "%UBT_DLL%" -projectfiles -project="%PROJECT_FILE%" -game -rocket -progress
set GEN_RC=%ERRORLEVEL%
popd

if not "%GEN_RC%"=="0" (
    echo [Xuanming] ERROR: Failed to generate project files.
    pause
    exit /b 1
)

echo.
echo [Xuanming] Done. Open Xuanming.sln in Visual Studio.
pause
