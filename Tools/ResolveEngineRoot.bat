@echo off
REM Resolves ENGINE_ROOT for local build scripts.
REM If ENGINE_ROOT is already set by the caller, keep it.

if not "%ENGINE_ROOT%"=="" (
    if exist "%ENGINE_ROOT%\Engine" exit /b 0
    echo [Xuanming] ERROR: ENGINE_ROOT is set but invalid: %ENGINE_ROOT%
    exit /b 1
)

if exist "D:\UnrealEngine\Engine" (
    set ENGINE_ROOT=D:\UnrealEngine
    exit /b 0
)

if exist "F:\work\UnrealEngine\Engine" (
    set ENGINE_ROOT=F:\work\UnrealEngine
    exit /b 0
)

echo [Xuanming] ERROR: UnrealEngine not found.
echo Set ENGINE_ROOT to your UnrealEngine checkout, for example:
echo   set ENGINE_ROOT=D:\UnrealEngine
exit /b 1
