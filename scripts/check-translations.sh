#!/bin/sh
# Fails when translations/*.ts no longer match the strings in the sources.
# Fix by running: cmake --build <build-dir> --target update_translations
set -eu
src=${1:-/src}
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cp -r "$src" "$work/src"
cmake -S "$work/src" -B "$work/build" -G Ninja >/dev/null
cmake --build "$work/build" --target update_translations >/dev/null
diff -ru "$src/translations" "$work/src/translations"
echo "Translations are up to date."
