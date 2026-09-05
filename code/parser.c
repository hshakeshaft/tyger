#include "da.h"
#include "parser.h"

static void parser__next_token(Parser *ps)
{
    ps->cur_token = ps->peek_token;
    ps->peek_token = lexer_next_token(ps->lx);
}

static int parser__cur_token_is(Parser *ps, Token_Type type)
{
    return ps->cur_token.type == type;
}

static int parser__peek_token_is(Parser *ps, Token_Type type)
{
    return ps->peek_token.type == type;
}

static int parser__expect_peek(Parser *ps, Token_Type type)
{
    int result;
    result = 0;
    if (parser__peek_token_is(ps, type))
    {
        parser__next_token(ps);
        result = 1;
    }
    return result;
}


/* NOTE(HS): only base-10 integers are supported */
static Error parser__parse_integer_expression(Program *p, Parser *ps, Expression_Handle *handle)
{
    Error error;
    Expression expr;
    long int parsed_integer;
    char *literal_end;

    memset(&error, 0x00, sizeof(error));

    literal_end  = ps->cur_token.literal.str;
    literal_end += ps->cur_token.literal.len;

    parsed_integer = strtol(ps->cur_token.literal.str, &literal_end, 10);

    /* NOTE(HS): if the literal is `0` and the parsed integer value is `0` I think
    it is safe to assume there was no error in converting the string value.

    If however the integer is any other value, and the parsed value is `0` then this
    is an error.
    */
    if (ps->cur_token.literal.str[0] != '0' && parsed_integer == 0)
    {
        ast__error_create_from_token(&error, ERT_INVALID_INTEGER, ps->cur_token);
    }
    else
    {
        expr.type             = ET_INTEGER;
        expr.as.integer.value = parsed_integer;
        *handle               = program_register_expression(p, &expr);
    }

    parser__next_token(ps);

    return error;
}

static Error parser__parse_string_expression(Program *p, Parser *ps, Expression_Handle *handle)
{
    Error error;
    Expression expr;
    char *strbuf;

    memset(&error, 0x00, sizeof(error));

    /* TODO(HS): handle allocation failure */
    strbuf = malloc(sizeof(*strbuf) * (ps->cur_token.literal.len + 1));
    strncpy(strbuf, ps->cur_token.literal.str, ps->cur_token.literal.len);

    expr.type          = ET_STRING;
    expr.as.string.ptr = strbuf;
    expr.as.string.len = ps->cur_token.literal.len;
    *handle            = program_register_expression(p, &expr);

    parser__next_token(ps);

    return error;
}

static Error parser__parse_ident_expression(Program *p, Parser *ps, Expression_Handle *handle)
{
    Error error;
    Expression expr;
    char *strbuf;

    memset(&error, 0x00, sizeof(error));

    strbuf = malloc(sizeof(*strbuf) * (ps->cur_token.literal.len + 1));
    strncpy(strbuf, ps->cur_token.literal.str, ps->cur_token.literal.len);
    strbuf[ps->cur_token.literal.len] = '\0';

    expr.type          = ET_IDENT;
    expr.as.ident.name = strbuf;
    *handle            = program_register_expression(p, &expr);

    parser__next_token(ps);

    return error;
}

static Error parser__parse_expression(Program *p, Parser *ps, Expression_Handle *handle)
{
    Error error;
    switch (ps->cur_token.type)
    {
        case TT_INT: {
            error = parser__parse_integer_expression(p, ps, handle);
        } break;

        case TT_STRING: {
            error = parser__parse_string_expression(p, ps, handle);
        } break;

        case TT_IDENT: {
            error = parser__parse_ident_expression(p, ps, handle);
        } break;

        default:;
    }
    return error;
}


static Error parser__parse_expression_statement(Program *p, Parser *ps, Statement *stmt)
{
    Error error;
    Expression_Handle handle;
    memset(&error, 0x00, sizeof(error));

    error = parser__parse_expression(p, ps, &handle);
    if (error.type != ERT_NONE)
    {
        return error;
    }

    stmt->type                 = ST_EXPRESSION;
    stmt->as.expression.handle = handle;

    return error;
}


static Error parser__parse_statement(Program *p, Parser *ps, Statement *stmt)
{
    Error error;
    memset(&error, 0x00, sizeof(error));

    switch (ps->cur_token.type)
    {
        case TT_INT:
        case TT_STRING:
        case TT_IDENT:
        {
            error = parser__parse_expression_statement(p, ps, stmt);
        } break;

        default: {
            ast__error_create_from_token(&error, ERT_INVALID_STATEMENT, ps->cur_token);
        }
    }

    if (error.type != ERT_NONE)
    {
        return error;
    }

    if (!parser__cur_token_is(ps, TT_SEMICOLON))
    {
        ast__error_create_from_token(&error, ERT_INVALID_STATEMENT, ps->cur_token);
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
        err = parser__parse_statement(&program, ps, &stmt);
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
