#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "repl.h"

void repl_init(Repl *repl)
{
    memset(repl->input_buffer, '\0', sizeof(repl->input_buffer));
}

// TODO(HS): handle Ctrl+c/d exits nicely?
// TODO(HS): control sequences
// TODO(HS): implement a "history" buffer
void repl_run(Repl *repl)
{
  do
  {
    fprintf(stdout, "tyger> ");

    if (!fgets(repl->input_buffer, sizeof(repl->input_buffer), stdin))
    {
      fprintf(stderr, "[ERROR]: Error getting user input from stdin\n");
      break;
    }

    Lexer lexer;
    lexer_init(&lexer, repl->input_buffer);

    Token token;
    while ((token = lexer_next_token(&lexer)).kind != TK_EOF)
    {
      char token_str[512];
      token_to_string(token, token_str, sizeof(token_str));
      fprintf(stdout, "    %s\n", token_str);
    }

  } while (true);
}

void repl_deinit(Repl *repl)
{
    memset(repl->input_buffer, '\0', sizeof(repl->input_buffer));
}
