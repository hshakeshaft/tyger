#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "lexer.h"
#include "util.h"

// NOTE(HS): way to fudge being able to pass a string containing the null terminator
// into va_array_append_n calls.
static const char *PARSER_NULL_TERMINATOR = "\0";

enum {
  PRECIDENCE_LOWEST      = 0,
  PRECIDENCE_EQUALS      = 10, // ==
  PRECIDENCE_LESSGREATER = 20, // > or <
  PRECIDENCE_SUM         = 30, // +
  PRECIDENCE_PRODUCT     = 40, // *
  PRECIDENCE_PREFIX      = 50, // -X or !X
  PRECIDENCE_CALL        = 60, // myFunction(X)
};

///
/// internal functions
///
  
static void parser_context_init(Parser_Context *ctx)
{
  va_array_init(char, ctx->identifiers);
  va_array_init(char, ctx->evaluated_identifiers);
  va_array_init(Expression, ctx->expressions);
  va_array_init(char, ctx->strings);
}

static inline void parser_next_token(Parser *p)
{
  p->cur_token = p->peek_token;
  p->peek_token = lexer_next_token(p->lexer);
}

static inline int precidence_of(Token_Kind k)
{
  int precidence;
  switch (k)
  {
  case TK_PLUS:
  case TK_MINUS:
  {
    precidence = PRECIDENCE_SUM;
  } break;

  case TK_ASTERISK:
  case TK_SLASH:
  {
    precidence = PRECIDENCE_PRODUCT;
  } break;

  case TK_LT:
  case TK_GT:
  {
    precidence = PRECIDENCE_LESSGREATER;
  } break;

  case TK_EQ:
  case TK_NOT_EQ:
  {
    precidence = PRECIDENCE_EQUALS;
  } break;

  default:
  {
    precidence = PRECIDENCE_LOWEST;
  } break;
  }
  return precidence;
}

static inline int peek_precidence(const Parser *p)
{
  return precidence_of(p->peek_token.kind);
}

static inline bool peek_token_is(Parser *p, Token_Kind kind)
{
  return p->peek_token.kind == kind;
}

static inline bool expect_peek(Parser *p, Token_Kind kind)
{
  if (peek_token_is(p, kind))
  {
    parser_next_token(p);
    return true;
  }
  else
  {
    return false;
  }
}

static inline Operator token_kind_to_operator(Token_Kind k)
{
  Operator op = OP_NONE;
  switch (k)
  {
  case TK_PLUS:     { op = OP_PLUS; } break;
  case TK_MINUS:    { op = OP_MINUS; } break;
  case TK_ASTERISK: { op = OP_ASTERISK; } break;
  case TK_SLASH:    { op = OP_SLASH; } break;
  case TK_EQ:       { op = OP_EQ; } break;
  case TK_NOT_EQ:   { op = OP_NOT_EQ; } break;
  case TK_LT:       { op = OP_LT; } break;
  case TK_GT:       { op = OP_GT; } break;
  default:
  {
    fprintf(stderr, "[ERROR] Cannot convert token %s to operator\n", token_kind_to_string(k));
    assert(0);
  } break;
  }
  return op;
}

// TODO(HS): decide where/when in `expr` lifetime I need to perform copy into backing
// ctx buffer
static Tyger_Error parse_expression(Parser *p, Parser_Context *ctx, Expression *expr, int precidence)
{
  Tyger_Error err = {0};

  switch (p->cur_token.kind)
  {
  case TK_INTEGER:
  {
    err = parse_int_expression(p, expr);
  } break;

  case TK_STRING:
  {
    err = parse_string_expression(p, ctx, expr);
  } break;

  case TK_IDENT:
  {
    err = parse_ident_expression(p, ctx, expr);
  } break;
  }

  if (err.kind == TYERR_NONE)
  {
    while (!peek_token_is(p, TK_SEMICOLON) && (precidence < peek_precidence(p)))
    {
      parser_next_token(p);
      err = parse_infix_expression(p, ctx, expr);
    }
  }

  return err;
}


///
/// public functions
///

