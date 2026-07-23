#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbol_table/symbol_table.h"

typedef bool (*TestFunction)(void);

typedef struct {
    const char *name;
    TestFunction function;
} TestCase;

static SourceLocation at_line(size_t line)
{
    return source_location_make(line);
}

static bool scope_is(const SymbolTable *table,
                     size_t expected_id,
                     size_t expected_depth)
{
    size_t id;
    size_t depth;

    return symbol_table_current_scope_info(table, &id, &depth)
        && id == expected_id
        && depth == expected_depth;
}

static bool symbol_is(const Symbol *symbol,
                      const char *name,
                      ValueType type,
                      size_t line,
                      size_t scope_id,
                      size_t scope_depth)
{
    SymbolInfo info;

    return symbol_get_info(symbol, &info)
        && strcmp(info.name, name) == 0
        && info.type == type
        && info.declaration_location.line == line
        && info.scope_id == scope_id
        && info.scope_depth == scope_depth;
}

static bool streams_are_equal(FILE *first, FILE *second)
{
    int first_character;
    int second_character;

    if (fflush(first) != 0 || fflush(second) != 0) {
        return false;
    }

    rewind(first);
    rewind(second);
    do {
        first_character = fgetc(first);
        second_character = fgetc(second);
        if (first_character != second_character) {
            return false;
        }
    } while (first_character != EOF);

    return ferror(first) == 0 && ferror(second) == 0;
}

static char *read_stream(FILE *stream)
{
    long length;
    char *text;

    if (fflush(stream) != 0 || fseek(stream, 0, SEEK_END) != 0) {
        return NULL;
    }
    length = ftell(stream);
    if (length < 0 || fseek(stream, 0, SEEK_SET) != 0) {
        return NULL;
    }

    text = malloc((size_t)length + 1);
    if (text == NULL) {
        return NULL;
    }
    if (fread(text, 1, (size_t)length, stream) != (size_t)length) {
        free(text);
        return NULL;
    }
    text[length] = '\0';
    return text;
}

static bool test_table_creation(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL;

    symbol_table_destroy(table);
    return passed;
}

static bool test_initial_global_scope(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL && scope_is(table, 0, 0);

    symbol_table_destroy(table);
    return passed;
}

static bool test_global_scope_id(void)
{
    SymbolTable *table = symbol_table_create();
    size_t id = 99;
    size_t depth = 99;
    bool passed = table != NULL
        && symbol_table_current_scope_info(table, &id, &depth)
        && id == 0;

    symbol_table_destroy(table);
    return passed;
}

static bool test_global_scope_depth(void)
{
    SymbolTable *table = symbol_table_create();
    size_t id = 99;
    size_t depth = 99;
    bool passed = table != NULL
        && symbol_table_current_scope_info(table, &id, &depth)
        && depth == 0;

    symbol_table_destroy(table);
    return passed;
}

static bool test_global_symbol_insertion(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_insert(table, "count", VALUE_TYPE_INT, at_line(3))
            == SYMBOL_INSERT_SUCCESS
        && symbol_is(symbol_table_lookup_current(table, "count"),
                     "count", VALUE_TYPE_INT, 3, 0, 0)
        && symbol_table_lookup_history(table, "count") == NULL;

    symbol_table_destroy(table);
    return passed;
}

static bool test_current_scope_lookup(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_insert(table, "ready", VALUE_TYPE_BOOL, at_line(4))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_lookup_current(table, "ready") != NULL;

    symbol_table_destroy(table);
    return passed;
}

static bool test_active_scope_lookup(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_insert(table, "outer", VALUE_TYPE_FLOAT, at_line(1))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_lookup_current(table, "outer") == NULL
        && symbol_is(symbol_table_lookup_active(table, "outer"),
                     "outer", VALUE_TYPE_FLOAT, 1, 0, 0);

    symbol_table_destroy(table);
    return passed;
}

static bool test_missing_name_lookup(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_lookup_current(table, "missing") == NULL
        && symbol_table_lookup_active(table, "missing") == NULL
        && symbol_table_lookup_history(table, "missing") == NULL;

    symbol_table_destroy(table);
    return passed;
}

