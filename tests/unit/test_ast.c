#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"

typedef bool (*TestFunction)(void);

typedef struct {
    const char *name;
    TestFunction function;
} TestCase;

static SourceLocation at_line(size_t line)
{
    return source_location_make(line);
}

/* These helpers consume their child arguments on both success and failure. */
static AstNode *binary_owned(size_t line, AstBinaryOperator operator,
                             AstNode *left, AstNode *right)
{
    AstNode *node = ast_new_binary_expression(at_line(line), operator,
                                              left, right);

    if (node == NULL) {
        ast_destroy(left);
        ast_destroy(right);
    }

    return node;
}

static AstNode *unary_owned(size_t line, AstUnaryOperator operator,
                            AstNode *operand)
{
    AstNode *node = ast_new_unary_expression(at_line(line), operator,
                                             operand);

    if (node == NULL) {
        ast_destroy(operand);
    }

    return node;
}

static AstNode *declaration_owned(size_t line, ValueType type,
                                  const char *name, AstNode *initializer)
{
    AstNode *node = ast_new_declaration(at_line(line), type, name,
                                        initializer);

    if (node == NULL) {
        ast_destroy(initializer);
    }

    return node;
}

static AstNode *assignment_owned(size_t line, const char *name,
                                 AstNode *value)
{
    AstNode *node = ast_new_assignment(at_line(line), name, value);

    if (node == NULL) {
        ast_destroy(value);
    }

    return node;
}

static AstNode *if_owned(size_t line, AstNode *condition,
                         AstNode *then_block, AstNode *else_block)
{
    AstNode *node = ast_new_if(at_line(line), condition,
                               then_block, else_block);

    if (node == NULL) {
        ast_destroy(condition);
        ast_destroy(then_block);
        ast_destroy(else_block);
    }

    return node;
}

static AstNode *while_owned(size_t line, AstNode *condition, AstNode *body)
{
    AstNode *node = ast_new_while(at_line(line), condition, body);

    if (node == NULL) {
        ast_destroy(condition);
        ast_destroy(body);
    }

    return node;
}

static bool append_owned(AstNode *container, AstNode *statement)
{
    if (statement != NULL && ast_add_statement(container, statement)) {
        return true;
    }

    ast_destroy(statement);
    return false;
}

static bool test_empty_block(void)
{
    AstNode *block = ast_new_block(at_line(1));
    bool passed = block != NULL
        && block->kind == AST_NODE_BLOCK
        && block->data.block.statements.count == 0;

    ast_destroy(block);
    return passed;
}

static bool test_multiple_statements(void)
{
    AstNode *block = ast_new_block(at_line(1));
    bool passed = block != NULL
        && append_owned(block, declaration_owned(2, VALUE_TYPE_INT,
                                                 "first", NULL))
        && append_owned(block, declaration_owned(3, VALUE_TYPE_BOOL,
                                                 "second", NULL))
        && block->data.block.statements.count == 2
        && strcmp(block->data.block.statements.items[0]
                      ->data.declaration.name,
                  "first") == 0
        && strcmp(block->data.block.statements.items[1]
                      ->data.declaration.name,
                  "second") == 0;

    ast_destroy(block);
    return passed;
}

static bool test_nested_block(void)
{
    AstNode *outer = ast_new_block(at_line(1));
    AstNode *inner = ast_new_block(at_line(2));
    bool passed = outer != NULL
        && inner != NULL
        && append_owned(inner, ast_new_print(at_line(3), "value"))
        && append_owned(outer, inner)
        && outer->data.block.statements.count == 1
        && outer->data.block.statements.items[0]->kind == AST_NODE_BLOCK;

    if (outer == NULL) {
        ast_destroy(inner);
    }
    ast_destroy(outer);
    return passed;
}

static bool test_uninitialized_declaration(void)
{
    AstNode *declaration = ast_new_declaration(at_line(4), VALUE_TYPE_INT,
                                               "count", NULL);
    bool passed = declaration != NULL
        && declaration->kind == AST_NODE_DECLARATION
        && declaration->data.declaration.type == VALUE_TYPE_INT
        && strcmp(declaration->data.declaration.name, "count") == 0
        && declaration->data.declaration.initializer == NULL;

    ast_destroy(declaration);
    return passed;
}

static bool test_initialized_declaration(void)
{
    AstNode *declaration = declaration_owned(
        5, VALUE_TYPE_FLOAT, "ratio",
        ast_new_float_literal(at_line(5), 3.5));
    bool passed = declaration != NULL
        && declaration->data.declaration.initializer != NULL
        && declaration->data.declaration.initializer->kind
            == AST_NODE_FLOAT_LITERAL
        && declaration->data.declaration.initializer
               ->data.float_literal.value == 3.5;

    ast_destroy(declaration);
    return passed;
}

