#include "ast/ast.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static AstNode *allocate_node(AstNodeKind kind, SourceLocation location)
{
    AstNode *node = calloc(1, sizeof(*node));

    if (node != NULL) {
        node->kind = kind;
        node->location = location;
    }

    return node;
}

static char *copy_text(const char *text)
{
    size_t length;
    char *copy;

    if (text == NULL || text[0] == '\0') {
        return NULL;
    }

    length = strlen(text);
    if (length == SIZE_MAX) {
        return NULL;
    }

    copy = malloc(length + 1);
    if (copy != NULL) {
        memcpy(copy, text, length + 1);
    }

    return copy;
}

static bool is_value_type(ValueType type)
{
    return type >= VALUE_TYPE_INT && type <= VALUE_TYPE_BOOL;
}

static bool is_binary_operator(AstBinaryOperator operator)
{
    return operator >= AST_BINARY_ADD && operator <= AST_BINARY_LOGICAL_OR;
}

static bool is_unary_operator(AstUnaryOperator operator)
{
    return operator == AST_UNARY_NOT;
}

static bool is_expression(const AstNode *node)
{
    if (node == NULL) {
        return false;
    }

    return node->kind == AST_NODE_IDENTIFIER
        || node->kind == AST_NODE_INT_LITERAL
        || node->kind == AST_NODE_FLOAT_LITERAL
        || node->kind == AST_NODE_BOOL_LITERAL
        || node->kind == AST_NODE_BINARY_EXPRESSION
        || node->kind == AST_NODE_UNARY_EXPRESSION;
}

static bool is_statement(const AstNode *node)
{
    if (node == NULL) {
        return false;
    }

    return node->kind == AST_NODE_BLOCK
        || node->kind == AST_NODE_DECLARATION
        || node->kind == AST_NODE_ASSIGNMENT
        || node->kind == AST_NODE_IF
        || node->kind == AST_NODE_WHILE
        || node->kind == AST_NODE_PRINT;
}

static bool append_to_list(AstNodeList *list, AstNode *node)
{
    AstNode **resized_items;
    size_t new_capacity;

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

    list->items[list->count] = node;
    list->count++;
    return true;
}

AstNode *ast_new_program(SourceLocation location)
{
    return allocate_node(AST_NODE_PROGRAM, location);
}

AstNode *ast_new_block(SourceLocation location)
{
    return allocate_node(AST_NODE_BLOCK, location);
}

bool ast_add_statement(AstNode *container, AstNode *statement)
{
    AstNodeList *statements;

    if (container == NULL || !is_statement(statement)) {
        return false;
    }

    if (container->kind == AST_NODE_PROGRAM) {
        statements = &container->data.program.statements;
    } else if (container->kind == AST_NODE_BLOCK) {
        statements = &container->data.block.statements;
    } else {
        return false;
    }

    return append_to_list(statements, statement);
}

AstNode *ast_new_declaration(SourceLocation location, ValueType type,
                             const char *name, AstNode *initializer)
{
    AstNode *node;
    char *owned_name;

    if (!is_value_type(type)
        || (initializer != NULL && !is_expression(initializer))) {
        return NULL;
    }

    owned_name = copy_text(name);
    if (owned_name == NULL) {
        return NULL;
    }

    node = allocate_node(AST_NODE_DECLARATION, location);
    if (node == NULL) {
        free(owned_name);
        return NULL;
    }

    node->data.declaration.type = type;
    node->data.declaration.name = owned_name;
    node->data.declaration.initializer = initializer;
    return node;
}

AstNode *ast_new_assignment(SourceLocation location, const char *name,
                            AstNode *value)
{
    AstNode *node;
    char *owned_name;

    if (!is_expression(value)) {
        return NULL;
    }

    owned_name = copy_text(name);
    if (owned_name == NULL) {
        return NULL;
    }

    node = allocate_node(AST_NODE_ASSIGNMENT, location);
    if (node == NULL) {
        free(owned_name);
        return NULL;
    }

    node->data.assignment.name = owned_name;
    node->data.assignment.value = value;
    return node;
}

AstNode *ast_new_if(SourceLocation location, AstNode *condition,
                    AstNode *then_block, AstNode *else_block)
{
    AstNode *node;

    if (!is_expression(condition)
        || then_block == NULL || then_block->kind != AST_NODE_BLOCK
        || (else_block != NULL && else_block->kind != AST_NODE_BLOCK)) {
        return NULL;
    }

    node = allocate_node(AST_NODE_IF, location);
    if (node == NULL) {
        return NULL;
    }

    node->data.if_statement.condition = condition;
    node->data.if_statement.then_block = then_block;
    node->data.if_statement.else_block = else_block;
    return node;
}