static bool test_same_scope_redeclaration(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_insert(table, "value", VALUE_TYPE_INT, at_line(1))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_insert(table, "value", VALUE_TYPE_FLOAT, at_line(2))
            == SYMBOL_INSERT_DUPLICATE;

    symbol_table_destroy(table);
    return passed;
}

static bool test_original_binding_preserved(void)
{
    SymbolTable *table = symbol_table_create();
    const Symbol *original;
    bool passed;

    if (table == NULL
        || symbol_table_insert(table, "value", VALUE_TYPE_INT, at_line(1))
            != SYMBOL_INSERT_SUCCESS) {
        symbol_table_destroy(table);
        return false;
    }

    original = symbol_table_lookup_current(table, "value");
    passed = symbol_table_insert(table, "value", VALUE_TYPE_FLOAT, at_line(2))
            == SYMBOL_INSERT_DUPLICATE
        && symbol_table_lookup_current(table, "value") == original
        && symbol_is(original, "value", VALUE_TYPE_INT, 1, 0, 0);

    symbol_table_destroy(table);
    return passed;
}

static bool test_child_scope_entry(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && scope_is(table, 1, 1);

    symbol_table_destroy(table);
    return passed;
}

static bool test_child_scope_insertion(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_insert(table, "local", VALUE_TYPE_BOOL, at_line(6))
            == SYMBOL_INSERT_SUCCESS
        && symbol_is(symbol_table_lookup_current(table, "local"),
                     "local", VALUE_TYPE_BOOL, 6, 1, 1);

    symbol_table_destroy(table);
    return passed;
}

static bool test_nested_shadowing(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_insert(table, "x", VALUE_TYPE_INT, at_line(1))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_insert(table, "x", VALUE_TYPE_FLOAT, at_line(4))
            == SYMBOL_INSERT_SUCCESS
        && symbol_is(symbol_table_lookup_active(table, "x"),
                     "x", VALUE_TYPE_FLOAT, 4, 1, 1);

    symbol_table_destroy(table);
    return passed;
}

static bool test_innermost_first_lookup(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_insert(table, "x", VALUE_TYPE_INT, at_line(1))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_insert(table, "x", VALUE_TYPE_FLOAT, at_line(3))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_insert(table, "x", VALUE_TYPE_BOOL, at_line(5))
            == SYMBOL_INSERT_SUCCESS
        && symbol_is(symbol_table_lookup_active(table, "x"),
                     "x", VALUE_TYPE_BOOL, 5, 2, 2);

    symbol_table_destroy(table);
    return passed;
}

static bool test_scope_exit(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_exit_scope(table) == SYMBOL_SCOPE_SUCCESS
        && scope_is(table, 0, 0);

    symbol_table_destroy(table);
    return passed;
}

static bool test_outer_binding_restoration(void)
{
    SymbolTable *table = symbol_table_create();
    const Symbol *outer = NULL;
    const Symbol *inner = NULL;
    bool passed = table != NULL;

    if (passed) {
        passed = symbol_table_insert(table, "x", VALUE_TYPE_INT, at_line(1))
                == SYMBOL_INSERT_SUCCESS;
        outer = symbol_table_lookup_active(table, "x");
    }
    if (passed) {
        passed = outer != NULL
            && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
            && symbol_table_lookup_active(table, "x") == outer
            && symbol_table_insert(table, "x", VALUE_TYPE_FLOAT, at_line(4))
                == SYMBOL_INSERT_SUCCESS;
        inner = symbol_table_lookup_active(table, "x");
    }
    passed = passed
        && inner != NULL
        && inner != outer
        && symbol_is(inner, "x", VALUE_TYPE_FLOAT, 4, 1, 1)
        && symbol_table_exit_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_lookup_active(table, "x") == outer
        && symbol_is(outer, "x", VALUE_TYPE_INT, 1, 0, 0);

    symbol_table_destroy(table);
    return passed;
}

static bool test_sibling_scope_isolation(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_insert(table, "a", VALUE_TYPE_INT, at_line(2))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_exit_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_insert(table, "b", VALUE_TYPE_INT, at_line(6))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_lookup_active(table, "a") == NULL
        && symbol_table_lookup_current(table, "b") != NULL
        && symbol_table_lookup_history(table, "a") != NULL;

    symbol_table_destroy(table);
    return passed;
}

