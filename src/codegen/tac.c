#include "codegen/tac.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "symbol_table/symbol_table.h"

typedef struct {
    const Symbol *symbol;
    char *storage_name;
} TacBinding;

typedef struct {
    TacProgram *program;
    SymbolTable *symbols;
    TacBinding *bindings;
    size_t binding_count;
    size_t binding_capacity;
    char **reserved_names;
    size_t reserved_name_count;
    size_t reserved_name_capacity;
    size_t next_temporary;
    size_t next_label;
} TacContext;

static char *copy_text(const char *text)
{
    size_t length;
    char *copy;

    if (text == NULL) {
        return NULL;
    }

    length = strlen(text) + 1;
    copy = malloc(length);
    if (copy != NULL) {
        memcpy(copy, text, length);
    }
    return copy;
}

static void instruction_destroy(TacInstruction *instruction)
{
    if (instruction == NULL) {
        return;
    }

    free(instruction->result);
    free(instruction->operator_text);
    free(instruction->first_operand);
    free(instruction->second_operand);
    free(instruction->label);
    *instruction = (TacInstruction){0};
}

void tac_program_destroy(TacProgram *program)
{
    size_t index;

    if (program == NULL) {
        return;
    }

    for (index = 0; index < program->count; index++) {
        instruction_destroy(&program->instructions[index]);
    }
    free(program->instructions);
    free(program);
}

static bool grow_instruction_storage(TacProgram *program)
{
    size_t new_capacity;
    TacInstruction *new_instructions;

    if (program->count < program->capacity) {
        return true;
    }

    new_capacity = program->capacity == 0 ? 8 : program->capacity * 2;
    if (new_capacity < program->capacity
        || new_capacity > SIZE_MAX / sizeof(*new_instructions)) {
        return false;
    }

    new_instructions = realloc(
        program->instructions, new_capacity * sizeof(*new_instructions));
    if (new_instructions == NULL) {
        return false;
    }

    program->instructions = new_instructions;
    program->capacity = new_capacity;
    return true;
}

static TacStatus append_instruction(TacProgram *program,
                                    TacInstructionKind kind,
                                    const char *result,
                                    const char *operator_text,
                                    const char *first_operand,
                                    const char *second_operand)
{
    TacInstruction instruction = {0};

    if (program == NULL || first_operand == NULL
        || (kind != TAC_INSTRUCTION_PRINT && result == NULL)
        || ((kind == TAC_INSTRUCTION_UNARY
             || kind == TAC_INSTRUCTION_BINARY)
            && operator_text == NULL)
        || (kind == TAC_INSTRUCTION_BINARY && second_operand == NULL)) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    instruction.kind = kind;
    if (result != NULL) {
        instruction.result = copy_text(result);
    }
    if (operator_text != NULL) {
        instruction.operator_text = copy_text(operator_text);
    }
    instruction.first_operand = copy_text(first_operand);
    if (second_operand != NULL) {
        instruction.second_operand = copy_text(second_operand);
    }

    if ((result != NULL && instruction.result == NULL)
        || (operator_text != NULL && instruction.operator_text == NULL)
        || instruction.first_operand == NULL
        || (second_operand != NULL && instruction.second_operand == NULL)) {
        instruction_destroy(&instruction);
        return TAC_STATUS_ALLOCATION_FAILURE;
    }

    if (!grow_instruction_storage(program)) {
        instruction_destroy(&instruction);
        return TAC_STATUS_ALLOCATION_FAILURE;
    }

    program->instructions[program->count++] = instruction;
    return TAC_STATUS_SUCCESS;
}

static TacStatus append_control_instruction(
    TacProgram *program,
    TacInstructionKind kind,
    const char *condition_operand,
    const char *label)
{
    TacInstruction instruction = {0};

    if (program == NULL || label == NULL
        || (kind != TAC_INSTRUCTION_LABEL
            && kind != TAC_INSTRUCTION_JUMP
            && kind != TAC_INSTRUCTION_JUMP_IF_FALSE)
        || (kind == TAC_INSTRUCTION_JUMP_IF_FALSE
            && condition_operand == NULL)
        || (kind != TAC_INSTRUCTION_JUMP_IF_FALSE
            && condition_operand != NULL)) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    instruction.kind = kind;
    instruction.label = copy_text(label);
    if (condition_operand != NULL) {
        instruction.first_operand = copy_text(condition_operand);
    }
    if (instruction.label == NULL
        || (condition_operand != NULL
            && instruction.first_operand == NULL)) {
        instruction_destroy(&instruction);
        return TAC_STATUS_ALLOCATION_FAILURE;
    }

    if (!grow_instruction_storage(program)) {
        instruction_destroy(&instruction);
        return TAC_STATUS_ALLOCATION_FAILURE;
    }

    program->instructions[program->count++] = instruction;
    return TAC_STATUS_SUCCESS;
}

