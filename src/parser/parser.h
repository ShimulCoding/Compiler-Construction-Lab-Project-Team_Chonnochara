#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdio.h>

#include "ast/ast.h"

typedef enum {
    PARSER_STATUS_SUCCESS = 0,
    PARSER_STATUS_LEXICAL_ERROR = 1,
    PARSER_STATUS_SYNTAX_ERROR = 2,
    PARSER_STATUS_INTERNAL_ERROR = 4
} ParserStatus;

/* On success, the caller owns the returned program AST. */
ParserStatus parser_parse(FILE *input, AstNode **parsed_program);
size_t parser_syntax_error_count(void);

#endif
