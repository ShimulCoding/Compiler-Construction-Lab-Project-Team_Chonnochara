#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "codegen/tac.h"

typedef bool (*TestFunction)(void);

typedef struct {
    const char *name;
    TestFunction function;
} TestCase;

static SourceLocation at_line(size_t line)
{
    return source_location_make(line);
}

static bool append_owned(AstNode *container, AstNode *statement)
{
    if (statement != NULL && ast_add_statement(container, statement)) {
        return true;
    }
    ast_destroy(statement);
    return false;
}

static AstNode *binary_owned(AstBinaryOperator operator,
                             AstNode *left,
                             AstNode *right)
{
    AstNode *node = ast_new_binary_expression(
        at_line(1), operator, left, right);

    if (node == NULL) {
        ast_destroy(left);
        ast_destroy(right);
    }
    return node;
}

static AstNode *unary_owned(AstUnaryOperator operator, AstNode *operand)
{
    AstNode *node = ast_new_unary_expression(
        at_line(1), operator, operand);

    if (node == NULL) {
        ast_destroy(operand);
    }
    return node;
}

static AstNode *declaration_owned(ValueType type,
                                  const char *name,
                                  AstNode *initializer)
{
    AstNode *node = ast_new_declaration(
        at_line(1), type, name, initializer);

    if (node == NULL) {
        ast_destroy(initializer);
    }
    return node;
}

static AstNode *assignment_owned(const char *name, AstNode *value)
{
    AstNode *node = ast_new_assignment(at_line(1), name, value);

    if (node == NULL) {
        ast_destroy(value);
    }
    return node;
}

static char *read_stream(FILE *stream)
{
    long length;
    char *text;

    if (fflush(stream) != 0 || fseek(stream, 0, SEEK_END) != 0) {
        return NULL;
    }
    length = ftell(stream);
    if (length < 0 || fseek(stream, 0, SEEK_SET) != 0) {
        return NULL;
    }

    text = malloc((size_t)length + 1);
    if (text == NULL) {
        return NULL;
    }
    if (fread(text, 1, (size_t)length, stream) != (size_t)length) {
        free(text);
        return NULL;
    }
    text[length] = '\0';
    return text;
}

static char *generate_text(const AstNode *program, TacStatus *status)
{
    TacProgram *tac_program = NULL;
    FILE *output = tmpfile();
    char *text = NULL;

    if (output == NULL) {
        *status = TAC_STATUS_INTERNAL_ERROR;
        return NULL;
    }

    *status = tac_generate(program, &tac_program);
    if (*status == TAC_STATUS_SUCCESS
        && tac_program_print(output, tac_program)) {
        text = read_stream(output);
    }

    tac_program_destroy(tac_program);
    fclose(output);
    return text;
}

static AstNode *build_expression_program(void)
{
    AstNode *program = ast_new_program(at_line(1));

    if (program == NULL
        || !append_owned(program, declaration_owned(
            VALUE_TYPE_INT, "a", NULL))
        || !append_owned(program, declaration_owned(
            VALUE_TYPE_INT, "b",
            ast_new_int_literal(at_line(1), 2)))
        || !append_owned(program, assignment_owned(
            "a", binary_owned(
                AST_BINARY_ADD,
                ast_new_identifier(at_line(1), "b"),
                ast_new_int_literal(at_line(1), 1))))) {
        ast_destroy(program);
        return NULL;
    }
    return program;
}

static bool test_empty_program(void)
{
    AstNode *program = ast_new_program(at_line(1));
    TacProgram *tac_program = NULL;
    FILE *output = tmpfile();
    char *text = NULL;
    bool passed = program != NULL
        && output != NULL
        && tac_generate(program, &tac_program) == TAC_STATUS_SUCCESS
        && tac_program != NULL
        && tac_program->count == 0
        && tac_program_print(output, tac_program);

    if (passed) {
        text = read_stream(output);
        passed = text != NULL && text[0] == '\0';
    }

    free(text);
    if (output != NULL) {
        fclose(output);
    }
    tac_program_destroy(tac_program);
    tac_program_destroy(NULL);
    ast_destroy(program);
    return passed;
}

