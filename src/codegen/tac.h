#ifndef TAC_H
#define TAC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "ast/ast.h"

typedef enum {
    TAC_STATUS_SUCCESS,
    TAC_STATUS_INVALID_ARGUMENT,
    TAC_STATUS_ALLOCATION_FAILURE,
    TAC_STATUS_UNSUPPORTED_NODE,
    TAC_STATUS_INTERNAL_ERROR
} TacStatus;

typedef enum {
    TAC_INSTRUCTION_ASSIGNMENT,
    TAC_INSTRUCTION_UNARY,
    TAC_INSTRUCTION_BINARY,
    TAC_INSTRUCTION_PRINT,
    TAC_INSTRUCTION_LABEL,
    TAC_INSTRUCTION_JUMP,
    TAC_INSTRUCTION_JUMP_IF_FALSE
} TacInstructionKind;

typedef struct {
    TacInstructionKind kind;
    char *result;
    char *operator_text;
    char *first_operand;
    char *second_operand;
    char *label;
} TacInstruction;

typedef struct {
    TacInstruction *instructions;
    size_t count;
    size_t capacity;
} TacProgram;

/*
 * Generates deterministic TAC from a semantically valid AST. The AST is
 * borrowed. On success, out_program receives an owned TAC program. Its
 * instruction fields are read-only to callers. On failure, out_program is
 * NULL and no partial program is returned.
 */
TacStatus tac_generate(const AstNode *program, TacProgram **out_program);

/*
 * Frees all instructions and copied strings. Passing NULL is safe.
 */
void tac_program_destroy(TacProgram *program);

/*
 * Prints one deterministic instruction per line. An empty program prints
 * nothing. Returns false for invalid arguments or an output failure.
 */
bool tac_program_print(FILE *output, const TacProgram *program);

/*
 * Returns a stable name for diagnostics in test/integration code.
 */
const char *tac_status_name(TacStatus status);

#endif
