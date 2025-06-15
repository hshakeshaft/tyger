#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "lexer.h"

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
  (void*) p;
  Program program = {0};
  while (p->cur_token.kind != TK_EOF)
  {
    parser_next_token(p);
  }
  return program;
}

const char *program_to_string(const Program *p)
{
  (void*) p;
  #define PTS_TEST_STR "Hello, World!\n"
  char *buffer = malloc(sizeof(char) * sizeof(PTS_TEST_STR) + 1);
  snprintf(buffer, sizeof(PTS_TEST_STR) + 1, PTS_TEST_STR);
  buffer[sizeof(PTS_TEST_STR)] = '\0';
  return buffer;
}
