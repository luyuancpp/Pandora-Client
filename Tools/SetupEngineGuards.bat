@echo off
chcp 65001 > nul
REM ============================================================
REM  Pandora - Setup Engine Guards
REM
REM  Installs two protections in F:\work\UnrealEngine to prevent
REM  Claude Code session state (.claude/) from polluting the
REM  engine repo:
REM
REM    1. Junction:  F:\work\UnrealEngine\.claude  ->
REM                  F:\work\Pandora\.claude
REM       So any tool writing to engine .claude actually writes
REM       to the Pandora project, where it's gitignored.
REM
REM    2. pre-commit hook in engine .git/hooks/pre-commit
REM       Hard-blocks any commit that stages .claude/ files,
REM       even if .gitignore is bypassed.
REM
REM  Run this once per machine. Idempotent (safe to re-run).
REM ============================================================
setlocal EnableDelayedExpansion

call "%~dp0ResolveEngineRoot.bat" || exit /b 1
for %%I in ("%~dp0..") do set PANDORA_ROOT=%%~fI
set ENGINE_CLAUDE=%ENGINE_ROOT%\.claude
set PANDORA_CLAUDE=%PANDORA_ROOT%\.claude
set HOOK_PATH=%ENGINE_ROOT%\.git\hooks\pre-commit

REM ---- precondition checks ----
if not exist "%ENGINE_ROOT%\.git" (
    echo [SetupGuards] ERROR: %ENGINE_ROOT% is not a git repo.
    exit /b 1
)
if not exist "%PANDORA_ROOT%" (
    echo [SetupGuards] ERROR: %PANDORA_ROOT% does not exist.
    exit /b 1
)

REM ---- 1. ensure Pandora .claude exists as a real directory ----
if not exist "%PANDORA_CLAUDE%" (
    mkdir "%PANDORA_CLAUDE%"
    echo [SetupGuards] Created %PANDORA_CLAUDE%
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

REM Not a junction. If real dir exists, merge contents into Pandora and remove.
if exist "%ENGINE_CLAUDE%" (
    echo [SetupGuards]   engine .claude exists as a real directory; merging...
    xcopy /E /Y /I /Q "%ENGINE_CLAUDE%\*" "%PANDORA_CLAUDE%\" >nul 2>&1
    rmdir /S /Q "%ENGINE_CLAUDE%" 2>nul
    if exist "%ENGINE_CLAUDE%" (
        echo [SetupGuards] WARN: could not remove %ENGINE_CLAUDE%
        echo [SetupGuards] Close any process using it ^(e.g. Claude Code^) and re-run.
        exit /b 1
    )
)

mklink /J "%ENGINE_CLAUDE%" "%PANDORA_CLAUDE%" >nul
if errorlevel 1 (
    echo [SetupGuards] ERROR: mklink failed
    exit /b 1
)
echo [SetupGuards]   junction created: %ENGINE_CLAUDE% -^> %PANDORA_CLAUDE%

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
echo   - Any .claude/ writes to engine root will land in Pandora project.
echo   - Any attempt to commit .claude/ in engine repo will be blocked.
echo.
exit /b 0
