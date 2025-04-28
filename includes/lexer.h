#ifndef TYGER_LEXER_H_
#define TYGER_LEXER_H_
#include <stddef.h>
#include "tstrings.h"

typedef enum token_kind
{
#define X(NAME) TK_##NAME,
  #include "defs/token-kind.def"
#undef X
} Token_Kind;

typedef struct location
{
  size_t pos;
  size_t col;
  size_t line;
} Location;

typedef struct token
{
  Location location;
  String_View literal;
  Token_Kind kind;
} Token;

typedef struct lexer
{
  const char *program;
  size_t program_len;
  size_t pos;
  size_t read_pos;
  size_t col;
  size_t line;
  char ch;
} Lexer;

#if defined(__cplusplus)
extern "C" {
#endif

void lexer_init(Lexer *lx, const char *program);
Token lexer_next_token(Lexer *lx);
const char *token_kind_to_string(Token_Kind kind);

#if defined (__cplusplus)
}
#endif

#endif // TYGER_LEXER_H_

