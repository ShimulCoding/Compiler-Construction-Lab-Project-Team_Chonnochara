#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.tab.h"
#include "lexer/lexer.h"

static const char *token_name(int token)
{
    switch (token) {
    case KW_INT: return "KW_INT";
    case KW_FLOAT: return "KW_FLOAT";
    case KW_BOOL: return "KW_BOOL";
    case KW_IF: return "KW_IF";
    case KW_ELSE: return "KW_ELSE";
    case KW_WHILE: return "KW_WHILE";
    case KW_PRINT: return "KW_PRINT";
    case KW_TRUE: return "KW_TRUE";
    case KW_FALSE: return "KW_FALSE";
    case IDENTIFIER: return "IDENTIFIER";
    case INTEGER_LITERAL: return "INTEGER_LITERAL";
    case FLOAT_LITERAL: return "FLOAT_LITERAL";
    case PLUS: return "PLUS";
    case MINUS: return "MINUS";
    case STAR: return "STAR";
    case SLASH: return "SLASH";
    case PERCENT: return "PERCENT";
    case LT: return "LT";
    case GT: return "GT";
    case LE: return "LE";
    case GE: return "GE";
    case EQ: return "EQ";
    case NE: return "NE";
    case AND: return "AND";
    case OR: return "OR";
    case NOT: return "NOT";
    case ASSIGN: return "ASSIGN";
    case LBRACE: return "LBRACE";
    case RBRACE: return "RBRACE";
    case LPAREN: return "LPAREN";
    case RPAREN: return "RPAREN";
    case SEMICOLON: return "SEMICOLON";
    default: return NULL;
    }
}

static bool token_has_display_lexeme(int token)
{
    return token == IDENTIFIER
        || token == INTEGER_LITERAL
        || token == FLOAT_LITERAL;
}

static bool print_token(int token)
{
    const char *name = token_name(token);
    SourceLocation location = lexer_current_location();

    if (name == NULL) {
        fprintf(stderr, "lexer test error: unknown token value %d\n", token);
        return false;
    }

    printf("%s(line=%zu", name, location.line);
    if (token_has_display_lexeme(token)) {
        printf(", lexeme=%s", lexer_current_lexeme());
    }
    puts(")");
    return true;
}

static void release_token_value(int token)
{
    if (token == IDENTIFIER) {
        free(yylval.text);
        yylval.text = NULL;
    }
}

int main(int argc, char **argv)
{
    FILE *input;
    int token;
    int result = EXIT_SUCCESS;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <source-file>\n", argv[0]);
        return 2;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "lexer test error: cannot open '%s'\n", argv[1]);
        return 2;
    }

    lexer_set_input(input);
    while ((token = yylex()) != YYEOF) {
        if (token == YYUNDEF) {
            result = EXIT_FAILURE;
            break;
        }
        if (!print_token(token)) {
            release_token_value(token);
            result = EXIT_FAILURE;
            break;
        }
        release_token_value(token);
    }

    if (ferror(input)) {
        fprintf(stderr, "lexer test error: failed while reading '%s'\n", argv[1]);
        result = EXIT_FAILURE;
    }
    if (lexer_error_count() != 0) {
        result = EXIT_FAILURE;
    }

    fclose(input);
    return result;
}
