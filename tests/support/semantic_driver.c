#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.h"
#include "parser/parser.h"
#include "semantic/semantic.h"

int main(int argc, char **argv)
{
    AstNode *program = NULL;
    SemanticResult semantic_result;
    ParserStatus parser_status;
    SemanticStatus semantic_status;
    FILE *input;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <source-file>\n", argv[0]);
        return SEMANTIC_STATUS_INTERNAL_ERROR;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "semantic test error: cannot open '%s'\n", argv[1]);
        return SEMANTIC_STATUS_INTERNAL_ERROR;
    }

    parser_status = parser_parse(input, &program);
    if (ferror(input)) {
        fprintf(stderr, "semantic test error: failed while reading '%s'\n",
                argv[1]);
        parser_status = PARSER_STATUS_INTERNAL_ERROR;
    }
    fclose(input);

    if (parser_status != PARSER_STATUS_SUCCESS) {
        ast_destroy(program);
        return (int)parser_status;
    }

    semantic_status = semantic_analyze(program, stderr, &semantic_result);
    ast_destroy(program);
    return (int)semantic_status;
}
