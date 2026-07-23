#!/usr/bin/env bash

set -eu

script_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repository_root=$(CDPATH= cd -- "$script_directory/.." && pwd)
result_directory="$repository_root/build/test-results"
actual_stdout="$result_directory/ast_unit.stdout"
actual_stderr="$result_directory/ast_unit.stderr"
normalized_expected="$result_directory/ast_unit.expected"
symbol_table_stdout="$result_directory/symbol_table_unit.stdout"
symbol_table_repeat_stdout="$result_directory/symbol_table_unit.repeat.stdout"
symbol_table_stderr="$result_directory/symbol_table_unit.stderr"
symbol_table_repeat_stderr="$result_directory/symbol_table_unit.repeat.stderr"
normalized_symbol_table_expected="$result_directory/symbol_table_unit.expected"
lexer_binary="$repository_root/build/lexer_test"
parser_binary="$repository_root/build/parser_test"
semantic_binary="$repository_root/build/semantic_test"

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

if ! "$repository_root/build/symbol_table_tests" \
    >"$symbol_table_stdout" 2>"$symbol_table_stderr"; then
    printf '%s\n' 'FAIL: symbol-table unit test executable returned nonzero' >&2
    cat "$symbol_table_stderr" >&2
    exit 1
fi

if ! "$repository_root/build/symbol_table_tests" \
    >"$symbol_table_repeat_stdout" 2>"$symbol_table_repeat_stderr"; then
    printf '%s\n' 'FAIL: repeated symbol-table test run returned nonzero' >&2
    cat "$symbol_table_repeat_stderr" >&2
    exit 1
fi

if [ -s "$symbol_table_stderr" ] || [ -s "$symbol_table_repeat_stderr" ]; then
    printf '%s\n' 'FAIL: symbol-table unit tests wrote unexpected standard error' >&2
    cat "$symbol_table_stderr" "$symbol_table_repeat_stderr" >&2
    exit 1
fi

sed 's/\r$//' "$repository_root/tests/expected/symbol_table_unit.stdout" \
    >"$normalized_symbol_table_expected"

if ! cmp -s "$normalized_symbol_table_expected" "$symbol_table_stdout"; then
    printf '%s\n' 'FAIL: symbol-table output differs from the golden file' >&2
    diff -u "$normalized_symbol_table_expected" "$symbol_table_stdout" >&2 \
        || true
    exit 1
fi

if ! cmp -s "$symbol_table_stdout" "$symbol_table_repeat_stdout"; then
    printf '%s\n' 'FAIL: symbol-table output changed across repeated runs' >&2
    diff -u "$symbol_table_stdout" "$symbol_table_repeat_stdout" >&2 || true
    exit 1
fi

printf '%s\n' 'PASS: 30 symbol-table unit tests and deterministic printer output'

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

run_parser_success()
{
    local case_name=$1
    local source_file=$2
    local expected_file=$3
    local case_stdout="$result_directory/parser_$case_name.stdout"
    local case_stderr="$result_directory/parser_$case_name.stderr"
    local normalized_parser_expected="$result_directory/parser_$case_name.expected"

    if ! "$parser_binary" "$source_file" >"$case_stdout" 2>"$case_stderr"; then
        printf 'FAIL: parser success case %s returned nonzero\n' \
            "$case_name" >&2
        cat "$case_stderr" >&2
        exit 1
    fi
    if [ -s "$case_stderr" ]; then
        printf 'FAIL: parser success case %s wrote standard error\n' \
            "$case_name" >&2
        cat "$case_stderr" >&2
        exit 1
    fi
    if [ ! -s "$case_stdout" ]; then
        printf 'FAIL: parser success case %s produced no AST output\n' \
            "$case_name" >&2
        exit 1
    fi

    if [ "$expected_file" != "-" ]; then
        sed 's/\r$//' "$expected_file" >"$normalized_parser_expected"
        if ! cmp -s "$normalized_parser_expected" "$case_stdout"; then
            printf 'FAIL: parser AST differs for %s\n' "$case_name" >&2
            diff -u "$normalized_parser_expected" "$case_stdout" >&2 || true
            exit 1
        fi
    fi
}

