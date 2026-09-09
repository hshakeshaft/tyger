#include "da.h"
#include "parser.h"

static Error parser__parse_expression(
    Program *p,
    Parser *ps,
    Expression_Handle *handle,
    int precidence
);


enum {
    PRECIDENCE_LOWEST      = 0,
    PRECIDENCE_EQUALS      = 10, /* ==            */
    PRECIDENCE_LESSGREATER = 20, /* > or <        */
    PRECIDENCE_SUM         = 30, /* +             */
    PRECIDENCE_PRODUCT     = 40, /* *             */
    PRECIDENCE_PREFIX      = 50, /* -X or !X      */
    PRECIDENCE_CALL        = 60  /* myFunction(X) */
};

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

static int parser__precidence_of(Token_Type type)
{
    int precidence;
    precidence = PRECIDENCE_LOWEST;
    switch (type)
    {
        case TT_EQ: {
            precidence = PRECIDENCE_EQUALS;
        } break;

        case TT_LT:
        case TT_GT:
        {
            precidence = PRECIDENCE_LESSGREATER;
        } break;

        case TT_ADD:
        case TT_SUB:
        {
            precidence = PRECIDENCE_SUM;
        } break;

        case TT_MUL:
        case TT_DIV:
        {
            precidence = PRECIDENCE_PRODUCT;
        } break;

        default:;
    }
    return precidence;
}

static int parser__peek_precidence(Parser *ps)
{
    return parser__precidence_of(ps->peek_token.type);
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

    return error;
}

static Error parser__parse_infix_expression(Program *p, Parser *ps, Expression_Handle *handle)
{
    Error error;
    Expression_Handle lhs_handle;
    Expression_Handle rhs_handle;
    Expression infix_expression;
    Expression_Handle infix_handle;
    int precidence;
    Operator operator;

    memset(&error, 0x00, sizeof(error));

    precidence = parser__precidence_of(ps->cur_token.type);
    operator   = ast_operator_from_token_type(ps->cur_token.type);
    lhs_handle = *handle;

    parser__next_token(ps);

    error = parser__parse_expression(p, ps, &rhs_handle, precidence);
    if (error.type != ERT_NONE)
    {
        return error;
    }

    infix_expression.type         = ET_INFIX;
    infix_expression.as.infix.op  = operator;
    infix_expression.as.infix.lhs = lhs_handle;
    infix_expression.as.infix.rhs = rhs_handle;

    infix_handle = program_register_expression(p, &infix_expression);
    *handle      = infix_handle;

    return error;
}

static Error parser__parse_expression(
    Program *p,
    Parser *ps,
    Expression_Handle *handle,
    int precidence
)
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

    if (error.type != ERT_NONE)
    {
        return error;
    }

    while (
        !parser__peek_token_is(ps, TT_SEMICOLON) 
        && (precidence < parser__peek_precidence(ps))
    )
    {
        parser__next_token(ps);
        error = parser__parse_infix_expression(p, ps, handle);
        if (error.type != ERT_NONE)
        {
            return error;
        }
    }

    return error;
}


static Error parser__parse_expression_statement(Program *p, Parser *ps, Statement *stmt)
{
    Error error;
    Expression_Handle handle;
    memset(&error, 0x00, sizeof(error));

    error = parser__parse_expression(p, ps, &handle, PRECIDENCE_LOWEST);
    if (error.type != ERT_NONE)
    {
        return error;
    }

    stmt->type                 = ST_EXPRESSION;
    stmt->as.expression.handle = handle;

    return error;
}

static Error parser__parse_var_statement(Program *p, Parser *ps, Statement *stmt)
{
    Error error;
    Expression_Handle handle;
    char *ident;
    memset(&error, 0x00, sizeof(error));

    if (!parser__expect_peek(ps, TT_IDENT))
    {
        ast__error_create_from_token(&error, ERT_INVALID_VAR_DECLARATION, ps->peek_token);
        return error;
    }

    ident = malloc(sizeof(*ident) * (ps->cur_token.literal.len + 1));
    strncpy(ident, ps->cur_token.literal.str, ps->cur_token.literal.len);
    ident[ps->cur_token.literal.len] = '\0';

    if (!parser__expect_peek(ps, TT_ASSIGN))
    {
        ast__error_create_from_token(&error, ERT_INVALID_VAR_DECLARATION, ps->peek_token);
        return error;
    }
    /* should consume the `=` */
    parser__next_token(ps);

    error = parser__parse_expression(p, ps, &handle, PRECIDENCE_LOWEST);
    if (error.type != ERT_NONE)
    {
        return error;
    }

    stmt->type              = ST_VAR;
    stmt->as.var.ident      = ident;
    stmt->as.var.expression = handle;

    return error;
}

static Error parser__parse_statement(Program *p, Parser *ps, Statement *stmt)
{
    Error error;
    memset(&error, 0x00, sizeof(error));

    switch (ps->cur_token.type)
    {
        case TT_KW_VAR: {
            error = parser__parse_var_statement(p, ps, stmt);
        } break;

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

    if (!parser__expect_peek(ps, TT_SEMICOLON))
    {
        ast__error_create_from_token(&error, ERT_UNTERMINATED_STATEMENT, ps->cur_token);
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
