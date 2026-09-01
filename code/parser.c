#include "da.h"
#include "parser.h"

static void parser__next_token(Parser *ps)
{
    ps->cur_token = ps->peek_token;
    ps->peek_token = lexer_next_token(ps->lx);
}

static Error parser__parse_statement(Parser *ps, Statement *stmt)
{
    Error error;

    (void) stmt;

    switch (ps->cur_token.type)
    {
        default: {
            ast__error_create_from_token(&error, ERT_INVALID_STATEMENT, ps->cur_token);
        }
    }

    return error;
}



void parser_init(Parser *ps, Lexer *lx)
{
    ps->lx = lx;
    parser__next_token(ps);
    parser__next_token(ps);
}

Program parser_parse_program(Parser *ps)
{
    Program program;

    program_init(&program);

    while (ps->cur_token.type != TT_EOF)
    {
        Statement stmt;
        Statement_Handle handle;
        Error err;
        err = parser__parse_statement(ps, &stmt);
        if (err.type != ERT_NONE)
        {
            DA_APPEND(&program.errors, &err);
        }
        else
        {
            handle = program_register_statement(&program, &stmt);
            (void) handle;
        }
        parser__next_token(ps);
    }

    return program;
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
