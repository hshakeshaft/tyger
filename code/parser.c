#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "lexer.h"
#include "util.h"

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
  
Program parser_parse_program(Parser *p)
{
  Program program = {0};
  va_array_init(Tyger_Error, program.errors);
  va_array_init(char, program.statements);

  while (p->cur_token.kind != TK_EOF)
  {
    Statement stmt = {0};
    Tyger_Error err = parser_parse_statement(p, &stmt);

    if (err.kind == TYERR_NONE)
    {
      va_array_append(program.errors, err);
    }
    else
    {
      va_array_append(program.statements, stmt);
    }
    parser_next_token(p);
  }

  return program;
}

Tyger_Error parser_parse_statement(Parser *p, Statement *stmt)
{
  Tyger_Error err = {0};

  switch (p->cur_token.kind)
  {
  case TK_VAR:
  {
    err = parse_var_statement(p, stmt);
  } break;

  default:
  {
    fprintf(
      stderr, "[ERROR] unexpected value for Token_Kind: %i\n", p->cur_token.kind
    );
    assert(0);
  } break;
  }

  return err;
}

static inline bool peek_token_is(Parser *p, Token_Kind kind)
{
  return p->cur_token.kind == kind;
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

Tyger_Error parse_var_statement(Parser *p, Statement *stmt)
{
  Tyger_Error err = {0};

  if (!expect_peek(p, TK_IDENT))
  {
    err.kind = TYERR_SYNTAX;
  }

  // TODO(HS): read ident

  if (!expect_peek(p, TK_ASSIGN))
  {
    err.kind = TYERR_SYNTAX;
  }

  // TODO(HS): parse expression
  while (p->cur_token.kind != TK_SEMICOLON)
  {
    parser_next_token(p);
  }

  stmt->kind = STMT_VAR;

  return err;
}
