@echo off
chcp 65001 > nul
REM ============================================================
REM  Engine Purity Check
REM  Verifies the configured UnrealEngine checkout has zero modifications to
REM  git-tracked files. Exits non-zero if violated.
REM
REM  Used by GenerateProjectFiles.bat and Build*.bat to fail fast
REM  if anyone (Claude, you, or a teammate) accidentally edited
REM  engine source.
REM
REM  What this checks:
REM    [BLOCKING]  Any tracked file modified  -> exit 1
REM    [INFO]      Untracked files (build out, .claude, etc) -> allowed
REM ============================================================
setlocal

call "%~dp0ResolveEngineRoot.bat" || exit /b 1

if not exist "%ENGINE_ROOT%\.git" (
    echo [PurityCheck] WARNING: %ENGINE_ROOT% is not a git repo. Skipping check.
    exit /b 0
)

pushd "%ENGINE_ROOT%"

REM Look only at TRACKED modifications (porcelain ' M' / 'M ' / etc., NOT '??').
REM Untracked files (build outputs, .claude, etc) are allowed by design.
git status --porcelain | findstr /v "^??" >nul 2>&1
if not errorlevel 1 (
    echo [PurityCheck] FAIL: tracked engine file has been modified:
    git status --porcelain | findstr /v "^??"
    popd
    exit /b 1
)

echo [PurityCheck] OK: engine tracked files are clean.
popd
exit /b 0