static bool grow_binding_storage(TacContext *context)
{
    size_t new_capacity;
    TacBinding *new_bindings;

    if (context->binding_count < context->binding_capacity) {
        return true;
    }

    new_capacity = context->binding_capacity == 0
        ? 8
        : context->binding_capacity * 2;
    if (new_capacity < context->binding_capacity
        || new_capacity > SIZE_MAX / sizeof(*new_bindings)) {
        return false;
    }

    new_bindings = realloc(
        context->bindings, new_capacity * sizeof(*new_bindings));
    if (new_bindings == NULL) {
        return false;
    }

    context->bindings = new_bindings;
    context->binding_capacity = new_capacity;
    return true;
}

static char *make_storage_name(const char *source_name, size_t scope_id)
{
    int required_length;
    char *storage_name;

    if (scope_id == 0) {
        return copy_text(source_name);
    }

    required_length = snprintf(NULL, 0, "%s@%zu", source_name, scope_id);
    if (required_length < 0) {
        return NULL;
    }

    storage_name = malloc((size_t)required_length + 1);
    if (storage_name == NULL) {
        return NULL;
    }
    snprintf(storage_name, (size_t)required_length + 1,
             "%s@%zu", source_name, scope_id);
    return storage_name;
}

static TacStatus remember_binding(TacContext *context,
                                  const Symbol *symbol)
{
    SymbolInfo info;
    char *storage_name;

    if (!symbol_get_info(symbol, &info)) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    storage_name = make_storage_name(info.name, info.scope_id);
    if (storage_name == NULL) {
        return TAC_STATUS_ALLOCATION_FAILURE;
    }
    if (!grow_binding_storage(context)) {
        free(storage_name);
        return TAC_STATUS_ALLOCATION_FAILURE;
    }

    context->bindings[context->binding_count].symbol = symbol;
    context->bindings[context->binding_count].storage_name = storage_name;
    context->binding_count++;
    return TAC_STATUS_SUCCESS;
}

static const char *find_storage_name(const TacContext *context,
                                     const Symbol *symbol)
{
    size_t index;

    for (index = 0; index < context->binding_count; index++) {
        if (context->bindings[index].symbol == symbol) {
            return context->bindings[index].storage_name;
        }
    }
    return NULL;
}

static const char *resolve_storage_name(const TacContext *context,
                                        const char *source_name)
{
    const Symbol *symbol = symbol_table_lookup_active(
        context->symbols, source_name);

    if (symbol == NULL) {
        return NULL;
    }
    return find_storage_name(context, symbol);
}

static TacStatus insert_declaration(TacContext *context,
                                    const AstNode *declaration)
{
    SymbolInsertResult insert_result;
    const Symbol *symbol;

    insert_result = symbol_table_insert(
        context->symbols,
        declaration->data.declaration.name,
        declaration->data.declaration.type,
        declaration->location);
    if (insert_result == SYMBOL_INSERT_ALLOCATION_FAILURE) {
        return TAC_STATUS_ALLOCATION_FAILURE;
    }
    if (insert_result != SYMBOL_INSERT_SUCCESS) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    symbol = symbol_table_lookup_current(
        context->symbols, declaration->data.declaration.name);
    if (symbol == NULL) {
        return TAC_STATUS_INTERNAL_ERROR;
    }
    return remember_binding(context, symbol);
}

static bool name_is_reserved(const TacContext *context, const char *name)
{
    size_t index;

    for (index = 0; index < context->reserved_name_count; index++) {
        if (strcmp(context->reserved_names[index], name) == 0) {
            return true;
        }
    }
    return false;
}

