#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

void parser_init(Parser *p, Lexer *lx)
{
  (void*) p;
  (void*) lx;
}
  
Program parser_parse_program(Parser *p)
{
  (void*) p;
  Program program = {0};
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
