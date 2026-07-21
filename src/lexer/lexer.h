#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include "common/source_location.h"

int yylex(void);

void lexer_set_input(FILE *input);
SourceLocation lexer_current_location(void);

/* The returned text remains valid only until the next call to yylex(). */
const char *lexer_current_lexeme(void);

size_t lexer_error_count(void);
bool lexer_had_internal_error(void);

#endif
