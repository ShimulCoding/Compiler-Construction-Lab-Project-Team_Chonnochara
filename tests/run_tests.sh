#!/usr/bin/env bash

set -eu

script_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repository_root=$(CDPATH= cd -- "$script_directory/.." && pwd)
result_directory="$repository_root/build/test-results"
actual_stdout="$result_directory/ast_unit.stdout"
actual_stderr="$result_directory/ast_unit.stderr"
normalized_expected="$result_directory/ast_unit.expected"

mkdir -p "$result_directory"

"$repository_root/build/token_interface_test"
printf '%s\n' 'PASS: generated Bison token header'

if ! "$repository_root/build/ast_tests" >"$actual_stdout" 2>"$actual_stderr"; then
    printf '%s\n' 'FAIL: AST unit test executable returned nonzero' >&2
    cat "$actual_stderr" >&2
    exit 1
fi

if [ -s "$actual_stderr" ]; then
    printf '%s\n' 'FAIL: AST unit tests wrote unexpected standard error' >&2
    cat "$actual_stderr" >&2
    exit 1
fi

sed 's/\r$//' "$repository_root/tests/expected/ast_unit.stdout" \
    >"$normalized_expected"

if ! cmp -s "$normalized_expected" "$actual_stdout"; then
    printf '%s\n' 'FAIL: AST printer output differs from the golden file' >&2
    diff -u "$normalized_expected" "$actual_stdout" >&2 || true
    exit 1
fi

printf '%s\n' 'PASS: AST unit tests and deterministic printer output'
