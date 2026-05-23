@echo off
chcp 65001 > nul
REM ============================================================
REM  Xuanming - Setup Engine Guards
REM
REM  Installs two protections in F:\work\UnrealEngine to prevent
REM  Claude Code session state (.claude/) from polluting the
REM  engine repo:
REM
REM    1. Junction:  F:\work\UnrealEngine\.claude  ->
REM                  F:\work\Xuanming\.claude
REM       So any tool writing to engine .claude actually writes
REM       to the Xuanming project, where it's gitignored.
REM
REM    2. pre-commit hook in engine .git/hooks/pre-commit
REM       Hard-blocks any commit that stages .claude/ files,
REM       even if .gitignore is bypassed.
REM
REM  Run this once per machine. Idempotent (safe to re-run).
REM ============================================================
setlocal EnableDelayedExpansion

set ENGINE_ROOT=F:\work\UnrealEngine
set XUANMING_ROOT=F:\work\Xuanming
set ENGINE_CLAUDE=%ENGINE_ROOT%\.claude
set XUANMING_CLAUDE=%XUANMING_ROOT%\.claude
set HOOK_PATH=%ENGINE_ROOT%\.git\hooks\pre-commit

REM ---- precondition checks ----
if not exist "%ENGINE_ROOT%\.git" (
    echo [SetupGuards] ERROR: %ENGINE_ROOT% is not a git repo.
    exit /b 1
)
if not exist "%XUANMING_ROOT%" (
    echo [SetupGuards] ERROR: %XUANMING_ROOT% does not exist.
    exit /b 1
)

REM ---- 1. ensure Xuanming .claude exists as a real directory ----
if not exist "%XUANMING_CLAUDE%" (
    mkdir "%XUANMING_CLAUDE%"
    echo [SetupGuards] Created %XUANMING_CLAUDE%
)

REM ---- 2. install junction ----
echo [SetupGuards] Installing .claude junction in engine root...

REM Detect if engine .claude is already a junction (using PowerShell for reliability)
set IS_JUNCTION=0
for /f %%R in ('powershell -NoProfile -Command "if ((Get-Item '%ENGINE_CLAUDE%' -ErrorAction SilentlyContinue).LinkType -eq 'Junction') { 'YES' } else { 'NO' }"') do set JCHECK=%%R

if "%JCHECK%"=="YES" (
    echo [SetupGuards]   junction already exists, skipping
    goto :install_hook
)

REM Not a junction. If real dir exists, merge contents into Xuanming and remove.
if exist "%ENGINE_CLAUDE%" (
    echo [SetupGuards]   engine .claude exists as a real directory; merging...
    xcopy /E /Y /I /Q "%ENGINE_CLAUDE%\*" "%XUANMING_CLAUDE%\" >nul 2>&1
    rmdir /S /Q "%ENGINE_CLAUDE%" 2>nul
    if exist "%ENGINE_CLAUDE%" (
        echo [SetupGuards] WARN: could not remove %ENGINE_CLAUDE%
        echo [SetupGuards] Close any process using it ^(e.g. Claude Code^) and re-run.
        exit /b 1
    )
)

mklink /J "%ENGINE_CLAUDE%" "%XUANMING_CLAUDE%" >nul
if errorlevel 1 (
    echo [SetupGuards] ERROR: mklink failed
    exit /b 1
)
echo [SetupGuards]   junction created: %ENGINE_CLAUDE% -^> %XUANMING_CLAUDE%

:install_hook
REM ---- 3. install pre-commit hook ----
echo [SetupGuards] Installing pre-commit hook...

if not exist "%ENGINE_ROOT%\.git\hooks" mkdir "%ENGINE_ROOT%\.git\hooks"

REM Copy the canonical hook script (preserves shebang exactly)
copy /Y "%~dp0engine-pre-commit.sh" "%HOOK_PATH%" >nul
if errorlevel 1 (
    echo [SetupGuards] ERROR: failed to install hook
    exit /b 1
)

echo [SetupGuards]   hook installed: %HOOK_PATH%

echo.
echo [SetupGuards] Done. Engine guards are active.
echo   - Any .claude/ writes to engine root will land in Xuanming project.
echo   - Any attempt to commit .claude/ in engine repo will be blocked.
echo.
exit /b 0