const char *tyger_error_kind_to_string(Tyger_Error_Kind kind)
{
  const char *str;

  switch (kind)
  {
#define X(NAME) case TYERR_##NAME: { str = #NAME; } break;
    #include "defs/tyger-error-kind.def"
#undef X

  default:
  {
    fprintf(stderr, "[ERROR] Invalid error kind encountered: %i\n", kind);
    str = NULL;
    assert(0);
  } break;
  }

  return str;
}

const char *statement_kind_to_string(Statement_Kind kind)
{
  const char *str;

  switch (kind)
  {
#define X(NAME) case STMT_##NAME: { str = #NAME; } break;
    #include "defs/statement-kind.def"
#undef X

  default:
  {
    fprintf(stderr, "[ERROR] Invalid statement kind encountered: %i\n", kind);
    str = NULL;
    assert(0);
  } break;
  }

  return str;
}

const char *expression_kind_to_string(Expression_Kind kind)
{
  const char *str;

  switch (kind)
  {
#define X(NAME) case EXPR_##NAME: { str = #NAME; } break;
    #include "defs/expression-kind.def"
#undef X

  default:
  {
    fprintf(stderr, "[ERROR] Invalid expression kind encountered: %i\n", kind);
    str = NULL;
    assert(0);
  } break;
  }

  return str;
}

const char *operator_to_string(Operator op)
{
  const char *str;
  switch (op)
  {
#define X(NAME, STR) case OP_##NAME: { str = STR; } break;
    #include "defs/operator.def"
#undef X

  default:
  {
    str = NULL;
    fprintf(stderr, "[ERROR] Invalid operator %i encountered\n", op);
    assert(0);
  } break;
  }
  return str;
}

void parser_init(Parser *p, Lexer *lx)
{
  p->lexer = lx;
  parser_next_token(p);
  parser_next_token(p);
}

Program parser_parse_program(Parser *p)
{
  Parser_Context ctx;
  parser_context_init(&ctx);

  Program program = {0};
  va_array_init(Tyger_Error, program.errors);
  va_array_init(Statement, program.statements);

  while (p->cur_token.kind != TK_EOF)
  {
    Statement stmt = {0};
    Tyger_Error err = parser_parse_statement(p, &ctx, &stmt);

    if (err.kind != TYERR_NONE)
    {
      va_array_append(program.errors, err);
    }
    else
    {
      va_array_append(program.statements, stmt);
    }
    parser_next_token(p);
  }

  program.context = ctx;
  return program;
}

Tyger_Error parser_parse_statement(Parser *p, Parser_Context *ctx, Statement *stmt)
{
  Tyger_Error err = {0};

  switch (p->cur_token.kind)
  {
  case TK_VAR:
  {
    err = parse_var_statement(p, ctx, stmt);
  } break;

  default:
  {
    err = parse_expression_statement(p, ctx, stmt);
  } break;
  }

  return err;
}

Tyger_Error parse_var_statement(Parser *p, Parser_Context *ctx, Statement *stmt)
{
  Tyger_Error err = {0};

  if (!expect_peek(p, TK_IDENT))
  {
    err.kind = TYERR_SYNTAX;
    return err;
  }

  char *ident_start;
  { // copy ident into dynamic array
    assert(p->cur_token.literal.str);
    assert(p->cur_token.literal.len > 0);
    ident_start = &(ctx->identifiers.elems[ctx->identifiers.len]);
    va_array_append_n(ctx->identifiers, p->cur_token.literal.str, p->cur_token.literal.len);
    va_array_append_n(ctx->identifiers, PARSER_NULL_TERMINATOR, 1);
  }

  if (!expect_peek(p, TK_ASSIGN))
  {
    err.kind = TYERR_SYNTAX;
    return err;
  }

  // TODO(HS): parse expression
  while (p->cur_token.kind != TK_SEMICOLON)
  {
    parser_next_token(p);
  }

  stmt->kind = STMT_VAR;
  stmt->statement.var_statement = (Var_Statement) {
    .ident = ident_start
  };

  return err;
}

