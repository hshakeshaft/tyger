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

static TyObject *eval__ident(TyVM *vm, Ident_Expression *expression)
{
    TyObject *object;
    object = vm_get_ident(vm, expression->name);
    return object;
}

static TyObject *eval__expression(TyVM *vm, Program *program, Expression *expression)
{
    TyObject *object;
    (void) program;
    switch (expression->type)
    {
        case ET_INTEGER: {
            object = eval__integer(vm, &expression->as.integer);
        } break;

        case ET_STRING: {
            object = eval__string(vm, &expression->as.string);
        } break;

        case ET_IDENT: {
            object = eval__ident(vm, &expression->as.ident);
        } break;

        default:;
    }
    return object;
}

static TyObject *eval__expression_statement(TyVM *vm, Program *program, Expression *expression)
{
    TyObject *object;
    object = eval__expression(vm, program, expression);
    return object;
}

/* TODO(HS): `None` type object needs to be registered as single global thing that
can be referenced by everything else (why create multiple "none"s?)
*/
static TyObject *eval__var_statement(TyVM *vm, Program *program, Statement *statement)
{
    TyObject *object;
    TyObject *ident_object;
    TyObject *expression_object;
    Expression *expression;

    expression        = program_expression_handle_to_expression(program, statement->as.var.expression) ;
    expression_object = eval__expression(vm, program, expression);
    ident_object      = vm_create_ident_object(vm, statement->as.var.ident, expression_object);
    object            = vm_create_object(vm, OBJ_NONE, NULL);

    (void) ident_object;

    return object;
}


static TyObject *eval__statement(TyVM *vm, Program *program, Statement *stmt)
{
    TyObject *object;
    object = NULL;

    switch (stmt->type)
    {
        case ST_VAR: {
            object = eval__var_statement(vm, program, stmt);
        } break;

        /* TODO(HS): move expression extraction into eval expression function */
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
