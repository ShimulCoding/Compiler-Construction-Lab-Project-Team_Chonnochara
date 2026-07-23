#include "symbol_table/symbol_table.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct Symbol {
    char *name;
    ValueType type;
    SourceLocation declaration_location;
    size_t scope_id;
    size_t scope_depth;
    Symbol *next;
};

typedef struct ScopeFrame {
    size_t id;
    size_t depth;
    bool active;
    struct ScopeFrame *parent;
    Symbol *first_symbol;
    Symbol *last_symbol;
} ScopeFrame;

struct SymbolTable {
    ScopeFrame **scopes;
    size_t scope_count;
    size_t scope_capacity;
    size_t next_scope_id;
    ScopeFrame *current_scope;
};

static bool value_type_is_valid(ValueType type)
{
    return type == VALUE_TYPE_INT
        || type == VALUE_TYPE_FLOAT
        || type == VALUE_TYPE_BOOL;
}

static char *copy_name(const char *name)
{
    size_t length;
    char *copy;

    if (name == NULL || name[0] == '\0') {
        return NULL;
    }

    length = strlen(name) + 1;
    copy = malloc(length);
    if (copy != NULL) {
        memcpy(copy, name, length);
    }
    return copy;
}

static const Symbol *lookup_in_scope(const ScopeFrame *scope,
                                     const char *name)
{
    const Symbol *symbol;

    if (scope == NULL || name == NULL || name[0] == '\0') {
        return NULL;
    }

    for (symbol = scope->first_symbol;
         symbol != NULL;
         symbol = symbol->next) {
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
    }

    return NULL;
}

static bool grow_scope_storage(SymbolTable *table)
{
    size_t new_capacity;
    ScopeFrame **new_scopes;

    if (table->scope_count < table->scope_capacity) {
        return true;
    }

    new_capacity = table->scope_capacity == 0
        ? 4
        : table->scope_capacity * 2;
    if (new_capacity < table->scope_capacity
        || new_capacity > SIZE_MAX / sizeof(*new_scopes)) {
        return false;
    }

    new_scopes = realloc(table->scopes,
                         new_capacity * sizeof(*new_scopes));
    if (new_scopes == NULL) {
        return false;
    }

    table->scopes = new_scopes;
    table->scope_capacity = new_capacity;
    return true;
}

static void destroy_symbols(Symbol *symbol)
{
    while (symbol != NULL) {
        Symbol *next = symbol->next;
        free(symbol->name);
        free(symbol);
        symbol = next;
    }
}

SymbolTable *symbol_table_create(void)
{
    SymbolTable *table = calloc(1, sizeof(*table));
    ScopeFrame *global_scope;

    if (table == NULL) {
        return NULL;
    }

    global_scope = calloc(1, sizeof(*global_scope));
    if (global_scope == NULL || !grow_scope_storage(table)) {
        free(global_scope);
        free(table->scopes);
        free(table);
        return NULL;
    }

    global_scope->id = 0;
    global_scope->depth = 0;
    global_scope->active = true;
    table->scopes[0] = global_scope;
    table->scope_count = 1;
    table->next_scope_id = 1;
    table->current_scope = global_scope;
    return table;
}

void symbol_table_destroy(SymbolTable *table)
{
    size_t index;

    if (table == NULL) {
        return;
    }

    for (index = 0; index < table->scope_count; index++) {
        destroy_symbols(table->scopes[index]->first_symbol);
        free(table->scopes[index]);
    }
    free(table->scopes);
    free(table);
}

SymbolScopeResult symbol_table_enter_scope(SymbolTable *table)
{
    ScopeFrame *scope;

    if (table == NULL || table->current_scope == NULL) {
        return SYMBOL_SCOPE_INVALID_ARGUMENT;
    }

    scope = calloc(1, sizeof(*scope));
    if (scope == NULL) {
        return SYMBOL_SCOPE_ALLOCATION_FAILURE;
    }
    if (!grow_scope_storage(table)) {
        free(scope);
        return SYMBOL_SCOPE_ALLOCATION_FAILURE;
    }

    scope->id = table->next_scope_id;
    scope->depth = table->current_scope->depth + 1;
    scope->active = true;
    scope->parent = table->current_scope;
    table->scopes[table->scope_count] = scope;
    table->scope_count++;
    table->next_scope_id++;
    table->current_scope = scope;
    return SYMBOL_SCOPE_SUCCESS;
}

SymbolScopeResult symbol_table_exit_scope(SymbolTable *table)
{
    ScopeFrame *scope;

    if (table == NULL || table->current_scope == NULL) {
        return SYMBOL_SCOPE_INVALID_ARGUMENT;
    }

    scope = table->current_scope;
    if (scope->parent == NULL) {
        return SYMBOL_SCOPE_CANNOT_EXIT_GLOBAL;
    }

    scope->active = false;
    table->current_scope = scope->parent;
    return SYMBOL_SCOPE_SUCCESS;
}

