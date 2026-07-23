#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.h"
#include "codegen/tac.h"
#include "parser/parser.h"
#include "semantic/semantic.h"

int main(int argc, char **argv)
{
    AstNode *program = NULL;
    TacProgram *tac_program = NULL;
    SemanticResult semantic_result;
    ParserStatus parser_status;
    SemanticStatus semantic_status;
    TacStatus tac_status;
    FILE *input;
    int result = SEMANTIC_STATUS_INTERNAL_ERROR;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <source-file>\n", argv[0]);
        return SEMANTIC_STATUS_INTERNAL_ERROR;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "TAC test error: cannot open '%s'\n", argv[1]);
        return SEMANTIC_STATUS_INTERNAL_ERROR;
    }

    parser_status = parser_parse(input, &program);
    if (ferror(input)) {
        fprintf(stderr, "TAC test error: failed while reading '%s'\n",
                argv[1]);
        parser_status = PARSER_STATUS_INTERNAL_ERROR;
    }
    fclose(input);

    if (parser_status != PARSER_STATUS_SUCCESS) {
        ast_destroy(program);
        return (int)parser_status;
    }

    semantic_status = semantic_analyze(
        program, stderr, &semantic_result);
    if (semantic_status != SEMANTIC_STATUS_SUCCESS) {
        ast_destroy(program);
        return (int)semantic_status;
    }

    tac_status = tac_generate(program, &tac_program);
    if (tac_status != TAC_STATUS_SUCCESS) {
        if (tac_status == TAC_STATUS_UNSUPPORTED_NODE) {
            fprintf(stderr,
                    "TAC generation error [TAC_UNSUPPORTED_NODE]: "
                    "control-flow TAC is deferred to M8\n");
        } else {
            fprintf(stderr,
                    "TAC generation error [%s]: generation failed\n",
                    tac_status_name(tac_status));
        }
        ast_destroy(program);
        return SEMANTIC_STATUS_INTERNAL_ERROR;
    }

    if (tac_program_print(stdout, tac_program)) {
        result = 0;
    } else {
        fprintf(stderr, "TAC test error: failed to print TAC\n");
    }

    tac_program_destroy(tac_program);
    ast_destroy(program);
    return result;
}
