/*
 * M2 token interface only. M4 will replace the placeholder production with
 * the complete approved grammar and AST-building actions.
 */

%code requires {
#include "common/source_location.h"
}

%token KW_INT KW_FLOAT KW_BOOL
%token KW_IF KW_ELSE KW_WHILE KW_PRINT KW_TRUE KW_FALSE
%token IDENTIFIER INTEGER_LITERAL FLOAT_LITERAL
%token PLUS MINUS STAR SLASH PERCENT
%token LT GT LE GE EQ NE
%token AND OR NOT ASSIGN
%token LBRACE RBRACE LPAREN RPAREN SEMICOLON

%start token_interface

%%

token_interface:
      KW_INT
    | KW_FLOAT
    | KW_BOOL
    | KW_IF
    | KW_ELSE
    | KW_WHILE
    | KW_PRINT
    | KW_TRUE
    | KW_FALSE
    | IDENTIFIER
    | INTEGER_LITERAL
    | FLOAT_LITERAL
    | PLUS
    | MINUS
    | STAR
    | SLASH
    | PERCENT
    | LT
    | GT
    | LE
    | GE
    | EQ
    | NE
    | AND
    | OR
    | NOT
    | ASSIGN
    | LBRACE
    | RBRACE
    | LPAREN
    | RPAREN
    | SEMICOLON
;

%%
