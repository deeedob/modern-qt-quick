#!/usr/bin/env bash
set -euo pipefail

root=$(git rev-parse --show-toplevel)
temporary_dir=$(mktemp -d)
trap 'rm -rf "$temporary_dir"' EXIT
git diff --cached --name-only --diff-filter=ACMR -z > "$temporary_dir/paths"
paths=()
while IFS= read -r -d '' path; do
    paths+=("$path")
done < "$temporary_dir/paths"

needs_clang_format=false
needs_qmlformat=false
for path in "${paths[@]+${paths[@]}}"; do
    case "$path" in
        *.cpp|*.cxx|*.cc|*.h|*.hpp|*.mm)
            needs_clang_format=true
            ;;
        *.qml)
            needs_qmlformat=true
            ;;
    esac
done

if $needs_clang_format && ! command -v clang-format >/dev/null; then
    printf 'Install clang-format before committing C++ changes.\n' >&2
    exit 1
fi
if $needs_qmlformat && ! command -v qmlformat >/dev/null; then
    printf "Install Qt's qmlformat before committing QML changes.\n" >&2
    exit 1
fi

index=0
for path in "${paths[@]+${paths[@]}}"; do
    case "$path" in
        *.cpp|*.cxx|*.cc|*.h|*.hpp|*.mm)
            snapshot="$temporary_dir/$index.${path##*.}"
            git show ":$path" > "$snapshot"
            clang-format --dry-run --Werror --style="file:$root/.clang-format" "$snapshot"
            ;;
        *.qml)
            snapshot="$temporary_dir/$index.qml"
            formatted="$temporary_dir/$index.formatted.qml"
            git show ":$path" > "$snapshot"
            qmlformat --settings "$root/.qmlformat.ini" "$snapshot" > "$formatted"
            if ! cmp -s "$snapshot" "$formatted"; then
                printf 'Format staged QML before committing: %s\n' "$path" >&2
                exit 1
            fi
            ;;
    esac
    ((++index))
done
