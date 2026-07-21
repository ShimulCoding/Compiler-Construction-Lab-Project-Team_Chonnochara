#!/usr/bin/env bash

set -eu

script_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repository_root=$(CDPATH= cd -- "$script_directory/.." && pwd)
result_directory="$repository_root/build/test-results"
actual_stdout="$result_directory/ast_unit.stdout"
actual_stderr="$result_directory/ast_unit.stderr"
normalized_expected="$result_directory/ast_unit.expected"
lexer_binary="$repository_root/build/lexer_test"

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

run_lexer_success()
{
    local case_name=$1
    local source_file=$2
    local expected_file=$3
    local case_stdout="$result_directory/$case_name.stdout"
    local case_stderr="$result_directory/$case_name.stderr"
    local normalized_lexer_expected="$result_directory/$case_name.expected"

    if ! "$lexer_binary" "$source_file" >"$case_stdout" 2>"$case_stderr"; then
        printf 'FAIL: lexer success case %s returned nonzero\n' "$case_name" >&2
        cat "$case_stderr" >&2
        exit 1
    fi
    if [ -s "$case_stderr" ]; then
        printf 'FAIL: lexer success case %s wrote standard error\n' "$case_name" >&2
        cat "$case_stderr" >&2
        exit 1
    fi

    sed 's/\r$//' "$expected_file" >"$normalized_lexer_expected"
    if ! cmp -s "$normalized_lexer_expected" "$case_stdout"; then
        printf 'FAIL: lexer output differs for %s\n' "$case_name" >&2
        diff -u "$normalized_lexer_expected" "$case_stdout" >&2 || true
        exit 1
    fi
}

run_lexer_failure()
{
    local case_name=$1
    local source_file=$2
    local expected_stderr_file=$3
    local expected_exit_file=$4
    local case_stdout="$result_directory/$case_name.stdout"
    local case_stderr="$result_directory/$case_name.stderr"
    local normalized_lexer_expected="$result_directory/$case_name.expected.stderr"
    local actual_status
    local expected_status

    if "$lexer_binary" "$source_file" >"$case_stdout" 2>"$case_stderr"; then
        actual_status=0
    else
        actual_status=$?
    fi
    expected_status=$(tr -d '\r\n' <"$expected_exit_file")

    if [ "$actual_status" -ne "$expected_status" ]; then
        printf 'FAIL: lexer failure case %s exited %s, expected %s\n' \
            "$case_name" "$actual_status" "$expected_status" >&2
        exit 1
    fi
    if [ -s "$case_stdout" ]; then
        printf 'FAIL: lexer failure case %s wrote unexpected standard output\n' \
            "$case_name" >&2
        cat "$case_stdout" >&2
        exit 1
    fi

    sed 's/\r$//' "$expected_stderr_file" >"$normalized_lexer_expected"
    if ! cmp -s "$normalized_lexer_expected" "$case_stderr"; then
        printf 'FAIL: lexer diagnostic differs for %s\n' "$case_name" >&2
        diff -u "$normalized_lexer_expected" "$case_stderr" >&2 || true
        exit 1
    fi
}

run_lexer_success \
    all_tokens \
    "$repository_root/tests/lexer/all_tokens.mc" \
    "$repository_root/tests/expected/lexer/all_tokens.stdout"

run_lexer_success \
    operator_adjacency \
    "$repository_root/tests/lexer/operator_adjacency.mc" \
    "$repository_root/tests/expected/lexer/operator_adjacency.stdout"

run_lexer_success \
    layout_comments_lf \
    "$repository_root/tests/lexer/layout_comments.txt" \
    "$repository_root/tests/expected/lexer/layout_comments.stdout"

crlf_source="$result_directory/layout_comments_crlf.mc"
sed 's/\r$//' "$repository_root/tests/lexer/layout_comments.txt" \
    | sed 's/$/\r/' >"$crlf_source"
run_lexer_success \
    layout_comments_crlf \
    "$crlf_source" \
    "$repository_root/tests/expected/lexer/layout_comments.stdout"

run_lexer_success \
    official_sample \
    "$repository_root/tests/lexer/official_sample.mc" \
    "$repository_root/tests/expected/lexer/official_sample.stdout"

run_lexer_success \
    block_comment_not_supported \
    "$repository_root/tests/lexer/block_comment_not_supported.mc" \
    "$repository_root/tests/expected/lexer/block_comment_not_supported.stdout"

run_lexer_failure \
    invalid_character \
    "$repository_root/tests/lexer/invalid_character.mc" \
    "$repository_root/tests/expected/lexer/invalid_character.stderr" \
    "$repository_root/tests/expected/lexer/invalid_character.exit"

run_lexer_failure \
    invalid_leading_dot \
    "$repository_root/tests/lexer/invalid_leading_dot.mc" \
    "$repository_root/tests/expected/lexer/invalid_leading_dot.stderr" \
    "$repository_root/tests/expected/lexer/invalid_leading_dot.exit"

run_lexer_failure \
    invalid_trailing_dot \
    "$repository_root/tests/lexer/invalid_trailing_dot.mc" \
    "$repository_root/tests/expected/lexer/invalid_trailing_dot.stderr" \
    "$repository_root/tests/expected/lexer/invalid_trailing_dot.exit"

run_lexer_failure \
    invalid_exponent \
    "$repository_root/tests/lexer/invalid_exponent.mc" \
    "$repository_root/tests/expected/lexer/invalid_exponent.stderr" \
    "$repository_root/tests/expected/lexer/invalid_exponent.exit"

printf '%s\n' 'PASS: 10 lexer cases including longest match, LF/CRLF, comments, sample, and errors'