run_parser_failure()
{
    local case_name=$1
    local source_file=$2
    local expected_stderr_file=$3
    local expected_exit_file=$4
    local case_stdout="$result_directory/parser_$case_name.stdout"
    local case_stderr="$result_directory/parser_$case_name.stderr"
    local normalized_parser_expected="$result_directory/parser_$case_name.expected.stderr"
    local actual_status
    local expected_status

    if "$parser_binary" "$source_file" >"$case_stdout" 2>"$case_stderr"; then
        actual_status=0
    else
        actual_status=$?
    fi
    expected_status=$(tr -d '\r\n' <"$expected_exit_file")

    if [ "$actual_status" -ne "$expected_status" ]; then
        printf 'FAIL: parser failure case %s exited %s, expected %s\n' \
            "$case_name" "$actual_status" "$expected_status" >&2
        exit 1
    fi
    if [ -s "$case_stdout" ]; then
        printf 'FAIL: parser failure case %s wrote unexpected standard output\n' \
            "$case_name" >&2
        cat "$case_stdout" >&2
        exit 1
    fi

    sed 's/\r$//' "$expected_stderr_file" >"$normalized_parser_expected"
    if ! cmp -s "$normalized_parser_expected" "$case_stderr"; then
        printf 'FAIL: parser diagnostic differs for %s\n' "$case_name" >&2
        diff -u "$normalized_parser_expected" "$case_stderr" >&2 || true
        exit 1
    fi
}

run_parser_success \
    declaration \
    "$repository_root/tests/parser/valid/declaration.mc" \
    -
run_parser_success \
    all_operators \
    "$repository_root/tests/parser/valid/all_operators.mc" \
    -
run_parser_success \
    initialized_declaration \
    "$repository_root/tests/parser/valid/initialized_declaration.mc" \
    "$repository_root/tests/expected/parser/initialized_declaration.stdout"
run_parser_success \
    assignment_print \
    "$repository_root/tests/parser/valid/assignment_print.mc" \
    -
run_parser_success \
    precedence \
    "$repository_root/tests/parser/valid/precedence.mc" \
    "$repository_root/tests/expected/parser/precedence.stdout"
run_parser_success \
    empty_block \
    "$repository_root/tests/parser/valid/empty_block.mc" \
    -
run_parser_success \
    nested_blocks \
    "$repository_root/tests/parser/valid/nested_blocks.mc" \
    "$repository_root/tests/expected/parser/nested_blocks.stdout"
run_parser_success \
    if_without_else \
    "$repository_root/tests/parser/valid/if_without_else.mc" \
    -
run_parser_success \
    if_else \
    "$repository_root/tests/parser/valid/if_else.mc" \
    "$repository_root/tests/expected/parser/if_else.stdout"
run_parser_success \
    while \
    "$repository_root/tests/parser/valid/while.mc" \
    "$repository_root/tests/expected/parser/while.stdout"
run_parser_success \
    line_tracking_lf \
    "$repository_root/tests/parser/valid/line_tracking.txt" \
    "$repository_root/tests/expected/parser/line_tracking.stdout"

parser_crlf_source="$result_directory/parser_line_tracking_crlf.mc"
sed 's/\r$//' "$repository_root/tests/parser/valid/line_tracking.txt" \
    | sed 's/$/\r/' >"$parser_crlf_source"
run_parser_success \
    line_tracking_crlf \
    "$parser_crlf_source" \
    "$repository_root/tests/expected/parser/line_tracking.stdout"

run_parser_success \
    official_sample \
    "$repository_root/tests/lexer/official_sample.mc" \
    "$repository_root/tests/expected/parser/official_sample.stdout"

for parser_case in \
    bare_expression \
    chained_equality \
    chained_relational \
    empty_source \
    invalid_else_placement \
    line_after_comments \
    missing_right_brace \
    missing_right_parenthesis \
    missing_semicolon \
    numeric_unary_minus \
    print_expression \
    print_literal \
    print_parenthesized \
    recovery \
    recovery_at_rbrace \
    unbraced_if \
    unbraced_while
do
    run_parser_failure \
        "$parser_case" \
        "$repository_root/tests/parser/invalid/$parser_case.mc" \
        "$repository_root/tests/expected/parser/$parser_case.stderr" \
        "$repository_root/tests/expected/parser/$parser_case.exit"
