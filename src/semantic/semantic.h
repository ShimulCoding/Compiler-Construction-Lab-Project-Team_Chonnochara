#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stddef.h>
#include <stdio.h>

#include "ast/ast.h"

typedef enum {
    SEMANTIC_STATUS_SUCCESS = 0,
    SEMANTIC_STATUS_ERRORS = 3,
    SEMANTIC_STATUS_INTERNAL_ERROR = 4
} SemanticStatus;

typedef struct {
    size_t error_count;
} SemanticResult;

/*
 * Analyzes one parser-built program AST and writes source-facing semantic
 * diagnostics to diagnostics. The AST remains owned by the caller.
 *
 * result receives the number of semantic diagnostics. Invalid arguments or
 * internal allocation/data-structure failures return INTERNAL_ERROR.
 */
SemanticStatus semantic_analyze(const AstNode *program,
                                FILE *diagnostics,
                                SemanticResult *result);

#endif