static bool test_assignment(void)
{
    AstNode *assignment = assignment_owned(
        6, "count", ast_new_int_literal(at_line(6), 10));
    bool passed = assignment != NULL
        && assignment->kind == AST_NODE_ASSIGNMENT
        && strcmp(assignment->data.assignment.name, "count") == 0
        && assignment->data.assignment.value->kind == AST_NODE_INT_LITERAL;

    ast_destroy(assignment);
    return passed;
}

static bool test_arithmetic_binary_expression(void)
{
    AstNode *product = binary_owned(
        7, AST_BINARY_MULTIPLY,
        ast_new_identifier(at_line(7), "b"),
        ast_new_int_literal(at_line(7), 2));
    AstNode *sum = binary_owned(
        7, AST_BINARY_ADD,
        ast_new_identifier(at_line(7), "a"), product);
    bool passed = sum != NULL
        && sum->kind == AST_NODE_BINARY_EXPRESSION
        && sum->data.binary_expression.operator == AST_BINARY_ADD
        && sum->data.binary_expression.right->kind
            == AST_NODE_BINARY_EXPRESSION
        && sum->data.binary_expression.right->data.binary_expression.operator
            == AST_BINARY_MULTIPLY;

    ast_destroy(sum);
    return passed;
}

static bool test_logical_unary_expression(void)
{
    AstNode *expression = unary_owned(
        8, AST_UNARY_NOT,
        ast_new_bool_literal(at_line(8), false));
    bool passed = expression != NULL
        && expression->kind == AST_NODE_UNARY_EXPRESSION
        && expression->data.unary_expression.operator == AST_UNARY_NOT
        && expression->data.unary_expression.operand->kind
            == AST_NODE_BOOL_LITERAL;

    ast_destroy(expression);
    return passed;
}

static bool test_if_without_else(void)
{
    AstNode *statement = if_owned(
        9, ast_new_bool_literal(at_line(9), true),
        ast_new_block(at_line(9)), NULL);
    bool passed = statement != NULL
        && statement->kind == AST_NODE_IF
        && statement->data.if_statement.then_block->kind == AST_NODE_BLOCK
        && statement->data.if_statement.else_block == NULL;

    ast_destroy(statement);
    return passed;
}

static bool test_if_with_else(void)
{
    AstNode *then_block = ast_new_block(at_line(10));
    AstNode *else_block = ast_new_block(at_line(11));
    AstNode *statement;

    if (then_block == NULL || else_block == NULL
        || !append_owned(then_block, ast_new_print(at_line(10), "left"))
        || !append_owned(else_block, ast_new_print(at_line(11), "right"))) {
        ast_destroy(then_block);
        ast_destroy(else_block);
        return false;
    }

    statement = if_owned(
        10, ast_new_identifier(at_line(10), "condition"),
        then_block, else_block);

    if (statement == NULL) {
        return false;
    }

    bool passed = statement->data.if_statement.else_block != NULL
        && statement->data.if_statement.else_block
               ->data.block.statements.count == 1;

    ast_destroy(statement);
    return passed;
}

static bool test_while(void)
{
    AstNode *condition = binary_owned(
        12, AST_BINARY_GREATER,
        ast_new_identifier(at_line(12), "count"),
        ast_new_int_literal(at_line(12), 0));
    AstNode *statement = while_owned(12, condition,
                                     ast_new_block(at_line(12)));
    bool passed = statement != NULL
        && statement->kind == AST_NODE_WHILE
        && statement->data.while_statement.body->kind == AST_NODE_BLOCK;

    ast_destroy(statement);
    return passed;
}

static bool test_print(void)
{
    AstNode *statement = ast_new_print(at_line(13), "answer");
    bool passed = statement != NULL
        && statement->kind == AST_NODE_PRINT
        && strcmp(statement->data.print_statement.name, "answer") == 0;

    ast_destroy(statement);
    return passed;
}

static bool test_constructor_validation(void)
{
    AstNode *block = ast_new_block(at_line(1));
    AstNode *expression = ast_new_int_literal(at_line(1), 1);
    AstNode *invalid_declaration = ast_new_declaration(
        at_line(1), (ValueType)99, "x", NULL);
    AstNode *invalid_assignment = ast_new_assignment(
        at_line(1), "x", NULL);
    bool passed = block != NULL
        && expression != NULL
        && invalid_declaration == NULL
        && invalid_assignment == NULL
        && !ast_add_statement(block, expression)
        && !ast_print(NULL, block);

    ast_destroy(invalid_declaration);
    ast_destroy(invalid_assignment);
    ast_destroy(expression);
    ast_destroy(block);
    ast_destroy(NULL);
    return passed;
}

