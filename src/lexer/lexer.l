%option noyywrap yylineno noinput nounput never-interactive nodefault

%{
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.tab.h"
#include "lexer/lexer.h"

static size_t lexical_error_count = 0;
static bool internal_error = false;

static int report_invalid_token(const char *description);
static int report_internal_error(void);
static int return_identifier(void);
static int return_integer_literal(void);
static int return_float_literal(void);

#define YY_USER_ACTION                                                     \
    do {                                                                   \
        yylloc.first_line = yylineno;                                      \
        yylloc.last_line = yylineno;                                       \
        yylloc.first_column = 1;                                           \
        yylloc.last_column = 1;                                            \
    } while (0);
%}

DIGIT                   [0-9]
IDENTIFIER_PATTERN      [A-Za-z_][A-Za-z0-9_]*
INTEGER_PATTERN         {DIGIT}+
FLOAT_PATTERN           {DIGIT}+\.{DIGIT}+
EXPONENT_PATTERN        {DIGIT}+[eE][+-]?{DIGIT}+
TRAILING_DOT_PATTERN    {DIGIT}+\.
LEADING_DOT_PATTERN     \.{DIGIT}+

%%

"int"                   { return KW_INT; }
"float"                 { return KW_FLOAT; }
"bool"                  { return KW_BOOL; }
"if"                    { return KW_IF; }
"else"                  { return KW_ELSE; }
"while"                 { return KW_WHILE; }
"print"                 { return KW_PRINT; }
"true"                  { return KW_TRUE; }
"false"                 { return KW_FALSE; }

{EXPONENT_PATTERN}      { return report_invalid_token("unsupported numeric literal"); }
{TRAILING_DOT_PATTERN}  { return report_invalid_token("unsupported numeric literal"); }
{LEADING_DOT_PATTERN}   { return report_invalid_token("unsupported numeric literal"); }
{FLOAT_PATTERN}         { return return_float_literal(); }
{INTEGER_PATTERN}       { return return_integer_literal(); }
{IDENTIFIER_PATTERN}    { return return_identifier(); }

"<="                    { return LE; }
">="                    { return GE; }
"=="                    { return EQ; }
"!="                    { return NE; }
"&&"                    { return AND; }
"||"                    { return OR; }

"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return STAR; }
"/"                     { return SLASH; }
"%"                     { return PERCENT; }
"<"                     { return LT; }
">"                     { return GT; }
"!"                     { return NOT; }
"="                     { return ASSIGN; }

"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
";"                     { return SEMICOLON; }

"//"[^\r\n]*           { /* Discard a line comment but leave its newline to yylineno. */ }
[ \t\r]+                { /* Discard horizontal whitespace and CR from CRLF input. */ }
\n                      { /* Flex updates yylineno for the consumed newline. */ }

.                       { return report_invalid_token("invalid token"); }
<<EOF>>                 {
                            yylloc.first_line = yylineno;
                            yylloc.last_line = yylineno;
                            return YYEOF;
                        }

%%

void lexer_set_input(FILE *input)
{
    yyrestart(input);
    yylineno = 1;
    lexical_error_count = 0;
    internal_error = false;
}

SourceLocation lexer_current_location(void)
{
    return source_location_make((size_t)yylineno);
}

const char *lexer_current_lexeme(void)
{
    return yytext;
}

size_t lexer_error_count(void)
{
    return lexical_error_count;
}

bool lexer_had_internal_error(void)
{
    return internal_error;
}

static int report_invalid_token(const char *description)
{
    lexical_error_count++;
    fprintf(stderr,
            "lexical error at line %d [LEX_INVALID_TOKEN]: %s '%s'\n",
            yylineno,
            description,
            yytext);
    return YYUNDEF;
}

static int report_internal_error(void)
{
    if (!internal_error) {
        fprintf(stderr,
                "lexer internal error at line %d: memory allocation failed\n",
                yylineno);
    }
    internal_error = true;
    return YYUNDEF;
}

static int return_identifier(void)
{
    size_t length = (size_t)yyleng;
    char *copy = malloc(length + 1);

    if (copy == NULL) {
        return report_internal_error();
    }

    memcpy(copy, yytext, length);
    copy[length] = '\0';
    yylval.text = copy;
    return IDENTIFIER;
}

static int return_integer_literal(void)
{
    errno = 0;
    yylval.integer_value = strtoll(yytext, NULL, 10);
    if (errno == ERANGE) {
        return report_invalid_token("integer literal out of range");
    }
    return INTEGER_LITERAL;
}

static int return_float_literal(void)
{
    errno = 0;
    yylval.float_value = strtod(yytext, NULL);
    if (errno == ERANGE) {
        return report_invalid_token("floating-point literal out of range");
    }
    return FLOAT_LITERAL;
}
