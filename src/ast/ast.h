#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "common/source_location.h"
#include "common/value_type.h"

typedef enum {
    AST_NODE_PROGRAM,
    AST_NODE_BLOCK,
    AST_NODE_DECLARATION,
    AST_NODE_ASSIGNMENT,
    AST_NODE_IF,
    AST_NODE_WHILE,
    AST_NODE_PRINT,
    AST_NODE_IDENTIFIER,
    AST_NODE_INT_LITERAL,
    AST_NODE_FLOAT_LITERAL,
    AST_NODE_BOOL_LITERAL,
    AST_NODE_BINARY_EXPRESSION,
    AST_NODE_UNARY_EXPRESSION
} AstNodeKind;

typedef enum {
    AST_BINARY_ADD,
    AST_BINARY_SUBTRACT,
    AST_BINARY_MULTIPLY,
    AST_BINARY_DIVIDE,
    AST_BINARY_REMAINDER,
    AST_BINARY_LESS,
    AST_BINARY_GREATER,
    AST_BINARY_LESS_EQUAL,
    AST_BINARY_GREATER_EQUAL,
    AST_BINARY_EQUAL,
    AST_BINARY_NOT_EQUAL,
    AST_BINARY_LOGICAL_AND,
    AST_BINARY_LOGICAL_OR
} AstBinaryOperator;

typedef enum {
    AST_UNARY_NOT
} AstUnaryOperator;

typedef struct AstNode AstNode;

typedef struct {
    AstNode **items;
    size_t count;
    size_t capacity;
} AstNodeList;

struct AstNode {
    AstNodeKind kind;
    SourceLocation location;

    union {
        struct {
            AstNodeList statements;
        } program;

        struct {
            AstNodeList statements;
        } block;

        struct {
            ValueType type;
            char *name;
            AstNode *initializer;
        } declaration;

        struct {
            char *name;
            AstNode *value;
        } assignment;

        struct {
            AstNode *condition;
            AstNode *then_block;
            AstNode *else_block;
        } if_statement;

        struct {
            AstNode *condition;
            AstNode *body;
        } while_statement;

        struct {
            char *name;
        } print_statement;

        struct {
            char *name;
        } identifier;

        struct {
            long long value;
        } int_literal;

        struct {
            double value;
        } float_literal;

        struct {
            bool value;
        } bool_literal;

        struct {
            AstBinaryOperator operator;
            AstNode *left;
            AstNode *right;
        } binary_expression;

        struct {
            AstUnaryOperator operator;
            AstNode *operand;
        } unary_expression;
    } data;
};

/*
 * Names are copied by constructors. A successful constructor owns its child
 * nodes; on failure, ownership remains with the caller.
 */
AstNode *ast_new_program(SourceLocation location);
AstNode *ast_new_block(SourceLocation location);
bool ast_add_statement(AstNode *container, AstNode *statement);

AstNode *ast_new_declaration(SourceLocation location, ValueType type,
                             const char *name, AstNode *initializer);
AstNode *ast_new_assignment(SourceLocation location, const char *name,
                            AstNode *value);
AstNode *ast_new_if(SourceLocation location, AstNode *condition,
                    AstNode *then_block, AstNode *else_block);
AstNode *ast_new_while(SourceLocation location, AstNode *condition,
                       AstNode *body);
AstNode *ast_new_print(SourceLocation location, const char *name);

AstNode *ast_new_identifier(SourceLocation location, const char *name);
AstNode *ast_new_int_literal(SourceLocation location, long long value);
AstNode *ast_new_float_literal(SourceLocation location, double value);
AstNode *ast_new_bool_literal(SourceLocation location, bool value);
AstNode *ast_new_binary_expression(SourceLocation location,
                                   AstBinaryOperator operator,
                                   AstNode *left, AstNode *right);
AstNode *ast_new_unary_expression(SourceLocation location,
                                  AstUnaryOperator operator,
                                  AstNode *operand);

void ast_destroy(AstNode *node);
bool ast_print(FILE *output, const AstNode *node);

#endif