static bool test_unique_monotonic_scope_ids(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && scope_is(table, 1, 1)
        && symbol_table_exit_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && scope_is(table, 2, 1);

    symbol_table_destroy(table);
    return passed;
}

static bool test_scope_depths(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && scope_is(table, 1, 1)
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && scope_is(table, 2, 2)
        && symbol_table_exit_scope(table) == SYMBOL_SCOPE_SUCCESS
        && scope_is(table, 1, 1);

    symbol_table_destroy(table);
    return passed;
}

static bool test_historical_lookup_after_exit(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_insert(table, "past", VALUE_TYPE_FLOAT, at_line(8))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_exit_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_insert(table, "past", VALUE_TYPE_BOOL, at_line(12))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_exit_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_lookup_active(table, "past") == NULL
        && symbol_is(symbol_table_lookup_history(table, "past"),
                     "past", VALUE_TYPE_BOOL, 12, 2, 1);

    symbol_table_destroy(table);
    return passed;
}

static bool test_inactive_versus_never_declared(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_insert(table, "inactive", VALUE_TYPE_INT, at_line(2))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_exit_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_lookup_active(table, "inactive") == NULL
        && symbol_table_lookup_history(table, "inactive") != NULL
        && symbol_table_lookup_active(table, "neverDeclared") == NULL
        && symbol_table_lookup_history(table, "neverDeclared") == NULL;

    symbol_table_destroy(table);
    return passed;
}

static bool test_multiple_nested_levels(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_insert(table, "global", VALUE_TYPE_BOOL, at_line(1))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && scope_is(table, 3, 3)
        && symbol_table_lookup_active(table, "global") != NULL;

    symbol_table_destroy(table);
    return passed;
}

static bool test_global_scope_exit_rejection(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_exit_scope(table) == SYMBOL_SCOPE_CANNOT_EXIT_GLOBAL
        && scope_is(table, 0, 0);

    symbol_table_destroy(table);
    return passed;
}

static bool test_invalid_table_arguments(void)
{
    SymbolTable *table = symbol_table_create();
    const Symbol *symbol = NULL;
    size_t id;
    size_t depth;
    SymbolInfo info;
    bool passed = table != NULL;

    if (passed) {
        passed = symbol_table_insert(table, "x", VALUE_TYPE_INT, at_line(1))
                == SYMBOL_INSERT_SUCCESS;
        symbol = symbol_table_lookup_current(table, "x");
    }

    passed = passed
        && symbol != NULL
        && !symbol_table_current_scope_info(table, NULL, &depth)
        && !symbol_table_current_scope_info(table, &id, NULL)
        && !symbol_get_info(symbol, NULL)
        && !symbol_table_print(table, NULL)
        && symbol_table_enter_scope(NULL) == SYMBOL_SCOPE_INVALID_ARGUMENT
        && symbol_table_exit_scope(NULL) == SYMBOL_SCOPE_INVALID_ARGUMENT
        && symbol_table_insert(NULL, "x", VALUE_TYPE_INT, at_line(1))
            == SYMBOL_INSERT_INVALID_ARGUMENT
        && symbol_table_lookup_current(NULL, "x") == NULL
        && symbol_table_lookup_active(NULL, "x") == NULL
        && symbol_table_lookup_history(NULL, "x") == NULL
        && !symbol_table_current_scope_info(NULL, &id, &depth)
        && !symbol_get_info(NULL, &info)
        && !symbol_table_print(NULL, stdout);

    symbol_table_destroy(table);
    return passed;
}

static bool test_invalid_names(void)
{
    SymbolTable *table = symbol_table_create();
    bool passed = table != NULL
        && symbol_table_insert(table, NULL, VALUE_TYPE_INT, at_line(1))
            == SYMBOL_INSERT_INVALID_ARGUMENT
        && symbol_table_insert(table, "", VALUE_TYPE_INT, at_line(1))
            == SYMBOL_INSERT_INVALID_ARGUMENT
        && symbol_table_insert(table, "x", (ValueType)99, at_line(1))
            == SYMBOL_INSERT_INVALID_ARGUMENT
        && symbol_table_lookup_current(table, NULL) == NULL
        && symbol_table_lookup_active(table, "") == NULL;

    symbol_table_destroy(table);
    return passed;
}

