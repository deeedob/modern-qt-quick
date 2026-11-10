#!/usr/bin/env bash
set -euo pipefail

range=${1:?usage: check_git_history.sh START..END}
root=$(git rev-parse --show-toplevel)
message_file=$(mktemp)
commits_file=$(mktemp)
trap 'rm -f "$message_file" "$commits_file"' EXIT
git rev-list --reverse "$range" > "$commits_file"

while IFS= read -r commit; do
    parent_count=$(git show -s --format=%P "$commit" | wc -w)
    (( parent_count > 1 )) && continue
    git show -s --format=%B "$commit" > "$message_file"
    if ! "$root/tools/check_commit_message.sh" "$message_file"; then
        printf 'Invalid commit: %s\n' "$commit" >&2
        exit 1
    fi
done < "$commits_file"