SymbolInsertResult symbol_table_insert(SymbolTable *table,
                                       const char *name,
                                       ValueType type,
                                       SourceLocation location)
{
    Symbol *symbol;

    if (table == NULL || table->current_scope == NULL
        || name == NULL || name[0] == '\0'
        || !value_type_is_valid(type)) {
        return SYMBOL_INSERT_INVALID_ARGUMENT;
    }
    if (lookup_in_scope(table->current_scope, name) != NULL) {
        return SYMBOL_INSERT_DUPLICATE;
    }

    symbol = calloc(1, sizeof(*symbol));
    if (symbol == NULL) {
        return SYMBOL_INSERT_ALLOCATION_FAILURE;
    }
    symbol->name = copy_name(name);
    if (symbol->name == NULL) {
        free(symbol);
        return SYMBOL_INSERT_ALLOCATION_FAILURE;
    }

    symbol->type = type;
    symbol->declaration_location = location;
    symbol->scope_id = table->current_scope->id;
    symbol->scope_depth = table->current_scope->depth;

    if (table->current_scope->last_symbol == NULL) {
        table->current_scope->first_symbol = symbol;
    } else {
        table->current_scope->last_symbol->next = symbol;
    }
    table->current_scope->last_symbol = symbol;
    return SYMBOL_INSERT_SUCCESS;
}

const Symbol *symbol_table_lookup_current(const SymbolTable *table,
                                          const char *name)
{
    if (table == NULL) {
        return NULL;
    }
    return lookup_in_scope(table->current_scope, name);
}

const Symbol *symbol_table_lookup_active(const SymbolTable *table,
                                         const char *name)
{
    const ScopeFrame *scope;

    if (table == NULL || name == NULL || name[0] == '\0') {
        return NULL;
    }

    for (scope = table->current_scope;
         scope != NULL;
         scope = scope->parent) {
        const Symbol *symbol = lookup_in_scope(scope, name);
        if (symbol != NULL) {
            return symbol;
        }
    }

    return NULL;
}

const Symbol *symbol_table_lookup_history(const SymbolTable *table,
                                          const char *name)
{
    size_t index;

    if (table == NULL || name == NULL || name[0] == '\0') {
        return NULL;
    }

    for (index = table->scope_count; index > 0; index--) {
        const ScopeFrame *scope = table->scopes[index - 1];
        const Symbol *symbol;

        if (scope->active) {
            continue;
        }
        symbol = lookup_in_scope(scope, name);
        if (symbol != NULL) {
            return symbol;
        }
    }

    return NULL;
}

bool symbol_table_current_scope_info(const SymbolTable *table,
                                     size_t *scope_id,
                                     size_t *scope_depth)
{
    if (table == NULL || table->current_scope == NULL
        || scope_id == NULL || scope_depth == NULL) {
        return false;
    }

    *scope_id = table->current_scope->id;
    *scope_depth = table->current_scope->depth;
    return true;
}

bool symbol_get_info(const Symbol *symbol, SymbolInfo *info)
{
    if (symbol == NULL || info == NULL) {
        return false;
    }

    info->name = symbol->name;
    info->type = symbol->type;
    info->declaration_location = symbol->declaration_location;
    info->scope_id = symbol->scope_id;
    info->scope_depth = symbol->scope_depth;
    return true;
}

static const char *value_type_name(ValueType type)
{
    switch (type) {
    case VALUE_TYPE_INT:
        return "int";
    case VALUE_TYPE_FLOAT:
        return "float";
    case VALUE_TYPE_BOOL:
        return "bool";
    }
    return "unknown";
}

bool symbol_table_print(const SymbolTable *table, FILE *output)
{
    size_t index;

    if (table == NULL || output == NULL) {
        return false;
    }

    for (index = 0; index < table->scope_count; index++) {
        const ScopeFrame *scope = table->scopes[index];
        const Symbol *symbol;

        if (fprintf(output,
                    "Scope(id=%zu, depth=%zu, active=%s)\n",
                    scope->id,
                    scope->depth,
                    scope->active ? "true" : "false") < 0) {
            return false;
        }

        if (scope->first_symbol == NULL
            && fprintf(output, "  <empty>\n") < 0) {
            return false;
        }

        for (symbol = scope->first_symbol;
             symbol != NULL;
             symbol = symbol->next) {
            if (fprintf(output,
                        "  Symbol(name=%s, type=%s, line=%zu)\n",
                        symbol->name,
                        value_type_name(symbol->type),
                        symbol->declaration_location.line) < 0) {
                return false;
            }
        }
    }

    return ferror(output) == 0;
}