Tyger_Error parse_expression_statement(Parser *p, Parser_Context *ctx, Statement *stmt)
{
  Expression expr;
  Tyger_Error err = parse_expression(p, ctx, &expr, PRECIDENCE_LOWEST);

  Expression *hexpr = &(ctx->expressions.elems[ctx->expressions.len]);
  if (err.kind == TYERR_NONE)
  {
    va_array_append(ctx->expressions, expr);
  }
  else
  {
    return err;
  }

  *stmt = (Statement) {
    .kind = STMT_EXPRESSION,
    .statement.expression_statement = (Expression_Statement) {
      .expression = hexpr,
    }
  };

  if (peek_token_is(p, TK_SEMICOLON))
  {
    parser_next_token(p);
  }

  return err;
}

Tyger_Error parse_int_expression(Parser *p, Expression *expr)
{
  Tyger_Error err = {0};

  int64_t result = atoll(p->cur_token.literal.str);
  if (result == 0 && !string_view_eq_str(p->cur_token.literal, "0"))
  {
    err.kind = TYERR_INVALID_INTEGER;
    return err;
  }

  *expr = (Expression) {
    .kind = EXPR_INT,
    .expression.int_expression = (Int_Expression) {
      .value = result,
    }
  };

  return err;
} 

// TODO(HS): more robust string parsing
Tyger_Error parse_string_expression(Parser *p, Parser_Context *ctx, Expression *expr)
{
  Tyger_Error err = {0};

  const char *str_value = &(ctx->strings.elems[ctx->strings.len]);
  va_array_append_n(ctx->strings, p->cur_token.literal.str, p->cur_token.literal.len);
  va_array_append_n(ctx->strings, PARSER_NULL_TERMINATOR, 1);

  *expr = (Expression) {
    .kind = EXPR_STRING,
    .expression.string_expression = (String_Expression) {
      .value = str_value,
      .len = p->cur_token.literal.len
    }
  };

  return err;
}

Tyger_Error parse_ident_expression(Parser *p, Parser_Context *ctx, Expression *expr)
{
  Tyger_Error err= {0};

  const char *ident = va_array_next(ctx->evaluated_identifiers);
  va_array_append_n(ctx->evaluated_identifiers, p->cur_token.literal.str, p->cur_token.literal.len);
  va_array_append_n(ctx->evaluated_identifiers, PARSER_NULL_TERMINATOR, 1);

  *expr = (Expression) {
    .kind = EXPR_IDENT,
    .expression.ident_expression = (Ident_Expression) {
      .ident = ident
    }
  };

  return err;
}

Tyger_Error parse_infix_expression(Parser *p, Parser_Context *ctx, Expression *expr)
{
  // NOTE(HS): Code does the following steps
  //   1. create handle for `LHS` of infix to be stored at
  //   2. copy supplied `expr` into `ctx->expressions` (`LHS`)
  //   3. parse expression using `RHS`
  //   4. assuming `RHS` parse correctly, create handle to `RHS`
  //   5. copy `RHS` into `ctx->expressions`
  //   6. override `expr` parameter with `infix` values
  //
  // This in this order as it guarentees that copies of both LHS and RHS into the
  // backing buffer is performed correctly.
  //
  // When both copies done after parse(RHS) it was noted that the `RHS` was getting
  // copied, incorrectly, into both the `LHS` and `RHS` side of more complex expressions,
  // such as `1 + 2 * 3`
  //   Expected: (+ 1 (* 2 3))
  //   Actual:   (+ 2 (* 2 3))
  // Such are the joys of overriding inout parameters an arbitrary number of times.
  //
  Tyger_Error err = {0};

  int precidence = precidence_of(p->cur_token.kind);
  Operator op = token_kind_to_operator(p->cur_token.kind);
  parser_next_token(p);

  Expression *lhs_handle = va_array_next(ctx->expressions);
  va_array_append(ctx->expressions, *expr);

  Expression rhs;
  err = parse_expression(p, ctx, &rhs, precidence);
  if (err.kind != TYERR_NONE)
  {
    return err;
  }
  Expression *rhs_handle = va_array_next(ctx->expressions);
  va_array_append(ctx->expressions, rhs);

  *expr = (Expression) {
    .kind = EXPR_INFIX,
    .expression.infix_expression = (Infix_Expression) {
      .op = op,
      .lhs = lhs_handle,
      .rhs = rhs_handle,
    }
  };

  return err;
}
