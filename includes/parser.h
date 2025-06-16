#ifndef TYGER_PARSER_H_
#define TYGER_PARSER_H_
#include <stddef.h>
#include "lexer.h"

typedef enum tyger_error_kind
{
#define X(NAME) TYERR_##NAME,
  #include "defs/tyger-error-kind.def"
#undef X
} Tyger_Error_Kind;

typedef enum statement_kind
{
#define X(NAME) STMT_##NAME,
  #include "defs/statement-kind.def"
#undef X
} Statement_Kind;

typedef struct tyger_error
{
  Tyger_Error_Kind kind;
} Tyger_Error;

typedef struct statement
{
  Statement_Kind kind;
} Statement;

typedef struct statement_vaarray
{
  Statement *elems;
  size_t capacity;
  size_t len;
} Statement_VaArray;

typedef struct error_vaarray
{
  Tyger_Error *elems;
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

const char *tyger_error_kind_to_string(Tyger_Error_Kind kind);
const char *statement_kind_to_string(Statement_Kind kind);

Tyger_Error parser_parse_statement(Parser *p, Statement *stmt);
Tyger_Error parse_var_statement(Parser *p, Statement *stmt);

#endif // TYGER_PARSER_H_
