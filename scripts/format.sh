#!/bin/sh
# Format C++ with clang-format and Python with ruff. Pass --check to verify without writing.
set -eu
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$project_dir"
cpp_files=$(git ls-files 'src/*.cpp' 'src/*.h' 'tests/*.cpp' 'tests/*.h')
if [ "${1:-}" = "--check" ]; then
    # shellcheck disable=SC2086
    clang-format --dry-run --Werror $cpp_files
    ruff format --check .
else
    # shellcheck disable=SC2086
    clang-format -i $cpp_files
    ruff format .
fi