static bool grow_reserved_name_storage(TacContext *context)
{
    size_t new_capacity;
    char **new_names;

    if (context->reserved_name_count < context->reserved_name_capacity) {
        return true;
    }

    new_capacity = context->reserved_name_capacity == 0
        ? 8
        : context->reserved_name_capacity * 2;
    if (new_capacity < context->reserved_name_capacity
        || new_capacity > SIZE_MAX / sizeof(*new_names)) {
        return false;
    }

    new_names = realloc(
        context->reserved_names, new_capacity * sizeof(*new_names));
    if (new_names == NULL) {
        return false;
    }

    context->reserved_names = new_names;
    context->reserved_name_capacity = new_capacity;
    return true;
}

static TacStatus reserve_name(TacContext *context, const char *name)
{
    char *copy;

    if (name == NULL || name[0] == '\0') {
        return TAC_STATUS_INTERNAL_ERROR;
    }
    if (name_is_reserved(context, name)) {
        return TAC_STATUS_SUCCESS;
    }

    copy = copy_text(name);
    if (copy == NULL) {
        return TAC_STATUS_ALLOCATION_FAILURE;
    }
    if (!grow_reserved_name_storage(context)) {
        free(copy);
        return TAC_STATUS_ALLOCATION_FAILURE;
    }

    context->reserved_names[context->reserved_name_count++] = copy;
    return TAC_STATUS_SUCCESS;
}

static TacStatus reserve_global_storage_names(
    TacContext *context, const AstNode *program)
{
    const AstNodeList *statements = &program->data.program.statements;
    size_t index;

    if (statements->count != 0 && statements->items == NULL) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    for (index = 0; index < statements->count; index++) {
        const AstNode *statement = statements->items[index];
        TacStatus status;

        if (statement == NULL) {
            return TAC_STATUS_INTERNAL_ERROR;
        }
        if (statement->kind != AST_NODE_DECLARATION) {
            continue;
        }

        status = reserve_name(
            context, statement->data.declaration.name);
        if (status != TAC_STATUS_SUCCESS) {
            return status;
        }
    }
    return TAC_STATUS_SUCCESS;
}

static TacStatus make_temporary(TacContext *context, char **out_temporary)
{
    int required_length;
    char *temporary;
    TacStatus status;

    if (out_temporary == NULL) {
        return TAC_STATUS_INTERNAL_ERROR;
    }
    *out_temporary = NULL;

    while (context->next_temporary != SIZE_MAX) {
        required_length = snprintf(
            NULL, 0, "t%zu", context->next_temporary);
        if (required_length < 0) {
            return TAC_STATUS_INTERNAL_ERROR;
        }

        temporary = malloc((size_t)required_length + 1);
        if (temporary == NULL) {
            return TAC_STATUS_ALLOCATION_FAILURE;
        }
        snprintf(temporary, (size_t)required_length + 1,
                 "t%zu", context->next_temporary);
        context->next_temporary++;

        if (name_is_reserved(context, temporary)) {
            free(temporary);
            continue;
        }

        status = reserve_name(context, temporary);
        if (status != TAC_STATUS_SUCCESS) {
            free(temporary);
            return status;
        }

        *out_temporary = temporary;
        return TAC_STATUS_SUCCESS;
    }

    return TAC_STATUS_INTERNAL_ERROR;
}

static TacStatus make_label(TacContext *context, char **out_label)
{
    int required_length;
    char *label;

    if (out_label == NULL) {
        return TAC_STATUS_INTERNAL_ERROR;
    }
    *out_label = NULL;
    if (context->next_label == SIZE_MAX) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    required_length = snprintf(
        NULL, 0, ".L%zu", context->next_label);
    if (required_length < 0) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    label = malloc((size_t)required_length + 1);
    if (label == NULL) {
        return TAC_STATUS_ALLOCATION_FAILURE;
    }
    snprintf(label, (size_t)required_length + 1,
             ".L%zu", context->next_label);
    context->next_label++;
    *out_label = label;
    return TAC_STATUS_SUCCESS;
}

static char *format_integer(long long value)
{
    int required_length = snprintf(NULL, 0, "%lld", value);
    char *text;

    if (required_length < 0) {
        return NULL;
    }
    text = malloc((size_t)required_length + 1);
    if (text != NULL) {
        snprintf(text, (size_t)required_length + 1, "%lld", value);
    }
    return text;
}

