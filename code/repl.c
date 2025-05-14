#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "repl.h"


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
    lexer_init(&lexer, input_buffer);

    Token token;
    while ((token = lexer_next_token(&lexer)).kind != TK_EOF)
    {
      char token_str[512];
      token_to_string(token, token_str, sizeof(token_str));
      fprintf(stdout, "    %s\n", token_str);
    }
  }
}
