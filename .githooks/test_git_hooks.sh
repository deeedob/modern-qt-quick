#!/usr/bin/env bash
set -euo pipefail

root=$(git rev-parse --show-toplevel)
temporary_dir=$(mktemp -d)
trap 'rm -rf "$temporary_dir"' EXIT

expect_failure()
{
    if "$@" >/dev/null 2>&1; then
        printf 'Expected failure: %s\n' "$*" >&2
        exit 1
    fi
}

write_message()
{
    printf '%s\n' "$1" > "$temporary_dir/message"
}

write_message $'feat(canvas): add undo command\n\nRecord each stroke before drawing. Users can restore accidental edits.\n\nCloses: #123'
"$root/tools/check_commit_message.sh" "$temporary_dir/message"

write_message $'feat(api)!: remove v1 endpoint\n\nClients must migrate to the v2 endpoint before upgrading.\n\nBREAKING CHANGE: The v1 endpoint no longer accepts requests.'
"$root/tools/check_commit_message.sh" "$temporary_dir/message"

write_message $'fix(core): handle v1.2 files\n\nKeep existing files readable.'
/bin/bash "$root/tools/check_commit_message.sh" "$temporary_dir/message"

write_message $'feature: add undo\n\nRecord an undo action.'
expect_failure "$root/tools/check_commit_message.sh" "$temporary_dir/message"

write_message $'feat(canvas): add undo'
expect_failure "$root/tools/check_commit_message.sh" "$temporary_dir/message"

write_message $'feat(canvas): add undo\n\nThis line is deliberately longer than seventy-two characters so that the policy rejects it. '
expect_failure "$root/tools/check_commit_message.sh" "$temporary_dir/message"

repository="$temporary_dir/repository"
mkdir -p "$repository/.githooks" "$repository/tools"
cp "$root/.githooks/commit-msg" "$root/.githooks/pre-commit" \
    "$root/.githooks/prepare-commit-msg" "$repository/.githooks/"
cp "$root/tools/check_commit_message.sh" "$root/tools/check_git_history.sh" \
    "$root/tools/check_staged_format.sh" "$repository/tools/"
cp "$root/.clang-format" "$root/.qmlformat.ini" "$repository/"
git -C "$repository" init --quiet
git -C "$repository" config user.name Tester
git -C "$repository" config user.email tester@example.com
git -C "$repository" config core.hooksPath .githooks
git -C "$repository" commit --allow-empty -m 'chore: initialize test repository' -m 'Create a temporary repository for hook tests.' --quiet
if git -C "$repository" log -1 --format=%B | grep -q '^#'; then
    printf 'Message template leaked into commit\n' >&2
    exit 1
fi
git -C "$repository" commit --amend --allow-empty --no-edit --quiet
dd if=/dev/zero of="$repository/large media.bin" bs=50001 count=1 2>/dev/null
git -C "$repository" add 'large media.bin'
printf 'small working copy\n' > "$repository/large media.bin"
git -C "$repository" commit -m 'test: accept a large staged file' \
    -m 'Warn about large files without blocking a commit.' --quiet 2> "$temporary_dir/large-warning"
if ! grep -F 'large\ media.bin' "$temporary_dir/large-warning" >/dev/null ||
   ! grep -F '50001' "$temporary_dir/large-warning" >/dev/null ||
   ! grep -F 'Git LFS' "$temporary_dir/large-warning" >/dev/null; then
    printf 'Expected warning for large staged file\n' >&2
    exit 1
fi
printf 'small staged file\n' > "$repository/large media.bin"
git -C "$repository" add 'large media.bin'
dd if=/dev/zero of="$repository/large media.bin" bs=50001 count=1 2>/dev/null
git -C "$repository" commit -m 'test: accept a small staged file' \
    -m 'Use the staged file size instead of the working copy.' --quiet 2> "$temporary_dir/small-warning"
if grep -F 'large\ media.bin' "$temporary_dir/small-warning" >/dev/null; then
    printf 'Unexpected warning for small staged file\n' >&2
    exit 1
fi

if git -C "$repository" log -1 --format=%B | grep -q '^#'; then
    printf 'Message template leaked into amended commit\n' >&2
    exit 1
fi

printf 'int main(){return 0;}\n' > "$repository/example.cpp"
git -C "$repository" add example.cpp
expect_failure git -C "$repository" commit -m 'test: reject unformatted c++' -m 'Exercise the staged C++ formatting gate.'
clang-format -i --style="file:$repository/.clang-format" "$repository/example.cpp"
git -C "$repository" add example.cpp
(
    cd "$repository"
    /bin/bash tools/check_staged_format.sh
)
printf '\nint broken(  ) {}\n' >> "$repository/example.cpp"
git -C "$repository" commit -m 'test: accept formatted c++' -m 'Exercise the staged C++ formatting gate.' --quiet
git -C "$repository" diff --quiet && exit 1

printf 'import QtQuick\nItem {property int value: 1}\n' > "$repository/Example.qml"
git -C "$repository" add Example.qml
expect_failure git -C "$repository" commit -m 'test: reject unformatted qml' -m 'Exercise the staged QML formatting gate.'
qmlformat --settings "$repository/.qmlformat.ini" -i "$repository/Example.qml"
git -C "$repository" add Example.qml
git -C "$repository" commit -m 'test: accept formatted qml' -m 'Exercise the staged QML formatting gate.' --quiet
git -C "$repository" switch --create topic --quiet
git -C "$repository" commit --allow-empty -m 'test: add topic commit' \
    -m 'Create a topic commit for merge-hook coverage.' --quiet
git -C "$repository" switch --quiet -
git -C "$repository" commit --allow-empty -m 'test: advance main branch' \
    -m 'Create a second parent for merge-hook coverage.' --quiet
git -C "$repository" merge --no-ff topic -m 'Merge topic branch' --quiet
(
    cd "$repository"
    tools/check_git_history.sh 'HEAD~4..HEAD'
    expect_failure tools/check_git_history.sh 'missing-ref..HEAD'
)
