#!/bin/sh
set -eu
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir="$project_dir/build"
cmake -S "$project_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "$build_dir" --parallel 4
ctest --test-dir "$build_dir" --output-on-failure
python3 -m unittest discover -s "$project_dir/tests" -p 'test_*.py'