static SymbolTable *build_printer_sample(void)
{
    SymbolTable *table = symbol_table_create();

    if (table == NULL
        || symbol_table_insert(table, "x", VALUE_TYPE_INT, at_line(1))
            != SYMBOL_INSERT_SUCCESS
        || symbol_table_enter_scope(table) != SYMBOL_SCOPE_SUCCESS
        || symbol_table_insert(table, "x", VALUE_TYPE_FLOAT, at_line(4))
            != SYMBOL_INSERT_SUCCESS
        || symbol_table_enter_scope(table) != SYMBOL_SCOPE_SUCCESS
        || symbol_table_insert(table, "flag", VALUE_TYPE_BOOL, at_line(7))
            != SYMBOL_INSERT_SUCCESS
        || symbol_table_exit_scope(table) != SYMBOL_SCOPE_SUCCESS
        || symbol_table_exit_scope(table) != SYMBOL_SCOPE_SUCCESS
        || symbol_table_enter_scope(table) != SYMBOL_SCOPE_SUCCESS
        || symbol_table_insert(table, "count", VALUE_TYPE_INT, at_line(11))
            != SYMBOL_INSERT_SUCCESS) {
        symbol_table_destroy(table);
        return NULL;
    }

    return table;
}

static bool test_deterministic_printer(void)
{
    SymbolTable *table = build_printer_sample();
    FILE *first = tmpfile();
    FILE *second = tmpfile();
    bool passed = table != NULL
        && first != NULL
        && second != NULL
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_exit_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_print(table, first)
        && symbol_table_print(table, second)
        && streams_are_equal(first, second);

    if (first != NULL) {
        fclose(first);
    }
    if (second != NULL) {
        fclose(second);
    }
    symbol_table_destroy(table);
    return passed;
}

static bool test_destruction_with_active_scopes(void)
{
    SymbolTable *table = symbol_table_create();

    if (table == NULL
        || symbol_table_enter_scope(table) != SYMBOL_SCOPE_SUCCESS
        || symbol_table_insert(table, "first", VALUE_TYPE_INT, at_line(1))
            != SYMBOL_INSERT_SUCCESS
        || symbol_table_enter_scope(table) != SYMBOL_SCOPE_SUCCESS
        || symbol_table_insert(table, "second", VALUE_TYPE_BOOL, at_line(2))
            != SYMBOL_INSERT_SUCCESS) {
        symbol_table_destroy(table);
        return false;
    }

    symbol_table_destroy(table);
    return true;
}

static bool test_destruction_with_exited_scopes(void)
{
    SymbolTable *table = symbol_table_create();

    if (table == NULL
        || symbol_table_enter_scope(table) != SYMBOL_SCOPE_SUCCESS
        || symbol_table_insert(table, "past", VALUE_TYPE_FLOAT, at_line(2))
            != SYMBOL_INSERT_SUCCESS
        || symbol_table_exit_scope(table) != SYMBOL_SCOPE_SUCCESS) {
        symbol_table_destroy(table);
        return false;
    }

    symbol_table_destroy(table);
    symbol_table_destroy(NULL);
    return true;
}

static bool test_lookup_pointer_stability_and_name_ownership(void)
{
    SymbolTable *table = symbol_table_create();
    char source_name[] = "stable";
    const Symbol *saved;
    SymbolInfo info;
    size_t index;
    bool passed = table != NULL
        && symbol_table_insert(table, source_name, VALUE_TYPE_INT, at_line(1))
            == SYMBOL_INSERT_SUCCESS;

    if (!passed) {
        symbol_table_destroy(table);
        return false;
    }

    saved = symbol_table_lookup_current(table, "stable");
    source_name[0] = 'X';
    for (index = 0; index < 40 && passed; index++) {
        char name[32];
        int written = snprintf(name, sizeof(name), "other_%zu", index);
        passed = written > 0
            && (size_t)written < sizeof(name)
            && symbol_table_insert(table, name, VALUE_TYPE_BOOL,
                                   at_line(index + 2))
                == SYMBOL_INSERT_SUCCESS;
    }

    passed = passed
        && symbol_table_enter_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_table_exit_scope(table) == SYMBOL_SCOPE_SUCCESS
        && symbol_get_info(saved, &info)
        && strcmp(info.name, "stable") == 0
        && saved == symbol_table_lookup_active(table, "stable");

    symbol_table_destroy(table);
    return passed;
}

