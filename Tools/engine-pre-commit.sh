#!/bin/sh
# pre-commit hook installed by Xuanming/Tools/SetupEngineGuards.bat.
# Refuse any commit that touches .claude/ in this engine repo.

if git diff --cached --name-only | grep -q '^\.claude/'; then
    echo "[pre-commit] BLOCKED: refusing to commit files under .claude/"
    echo "[pre-commit] These are Claude Code session state, not engine source."
    echo "[pre-commit] Unstage them with: git reset HEAD .claude/"
    exit 1
fi

exit 0