static AstNode *build_official_sample(void)
{
    AstNode *program = ast_new_program(at_line(1));
    AstNode *while_body;
    AstNode *then_block;
    AstNode *else_block;
    AstNode *condition;

    if (program == NULL
        || !append_owned(program, declaration_owned(
            1, VALUE_TYPE_INT, "x", NULL))
        || !append_owned(program, declaration_owned(
            2, VALUE_TYPE_INT, "y", NULL))
        || !append_owned(program, declaration_owned(
            3, VALUE_TYPE_BOOL, "flag", NULL))
        || !append_owned(program, assignment_owned(
            5, "x", ast_new_int_literal(at_line(5), 10)))
        || !append_owned(program, assignment_owned(
            6, "y", ast_new_int_literal(at_line(6), 0)))
        || !append_owned(program, assignment_owned(
            7, "flag", ast_new_bool_literal(at_line(7), true)))) {
        ast_destroy(program);
        return NULL;
    }

    while_body = ast_new_block(at_line(9));
    if (while_body == NULL
        || !append_owned(while_body, assignment_owned(
            10, "y", binary_owned(
                10, AST_BINARY_ADD,
                ast_new_identifier(at_line(10), "y"),
                ast_new_identifier(at_line(10), "x"))))
        || !append_owned(while_body, assignment_owned(
            11, "x", binary_owned(
                11, AST_BINARY_SUBTRACT,
                ast_new_identifier(at_line(11), "x"),
                ast_new_int_literal(at_line(11), 1))))) {
        ast_destroy(while_body);
        ast_destroy(program);
        return NULL;
    }

    condition = binary_owned(
        9, AST_BINARY_GREATER,
        ast_new_identifier(at_line(9), "x"),
        ast_new_int_literal(at_line(9), 0));
    if (!append_owned(program, while_owned(9, condition, while_body))) {
        ast_destroy(program);
        return NULL;
    }

    then_block = ast_new_block(at_line(14));
    else_block = ast_new_block(at_line(16));
    if (then_block == NULL || else_block == NULL
        || !append_owned(then_block, ast_new_print(at_line(15), "y"))
        || !append_owned(else_block, ast_new_print(at_line(17), "x"))) {
        ast_destroy(then_block);
        ast_destroy(else_block);
        ast_destroy(program);
        return NULL;
    }

    condition = binary_owned(
        14, AST_BINARY_EQUAL,
        ast_new_identifier(at_line(14), "flag"),
        ast_new_bool_literal(at_line(14), true));
    if (!append_owned(program, if_owned(
            14, condition, then_block, else_block))) {
        ast_destroy(program);
        return NULL;
    }

    return program;
}

static bool streams_are_equal(FILE *first, FILE *second)
{
    int first_character;
    int second_character;

    if (fflush(first) != 0 || fflush(second) != 0) {
        return false;
    }

    rewind(first);
    rewind(second);

    do {
        first_character = fgetc(first);
        second_character = fgetc(second);
        if (first_character != second_character) {
            return false;
        }
    } while (first_character != EOF);

    return ferror(first) == 0 && ferror(second) == 0;
}

static bool copy_stream_to_stdout(FILE *stream)
{
    int character;

    rewind(stream);
    while ((character = fgetc(stream)) != EOF) {
        if (putchar(character) == EOF) {
            return false;
        }
    }

    return ferror(stream) == 0;
}

static bool test_ast_printer_determinism(void)
{
    AstNode *program = build_official_sample();
    FILE *first = tmpfile();
    FILE *second = tmpfile();
    bool passed = program != NULL
        && first != NULL
        && second != NULL
        && ast_print(first, program)
        && ast_print(second, program)
        && streams_are_equal(first, second)
        && printf("AST sample:\n") >= 0
        && copy_stream_to_stdout(first);

    if (first != NULL) {
        fclose(first);
    }
    if (second != NULL) {
        fclose(second);
    }
    ast_destroy(program);
    return passed;
}

static bool test_recursive_cleanup_execution(void)
{
    AstNode *program = build_official_sample();

    if (program == NULL) {
        return false;
    }

    ast_destroy(program);
    return true;
}

int main(void)
{
    const TestCase tests[] = {
        {"empty block", test_empty_block},
        {"multiple statements", test_multiple_statements},
        {"nested block", test_nested_block},
        {"uninitialized declaration", test_uninitialized_declaration},
        {"initialized declaration", test_initialized_declaration},
        {"assignment", test_assignment},
        {"arithmetic binary expression", test_arithmetic_binary_expression},
        {"logical unary expression", test_logical_unary_expression},
        {"if without else", test_if_without_else},
        {"if with else", test_if_with_else},
        {"while", test_while},
        {"print", test_print},
        {"constructor validation", test_constructor_validation},
        {"AST printer determinism", test_ast_printer_determinism},
        {"recursive cleanup execution", test_recursive_cleanup_execution}
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

    printf("AST unit tests: %zu passed\n", test_count);
    return EXIT_SUCCESS;
}
