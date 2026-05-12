#!/bin/sh
# Same as enable-git-hooks.ps1 but for Git Bash / macOS / Linux.
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT" || exit 1
git config core.hooksPath .githooks
echo "Git hooksPath set to .githooks"
