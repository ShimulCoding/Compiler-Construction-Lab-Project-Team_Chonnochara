#include "semantic/semantic.h"

#include <stdbool.h>
#include <stdarg.h>

#include "symbol_table/symbol_table.h"

typedef struct {
    bool valid;
    ValueType type;
} ExpressionResult;

typedef struct {
    SymbolTable *symbols;
    FILE *diagnostics;
    size_t error_count;
    bool internal_failure;
} SemanticContext;

static ExpressionResult analyze_expression(SemanticContext *context,
                                           const AstNode *expression);
static void analyze_statement(SemanticContext *context,
                              const AstNode *statement);

static ExpressionResult expression_type(ValueType type)
{
    ExpressionResult result = {true, type};
    return result;
}

static ExpressionResult expression_error(void)
{
    ExpressionResult result = {false, VALUE_TYPE_INT};
    return result;
}

static bool is_numeric(ValueType type)
{
    return type == VALUE_TYPE_INT || type == VALUE_TYPE_FLOAT;
}

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
    return "?";
}

static void mark_internal_failure(SemanticContext *context,
                                  const char *message)
{
    if (context->internal_failure) {
        return;
    }

    context->internal_failure = true;
    fprintf(context->diagnostics, "semantic internal error: %s\n", message);
}

static void report_semantic_error(SemanticContext *context,
                                  SourceLocation location,
                                  const char *code,
                                  const char *format,
                                  ...)
{
    va_list arguments;
    bool output_failed = false;

    if (fprintf(context->diagnostics,
                "semantic error at line %zu [%s]: ",
                location.line,
                code) < 0) {
        output_failed = true;
    }

    va_start(arguments, format);
    if (vfprintf(context->diagnostics, format, arguments) < 0) {
        output_failed = true;
    }
    va_end(arguments);

    if (fputc('\n', context->diagnostics) == EOF) {
        output_failed = true;
    }

    if (output_failed) {
        context->internal_failure = true;
    } else {
        context->error_count++;
    }
}

static ExpressionResult symbol_type(SemanticContext *context,
                                    const Symbol *symbol)
{
    SymbolInfo info;

    if (!symbol_get_info(symbol, &info)) {
        mark_internal_failure(context, "failed to read a symbol record");
        return expression_error();
    }
    return expression_type(info.type);
}

static ExpressionResult resolve_identifier(SemanticContext *context,
                                           const char *name,
                                           SourceLocation location)
{
    const Symbol *symbol = symbol_table_lookup_active(context->symbols, name);

    if (symbol != NULL) {
        return symbol_type(context, symbol);
    }

    if (symbol_table_lookup_history(context->symbols, name) != NULL) {
        report_semantic_error(context,
                              location,
                              "SEM_SCOPE_VIOLATION",
                              "identifier '%s' is outside its declaring scope",
                              name);
    } else {
        report_semantic_error(context,
                              location,
                              "SEM_UNDECLARED",
                              "identifier '%s' is not declared",
                              name);
    }
    return expression_error();
}

static ExpressionResult invalid_binary_operator(
    SemanticContext *context,
    const AstNode *expression,
    ExpressionResult left,
    ExpressionResult right)
{
    AstBinaryOperator operator =
        expression->data.binary_expression.operator;

    report_semantic_error(
        context,
        expression->location,
        "SEM_INVALID_OPERATOR",
        "operator '%s' does not accept operands of type %s and %s",
        binary_operator_name(operator),
        value_type_name(left.type),
        value_type_name(right.type));
    return expression_error();
}