AstNode *ast_new_while(SourceLocation location, AstNode *condition,
                       AstNode *body)
{
    AstNode *node;

    if (!is_expression(condition)
        || body == NULL || body->kind != AST_NODE_BLOCK) {
        return NULL;
    }

    node = allocate_node(AST_NODE_WHILE, location);
    if (node == NULL) {
        return NULL;
    }

    node->data.while_statement.condition = condition;
    node->data.while_statement.body = body;
    return node;
}

AstNode *ast_new_print(SourceLocation location, const char *name)
{
    AstNode *node;
    char *owned_name = copy_text(name);

    if (owned_name == NULL) {
        return NULL;
    }

    node = allocate_node(AST_NODE_PRINT, location);
    if (node == NULL) {
        free(owned_name);
        return NULL;
    }

    node->data.print_statement.name = owned_name;
    return node;
}

AstNode *ast_new_identifier(SourceLocation location, const char *name)
{
    AstNode *node;
    char *owned_name = copy_text(name);

    if (owned_name == NULL) {
        return NULL;
    }

    node = allocate_node(AST_NODE_IDENTIFIER, location);
    if (node == NULL) {
        free(owned_name);
        return NULL;
    }

    node->data.identifier.name = owned_name;
    return node;
}

AstNode *ast_new_int_literal(SourceLocation location, long long value)
{
    AstNode *node = allocate_node(AST_NODE_INT_LITERAL, location);

    if (node != NULL) {
        node->data.int_literal.value = value;
    }

    return node;
}

AstNode *ast_new_float_literal(SourceLocation location, double value)
{
    AstNode *node = allocate_node(AST_NODE_FLOAT_LITERAL, location);

    if (node != NULL) {
        node->data.float_literal.value = value;
    }

    return node;
}

AstNode *ast_new_bool_literal(SourceLocation location, bool value)
{
    AstNode *node = allocate_node(AST_NODE_BOOL_LITERAL, location);

    if (node != NULL) {
        node->data.bool_literal.value = value;
    }

    return node;
}

AstNode *ast_new_binary_expression(SourceLocation location,
                                   AstBinaryOperator operator,
                                   AstNode *left, AstNode *right)
{
    AstNode *node;

    if (!is_binary_operator(operator)
        || !is_expression(left) || !is_expression(right)) {
        return NULL;
    }

    node = allocate_node(AST_NODE_BINARY_EXPRESSION, location);
    if (node == NULL) {
        return NULL;
    }

    node->data.binary_expression.operator = operator;
    node->data.binary_expression.left = left;
    node->data.binary_expression.right = right;
    return node;
}

AstNode *ast_new_unary_expression(SourceLocation location,
                                  AstUnaryOperator operator,
                                  AstNode *operand)
{
    AstNode *node;

    if (!is_unary_operator(operator) || !is_expression(operand)) {
        return NULL;
    }

    node = allocate_node(AST_NODE_UNARY_EXPRESSION, location);
    if (node == NULL) {
        return NULL;
    }

    node->data.unary_expression.operator = operator;
    node->data.unary_expression.operand = operand;
    return node;
}

static void destroy_list(AstNodeList *list)
{
    size_t index;

    for (index = 0; index < list->count; index++) {
        ast_destroy(list->items[index]);
    }

    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void ast_destroy(AstNode *node)
{
    if (node == NULL) {
        return;
    }

    switch (node->kind) {
    case AST_NODE_PROGRAM:
        destroy_list(&node->data.program.statements);
        break;
    case AST_NODE_BLOCK:
        destroy_list(&node->data.block.statements);
        break;
    case AST_NODE_DECLARATION:
        free(node->data.declaration.name);
        ast_destroy(node->data.declaration.initializer);
        break;
    case AST_NODE_ASSIGNMENT:
        free(node->data.assignment.name);
        ast_destroy(node->data.assignment.value);
        break;
    case AST_NODE_IF:
        ast_destroy(node->data.if_statement.condition);
        ast_destroy(node->data.if_statement.then_block);
        ast_destroy(node->data.if_statement.else_block);
        break;
    case AST_NODE_WHILE:
        ast_destroy(node->data.while_statement.condition);
        ast_destroy(node->data.while_statement.body);
        break;
    case AST_NODE_PRINT:
        free(node->data.print_statement.name);
        break;
    case AST_NODE_IDENTIFIER:
        free(node->data.identifier.name);
        break;
    case AST_NODE_BINARY_EXPRESSION:
        ast_destroy(node->data.binary_expression.left);
        ast_destroy(node->data.binary_expression.right);
        break;
    case AST_NODE_UNARY_EXPRESSION:
        ast_destroy(node->data.unary_expression.operand);
        break;
    case AST_NODE_INT_LITERAL:
    case AST_NODE_FLOAT_LITERAL:
    case AST_NODE_BOOL_LITERAL:
        break;
    }

    free(node);
}
