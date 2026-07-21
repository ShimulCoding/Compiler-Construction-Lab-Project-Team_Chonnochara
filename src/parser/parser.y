%code requires {
#include "ast/ast.h"
}

%code {
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer/lexer.h"
#include "parser/parser.h"

static size_t syntax_error_count = 0;
static size_t seen_lexical_error_count = 0;
static bool seen_lexer_internal_error = false;
static bool parser_internal_error = false;

static SourceLocation parser_location(YYLTYPE location);
static AstNodeList *parser_statement_list_new(void);
static bool parser_statement_list_append(AstNodeList *list,
                                         AstNode *statement);
static bool parser_attach_statements(AstNode *container,
                                     AstNodeList *statements);
static void parser_statement_list_destroy(AstNodeList *list);
static void yyerror(AstNode **parsed_program, const char *message);
}

%define parse.error detailed
%expect 0
%locations
%parse-param { AstNode **parsed_program }

%union {
    AstNode *node;
    AstNodeList *statements;
    char *text;
    long long integer_value;
    double float_value;
    ValueType value_type;
    AstBinaryOperator binary_operator;
}

%token KW_INT "int"
%token KW_FLOAT "float"
%token KW_BOOL "bool"
%token KW_IF "if"
%token KW_ELSE "else"
%token KW_WHILE "while"
%token KW_PRINT "print"
%token KW_TRUE "true"
%token KW_FALSE "false"
%token <text> IDENTIFIER "identifier"
%token <integer_value> INTEGER_LITERAL "integer literal"
%token <float_value> FLOAT_LITERAL "floating-point literal"
%token PLUS "+"
%token MINUS "-"
%token STAR "*"
%token SLASH "/"
%token PERCENT "%"
%token LT "<"
%token GT ">"
%token LE "<="
%token GE ">="
%token EQ "=="
%token NE "!="
%token AND "&&"
%token OR "||"
%token NOT "!"
%token ASSIGN "="
%token LBRACE "{"
%token RBRACE "}"
%token LPAREN "("
%token RPAREN ")"
%token SEMICOLON ";"

%type <node> program statement declaration assignment if_statement
%type <node> else_part while_statement print_statement block expression
%type <node> logical_or_expression logical_and_expression
%type <node> equality_expression relational_expression
%type <node> additive_expression multiplicative_expression
%type <node> unary_expression primary_expression
%type <statements> statement_list optional_statement_list
%type <value_type> type
%type <binary_operator> equality_operator relational_operator

%destructor { ast_destroy($$); } <node>
%destructor { parser_statement_list_destroy($$); } <statements>
%destructor { free($$); } <text>

%start program

%%

program:
      statement_list
        {
            AstNode *program = ast_new_program(parser_location(@1));

            if (program == NULL) {
                YYNOMEM;
            }
            if (!parser_attach_statements(program, $1)) {
                ast_destroy(program);
                YYNOMEM;
            }

            parser_statement_list_destroy($1);
            $1 = NULL;
            *parsed_program = program;
            $$ = NULL;
        }
;

statement_list:
      statement
        {
            $$ = parser_statement_list_new();
            if ($$ == NULL) {
                YYNOMEM;
            }
            if (!parser_statement_list_append($$, $1)) {
                parser_statement_list_destroy($$);
                $$ = NULL;
                YYNOMEM;
            }
        }
    | statement_list statement
        {
            $$ = $1;
            if (!parser_statement_list_append($$, $2)) {
                YYNOMEM;
            }
        }
;

statement:
      declaration SEMICOLON
        { $$ = $1; }
    | assignment SEMICOLON
        { $$ = $1; }
    | if_statement
        { $$ = $1; }
    | while_statement
        { $$ = $1; }
    | print_statement SEMICOLON
        { $$ = $1; }
    | block
        { $$ = $1; }
    | error SEMICOLON
        {
            yyerrok;
            $$ = NULL;
        }
;

declaration:
      type IDENTIFIER
        {
            $$ = ast_new_declaration(parser_location(@1), $1, $2, NULL);
            if ($$ == NULL) {
                YYNOMEM;
            }
            free($2);
            $2 = NULL;
        }
    | type IDENTIFIER ASSIGN expression
        {
            $$ = ast_new_declaration(parser_location(@1), $1, $2, $4);
            if ($$ == NULL) {
                YYNOMEM;
            }
            free($2);
            $2 = NULL;
        }
;

type:
      KW_INT
        { $$ = VALUE_TYPE_INT; }
    | KW_FLOAT
        { $$ = VALUE_TYPE_FLOAT; }
    | KW_BOOL
        { $$ = VALUE_TYPE_BOOL; }
;

assignment:
      IDENTIFIER ASSIGN expression
        {
            $$ = ast_new_assignment(parser_location(@1), $1, $3);
            if ($$ == NULL) {
                YYNOMEM;
            }
            free($1);
            $1 = NULL;
        }
