#include <stdio.h>

#include "eval.h"
#include "memory.h"

static TyObject *eval__integer(TyVM *vm, Integer_Expression *expression)
{
    TyObject *object;
    object = NULL;
    object = vm_create_object(vm, OBJ_INTEGER, (void*) &expression->value);
    return object;
}

static TyObject *eval__string(TyVM *vm, String_Expression *expression)
{
    TyObject *object;
    object = NULL;
    object = vm_create_object(vm, OBJ_STRING, (void*) &expression->len);
    TY_MOVE_PTR(expression->ptr, object->as.string.str);
    return object;
}


static TyObject *eval__expression_statement(TyVM *vm, Program *program, Expression *expression)
{
    TyObject *object;
    object = NULL;

    (void) program;

    switch (expression->type)
    {
        case ET_INTEGER: {
            object = eval__integer(vm, &expression->as.integer);
        } break;

        case ET_STRING: {
            object = eval__string(vm, &expression->as.string);
        } break;

        default:;
    }

    return object;
}

static TyObject *eval__statement(TyVM *vm, Program *program, Statement *stmt)
{
    TyObject *object;
    object = NULL;

    switch (stmt->type)
    {
        case ST_EXPRESSION: {
            Expression_Handle expression_handle;
            Expression *expression;
            expression_handle = stmt->as.expression.handle;
            expression        = program_expression_handle_to_expression(program, expression_handle);
            object            = eval__expression_statement(vm, program, expression);
        } break;

        default:;
    }

    return object;
}

TyObject *eval(TyVM *vm, Program *program)
{
    TyObject *object;
    Statement *stmt;
    size_t i;

    object = NULL;

    for (i = 0; i < program->___statements.count; ++i)
    {
        stmt = &program->___statements.elems[i];
        object = eval__statement(vm, program, stmt);
    }

    return object;
}
