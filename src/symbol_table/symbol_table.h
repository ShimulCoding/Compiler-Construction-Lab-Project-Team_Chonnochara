#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "common/source_location.h"
#include "common/value_type.h"

typedef struct Symbol Symbol;
typedef struct SymbolTable SymbolTable;

typedef enum {
    SYMBOL_SCOPE_SUCCESS,
    SYMBOL_SCOPE_INVALID_ARGUMENT,
    SYMBOL_SCOPE_CANNOT_EXIT_GLOBAL,
    SYMBOL_SCOPE_ALLOCATION_FAILURE
} SymbolScopeResult;

typedef enum {
    SYMBOL_INSERT_SUCCESS,
    SYMBOL_INSERT_DUPLICATE,
    SYMBOL_INSERT_INVALID_ARGUMENT,
    SYMBOL_INSERT_ALLOCATION_FAILURE
} SymbolInsertResult;

typedef struct {
    const char *name;
    ValueType type;
    SourceLocation declaration_location;
    size_t scope_id;
    size_t scope_depth;
} SymbolInfo;

/*
 * Creates a table with active global scope ID 0 at depth 0.
 * Returns NULL if allocation fails. The caller owns the returned table.
 */
SymbolTable *symbol_table_create(void);

/*
 * Frees the table, all scope history, symbols, and copied names.
 * Passing NULL is safe.
 */
void symbol_table_destroy(SymbolTable *table);

/*
 * Creates and activates one child of the current scope. Scope IDs increase
 * monotonically and are never reused. Returns SUCCESS, INVALID_ARGUMENT, or
 * ALLOCATION_FAILURE; the table is unchanged on failure.
 */
SymbolScopeResult symbol_table_enter_scope(SymbolTable *table);

/*
 * Deactivates the current non-global scope and restores its parent.
 * Returns SUCCESS, INVALID_ARGUMENT, or CANNOT_EXIT_GLOBAL. A rejected exit
 * leaves the table unchanged.
 */
SymbolScopeResult symbol_table_exit_scope(SymbolTable *table);

/*
 * Copies name and inserts a declaration in the current scope. The original
 * binding is preserved when a same-scope duplicate is rejected. Returns the
 * corresponding SUCCESS, DUPLICATE, INVALID_ARGUMENT, or ALLOCATION_FAILURE
 * status; only SUCCESS transfers a copied name into table ownership.
 */
SymbolInsertResult symbol_table_insert(SymbolTable *table,
                                       const char *name,
                                       ValueType type,
                                       SourceLocation location);

/*
 * Returns the declaration named name only in the current scope, or NULL for
 * a missing name or invalid argument. The returned pointer is borrowed.
 */
const Symbol *symbol_table_lookup_current(const SymbolTable *table,
                                          const char *name);

/*
 * Searches active scopes from innermost to global. Returns NULL for a missing
 * name or invalid argument. The returned pointer is borrowed.
 */
const Symbol *symbol_table_lookup_active(const SymbolTable *table,
                                         const char *name);

/*
 * Searches only exited scopes, newest scope first. Returns NULL when no
 * inactive declaration exists or an argument is invalid. The result does not
 * make the declaration active and is borrowed from the table.
 */
const Symbol *symbol_table_lookup_history(const SymbolTable *table,
                                          const char *name);

/*
 * Copies the current scope ID and depth into the required output pointers.
 * Returns false for an invalid table or NULL output pointer.
 */
bool symbol_table_current_scope_info(const SymbolTable *table,
                                     size_t *scope_id,
                                     size_t *scope_depth);

/*
 * Copies a read-only view of a borrowed symbol into info. info->name remains
 * owned by the table and valid until table destruction. Returns false for an
 * invalid symbol or output pointer.
 */
bool symbol_get_info(const Symbol *symbol, SymbolInfo *info);

/*
 * Prints scopes in creation order and symbols in declaration order.
 * Returns false for invalid arguments or an output failure.
 */
bool symbol_table_print(const SymbolTable *table, FILE *output);

#endif