;

if_statement:
      KW_IF LPAREN expression RPAREN block else_part
        {
            $$ = ast_new_if(parser_location(@1), $3, $5, $6);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
;

else_part:
      KW_ELSE block
        { $$ = $2; }
    | %empty
        { $$ = NULL; }
;

while_statement:
      KW_WHILE LPAREN expression RPAREN block
        {
            $$ = ast_new_while(parser_location(@1), $3, $5);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
;

print_statement:
      KW_PRINT IDENTIFIER
        {
            $$ = ast_new_print(parser_location(@1), $2);
            if ($$ == NULL) {
                YYNOMEM;
            }
            free($2);
            $2 = NULL;
        }
;

block:
      LBRACE optional_statement_list RBRACE
        {
            AstNode *block = ast_new_block(parser_location(@1));

            if (block == NULL) {
                YYNOMEM;
            }
            if (!parser_attach_statements(block, $2)) {
                ast_destroy(block);
                YYNOMEM;
            }

            parser_statement_list_destroy($2);
            $2 = NULL;
            $$ = block;
        }
    | LBRACE error RBRACE
        {
            yyerrok;
            $$ = ast_new_block(parser_location(@1));
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
;

optional_statement_list:
      %empty
        {
            $$ = parser_statement_list_new();
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
    | statement_list
        { $$ = $1; }
;

expression:
      logical_or_expression
        { $$ = $1; }
;

logical_or_expression:
      logical_and_expression
        { $$ = $1; }
    | logical_or_expression OR logical_and_expression
        {
            $$ = ast_new_binary_expression(parser_location(@2),
                                            AST_BINARY_LOGICAL_OR,
                                            $1, $3);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
;

logical_and_expression:
      equality_expression
        { $$ = $1; }
    | logical_and_expression AND equality_expression
        {
            $$ = ast_new_binary_expression(parser_location(@2),
                                            AST_BINARY_LOGICAL_AND,
                                            $1, $3);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
;

equality_expression:
      relational_expression
        { $$ = $1; }
    | relational_expression equality_operator relational_expression
        {
            $$ = ast_new_binary_expression(parser_location(@2), $2,
                                            $1, $3);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
;

equality_operator:
      EQ
        { $$ = AST_BINARY_EQUAL; }
    | NE
        { $$ = AST_BINARY_NOT_EQUAL; }
;

relational_expression:
      additive_expression
        { $$ = $1; }
    | additive_expression relational_operator additive_expression
        {
            $$ = ast_new_binary_expression(parser_location(@2), $2,
                                            $1, $3);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
;

relational_operator:
      LT
        { $$ = AST_BINARY_LESS; }
    | GT
        { $$ = AST_BINARY_GREATER; }
    | LE
        { $$ = AST_BINARY_LESS_EQUAL; }
    | GE
        { $$ = AST_BINARY_GREATER_EQUAL; }
;

additive_expression:
      multiplicative_expression
        { $$ = $1; }
    | additive_expression PLUS multiplicative_expression
        {
            $$ = ast_new_binary_expression(parser_location(@2),
                                            AST_BINARY_ADD, $1, $3);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
    | additive_expression MINUS multiplicative_expression
        {
            $$ = ast_new_binary_expression(parser_location(@2),
                                            AST_BINARY_SUBTRACT, $1, $3);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
;

multiplicative_expression:
      unary_expression
        { $$ = $1; }
    | multiplicative_expression STAR unary_expression
        {
            $$ = ast_new_binary_expression(parser_location(@2),
                                            AST_BINARY_MULTIPLY, $1, $3);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
    | multiplicative_expression SLASH unary_expression
        {
            $$ = ast_new_binary_expression(parser_location(@2),
                                            AST_BINARY_DIVIDE, $1, $3);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
    | multiplicative_expression PERCENT unary_expression
        {
            $$ = ast_new_binary_expression(parser_location(@2),
                                            AST_BINARY_REMAINDER, $1, $3);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
;

unary_expression:
      NOT unary_expression
        {
            $$ = ast_new_unary_expression(parser_location(@1),
                                           AST_UNARY_NOT, $2);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
    | primary_expression
        { $$ = $1; }
;

primary_expression:
      IDENTIFIER
        {
            $$ = ast_new_identifier(parser_location(@1), $1);
            if ($$ == NULL) {
                YYNOMEM;
            }
            free($1);
            $1 = NULL;
        }
    | INTEGER_LITERAL
        {
            $$ = ast_new_int_literal(parser_location(@1), $1);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
    | FLOAT_LITERAL
        {
            $$ = ast_new_float_literal(parser_location(@1), $1);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
    | KW_TRUE
        {
            $$ = ast_new_bool_literal(parser_location(@1), true);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
    | KW_FALSE
        {
            $$ = ast_new_bool_literal(parser_location(@1), false);
            if ($$ == NULL) {
                YYNOMEM;
            }
        }
    | LPAREN expression RPAREN
        { $$ = $2; }
;

%%

static SourceLocation parser_location(YYLTYPE location)
{
    size_t line = location.first_line > 0
        ? (size_t)location.first_line
        : 1;

    return source_location_make(line);
}

static AstNodeList *parser_statement_list_new(void)
{
    return calloc(1, sizeof(AstNodeList));
}

static bool parser_statement_list_append(AstNodeList *list,
                                         AstNode *statement)
{
    AstNode **resized_items;
    size_t new_capacity;

    if (list == NULL) {
        return false;
    }
    if (statement == NULL) {
        return true;
    }

    if (list->count == list->capacity) {
        if (list->capacity > SIZE_MAX / 2) {
            return false;
        }

        new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        if (new_capacity > SIZE_MAX / sizeof(*list->items)) {
            return false;
        }

        resized_items = realloc(list->items,
                                new_capacity * sizeof(*list->items));
        if (resized_items == NULL) {
            return false;
        }

        list->items = resized_items;
        list->capacity = new_capacity;
    }

    list->items[list->count] = statement;
    list->count++;
    return true;
}

static bool parser_attach_statements(AstNode *container,
                                     AstNodeList *statements)
{
    size_t index;

    if (container == NULL || statements == NULL) {
        return false;
    }

    for (index = 0; index < statements->count; index++) {
        AstNode *statement = statements->items[index];

        if (!ast_add_statement(container, statement)) {
            return false;
        }
        statements->items[index] = NULL;
    }

    return true;
}

static void parser_statement_list_destroy(AstNodeList *list)
{
    size_t index;

    if (list == NULL) {
        return;
    }

    for (index = 0; index < list->count; index++) {
        ast_destroy(list->items[index]);
    }
    free(list->items);
    free(list);
}

static void yyerror(AstNode **parsed_program, const char *message)
{
    const char syntax_prefix[] = "syntax error, ";
    const char *detail = message;
    size_t current_lexical_errors = lexer_error_count();
    size_t line = yylloc.first_line > 0
        ? (size_t)yylloc.first_line
        : lexer_current_location().line;

    (void)parsed_program;

    if (current_lexical_errors > seen_lexical_error_count) {
        seen_lexical_error_count = current_lexical_errors;
        return;
    }
    if (lexer_had_internal_error() && !seen_lexer_internal_error) {
        seen_lexer_internal_error = true;
        return;
    }
    if (message != NULL && strcmp(message, "memory exhausted") == 0) {
        parser_internal_error = true;
        fprintf(stderr, "parser internal error: memory exhausted\n");
        return;
    }

    if (detail == NULL || detail[0] == '\0') {
        detail = "unexpected input";
    } else if (strncmp(detail, syntax_prefix,
                       sizeof(syntax_prefix) - 1) == 0) {
        detail += sizeof(syntax_prefix) - 1;
    } else if (strcmp(detail, "syntax error") == 0) {
        detail = "unexpected input";
    }

    syntax_error_count++;
    fprintf(stderr,
            "syntax error at line %zu [SYN_UNEXPECTED_TOKEN]: %s\n",
            line,
            detail);
}

ParserStatus parser_parse(FILE *input, AstNode **parsed_program)
{
    int parse_result;

    if (input == NULL || parsed_program == NULL) {
        return PARSER_STATUS_INTERNAL_ERROR;
    }

    *parsed_program = NULL;
    syntax_error_count = 0;
    seen_lexical_error_count = 0;
    seen_lexer_internal_error = false;
    parser_internal_error = false;
    lexer_set_input(input);

    parse_result = yyparse(parsed_program);

    if (lexer_had_internal_error() || parser_internal_error
        || parse_result == 2) {
        ast_destroy(*parsed_program);
        *parsed_program = NULL;
        return PARSER_STATUS_INTERNAL_ERROR;
    }
    if (lexer_error_count() != 0) {
        ast_destroy(*parsed_program);
        *parsed_program = NULL;
        return PARSER_STATUS_LEXICAL_ERROR;
    }
    if (syntax_error_count != 0 || parse_result != 0) {
        ast_destroy(*parsed_program);
        *parsed_program = NULL;
        return PARSER_STATUS_SYNTAX_ERROR;
    }
    if (*parsed_program == NULL) {
        fprintf(stderr, "parser internal error: no AST was produced\n");
        return PARSER_STATUS_INTERNAL_ERROR;
    }

    return PARSER_STATUS_SUCCESS;
}

size_t parser_syntax_error_count(void)
{
    return syntax_error_count;
}