static char *format_float(double value)
{
    char buffer[64];
    int length = snprintf(buffer, sizeof(buffer), "%.15g", value);
    bool needs_decimal;
    char *text;

    if (length < 0 || (size_t)length >= sizeof(buffer)) {
        return NULL;
    }

    needs_decimal = strchr(buffer, '.') == NULL
        && strchr(buffer, 'e') == NULL
        && strchr(buffer, 'E') == NULL;
    if (needs_decimal) {
        if ((size_t)length + 2 >= sizeof(buffer)) {
            return NULL;
        }
        buffer[length++] = '.';
        buffer[length++] = '0';
        buffer[length] = '\0';
    }

    text = malloc((size_t)length + 1);
    if (text != NULL) {
        memcpy(text, buffer, (size_t)length + 1);
    }
    return text;
}

static const char *binary_operator_text(AstBinaryOperator operator)
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
    return NULL;
}

static TacStatus generate_expression(TacContext *context,
                                     const AstNode *expression,
                                     char **result_operand)
{
    TacStatus status;
    char *left = NULL;
    char *right = NULL;
    char *temporary = NULL;
    const char *operator_text;
    const char *storage_name;

    if (expression == NULL || result_operand == NULL) {
        return TAC_STATUS_INTERNAL_ERROR;
    }
    *result_operand = NULL;

    switch (expression->kind) {
    case AST_NODE_IDENTIFIER:
        storage_name = resolve_storage_name(
            context, expression->data.identifier.name);
        if (storage_name == NULL) {
            return TAC_STATUS_INTERNAL_ERROR;
        }
        *result_operand = copy_text(storage_name);
        return *result_operand == NULL
            ? TAC_STATUS_ALLOCATION_FAILURE
            : TAC_STATUS_SUCCESS;

    case AST_NODE_INT_LITERAL:
        *result_operand = format_integer(expression->data.int_literal.value);
        return *result_operand == NULL
            ? TAC_STATUS_ALLOCATION_FAILURE
            : TAC_STATUS_SUCCESS;

    case AST_NODE_FLOAT_LITERAL:
        *result_operand = format_float(
            expression->data.float_literal.value);
        return *result_operand == NULL
            ? TAC_STATUS_ALLOCATION_FAILURE
            : TAC_STATUS_SUCCESS;

    case AST_NODE_BOOL_LITERAL:
        *result_operand = copy_text(
            expression->data.bool_literal.value ? "true" : "false");
        return *result_operand == NULL
            ? TAC_STATUS_ALLOCATION_FAILURE
            : TAC_STATUS_SUCCESS;

    case AST_NODE_UNARY_EXPRESSION:
        if (expression->data.unary_expression.operator != AST_UNARY_NOT) {
            return TAC_STATUS_INTERNAL_ERROR;
        }
        status = generate_expression(
            context,
            expression->data.unary_expression.operand,
            &left);
        if (status != TAC_STATUS_SUCCESS) {
            return status;
        }
        status = make_temporary(context, &temporary);
        if (status != TAC_STATUS_SUCCESS) {
            free(left);
            return status;
        }
        status = append_instruction(
            context->program, TAC_INSTRUCTION_UNARY,
            temporary, "!", left, NULL);
        free(left);
        if (status != TAC_STATUS_SUCCESS) {
            free(temporary);
            return status;
        }
        *result_operand = temporary;
        return TAC_STATUS_SUCCESS;

    case AST_NODE_BINARY_EXPRESSION:
        operator_text = binary_operator_text(
            expression->data.binary_expression.operator);
        if (operator_text == NULL) {
            return TAC_STATUS_INTERNAL_ERROR;
        }
        status = generate_expression(
            context, expression->data.binary_expression.left, &left);
        if (status != TAC_STATUS_SUCCESS) {
            return status;
        }
        status = generate_expression(
            context, expression->data.binary_expression.right, &right);
        if (status != TAC_STATUS_SUCCESS) {
            free(left);
            return status;
        }
        status = make_temporary(context, &temporary);
        if (status != TAC_STATUS_SUCCESS) {
            free(left);
            free(right);
            return status;
        }
        status = append_instruction(
            context->program, TAC_INSTRUCTION_BINARY,
            temporary, operator_text, left, right);
        free(left);
        free(right);
        if (status != TAC_STATUS_SUCCESS) {
            free(temporary);
            return status;
        }
        *result_operand = temporary;
        return TAC_STATUS_SUCCESS;

    default:
        return TAC_STATUS_INTERNAL_ERROR;
    }
}

