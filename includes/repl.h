#ifndef TYGER_REPL_H_
#define TYGER_REPL_H_
#include "lexer.h"

#define REPL_INPUT_BUFFER_SIZE 2048

typedef struct repl
{
  char input_buffer[REPL_INPUT_BUFFER_SIZE];
} Repl;

void repl_init(Repl *repl);
void repl_run(Repl *repl);
void repl_deinit(Repl *repl);

#endif // TYGER_REPL_H_
