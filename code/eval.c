#include <assert.h>
#include <stdio.h>

#include "eval.h"
#include "memory.h"

static TyObject *eval__expression(TyVM *vm, Program *program, Expression *expression);


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

static TyObject *eval__infix(TyVM *vm, Program *program, Infix_Expression *expression)
{
    TyObject *object;
    TyObject *lhs_object;
    TyObject *rhs_object;
    Expression *lhs_expression;
    Expression *rhs_expression;

    object         = NULL;
    lhs_expression = program_expression_handle_to_expression(program, expression->lhs);
    rhs_expression = program_expression_handle_to_expression(program, expression->rhs);
    lhs_object     = eval__expression(vm, program, lhs_expression);
    rhs_object     = eval__expression(vm, program, rhs_expression);

    switch (expression->op)
    {
        case OP_ADD: { object = vm_intrinsic__object_add(lhs_object, rhs_object); } break;
        case OP_SUB: { object = vm_intrinsic__object_sub(lhs_object, rhs_object); } break;
        case OP_MUL: { object = vm_intrinsic__object_mul(lhs_object, rhs_object); } break;
        case OP_DIV: { object = vm_intrinsic__object_div(lhs_object, rhs_object); } break;
        default: {
            assert(0 && "[ERROR] :: invalid operator used in expression");
        }
    }

    return object;
}


static TyObject *eval__expression(TyVM *vm, Program *program, Expression *expression)
{
    TyObject *object;
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

        case ET_INFIX: {
            object = eval__infix(vm, program, &expression->as.infix);
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
    TyObject *expression_object;
    Expression *expression;

    expression        = program_expression_handle_to_expression(program, statement->as.var.expression) ;
    expression_object = eval__expression(vm, program, expression);
    vm_create_ident_object(vm, statement->as.var.ident, expression_object);
    object = vm_create_object(vm, OBJ_NONE, NULL);

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

    for (i = 0; i < program->statements.count; ++i)
    {
        stmt = &program->statements.elems[i];
        object = eval__statement(vm, program, stmt);
    }

    return object;
}
