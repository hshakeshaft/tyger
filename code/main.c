#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "eval.h"
#include "vm.h"

#define REPL_INPUT_BUFFER_MAX_SIZE 4096

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


/* TODO(HS): "delete" program */

int main(void)
{
    TyVM vm;

    tyvm_init(&vm);

    while (1)
    {
        char input_buffer[REPL_INPUT_BUFFER_MAX_SIZE];
        Lexer lexer;
        Parser parser;
        Program program;
        TyObject *object;

        memset(input_buffer, '\0', REPL_INPUT_BUFFER_MAX_SIZE);

        fprintf(stdout, "tyger> ");

        if (!fgets(input_buffer, REPL_INPUT_BUFFER_MAX_SIZE, stdin))
        {
            fprintf(stderr, "[ERROR]: error reading from stdin\n");
            break;
        }

        lexer_init_from_buffer(&lexer, input_buffer);
        parser_init(&parser, &lexer);
        program = parser_parse_program(&parser);

        if (program.errors.count != 0)
        {
            size_t i;
            for (i = 0; i < program.errors.count; ++i)
            {
                fprintf(stderr, "%s\n", program.errors.elems[i].what);
            }
            continue;
        }

        object  = eval(&vm, &program);

        if (!object)
        {
            fprintf(stderr, "[ERROR]: error evaluating program\n");
            break;
        }

        repl__print_object(object);
    }

    tyvm_deinit(&vm);

    return 0;
}
