#include "lexer.h"

void lexer_init(Lexer *lx, const char *program)
{
  (void*) lx;
  (void*) program;
}

const char *token_kind_to_string(Token_Kind kind)
{
  (void*) &kind;
  return NULL;
}

Token lexer_next_token(Lexer *lx)
{
  (void*) lx;
  Token token = {0};
  return token;
}