static bool test_declaration_order_preserved(void)
{
    SymbolTable *table = symbol_table_create();
    FILE *output = tmpfile();
    char *text = NULL;
    const char *first;
    const char *second;
    const char *third;
    bool passed = table != NULL
        && output != NULL
        && symbol_table_insert(table, "zeta", VALUE_TYPE_INT, at_line(1))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_insert(table, "alpha", VALUE_TYPE_FLOAT, at_line(2))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_insert(table, "middle", VALUE_TYPE_BOOL, at_line(3))
            == SYMBOL_INSERT_SUCCESS
        && symbol_table_print(table, output);

    if (passed) {
        text = read_stream(output);
        first = text == NULL ? NULL : strstr(text, "name=zeta");
        second = text == NULL ? NULL : strstr(text, "name=alpha");
        third = text == NULL ? NULL : strstr(text, "name=middle");
        passed = first != NULL && second != NULL && third != NULL
            && first < second && second < third;
    }

    free(text);
    if (output != NULL) {
        fclose(output);
    }
    symbol_table_destroy(table);
    return passed;
}

static bool print_sample(void)
{
    SymbolTable *table = build_printer_sample();
    bool passed = table != NULL
        && printf("Symbol table sample:\n") >= 0
        && symbol_table_print(table, stdout);

    symbol_table_destroy(table);
    return passed;
}

int main(void)
{
    const TestCase tests[] = {
        {"table creation", test_table_creation},
        {"initial global scope", test_initial_global_scope},
        {"global scope ID", test_global_scope_id},
        {"global scope depth", test_global_scope_depth},
        {"global symbol insertion", test_global_symbol_insertion},
        {"current-scope lookup", test_current_scope_lookup},
        {"active-scope lookup", test_active_scope_lookup},
        {"missing-name lookup", test_missing_name_lookup},
        {"same-scope redeclaration rejection", test_same_scope_redeclaration},
        {"original binding preservation", test_original_binding_preserved},
        {"child-scope entry", test_child_scope_entry},
        {"child-scope insertion", test_child_scope_insertion},
        {"nested shadowing", test_nested_shadowing},
        {"innermost-first lookup", test_innermost_first_lookup},
        {"scope exit", test_scope_exit},
        {"outer-binding restoration", test_outer_binding_restoration},
        {"sibling-scope isolation", test_sibling_scope_isolation},
        {"unique monotonic scope IDs", test_unique_monotonic_scope_ids},
        {"scope depths", test_scope_depths},
        {"historical lookup after exit", test_historical_lookup_after_exit},
        {"inactive versus never declared", test_inactive_versus_never_declared},
        {"multiple nested levels", test_multiple_nested_levels},
        {"global-scope exit rejection", test_global_scope_exit_rejection},
        {"invalid table arguments", test_invalid_table_arguments},
        {"invalid names and types", test_invalid_names},
        {"deterministic printer", test_deterministic_printer},
        {"destruction with active scopes", test_destruction_with_active_scopes},
        {"destruction with exited scopes", test_destruction_with_exited_scopes},
        {"stable borrowed lookup and owned name",
         test_lookup_pointer_stability_and_name_ownership},
        {"declaration-order preservation", test_declaration_order_preserved}
    };
    const size_t test_count = sizeof(tests) / sizeof(tests[0]);
    size_t index;

    for (index = 0; index < test_count; index++) {
        if (!tests[index].function()) {
            fprintf(stderr, "FAIL: %s\n", tests[index].name);
            return EXIT_FAILURE;
        }
        printf("PASS: %s\n", tests[index].name);
    }

    printf("Symbol table unit tests: %zu passed\n", test_count);
    if (!print_sample()) {
        fprintf(stderr, "FAIL: symbol table sample output\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
