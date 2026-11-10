#!/usr/bin/env bash
set -euo pipefail

message_file=${1:?usage: check_commit_message.sh MESSAGE_FILE}
clean_message=$(mktemp)
trap 'rm -f "$clean_message"' EXIT

git stripspace --strip-comments < "$message_file" > "$clean_message"
lines=()
while IFS= read -r line; do
    lines+=("$line")
done < "$clean_message"

fail()
{
    printf 'Commit message policy: %s\n' "$1" >&2
    exit 1
}

(( ${#lines[@]} > 2 )) || fail "write a subject, a blank line, and a body."

subject=${lines[0]}
(( ${#subject} <= 72 )) || fail "keep the subject to 72 characters or fewer."
[[ "$subject" =~ ^(build|ci|chore|docs|feat|fix|perf|refactor|revert|style|test)(\([a-z0-9][a-z0-9./-]*\))?!?:\ [a-z][^[:cntrl:]]*$ && "$subject" != *. ]] \
    || fail "use '<type>(<scope>): <lowercase summary>' without a final period."
[[ -z "${lines[1]}" ]] || fail "put one blank line between the subject and body."

footer_start=${#lines[@]}
last_blank=1
for ((index = 2; index < ${#lines[@]}; ++index)); do
    [[ -z "${lines[index]}" ]] && last_blank=$index
done

if (( last_blank + 1 < ${#lines[@]} )); then
    trailers=true
    for ((index = last_blank + 1; index < ${#lines[@]}; ++index)); do
        [[ "${lines[index]}" =~ ^([A-Za-z][A-Za-z-]*|BREAKING\ CHANGE):\ .+$ ]] || trailers=false
    done
    $trailers && footer_start=$((last_blank + 1))
fi

has_body=false
for ((index = 2; index < footer_start; ++index)); do
    line=${lines[index]}
    [[ -z "$line" ]] && continue
    has_body=true
    (( ${#line} <= 72 )) || fail "wrap body prose at 72 characters or fewer."
done

$has_body || fail "add a body that explains the change and why it matters."