static bool test_invalid_arguments_and_status_names(void)
{
    AstNode *literal = ast_new_int_literal(at_line(1), 1);
    TacProgram *output = (TacProgram *)literal;
    FILE *stream = tmpfile();
    bool passed = literal != NULL
        && tac_generate(NULL, &output) == TAC_STATUS_INVALID_ARGUMENT
        && output == NULL
        && tac_generate(literal, &output) == TAC_STATUS_INVALID_ARGUMENT
        && output == NULL
        && tac_generate(literal, NULL) == TAC_STATUS_INVALID_ARGUMENT
        && !tac_program_print(NULL, NULL)
        && !tac_program_print(stream, NULL)
        && strcmp(tac_status_name(TAC_STATUS_SUCCESS),
                  "TAC_SUCCESS") == 0
        && strcmp(tac_status_name(TAC_STATUS_UNSUPPORTED_NODE),
                  "TAC_UNSUPPORTED_NODE") == 0;

    if (stream != NULL) {
        fclose(stream);
    }
    ast_destroy(literal);
    return passed;
}

static bool test_instruction_model(void)
{
    AstNode *program = build_expression_program();
    TacProgram *tac_program = NULL;
    bool passed = program != NULL
        && tac_generate(program, &tac_program) == TAC_STATUS_SUCCESS
        && tac_program != NULL
        && tac_program->count == 3
        && tac_program->instructions[0].kind
            == TAC_INSTRUCTION_ASSIGNMENT
        && strcmp(tac_program->instructions[0].result, "b") == 0
        && strcmp(tac_program->instructions[0].first_operand, "2") == 0
        && tac_program->instructions[1].kind == TAC_INSTRUCTION_BINARY
        && strcmp(tac_program->instructions[1].result, "t1") == 0
        && strcmp(tac_program->instructions[1].operator_text, "+") == 0
        && strcmp(tac_program->instructions[1].first_operand, "b") == 0
        && strcmp(tac_program->instructions[1].second_operand, "1") == 0
        && tac_program->instructions[2].kind
            == TAC_INSTRUCTION_ASSIGNMENT
        && strcmp(tac_program->instructions[2].result, "a") == 0
        && strcmp(tac_program->instructions[2].first_operand, "t1") == 0;

    tac_program_destroy(tac_program);
    ast_destroy(program);
    return passed;
}

static bool test_instruction_owns_copied_text(void)
{
    AstNode *program = build_expression_program();
    TacProgram *tac_program = NULL;
    bool passed = program != NULL
        && tac_generate(program, &tac_program) == TAC_STATUS_SUCCESS
        && tac_program != NULL;

    if (passed) {
        strcpy(program->data.program.statements.items[1]
                   ->data.declaration.name,
               "c");
        passed = strcmp(tac_program->instructions[0].result, "b") == 0
            && strcmp(tac_program->instructions[1].first_operand, "b") == 0;
    }

    tac_program_destroy(tac_program);
    ast_destroy(program);
    return passed;
}

static bool test_temporary_reset(void)
{
    AstNode *program = build_expression_program();
    TacProgram *first = NULL;
    TacProgram *second = NULL;
    bool passed = program != NULL
        && tac_generate(program, &first) == TAC_STATUS_SUCCESS
        && tac_generate(program, &second) == TAC_STATUS_SUCCESS
        && first != NULL
        && second != NULL
        && strcmp(first->instructions[1].result, "t1") == 0
        && strcmp(second->instructions[1].result, "t1") == 0;

    tac_program_destroy(first);
    tac_program_destroy(second);
    ast_destroy(program);
    return passed;
}

typedef enum {
    COLLISION_DECLARATION_BEFORE,
    COLLISION_DECLARATION_AFTER,
    COLLISION_INITIALIZED_DECLARATION
} CollisionProgramKind;

