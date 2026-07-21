%option noyywrap yylineno noinput nounput never-interactive nodefault

%{
#include <stddef.h>
#include <stdio.h>

#include "parser.tab.h"
#include "lexer/lexer.h"

static size_t lexical_error_count = 0;

static int report_invalid_token(const char *description);
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
{FLOAT_PATTERN}         { return FLOAT_LITERAL; }
{INTEGER_PATTERN}       { return INTEGER_LITERAL; }
{IDENTIFIER_PATTERN}    { return IDENTIFIER; }

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

%%

void lexer_set_input(FILE *input)
{
    yyrestart(input);
    yylineno = 1;
    lexical_error_count = 0;
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
