#include <assert.h>
// TODO(HS): impl own c-string functions
#include <string.h>
#include "lexer.h"
#include "lexer_internal.h"

void lexer_init(Lexer *lx, const char *program)
{
  lx->program = program;
  lx->pos = 0;
  lx->read_pos = 0;
  lx->col = 0;
  lx->line = 0;
  lx->ch = '\0';

  size_t prog_len = strlen(program);
  lx->program_len = prog_len;

  lexer_read_char(lx);
}

const char *token_kind_to_string(Token_Kind kind)
{
  char *str = NULL;
  switch (kind)
  {
#define X(NAME) case TK_##NAME: { str = #NAME ; } break;
    #include "defs/token-kind.def"
#undef X
  default:
  {
    assert(0 && "Invalid Token_Kind value");
  } break;
  }
  return str;
}

Token lexer_next_token(Lexer *lx)
{
  lexer_skip_whitespace(lx);

  // NOTE(HS): default initialisation of token in
  Token token = {
    .location = make_location(lx->pos, lx->col, lx->line),
    .literal = make_string_view_ex(lx->program, lx->pos, 1),
  };

  switch (lx->ch)
  {
  case '\0': { token.kind = TK_EOF; } break;

  case '(':  { token.kind = TK_LPAREN; } break;
  case ')':  { token.kind = TK_RPAREN; } break;
  case '{':  { token.kind = TK_LBRACE; } break;
  case '}':  { token.kind = TK_RBRACE; } break;
  case '[':  { token.kind = TK_LBRACKET; } break;
  case ']':  { token.kind = TK_RBRACKET; } break;
  case ';':  { token.kind = TK_SEMICOLON; } break;

  case '+':  { token.kind = TK_PLUS; } break;
  case '-':  { token.kind = TK_MINUS; } break;
  case '*':  { token.kind = TK_ASTERISK; } break;
  case '/':  { token.kind = TK_SLASH; } break;

  case '!':
  {
    if (lexer_peek_char(lx) == '=')
    {
      token.kind = TK_NOT_EQ;
      token.literal.len += 1;
      lexer_read_char(lx);
    }
    else
    {
      token.kind = TK_BANG;
    }
  } break;

  case '=':
  {
    if (lexer_peek_char(lx) == '=')
    {
      token.kind = TK_EQ;
      token.literal.len += 1;
      lexer_read_char(lx);
    }
    else
    {
      token.kind = TK_ASSIGN;
    }
  } break;

  case '<':
  {
    if (lexer_peek_char(lx) == '=')
    {
      token.kind = TK_LTE;
      token.literal.len += 1;
      lexer_read_char(lx);
    }
    else
    {
      token.kind = TK_LT;
    }
  } break;

  case '>':
  {
    if (lexer_peek_char(lx) == '=')
    {
      token.kind = TK_GTE;
      token.literal.len += 1;
      lexer_read_char(lx);
    }
    else
    {
      token.kind = TK_GT;
    }
  } break;

  }

  lexer_read_char(lx);
  return token;
}

// TODO(HS): implement col and line tracking
void lexer_read_char(Lexer *lx)
{
  if (lx->read_pos >= lx->program_len)
  {
    lx->ch = '\0';
  }
  else
  {
    lx->ch = lx->program[lx->read_pos];
  }
  lx->pos = lx->read_pos++;
}

char lexer_peek_char(Lexer *lx)
{
  if (lx->read_pos >= lx->program_len)
  {
    return '\0';
  }
  else
  {
    return lx->program[lx->read_pos];
  }
}

void lexer_skip_whitespace(Lexer *lx)
{
  while (is_whitespace(lx->ch))
  {
    lexer_read_char(lx);
  }
}


// TODO(HS): make UTF8 compliant
inline bool is_whitespace(char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
