#include <assert.h>
// TODO(HS): impl own c-string functions
#include <string.h>
#include <stdio.h>
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
  case ',':  { token.kind = TK_COMMA; } break;

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

  default:
  {
    // TODO(HS): impl float support
    if (is_digit(lx->ch))
    {
      size_t pos = lx->pos;
      lexer_read_number(lx);
      size_t num_len = lx->pos - pos;
      token.kind = TK_INTEGER;
      token.literal.len = num_len;
      return token;
    }
    else if (lx->ch == '\"')
    {
      // NOTE(HS): "consumes" the surroinding quotes as these aren't needed to store
      // the fact this is a string
      lexer_read_char(lx);
      size_t pos = lx->pos;
      lexer_read_string(lx);
      size_t str_len = lx->pos - pos;
      lexer_read_char(lx);
      
      token.kind = TK_STRING;
      token.location.pos = pos;
      token.literal = (String_View) {
        .str = token.literal.str + 1,
        .len = str_len
      };

      return token;
    }
    else if (is_char(lx->ch))
    {
      // NOTE(HS): ident/keyword/builtin parsing
      size_t pos = lx->pos;
      lexer_read_ident_or_keyword(lx);
      size_t len = lx->pos - pos;
      token.literal.len = len;
      token.kind = string_view_to_token_kind(token.literal);
      return token;
    }
    else
    {
      token.kind = TK_ILLEGAL;
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
  while (is_whitespace(lx->ch) && lx->ch != '\0')
  {
    lexer_read_char(lx);
  }
}

void lexer_read_number(Lexer *lx)
{
  while (is_digit(lx->ch) && lx->ch != '\0')
  {
    lexer_read_char(lx);
  }
}

void lexer_read_string(Lexer *lx)
{
  while (lx->ch != '\"' && lx->ch != '\0')
  {
    if (lx->ch == '\\' && lexer_peek_char(lx) == '\"')
    {
      lexer_read_char(lx);
    }
    lexer_read_char(lx);
  }
}

void lexer_read_ident_or_keyword(Lexer *lx)
{
  while ((is_char(lx->ch) || is_digit(lx->ch)) && lx->ch != '\0')
  {
    lexer_read_char(lx);
  }
}

Token_Kind string_view_to_token_kind(String_View sv)
{
#define lexer_sv_cmp_str(SV, S) \
  ((strlen((S)) == (SV).len) && (strncmp((S), (SV).str, (SV).len) == 0))

  Token_Kind kind;
  if      (lexer_sv_cmp_str(sv, "var"))     { kind = TK_VAR; }
  else if (lexer_sv_cmp_str(sv, "println")) { kind = TK_PRINTLN; }
  else                                      { kind = TK_IDENT; }

  return kind;
}

// TODO(HS): make UTF8 compliant
inline bool is_whitespace(char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool is_digit(char c)
{
  return ('0' <= c && c <= '9');
}

inline bool is_char(char c)
{
  return ( ('a' <= c && c <= 'z') || ('A' <= c && c <= 'z') );
}

void token_to_string(Token t, char *buffer, int buffer_size)
{
  const char *tk_str = token_kind_to_string(t.kind);
  int bytes_to_write = snprintf(NULL, 0, TOKEN_FMT, TOKEN_ARGS(t, tk_str));
  assert(bytes_to_write <= buffer_size);
  int bytes_written = snprintf(buffer,
                               bytes_to_write + 1,
                               TOKEN_FMT,
                               TOKEN_ARGS(t, tk_str));
  assert(bytes_written == bytes_to_write);
  buffer[bytes_to_write] = '\0';
}