done

run_parser_failure \
    lexical_error_no_duplicate_syntax \
    "$repository_root/tests/lexer/invalid_character.mc" \
    "$repository_root/tests/expected/lexer/invalid_character.stderr" \
    "$repository_root/tests/expected/lexer/invalid_character.exit"

run_parser_failure \
    lexical_then_independent_syntax \
    "$repository_root/tests/parser/invalid/lexical_then_syntax.mc" \
    "$repository_root/tests/expected/parser/lexical_then_syntax.stderr" \
    "$repository_root/tests/expected/parser/lexical_then_syntax.exit"

printf '%s\n' 'PASS: 32 parser cases including all operators, AST goldens, recovery, LF/CRLF, and lexical-error suppression'

run_semantic_success()
{
    local case_name=$1
    local source_file=$2
    local case_stdout="$result_directory/semantic_$case_name.stdout"
    local case_stderr="$result_directory/semantic_$case_name.stderr"

    if ! "$semantic_binary" "$source_file" \
        >"$case_stdout" 2>"$case_stderr"; then
        printf 'FAIL: semantic success case %s returned nonzero\n' \
            "$case_name" >&2
        cat "$case_stderr" >&2
        exit 1
    fi
    if [ -s "$case_stdout" ] || [ -s "$case_stderr" ]; then
        printf 'FAIL: semantic success case %s produced unexpected output\n' \
            "$case_name" >&2
        cat "$case_stdout" "$case_stderr" >&2
        exit 1
    fi
}

run_semantic_failure()
{
    local case_name=$1
    local source_file=$2
    local expected_stderr_file=$3
    local expected_exit_file=$4
    local case_stdout="$result_directory/semantic_$case_name.stdout"
    local case_stderr="$result_directory/semantic_$case_name.stderr"
    local normalized_semantic_expected="$result_directory/semantic_$case_name.expected.stderr"
    local actual_status
    local expected_status

    if "$semantic_binary" "$source_file" \
        >"$case_stdout" 2>"$case_stderr"; then
        actual_status=0
    else
        actual_status=$?
    fi
    expected_status=$(tr -d '\r\n' <"$expected_exit_file")

    if [ "$actual_status" -ne "$expected_status" ]; then
        printf 'FAIL: semantic failure case %s exited %s, expected %s\n' \
            "$case_name" "$actual_status" "$expected_status" >&2
        exit 1
    fi
    if [ -s "$case_stdout" ]; then
        printf 'FAIL: semantic failure case %s wrote unexpected standard output\n' \
            "$case_name" >&2
        cat "$case_stdout" >&2
        exit 1
    fi

    sed 's/\r$//' "$expected_stderr_file" \
        >"$normalized_semantic_expected"
    if ! cmp -s "$normalized_semantic_expected" "$case_stderr"; then
        printf 'FAIL: semantic diagnostic differs for %s\n' \
            "$case_name" >&2
        diff -u "$normalized_semantic_expected" "$case_stderr" >&2 || true
        exit 1
    fi
}

for semantic_case in \
    core \
    initializer_outer \
    nested_blocks \
    numeric_promotion \
    shadow_restore \
    sibling_scopes
do
    run_semantic_success \
        "$semantic_case" \
        "$repository_root/tests/semantic/valid/$semantic_case.mc"
done

for semantic_case in \
    equality_mismatch \
    if_condition \
    initializer_cascade \
    initializer_exact_mismatch \
    initializer_mismatch \
    invalid_arithmetic \
    invalid_assignment \
    invalid_logical \
    invalid_not \
    invalid_numeric_assignment \
    invalid_relational \
    invalid_remainder \
    multiple_errors \
    redeclaration \
    redeclaration_initializer \
    scope_violation \
    self_initializer \
    sibling_scope \
    undeclared \
    while_condition
do
    run_semantic_failure \
        "$semantic_case" \
        "$repository_root/tests/semantic/invalid/$semantic_case.mc" \
        "$repository_root/tests/expected/semantic/$semantic_case.stderr" \
        "$repository_root/tests/expected/semantic/$semantic_case.exit"
done

printf '%s\n' 'PASS: 26 semantic cases covering scopes, declarations, types, operators, conditions, and deterministic diagnostics'