static AstNode *build_collision_program(CollisionProgramKind kind)
{
    AstNode *program = ast_new_program(at_line(1));
    AstNode *sum;

    if (program == NULL) {
        return NULL;
    }

    if (kind == COLLISION_DECLARATION_BEFORE
        && !append_owned(program, declaration_owned(
            VALUE_TYPE_INT, "t1", NULL))) {
        ast_destroy(program);
        return NULL;
    }

    if (kind == COLLISION_INITIALIZED_DECLARATION) {
        sum = binary_owned(
            AST_BINARY_ADD,
            ast_new_int_literal(at_line(1), 1),
            ast_new_int_literal(at_line(1), 2));
        if (!append_owned(program, declaration_owned(
                VALUE_TYPE_INT, "t1", sum))) {
            ast_destroy(program);
            return NULL;
        }
        return program;
    }

    if (!append_owned(program, declaration_owned(
            VALUE_TYPE_INT, "x", NULL))) {
        ast_destroy(program);
        return NULL;
    }

    sum = binary_owned(
        AST_BINARY_ADD,
        ast_new_int_literal(at_line(1), 1),
        ast_new_int_literal(at_line(1), 2));
    if (!append_owned(program, assignment_owned("x", sum))) {
        ast_destroy(program);
        return NULL;
    }

    if (kind == COLLISION_DECLARATION_AFTER
        && !append_owned(program, declaration_owned(
            VALUE_TYPE_INT, "t1", NULL))) {
        ast_destroy(program);
        return NULL;
    }
    return program;
}

static bool collision_program_matches(CollisionProgramKind kind,
                                      const char *expected)
{
    AstNode *program = build_collision_program(kind);
    TacStatus status;
    char *text = generate_text(program, &status);
    bool passed = program != NULL
        && status == TAC_STATUS_SUCCESS
        && text != NULL
        && strcmp(text, expected) == 0;

    free(text);
    ast_destroy(program);
    return passed;
}

static bool test_global_temporary_collision_before(void)
{
    return collision_program_matches(
        COLLISION_DECLARATION_BEFORE,
        "t2 = 1 + 2\n"
        "x = t2\n");
}

static bool test_global_temporary_collision_after(void)
{
    return collision_program_matches(
        COLLISION_DECLARATION_AFTER,
        "t2 = 1 + 2\n"
        "x = t2\n");
}

static bool test_initialized_temporary_collision(void)
{
    return collision_program_matches(
        COLLISION_INITIALIZED_DECLARATION,
        "t2 = 1 + 2\n"
        "t1 = t2\n");
}

static bool test_collision_counter_reset(void)
{
    AstNode *program = build_collision_program(
        COLLISION_DECLARATION_AFTER);
    TacStatus first_status;
    TacStatus second_status;
    char *first = generate_text(program, &first_status);
    char *second = generate_text(program, &second_status);
    bool passed = program != NULL
        && first_status == TAC_STATUS_SUCCESS
        && second_status == TAC_STATUS_SUCCESS
        && first != NULL
        && second != NULL
        && strncmp(first, "t2 = ", 5) == 0
        && strcmp(first, second) == 0;

    free(first);
    free(second);
    ast_destroy(program);
    return passed;
}

static bool test_repeated_output_determinism(void)
{
    AstNode *program = build_expression_program();
    TacStatus first_status;
    TacStatus second_status;
    char *first = generate_text(program, &first_status);
    char *second = generate_text(program, &second_status);
    bool passed = program != NULL
        && first_status == TAC_STATUS_SUCCESS
        && second_status == TAC_STATUS_SUCCESS
        && first != NULL
        && second != NULL
        && strcmp(first, second) == 0;

    free(first);
    free(second);
    ast_destroy(program);
    return passed;
}

static AstNode *build_shadow_program(void)
{
    AstNode *program = ast_new_program(at_line(1));
    AstNode *block = ast_new_block(at_line(2));

    if (program == NULL || block == NULL
        || !append_owned(program, declaration_owned(
            VALUE_TYPE_INT, "x",
            ast_new_int_literal(at_line(1), 1)))
        || !append_owned(block, declaration_owned(
            VALUE_TYPE_INT, "x",
            binary_owned(AST_BINARY_ADD,
                         ast_new_identifier(at_line(2), "x"),
                         ast_new_int_literal(at_line(2), 1))))
        || !append_owned(block, ast_new_print(at_line(3), "x"))
        || !append_owned(program, block)
        || !append_owned(program, ast_new_print(at_line(4), "x"))) {
        if (program == NULL) {
            ast_destroy(block);
        }
        ast_destroy(program);
        return NULL;
    }
    return program;
}

static bool test_shadow_storage_and_restoration(void)
{
    const char *expected =
        "x = 1\n"
        "t1 = x + 1\n"
        "x@1 = t1\n"
        "print x@1\n"
        "print x\n";
    AstNode *program = build_shadow_program();
    TacStatus status;
    char *text = generate_text(program, &status);
    bool passed = program != NULL
        && status == TAC_STATUS_SUCCESS
        && text != NULL
        && strcmp(text, expected) == 0;

    free(text);
    ast_destroy(program);
    return passed;
}

