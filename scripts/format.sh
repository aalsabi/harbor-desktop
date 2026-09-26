#!/bin/sh
# Format C++ with clang-format, Python with ruff and QML with qmlformat.
#   --check      verify all three without writing (QML is skipped if qmlformat is not installed)
#   --check-qml  verify QML only; used by CI inside the Debian image, which ships qmlformat
set -eu
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$project_dir"
mode=${1:-}

qmlformat=${QMLFORMAT:-}
if [ -z "$qmlformat" ]; then
    for candidate in qmlformat6 qmlformat /usr/lib/qt6/bin/qmlformat; do
        if command -v "$candidate" >/dev/null 2>&1; then
            qmlformat=$candidate
            break
        fi
    done
fi

format_qml() { # format_qml <directory>
    for file in "$1"/*.qml; do
        "$qmlformat" --inplace --indent-width 4 --newline unix "$file"
    done
}

check_qml() {
    [ -n "$qmlformat" ] || {
        echo "qmlformat not found; set QMLFORMAT or install qt6-declarative-dev" >&2
        return 1
    }
    work=$(mktemp -d)
    cp qml/*.qml "$work"
    format_qml "$work" 2>/dev/null
    status=0
    for file in qml/*.qml; do
        diff -u "$file" "$work/${file#qml/}" || status=1
    done
    rm -rf "$work"
    [ "$status" -eq 0 ] || echo "QML is not formatted; run ./scripts/format.sh" >&2
    return "$status"
}

if [ "$mode" = "--check-qml" ]; then
    check_qml
    exit
fi

cpp_files=$(git ls-files 'src/*.cpp' 'src/*.h' 'tests/*.cpp' 'tests/*.h')
if [ "$mode" = "--check" ]; then
    # shellcheck disable=SC2086
    clang-format --dry-run --Werror $cpp_files
    ruff format --check .
    if [ -n "$qmlformat" ]; then
        check_qml
    else
        echo "qmlformat not found; QML was not checked" >&2
    fi
else
    # shellcheck disable=SC2086
    clang-format -i $cpp_files
    ruff format .
    if [ -n "$qmlformat" ]; then
        format_qml qml 2>/dev/null
    else
        echo "qmlformat not found; QML was not formatted" >&2
    fi
fi