static ExpressionResult analyze_binary_expression(
    SemanticContext *context,
    const AstNode *expression)
{
    AstBinaryOperator operator =
        expression->data.binary_expression.operator;
    ExpressionResult left = analyze_expression(
        context, expression->data.binary_expression.left);
    ExpressionResult right = analyze_expression(
        context, expression->data.binary_expression.right);

    if (!left.valid || !right.valid || context->internal_failure) {
        return expression_error();
    }

    switch (operator) {
    case AST_BINARY_ADD:
    case AST_BINARY_SUBTRACT:
    case AST_BINARY_MULTIPLY:
    case AST_BINARY_DIVIDE:
        if (!is_numeric(left.type) || !is_numeric(right.type)) {
            return invalid_binary_operator(context, expression, left, right);
        }
        if (left.type == VALUE_TYPE_FLOAT
            || right.type == VALUE_TYPE_FLOAT) {
            return expression_type(VALUE_TYPE_FLOAT);
        }
        return expression_type(VALUE_TYPE_INT);

    case AST_BINARY_REMAINDER:
        if (left.type != VALUE_TYPE_INT || right.type != VALUE_TYPE_INT) {
            return invalid_binary_operator(context, expression, left, right);
        }
        return expression_type(VALUE_TYPE_INT);

    case AST_BINARY_LESS:
    case AST_BINARY_GREATER:
    case AST_BINARY_LESS_EQUAL:
    case AST_BINARY_GREATER_EQUAL:
        if (!is_numeric(left.type) || !is_numeric(right.type)) {
            return invalid_binary_operator(context, expression, left, right);
        }
        return expression_type(VALUE_TYPE_BOOL);

    case AST_BINARY_EQUAL:
    case AST_BINARY_NOT_EQUAL:
        if ((is_numeric(left.type) && is_numeric(right.type))
            || (left.type == VALUE_TYPE_BOOL
                && right.type == VALUE_TYPE_BOOL)) {
            return expression_type(VALUE_TYPE_BOOL);
        }
        report_semantic_error(
            context,
            expression->location,
            "SEM_TYPE_MISMATCH",
            "operator '%s' cannot compare values of type %s and %s",
            binary_operator_name(operator),
            value_type_name(left.type),
            value_type_name(right.type));
        return expression_error();

    case AST_BINARY_LOGICAL_AND:
    case AST_BINARY_LOGICAL_OR:
        if (left.type != VALUE_TYPE_BOOL
            || right.type != VALUE_TYPE_BOOL) {
            return invalid_binary_operator(context, expression, left, right);
        }
        return expression_type(VALUE_TYPE_BOOL);
    }

    mark_internal_failure(context, "unknown binary operator");
    return expression_error();
}

static ExpressionResult analyze_unary_expression(
    SemanticContext *context,
    const AstNode *expression)
{
    ExpressionResult operand = analyze_expression(
        context, expression->data.unary_expression.operand);

    if (!operand.valid || context->internal_failure) {
        return expression_error();
    }

    if (expression->data.unary_expression.operator != AST_UNARY_NOT) {
        mark_internal_failure(context, "unknown unary operator");
        return expression_error();
    }
    if (operand.type != VALUE_TYPE_BOOL) {
        report_semantic_error(
            context,
            expression->location,
            "SEM_INVALID_OPERATOR",
            "operator '!' does not accept an operand of type %s",
            value_type_name(operand.type));
        return expression_error();
    }
    return expression_type(VALUE_TYPE_BOOL);
}

static ExpressionResult analyze_expression(SemanticContext *context,
                                           const AstNode *expression)
{
    if (expression == NULL) {
        mark_internal_failure(context, "encountered a missing expression");
        return expression_error();
    }

    switch (expression->kind) {
    case AST_NODE_IDENTIFIER:
        return resolve_identifier(context,
                                  expression->data.identifier.name,
                                  expression->location);
    case AST_NODE_INT_LITERAL:
        return expression_type(VALUE_TYPE_INT);
    case AST_NODE_FLOAT_LITERAL:
        return expression_type(VALUE_TYPE_FLOAT);
    case AST_NODE_BOOL_LITERAL:
        return expression_type(VALUE_TYPE_BOOL);
    case AST_NODE_BINARY_EXPRESSION:
        return analyze_binary_expression(context, expression);
    case AST_NODE_UNARY_EXPRESSION:
        return analyze_unary_expression(context, expression);
    default:
        mark_internal_failure(context,
                              "encountered a non-expression AST node");
        return expression_error();
    }
}

static void insert_declaration(SemanticContext *context,
                               const AstNode *declaration)
{
    SymbolInsertResult result = symbol_table_insert(
        context->symbols,
        declaration->data.declaration.name,
        declaration->data.declaration.type,
        declaration->location);

    if (result != SYMBOL_INSERT_SUCCESS) {
        mark_internal_failure(context,
                              "failed to insert a declaration symbol");
    }
}

static void analyze_declaration(SemanticContext *context,
                                const AstNode *declaration)
{
    const char *name = declaration->data.declaration.name;
    const AstNode *initializer = declaration->data.declaration.initializer;
    bool redeclared =
        symbol_table_lookup_current(context->symbols, name) != NULL;
    ExpressionResult initializer_type = expression_error();

    if (redeclared) {
        report_semantic_error(context,
                              declaration->location,
                              "SEM_REDECLARATION",
                              "identifier '%s' is already declared in this scope",
                              name);
    }

    if (initializer != NULL) {
        initializer_type = analyze_expression(context, initializer);
    }

    if (!redeclared && initializer != NULL && initializer_type.valid
        && initializer_type.type != declaration->data.declaration.type) {
        report_semantic_error(
            context,
            declaration->location,
            "SEM_TYPE_MISMATCH",
            "cannot initialize '%s' of type %s with a value of type %s",
            name,
            value_type_name(declaration->data.declaration.type),
            value_type_name(initializer_type.type));
    }

    /*
     * The initializer is always visited before insertion. Even an invalid
     * initializer still establishes a non-redeclared name for later source
     * statements, preventing an undeclared-variable cascade.
     */
    if (!redeclared && !context->internal_failure) {
        insert_declaration(context, declaration);
    }
}