static AstNode *build_control_flow_program(AstNodeKind kind)
{
    AstNode *program = ast_new_program(at_line(1));
    AstNode *block = ast_new_block(at_line(1));
    AstNode *statement = NULL;

    if (program == NULL || block == NULL
        || !append_owned(program, declaration_owned(
            VALUE_TYPE_BOOL, "ready",
            ast_new_bool_literal(at_line(1), true)))) {
        ast_destroy(block);
        ast_destroy(program);
        return NULL;
    }

    if (kind == AST_NODE_IF
        && append_owned(block, ast_new_print(at_line(1), "ready"))) {
        statement = ast_new_if(
            at_line(1),
            ast_new_identifier(at_line(1), "ready"),
            block,
            NULL);
    } else if (kind == AST_NODE_WHILE
               && append_owned(block, assignment_owned(
                   "ready",
                   unary_owned(
                       AST_UNARY_NOT,
                       ast_new_identifier(at_line(1), "ready"))))) {
        statement = ast_new_while(
            at_line(1),
            binary_owned(
                AST_BINARY_EQUAL,
                ast_new_identifier(at_line(1), "ready"),
                ast_new_bool_literal(at_line(1), true)),
            block);
    }

    if (statement == NULL) {
        ast_destroy(block);
        ast_destroy(program);
        return NULL;
    }

    if (!append_owned(program, statement)) {
        ast_destroy(program);
        return NULL;
    }
    return program;
}

static bool test_if_lowering(void)
{
    AstNode *program = build_control_flow_program(AST_NODE_IF);
    const char *expected =
        "ready = true\n"
        "ifFalse ready goto .L1\n"
        "print ready\n"
        ".L1:\n";
    TacStatus status;
    char *text = generate_text(program, &status);
    bool passed = program != NULL
        && status == TAC_STATUS_SUCCESS
        && text != NULL
        && strcmp(text, expected) == 0;

    free(text);
    ast_destroy(program);
    return passed;
}

static bool test_while_lowering_and_condition_placement(void)
{
    AstNode *program = build_control_flow_program(AST_NODE_WHILE);
    const char *expected =
        "ready = true\n"
        ".L1:\n"
        "t1 = ready == true\n"
        "ifFalse t1 goto .L2\n"
        "t2 = ! ready\n"
        "ready = t2\n"
        "goto .L1\n"
        ".L2:\n";
    TacStatus status;
    char *text = generate_text(program, &status);
    bool passed = program != NULL
        && status == TAC_STATUS_SUCCESS
        && text != NULL
        && strcmp(text, expected) == 0;

    free(text);
    ast_destroy(program);
    return passed;
}

static AstNode *build_if_else_program(void)
{
    AstNode *program = ast_new_program(at_line(1));
    AstNode *then_block = ast_new_block(at_line(1));
    AstNode *else_block = ast_new_block(at_line(1));
    AstNode *statement;

    if (program == NULL || then_block == NULL || else_block == NULL
        || !append_owned(program, declaration_owned(
            VALUE_TYPE_BOOL, "ready",
            ast_new_bool_literal(at_line(1), true)))
        || !append_owned(then_block,
                         ast_new_print(at_line(1), "ready"))
        || !append_owned(else_block, assignment_owned(
            "ready", ast_new_bool_literal(at_line(1), false)))) {
        ast_destroy(then_block);
        ast_destroy(else_block);
        ast_destroy(program);
        return NULL;
    }

    statement = ast_new_if(
        at_line(1),
        ast_new_identifier(at_line(1), "ready"),
        then_block,
        else_block);
    if (statement == NULL) {
        ast_destroy(then_block);
        ast_destroy(else_block);
        ast_destroy(program);
        return NULL;
    }
    if (!append_owned(program, statement)) {
        ast_destroy(program);
        return NULL;
    }
    return program;
}

