#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."

mapfile -t files < <(find include tests tools -type f \( -name '*.hpp' -o -name '*.cpp' -o -name '*.h' \) 2>/dev/null)

if [ "${#files[@]}" -eq 0 ]; then
    echo "no source files found"
    exit 0
fi

nix develop . --command clang-format -i "${files[@]}"
echo "formatted ${#files[@]} file(s)"