static TacStatus generate_statement(TacContext *context,
                                    const AstNode *statement);

static TacStatus generate_statement_list(TacContext *context,
                                         const AstNodeList *statements)
{
    size_t index;

    if (statements == NULL
        || (statements->count != 0 && statements->items == NULL)) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    for (index = 0; index < statements->count; index++) {
        TacStatus status = generate_statement(
            context, statements->items[index]);
        if (status != TAC_STATUS_SUCCESS) {
            return status;
        }
    }
    return TAC_STATUS_SUCCESS;
}

static TacStatus generate_block(TacContext *context,
                                const AstNode *block)
{
    TacStatus status;
    SymbolScopeResult enter_result;
    SymbolScopeResult exit_result;

    enter_result = symbol_table_enter_scope(context->symbols);
    if (enter_result == SYMBOL_SCOPE_ALLOCATION_FAILURE) {
        return TAC_STATUS_ALLOCATION_FAILURE;
    }
    if (enter_result != SYMBOL_SCOPE_SUCCESS) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    status = generate_statement_list(
        context, &block->data.block.statements);
    exit_result = symbol_table_exit_scope(context->symbols);
    if (status == TAC_STATUS_SUCCESS
        && exit_result != SYMBOL_SCOPE_SUCCESS) {
        return TAC_STATUS_INTERNAL_ERROR;
    }
    return status;
}

static TacStatus generate_declaration(TacContext *context,
                                      const AstNode *declaration)
{
    const AstNode *initializer =
        declaration->data.declaration.initializer;
    char *initializer_operand = NULL;
    const char *storage_name;
    TacStatus status;

    if (initializer != NULL) {
        status = generate_expression(
            context, initializer, &initializer_operand);
        if (status != TAC_STATUS_SUCCESS) {
            return status;
        }
    }

    status = insert_declaration(context, declaration);
    if (status != TAC_STATUS_SUCCESS) {
        free(initializer_operand);
        return status;
    }

    if (initializer == NULL) {
        return TAC_STATUS_SUCCESS;
    }

    storage_name = resolve_storage_name(
        context, declaration->data.declaration.name);
    if (storage_name == NULL) {
        free(initializer_operand);
        return TAC_STATUS_INTERNAL_ERROR;
    }

    status = append_instruction(
        context->program, TAC_INSTRUCTION_ASSIGNMENT,
        storage_name, NULL, initializer_operand, NULL);
    free(initializer_operand);
    return status;
}

static TacStatus generate_assignment(TacContext *context,
                                     const AstNode *assignment)
{
    const char *storage_name = resolve_storage_name(
        context, assignment->data.assignment.name);
    char *value_operand = NULL;
    TacStatus status;

    if (storage_name == NULL) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    status = generate_expression(
        context, assignment->data.assignment.value, &value_operand);
    if (status != TAC_STATUS_SUCCESS) {
        return status;
    }

    status = append_instruction(
        context->program, TAC_INSTRUCTION_ASSIGNMENT,
        storage_name, NULL, value_operand, NULL);
    free(value_operand);
    return status;
}

static TacStatus generate_print(TacContext *context,
                                const AstNode *print_statement)
{
    const char *storage_name = resolve_storage_name(
        context, print_statement->data.print_statement.name);

    if (storage_name == NULL) {
        return TAC_STATUS_INTERNAL_ERROR;
    }
    return append_instruction(
        context->program, TAC_INSTRUCTION_PRINT,
        NULL, NULL, storage_name, NULL);
}

