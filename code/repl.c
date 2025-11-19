#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "repl.h"
#include "lexer.h"
#include "parser.h"
#include "eval.h"
#include "object.h"

// TODO(HS): handle Ctrl+c/d exits nicely?
// TODO(HS): control sequences
// TODO(HS): implement a "history" buffer
void repl_run(void)
{
  while (true)
  {
    fprintf(stdout, "tyger> ");

    char input_buffer[REPL_INPUT_BUFFER_SIZE];
    memset(input_buffer, '\0', sizeof(input_buffer));

    if (!fgets(input_buffer, sizeof(input_buffer), stdin))
    {
      fprintf(stderr, "[ERROR]: Error getting user input from stdin\n");
      break;
    }

    Lexer lexer;
    Parser parser;

    lexer_init(&lexer, input_buffer);
    parser_init(&parser, &lexer);

    Program program = parser_parse_program(&parser);
    TyObj res = eval(&program);
    tyobj_inspect(&res);

    program_free(&program);
  }
}
