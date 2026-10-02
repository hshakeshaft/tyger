/*
Repl improvements:
- TODO: incremental evaluation, i.e. `var x = 10;` is evaluated once, then `x + 10;`
is evaluated separately, but as part of the same "VM"
    - NOTE: might mean freeing "line" after each eval, also VM needs to persist in
    outer loop

- TODO: history support
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "repl.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "vm.h"
#include "eval.h"

#define REPL_PROMPT "tyger>"
#define REPL_INIT_LINE_BUFFER_SIZE 1024

size_t G__line_length = REPL_INIT_LINE_BUFFER_SIZE;


static void repl__print_object(TyObject *object)
{
    switch (object->type)
    {
        case OBJ_NONE: {
            fprintf(stdout, "(nil)\n");
        } break;

        case OBJ_INTEGER: {
            fprintf(stdout, "%i\n", object->as.integer);
        } break;

        case OBJ_STRING: {
            fprintf(stdout, "%.*s\n", object->as.string.len, object->as.string.str);
        } break;

        case OBJ_IDENT: {
            repl__print_object(object->as.ident.value);
        } break;

        default:;
    }
}

static char *repl__read_line(char **line, size_t *offset)
{
    char ch;
    size_t i;

    if (!(*line))
    {
        *line = malloc(sizeof(**line) * (REPL_INIT_LINE_BUFFER_SIZE + 1));
        assert(line && "failed to allocate memory for repl line read");
    }

    i = *offset;

    while (1)
    {
        ch = getchar();
        if (ch == EOF || ch == '\n')
        {
            (*line)[i] = '\0';
            *offset    = i;
            return *line;
        }
        else
        {
            (*line)[i] = ch;
        }
        i += 1;

        if (i >= G__line_length)
        {
            G__line_length *= 2;
            *line           = realloc(*line, sizeof(**line) * G__line_length);
            assert(*line && "failed to resize repl buffer");
        }
    }

    *offset = i;
    return *line;
}

static repl__eval(char *line)
{
    TyVM vm;
    Lexer lexer;
    Parser parser;
    Program program;
    TyObject *eval_result;

    lexer_init_from_buffer(&lexer, line);
    parser_init(&parser, &lexer);
    tyvm_init(&vm);
    program = parser_parse_program(&parser);

    if (program.errors.count != 0)
    {
        size_t i;
        for (i = 0; i < program.errors.count; ++i)
        {
            fprintf(stderr, "%s\n", program.errors.elems[i].what);
        }
    }
    else
    {
        eval_result = eval(&vm, &program);
        if (eval_result) repl__print_object(eval_result);
    }

    program_deinit(&program);
    tyvm_deinit(&vm);
}

void repl_run(void)
{
    char *line;
    size_t line_offset;

    line_offset = 0;

    while (1)
    {
        fprintf(stdout, "%s ", REPL_PROMPT);
        line = repl__read_line(&line, &line_offset);
        repl__eval(line);
    }

    if (line) { free(line); }
}