static bool test_control_instruction_model(void)
{
    AstNode *program = build_if_else_program();
    TacProgram *tac_program = NULL;
    bool passed = program != NULL
        && tac_generate(program, &tac_program) == TAC_STATUS_SUCCESS
        && tac_program != NULL
        && tac_program->count == 7
        && tac_program->instructions[1].kind
            == TAC_INSTRUCTION_JUMP_IF_FALSE
        && strcmp(tac_program->instructions[1].first_operand,
                  "ready") == 0
        && strcmp(tac_program->instructions[1].label, ".L1") == 0
        && tac_program->instructions[3].kind
            == TAC_INSTRUCTION_JUMP
        && strcmp(tac_program->instructions[3].label, ".L2") == 0
        && tac_program->instructions[4].kind
            == TAC_INSTRUCTION_LABEL
        && strcmp(tac_program->instructions[4].label, ".L1") == 0
        && tac_program->instructions[6].kind
            == TAC_INSTRUCTION_LABEL
        && strcmp(tac_program->instructions[6].label, ".L2") == 0;

    tac_program_destroy(tac_program);
    ast_destroy(program);
    return passed;
}

static bool test_label_and_temporary_counters_reset(void)
{
    AstNode *program = build_control_flow_program(AST_NODE_WHILE);
    TacStatus first_status;
    TacStatus second_status;
    char *first = generate_text(program, &first_status);
    char *second = generate_text(program, &second_status);
    bool passed = program != NULL
        && first_status == TAC_STATUS_SUCCESS
        && second_status == TAC_STATUS_SUCCESS
        && first != NULL
        && second != NULL
        && strstr(first, ".L1:\nt1 = ready == true\n") != NULL
        && strcmp(first, second) == 0;

    free(first);
    free(second);
    ast_destroy(program);
    return passed;
}

static bool test_corrupt_statement_returns_no_partial_program(void)
{
    AstNode *program = ast_new_program(at_line(1));
    AstNode *statement = ast_new_print(at_line(1), "ready");
    TacProgram *tac_program = (TacProgram *)program;
    bool passed;

    if (program == NULL || statement == NULL
        || !append_owned(program, declaration_owned(
            VALUE_TYPE_BOOL, "ready",
            ast_new_bool_literal(at_line(1), true)))
        || !append_owned(program, statement)) {
        if (program == NULL) {
            ast_destroy(statement);
        }
        ast_destroy(program);
        return false;
    }

    statement->kind = (AstNodeKind)999;
    passed = tac_generate(program, &tac_program)
        == TAC_STATUS_INTERNAL_ERROR
        && tac_program == NULL;
    statement->kind = AST_NODE_PRINT;
    ast_destroy(program);
    return passed;
}

static bool test_print_validation(void)
{
    TacProgram invalid = {
        .instructions = NULL,
        .count = 1,
        .capacity = 0
    };
    FILE *output = tmpfile();
    bool passed = output != NULL
        && !tac_program_print(output, &invalid);

    if (output != NULL) {
        fclose(output);
    }
    return passed;
}

int main(void)
{
    const TestCase tests[] = {
        {"empty TAC program", test_empty_program},
        {"invalid arguments and status names",
         test_invalid_arguments_and_status_names},
        {"instruction data model", test_instruction_model},
        {"owned instruction strings", test_instruction_owns_copied_text},
        {"temporary numbering resets", test_temporary_reset},
        {"global t1 before temporary generation",
         test_global_temporary_collision_before},
        {"global t1 after temporary generation",
         test_global_temporary_collision_after},
        {"initialized global t1 avoids collision",
         test_initialized_temporary_collision},
        {"collision-aware counter resets",
         test_collision_counter_reset},
        {"repeated output determinism", test_repeated_output_determinism},
        {"shadow storage and outer restoration",
         test_shadow_storage_and_restoration},
        {"if lowers to conditional jump and label",
         test_if_lowering},
        {"while reevaluates condition after start label",
         test_while_lowering_and_condition_placement},
        {"control instruction data model",
         test_control_instruction_model},
        {"label and temporary counters reset",
         test_label_and_temporary_counters_reset},
        {"corrupt statement returns no partial TAC",
         test_corrupt_statement_returns_no_partial_program},
        {"printer rejects invalid programs", test_print_validation}
    };
    const size_t test_count = sizeof(tests) / sizeof(tests[0]);
    size_t index;

    for (index = 0; index < test_count; index++) {
        if (!tests[index].function()) {
            fprintf(stderr, "FAIL: %s\n", tests[index].name);
            return EXIT_FAILURE;
        }
        printf("PASS: %s\n", tests[index].name);
    }

    printf("TAC unit tests: %zu passed\n", test_count);
    return EXIT_SUCCESS;
}
