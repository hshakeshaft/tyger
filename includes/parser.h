#ifndef TYGER_PARSER_H_
#define TYGER_PARSER_H_
#include <stddef.h>
#include "lexer.h"

typedef struct statement_vaarray
{
  char *ptr;
  size_t capacity;
  size_t len;
} Statement_VaArray;

typedef struct error_vaarray
{
  char *ptr;
  size_t capacity;
  size_t len;
} Error_VaArray;

typedef struct parser
{
  Lexer *lexer;
  Token cur_token;
  Token peek_token;
} Parser;

typedef struct program
{
  Statement_VaArray statements;
  Error_VaArray errors;
} Program;

void parser_init(Parser *p, Lexer *lx);
Program parser_parse_program(Parser *p);
const char *program_to_string(const Program *p);

#endif // TYGER_PARSER_H_
