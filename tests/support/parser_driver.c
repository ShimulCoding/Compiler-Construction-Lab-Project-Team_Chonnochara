#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.h"
#include "parser/parser.h"

int main(int argc, char **argv)
{
    AstNode *program = NULL;
    ParserStatus status;
    FILE *input;
    int result;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <source-file>\n", argv[0]);
        return PARSER_STATUS_INTERNAL_ERROR;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "parser test error: cannot open '%s'\n", argv[1]);
        return PARSER_STATUS_INTERNAL_ERROR;
    }

    status = parser_parse(input, &program);
    if (ferror(input)) {
        fprintf(stderr, "parser test error: failed while reading '%s'\n",
                argv[1]);
        status = PARSER_STATUS_INTERNAL_ERROR;
    }

    result = (int)status;
    if (status == PARSER_STATUS_SUCCESS && !ast_print(stdout, program)) {
        fprintf(stderr, "parser test error: failed to print AST\n");
        result = PARSER_STATUS_INTERNAL_ERROR;
    }

    ast_destroy(program);
    fclose(input);
    return result;
}
