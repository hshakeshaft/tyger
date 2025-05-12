#include <stdio.h>
#include "lexer.h"
#include "repl.h"

int main(void)
{
  Repl repl;
  repl_init(&repl);
  repl_run(&repl);
  repl_deinit(&repl);
}