static void analyze_assignment(SemanticContext *context,
                               const AstNode *assignment)
{
    ExpressionResult target = resolve_identifier(
        context, assignment->data.assignment.name, assignment->location);
    ExpressionResult value = analyze_expression(
        context, assignment->data.assignment.value);

    if (target.valid && value.valid && target.type != value.type) {
        report_semantic_error(
            context,
            assignment->location,
            "SEM_INVALID_ASSIGNMENT",
            "cannot assign a value of type %s to '%s' of type %s",
            value_type_name(value.type),
            assignment->data.assignment.name,
            value_type_name(target.type));
    }
}

static void analyze_statement_list(SemanticContext *context,
                                   const AstNodeList *statements)
{
    size_t index;

    for (index = 0;
         index < statements->count && !context->internal_failure;
         index++) {
        analyze_statement(context, statements->items[index]);
    }
}

static void analyze_block(SemanticContext *context, const AstNode *block)
{
    SymbolScopeResult enter_result;
    SymbolScopeResult exit_result;

    enter_result = symbol_table_enter_scope(context->symbols);
    if (enter_result != SYMBOL_SCOPE_SUCCESS) {
        mark_internal_failure(context, "failed to enter a semantic scope");
        return;
    }

    analyze_statement_list(context, &block->data.block.statements);

    exit_result = symbol_table_exit_scope(context->symbols);
    if (exit_result != SYMBOL_SCOPE_SUCCESS) {
        mark_internal_failure(context, "failed to exit a semantic scope");
    }
}

static void analyze_condition(SemanticContext *context,
                              const AstNode *condition,
                              const char *statement_name)
{
    ExpressionResult type = analyze_expression(context, condition);

    if (type.valid && type.type != VALUE_TYPE_BOOL) {
        report_semantic_error(
            context,
            condition->location,
            "SEM_TYPE_MISMATCH",
            "%s condition must have type bool, found %s",
            statement_name,
            value_type_name(type.type));
    }
}

static void analyze_statement(SemanticContext *context,
                              const AstNode *statement)
{
    if (statement == NULL) {
        mark_internal_failure(context, "encountered a missing statement");
        return;
    }

    switch (statement->kind) {
    case AST_NODE_BLOCK:
        analyze_block(context, statement);
        break;
    case AST_NODE_DECLARATION:
        analyze_declaration(context, statement);
        break;
    case AST_NODE_ASSIGNMENT:
        analyze_assignment(context, statement);
        break;
    case AST_NODE_PRINT:
        (void)resolve_identifier(context,
                                 statement->data.print_statement.name,
                                 statement->location);
        break;
    case AST_NODE_IF:
        analyze_condition(context,
                          statement->data.if_statement.condition,
                          "if");
        analyze_block(context, statement->data.if_statement.then_block);
        if (statement->data.if_statement.else_block != NULL) {
            analyze_block(context, statement->data.if_statement.else_block);
        }
        break;
    case AST_NODE_WHILE:
        analyze_condition(context,
                          statement->data.while_statement.condition,
                          "while");
        analyze_block(context, statement->data.while_statement.body);
        break;
    default:
        mark_internal_failure(context,
                              "encountered a non-statement AST node");
        break;
    }
}

SemanticStatus semantic_analyze(const AstNode *program,
                                FILE *diagnostics,
                                SemanticResult *result)
{
    SemanticContext context = {0};

    if (result != NULL) {
        result->error_count = 0;
    }
    if (program == NULL || program->kind != AST_NODE_PROGRAM
        || diagnostics == NULL || result == NULL) {
        return SEMANTIC_STATUS_INTERNAL_ERROR;
    }

    context.symbols = symbol_table_create();
    context.diagnostics = diagnostics;
    if (context.symbols == NULL) {
        fprintf(diagnostics,
                "semantic internal error: failed to create symbol table\n");
        return SEMANTIC_STATUS_INTERNAL_ERROR;
    }

    analyze_statement_list(&context, &program->data.program.statements);
    result->error_count = context.error_count;
    symbol_table_destroy(context.symbols);

    if (context.internal_failure || ferror(diagnostics)) {
        return SEMANTIC_STATUS_INTERNAL_ERROR;
    }
    if (context.error_count != 0) {
        return SEMANTIC_STATUS_ERRORS;
    }
    return SEMANTIC_STATUS_SUCCESS;
}