static TacStatus generate_if(TacContext *context,
                             const AstNode *if_statement)
{
    const AstNode *then_block =
        if_statement->data.if_statement.then_block;
    const AstNode *else_block =
        if_statement->data.if_statement.else_block;
    char *condition_operand = NULL;
    char *false_label = NULL;
    char *end_label = NULL;
    TacStatus status;

    if (if_statement->data.if_statement.condition == NULL
        || then_block == NULL || then_block->kind != AST_NODE_BLOCK
        || (else_block != NULL && else_block->kind != AST_NODE_BLOCK)) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    status = generate_expression(
        context,
        if_statement->data.if_statement.condition,
        &condition_operand);
    if (status != TAC_STATUS_SUCCESS) {
        return status;
    }

    status = make_label(context, &false_label);
    if (status == TAC_STATUS_SUCCESS && else_block != NULL) {
        status = make_label(context, &end_label);
    }
    if (status == TAC_STATUS_SUCCESS) {
        status = append_control_instruction(
            context->program,
            TAC_INSTRUCTION_JUMP_IF_FALSE,
            condition_operand,
            false_label);
    }
    free(condition_operand);

    if (status == TAC_STATUS_SUCCESS) {
        status = generate_block(context, then_block);
    }
    if (status == TAC_STATUS_SUCCESS && else_block != NULL) {
        status = append_control_instruction(
            context->program,
            TAC_INSTRUCTION_JUMP,
            NULL,
            end_label);
    }
    if (status == TAC_STATUS_SUCCESS) {
        status = append_control_instruction(
            context->program,
            TAC_INSTRUCTION_LABEL,
            NULL,
            false_label);
    }
    if (status == TAC_STATUS_SUCCESS && else_block != NULL) {
        status = generate_block(context, else_block);
    }
    if (status == TAC_STATUS_SUCCESS && else_block != NULL) {
        status = append_control_instruction(
            context->program,
            TAC_INSTRUCTION_LABEL,
            NULL,
            end_label);
    }

    free(false_label);
    free(end_label);
    return status;
}

static TacStatus generate_while(TacContext *context,
                                const AstNode *while_statement)
{
    const AstNode *body = while_statement->data.while_statement.body;
    char *condition_operand = NULL;
    char *start_label = NULL;
    char *exit_label = NULL;
    TacStatus status;

    if (while_statement->data.while_statement.condition == NULL
        || body == NULL || body->kind != AST_NODE_BLOCK) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    status = make_label(context, &start_label);
    if (status == TAC_STATUS_SUCCESS) {
        status = make_label(context, &exit_label);
    }
    if (status == TAC_STATUS_SUCCESS) {
        status = append_control_instruction(
            context->program,
            TAC_INSTRUCTION_LABEL,
            NULL,
            start_label);
    }
    if (status == TAC_STATUS_SUCCESS) {
        status = generate_expression(
            context,
            while_statement->data.while_statement.condition,
            &condition_operand);
    }
    if (status == TAC_STATUS_SUCCESS) {
        status = append_control_instruction(
            context->program,
            TAC_INSTRUCTION_JUMP_IF_FALSE,
            condition_operand,
            exit_label);
    }
    free(condition_operand);

    if (status == TAC_STATUS_SUCCESS) {
        status = generate_block(context, body);
    }
    if (status == TAC_STATUS_SUCCESS) {
        status = append_control_instruction(
            context->program,
            TAC_INSTRUCTION_JUMP,
            NULL,
            start_label);
    }
    if (status == TAC_STATUS_SUCCESS) {
        status = append_control_instruction(
            context->program,
            TAC_INSTRUCTION_LABEL,
            NULL,
            exit_label);
    }

    free(start_label);
    free(exit_label);
    return status;
}

static TacStatus generate_statement(TacContext *context,
                                    const AstNode *statement)
{
    if (statement == NULL) {
        return TAC_STATUS_INTERNAL_ERROR;
    }

    switch (statement->kind) {
    case AST_NODE_BLOCK:
        return generate_block(context, statement);
    case AST_NODE_DECLARATION:
        return generate_declaration(context, statement);
    case AST_NODE_ASSIGNMENT:
        return generate_assignment(context, statement);
    case AST_NODE_PRINT:
        return generate_print(context, statement);
    case AST_NODE_IF:
        return generate_if(context, statement);
    case AST_NODE_WHILE:
        return generate_while(context, statement);
    default:
        return TAC_STATUS_INTERNAL_ERROR;
    }
}

static void context_destroy(TacContext *context)
{
    size_t index;

    for (index = 0; index < context->binding_count; index++) {
        free(context->bindings[index].storage_name);
    }
    free(context->bindings);
    for (index = 0; index < context->reserved_name_count; index++) {
        free(context->reserved_names[index]);
    }
    free(context->reserved_names);
    symbol_table_destroy(context->symbols);
}

