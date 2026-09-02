#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "da.h"

const char *ast_statement_type_to_string(Statement_Type type)
{
    char *result;
    result = NULL;
    switch (type)
    {
    #define X(NAME) case ST_ ## NAME: { result = #NAME; } break;
        #include "defs/statement-type.def"
    #undef X
        default:;
    }
    return result;
}

const char *ast_expression_type_to_string(Expression_Type type)
{
    char *result;
    result = NULL;
    switch (type)
    {
    #define X(NAME) case ET_ ## NAME: { result = #NAME; } break;
        #include "defs/expression-type.def"
    #undef X
        default:;
    }
    return result;
}

void program_init(Program *p)
{
    int statement_init_capacity;
    int expression_init_capacity;
    volatile void *nil_statement;
    volatile void *nil_expression;
    
    statement_init_capacity  = 16;
    expression_init_capacity = 16;

    p->errors.count            = 0;
    p->errors.capacity         = 0;
    p->errors.elems            = NULL;
    p->___statements.count     = 0;
    p->___statements.capacity  = 0;
    p->___statements.elems     = NULL;
    p->___expressions.count    = 0;
    p->___expressions.capacity = 0;
    p->___expressions.elems    = NULL;

    DA_INIT(&p->errors, 8);
    DA_INIT(&p->___statements, statement_init_capacity);
    DA_INIT(&p->___expressions, expression_init_capacity);

    /* NOTE(HS): reserving first elem as "nil" value */
    p->___statements.count++;
    p->___expressions.count++;
    nil_statement  = (volatile void*) &p->___statements.elems[0];
    nil_expression = (volatile void*) &p->___expressions.elems[0];
    memset((void*) nil_statement,  0x00, sizeof(*p->___statements.elems));
    memset((void*) nil_expression, 0x00, sizeof(*p->___expressions.elems));
}

Statement_Handle program_register_statement(Program *p, Statement *stmt)
{
    Statement_Handle handle;
    handle.id = p->___statements.count;
    DA_APPEND(&p->___statements, stmt);
    return handle;
}

Expression_Handle program_register_expression(Program *p, Expression *expr)
{
    Expression_Handle handle;
    handle.id = p->___expressions.count;
    DA_APPEND(&p->___expressions, expr);
    return handle;
}

Statement *program_statement_handle_to_statement(Program *p, Statement_Handle handle)
{
    Statement *stmt;
    stmt = NULL;
    if (handle.id < p->___statements.count)
    {
        stmt = &p->___statements.elems[handle.id];
    }
    return stmt;
}

Expression *program_expression_handle_to_expression(Program *p, Expression_Handle handle)
{
    Expression *expr;
    expr = NULL;
    if (handle.id < p->___expressions.count)
    {
        expr = &p->___expressions.elems[handle.id];
    }
    return expr;
}

void ast__error_create_from_token(Error *error, Error_Type type, Token token)
{
    size_t error_message_buffer_size;
    size_t bytes_written;
    char *message;

    error_message_buffer_size = 2048;
    message = malloc(sizeof(*message) * error_message_buffer_size);

    error->where.file   = token.file;
    error->where.line   = token.line;
    error->where.col    = token.col;
    error->where.offset = token.offset;

    switch (type)
    {
        case ERT_INVALID_STATEMENT: {
            bytes_written = sprintf(
                message,
                "Invalid statement encountered at [%s:%i:%i] token `" SV_FMT "` is not allowed in statement",
                token.file, token.line, token.col,
                SV_ARGS(token.literal)
            );
        } break;

        case ERT_INVALID_INTEGER: {
            bytes_written = sprintf(
                message,
                "Invalid integer encountered at [%s:%i:%i], `" SV_FMT "`: only base-10 integers are supported",
                token.file, token.line, token.col,
                SV_ARGS(token.literal)
            );
        } break;

        default:;
    }

    message = realloc(message, sizeof(*message) * bytes_written);

    error->what = message;
}
