#include "ast/ast.h"

static const char *value_type_name(ValueType type)
{
    switch (type) {
    case VALUE_TYPE_INT:
        return "int";
    case VALUE_TYPE_FLOAT:
        return "float";
    case VALUE_TYPE_BOOL:
        return "bool";
    }

    return "unknown";
}

static const char *binary_operator_name(AstBinaryOperator operator)
{
    switch (operator) {
    case AST_BINARY_ADD:
        return "+";
    case AST_BINARY_SUBTRACT:
        return "-";
    case AST_BINARY_MULTIPLY:
        return "*";
    case AST_BINARY_DIVIDE:
        return "/";
    case AST_BINARY_REMAINDER:
        return "%";
    case AST_BINARY_LESS:
        return "<";
    case AST_BINARY_GREATER:
        return ">";
    case AST_BINARY_LESS_EQUAL:
        return "<=";
    case AST_BINARY_GREATER_EQUAL:
        return ">=";
    case AST_BINARY_EQUAL:
        return "==";
    case AST_BINARY_NOT_EQUAL:
        return "!=";
    case AST_BINARY_LOGICAL_AND:
        return "&&";
    case AST_BINARY_LOGICAL_OR:
        return "||";
    }

    return "unknown";
}

static const char *unary_operator_name(AstUnaryOperator operator)
{
    if (operator == AST_UNARY_NOT) {
        return "!";
    }

    return "unknown";
}

static bool print_indent(FILE *output, size_t depth)
{
    size_t level;

    for (level = 0; level < depth; level++) {
        if (fputs("  ", output) == EOF) {
            return false;
        }
    }

    return true;
}

static bool print_label(FILE *output, size_t depth, const char *label)
{
    return print_indent(output, depth)
        && fprintf(output, "%s\n", label) >= 0;
}

static bool print_node(FILE *output, const AstNode *node, size_t depth);

static bool print_statements(FILE *output, const AstNodeList *statements,
                             size_t depth)
{
    size_t index;

    if (statements->count == 0) {
        return print_label(output, depth, "<empty>");
    }

    for (index = 0; index < statements->count; index++) {
        if (!print_node(output, statements->items[index], depth)) {
            return false;
        }
    }

    return true;
}

static bool print_node(FILE *output, const AstNode *node, size_t depth)
{
    if (node == NULL || !print_indent(output, depth)) {
        return false;
    }

    switch (node->kind) {
    case AST_NODE_PROGRAM:
        if (fprintf(output, "Program(line=%zu)\n", node->location.line) < 0) {
            return false;
        }
        return print_statements(output, &node->data.program.statements,
                                depth + 1);

    case AST_NODE_BLOCK:
        if (fprintf(output, "Block(line=%zu)\n", node->location.line) < 0) {
            return false;
        }
        return print_statements(output, &node->data.block.statements,
                                depth + 1);

    case AST_NODE_DECLARATION:
        if (fprintf(output, "Declaration(line=%zu, type=%s, name=%s)\n",
                    node->location.line,
                    value_type_name(node->data.declaration.type),
                    node->data.declaration.name) < 0) {
            return false;
        }
        if (node->data.declaration.initializer == NULL) {
            return true;
        }
        return print_label(output, depth + 1, "Initializer")
            && print_node(output, node->data.declaration.initializer,
                          depth + 2);

    case AST_NODE_ASSIGNMENT:
        if (fprintf(output, "Assignment(line=%zu, name=%s)\n",
                    node->location.line,
                    node->data.assignment.name) < 0) {
            return false;
        }
        return print_label(output, depth + 1, "Value")
            && print_node(output, node->data.assignment.value, depth + 2);

    case AST_NODE_IF:
        if (fprintf(output, "If(line=%zu)\n", node->location.line) < 0
            || !print_label(output, depth + 1, "Condition")
            || !print_node(output, node->data.if_statement.condition,
                           depth + 2)
            || !print_label(output, depth + 1, "Then")
            || !print_node(output, node->data.if_statement.then_block,
                           depth + 2)) {
            return false;
        }

        if (node->data.if_statement.else_block == NULL) {
            return true;
        }

        return print_label(output, depth + 1, "Else")
            && print_node(output, node->data.if_statement.else_block,
                          depth + 2);

    case AST_NODE_WHILE:
        if (fprintf(output, "While(line=%zu)\n", node->location.line) < 0) {
            return false;
        }
        return print_label(output, depth + 1, "Condition")
            && print_node(output, node->data.while_statement.condition,
                          depth + 2)
            && print_label(output, depth + 1, "Body")
            && print_node(output, node->data.while_statement.body,
                          depth + 2);

    case AST_NODE_PRINT:
        return fprintf(output, "Print(line=%zu, name=%s)\n",
                       node->location.line,
                       node->data.print_statement.name) >= 0;

    case AST_NODE_IDENTIFIER:
        return fprintf(output, "Identifier(line=%zu, name=%s)\n",
                       node->location.line,
                       node->data.identifier.name) >= 0;

    case AST_NODE_INT_LITERAL:
        return fprintf(output, "IntegerLiteral(line=%zu, value=%lld)\n",
                       node->location.line,
                       node->data.int_literal.value) >= 0;

    case AST_NODE_FLOAT_LITERAL:
        return fprintf(output, "FloatLiteral(line=%zu, value=%.17g)\n",
                       node->location.line,
                       node->data.float_literal.value) >= 0;

    case AST_NODE_BOOL_LITERAL:
        return fprintf(output, "BooleanLiteral(line=%zu, value=%s)\n",
                       node->location.line,
                       node->data.bool_literal.value ? "true" : "false") >= 0;

    case AST_NODE_BINARY_EXPRESSION:
        if (fprintf(output,
                    "BinaryExpression(line=%zu, operator=\"%s\")\n",
                    node->location.line,
                    binary_operator_name(
                        node->data.binary_expression.operator)) < 0) {
            return false;
        }
        return print_label(output, depth + 1, "Left")
            && print_node(output, node->data.binary_expression.left,
                          depth + 2)
            && print_label(output, depth + 1, "Right")
            && print_node(output, node->data.binary_expression.right,
                          depth + 2);

    case AST_NODE_UNARY_EXPRESSION:
        if (fprintf(output,
                    "UnaryExpression(line=%zu, operator=\"%s\")\n",
                    node->location.line,
                    unary_operator_name(
                        node->data.unary_expression.operator)) < 0) {
            return false;
        }
        return print_label(output, depth + 1, "Operand")
            && print_node(output, node->data.unary_expression.operand,
                          depth + 2);
    }

    return false;
}

bool ast_print(FILE *output, const AstNode *node)
{
    if (output == NULL || node == NULL) {
        return false;
    }

    return print_node(output, node, 0) && ferror(output) == 0;
}
