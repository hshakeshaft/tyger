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

static void parser_next_token(Parser *p)
{
  p->cur_token = p->peek_token;
  p->peek_token = lexer_next_token(p->lexer);
}

void parser_init(Parser *p, Lexer *lx)
{
  p->lexer = lx;
  parser_next_token(p);
  parser_next_token(p);
}
  
static void parser_context_init(Parser_Context *ctx)
{
  va_array_init(char, ctx->identifiers);
  va_array_init(Expression, ctx->expressions);
  va_array_init(char, ctx->strings);
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
  Tyger_Error err = {0};

  Expression expr;
  switch (p->cur_token.kind)
  {
  case TK_INTEGER:
  {
    err = parse_int_expression(p, &expr);
  } break;

  case TK_STRING:
  {
    err = parse_string_expression(p, ctx, &expr);
  } break;

  default:
  {
    fprintf(
      stderr, "[ERROR] Unhanlded Token_Kind whilst parsing expression, %s\n",
      token_kind_to_string(p->cur_token.kind)
    );
  } break;
  }

  Expression *hexpr = &(ctx->expressions.elems[ctx->expressions.len]);
  if (err.kind == TYERR_NONE)
  {
    va_array_append(ctx->expressions, expr);
  }

  // TODO(HS): assign stmt->->expression
  stmt->kind = STMT_EXPRESSION;
  stmt->statement.expression_statement.expression = hexpr;

  return err;
}

// TODO(HS): parse int into int64_t
Tyger_Error parse_int_expression(Parser *p, Expression *expr)
{
  (void*) expr;
  Tyger_Error err = {0};

  int64_t result = atoll(p->cur_token.literal.str);
  if (result == 0 && !string_view_eq_str(p->cur_token.literal, "0"))
  {
    err.kind = TYERR_INVALID_INTEGER;
    return err;
  }

  expr->kind = EXPR_INT;
  expr->expression.int_expression.value = result;

  parser_next_token(p);
  return err;
} 

// TODO(HS): more robust string parsing
Tyger_Error parse_string_expression(Parser *p, Parser_Context *ctx, Expression *expr)
{
  Tyger_Error err = {0};

  const char *str_value = &(ctx->strings.elems[ctx->strings.len]);
  va_array_append_n(ctx->strings, p->cur_token.literal.str, p->cur_token.literal.len);
  va_array_append_n(ctx->strings, PARSER_NULL_TERMINATOR, 1);

  expr->kind = EXPR_STRING;
  expr->expression.string_expression = (String_Expression) {
    .value = str_value,
    .len = p->cur_token.literal.len
  };

  parser_next_token(p);
  return err;
}