TacStatus tac_generate(const AstNode *program, TacProgram **out_program)
{
    TacContext context = {0};
    TacStatus status;

    if (out_program != NULL) {
        *out_program = NULL;
    }
    if (program == NULL || program->kind != AST_NODE_PROGRAM
        || out_program == NULL) {
        return TAC_STATUS_INVALID_ARGUMENT;
    }

    context.program = calloc(1, sizeof(*context.program));
    context.symbols = symbol_table_create();
    context.next_temporary = 1;
    context.next_label = 1;
    if (context.program == NULL || context.symbols == NULL) {
        tac_program_destroy(context.program);
        symbol_table_destroy(context.symbols);
        return TAC_STATUS_ALLOCATION_FAILURE;
    }

    status = reserve_global_storage_names(&context, program);
    if (status == TAC_STATUS_SUCCESS) {
        status = generate_statement_list(
            &context, &program->data.program.statements);
    }
    context_destroy(&context);
    if (status != TAC_STATUS_SUCCESS) {
        tac_program_destroy(context.program);
        return status;
    }

    *out_program = context.program;
    return TAC_STATUS_SUCCESS;
}

bool tac_program_print(FILE *output, const TacProgram *program)
{
    size_t index;

    if (output == NULL || program == NULL
        || (program->count != 0 && program->instructions == NULL)) {
        return false;
    }

    for (index = 0; index < program->count; index++) {
        const TacInstruction *instruction = &program->instructions[index];
        int result;

        switch (instruction->kind) {
        case TAC_INSTRUCTION_ASSIGNMENT:
            if (instruction->result == NULL
                || instruction->first_operand == NULL) {
                return false;
            }
            result = fprintf(output, "%s = %s\n",
                             instruction->result,
                             instruction->first_operand);
            break;
        case TAC_INSTRUCTION_UNARY:
            if (instruction->result == NULL
                || instruction->operator_text == NULL
                || instruction->first_operand == NULL) {
                return false;
            }
            result = fprintf(output, "%s = %s %s\n",
                             instruction->result,
                             instruction->operator_text,
                             instruction->first_operand);
            break;
        case TAC_INSTRUCTION_BINARY:
            if (instruction->result == NULL
                || instruction->operator_text == NULL
                || instruction->first_operand == NULL
                || instruction->second_operand == NULL) {
                return false;
            }
            result = fprintf(output, "%s = %s %s %s\n",
                             instruction->result,
                             instruction->first_operand,
                             instruction->operator_text,
                             instruction->second_operand);
            break;
        case TAC_INSTRUCTION_PRINT:
            if (instruction->first_operand == NULL) {
                return false;
            }
            result = fprintf(output, "print %s\n",
                             instruction->first_operand);
            break;
        case TAC_INSTRUCTION_LABEL:
            if (instruction->label == NULL) {
                return false;
            }
            result = fprintf(output, "%s:\n", instruction->label);
            break;
        case TAC_INSTRUCTION_JUMP:
            if (instruction->label == NULL) {
                return false;
            }
            result = fprintf(output, "goto %s\n", instruction->label);
            break;
        case TAC_INSTRUCTION_JUMP_IF_FALSE:
            if (instruction->first_operand == NULL
                || instruction->label == NULL) {
                return false;
            }
            result = fprintf(output, "ifFalse %s goto %s\n",
                             instruction->first_operand,
                             instruction->label);
            break;
        default:
            return false;
        }

        if (result < 0) {
            return false;
        }
    }
    return ferror(output) == 0;
}

const char *tac_status_name(TacStatus status)
{
    switch (status) {
    case TAC_STATUS_SUCCESS:
        return "TAC_SUCCESS";
    case TAC_STATUS_INVALID_ARGUMENT:
        return "TAC_INVALID_ARGUMENT";
    case TAC_STATUS_ALLOCATION_FAILURE:
        return "TAC_ALLOCATION_FAILURE";
    case TAC_STATUS_UNSUPPORTED_NODE:
        return "TAC_UNSUPPORTED_NODE";
    case TAC_STATUS_INTERNAL_ERROR:
        return "TAC_INTERNAL_ERROR";
    }
    return "TAC_UNKNOWN_STATUS";
}
