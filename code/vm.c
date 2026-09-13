#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "vm.h"

#define VM_IDENTIDIER_BUCKETS 64


void tyvm_init(TyVM *vm)
{
    vm->objects = malloc(sizeof(*vm->objects) * VM_MAX_OBJECTS);
    vm->object_count = 0;

    ht_init(&vm->identifiers, VM_IDENTIDIER_BUCKETS);
}

void tyvm_deinit(TyVM *vm)
{
    if (vm->objects)
    {
        size_t i;
        TyObject *cur_object;
        for (i = 0; i < vm->object_count; ++i)
        {
            cur_object = vm->objects[i];
            if (cur_object != NULL)
            {
                tyobject_destroy(cur_object);
            }
        }
        free(vm->objects);
    }

    vm->object_count = 0;

    ht_deinit(&vm->identifiers);
}

TyObject *vm_create_object(TyVM *vm, TyObject_Type type, void *data)
{
    TyObject *object;

    if (vm->object_count + 1 >= VM_MAX_OBJECTS)
    {
        fprintf(
            stderr,
            "[ERROR] VM attempted to allocate an object beyond the maximum capacity: %i\n",
            VM_MAX_OBJECTS
        );
        exit(1);
    }

    object = tyobject_create(type, data);
    if (!object)
    {
        fprintf(stderr, "[ERROR] failed to create object\n");
    }

    vm->objects[vm->object_count++] = object;

    return object;
}

TyObject *vm_create_ident_object(TyVM *vm, const char *ident, TyObject *value)
{
    TyObject *object;

    object = vm_create_object(vm, OBJ_IDENT, (void*) ident);
    object->as.ident.value = value;

    ht_insert(&vm->identifiers, ident, object);

    return object;
}

TyObject *vm_get_ident(TyVM *vm, const char *ident)
{
    TyObject *object;
    HT_Slot *slot;
    object = NULL;
    slot = ht_get(&vm->identifiers, ident);
    if (slot->key != NULL)
    {
        object = slot->value;
    }
    return object;
}


/*==============================================================================
                            VM Intrinsic functions
==============================================================================*/

/* resolve an object to the final instance - used for identifiers */
static TyObject *vm__resolve_object(TyObject *object)
{
    TyObject *result;
    result = object;
    switch (result->type)
    {
        case OBJ_IDENT: {
            while (result->type == OBJ_IDENT)
            {
                result = result->as.ident.value;
            }
        } break;

        default:;
    }
    return result;
}

TyObject *vm_object_binop(
    TyVM *vm, TyObject *lhs, TyObject *rhs,
    TyObject *(*do_binop_fn)(TyVM *vm, TyObject *lhs, TyObject *rhs)
)
{
    TyObject *result;
    lhs = vm__resolve_object(lhs);
    rhs = vm__resolve_object(rhs);

    assert(lhs->type == OBJ_INTEGER && "LHS did not resolve to integer - operator only valid between integers or identifiers");
    assert(rhs->type == OBJ_INTEGER && "rHS did not resolve to integer - operator only valid between integers or identifiers");

    result = do_binop_fn(vm, lhs, rhs);
    return result;
}

TyObject *vm_intrinsic__object_add(TyVM *vm, TyObject *lhs, TyObject *rhs)
{
    TyObject *result;
    int value;
    value  = lhs->as.integer + rhs->as.integer;
    result = vm_create_object(vm, OBJ_INTEGER, (void*) &value);
    return result;
}

TyObject *vm_intrinsic__object_sub(TyVM *vm, TyObject *lhs, TyObject *rhs)
{
    TyObject *result;
    int value;
    value  = lhs->as.integer - rhs->as.integer;
    result = vm_create_object(vm, OBJ_INTEGER, (void*) &value);
    return result;
}

TyObject *vm_intrinsic__object_mul(TyVM *vm, TyObject *lhs, TyObject *rhs)
{
    TyObject *result;
    int value;
    value  = lhs->as.integer * rhs->as.integer;
    result = vm_create_object(vm, OBJ_INTEGER, (void*) &value);
    return result;
}

TyObject *vm_intrinsic__object_div(TyVM *vm, TyObject *lhs, TyObject *rhs)
{
    TyObject *result;
    int value;
    value  = lhs->as.integer / rhs->as.integer;
    result = vm_create_object(vm, OBJ_INTEGER, (void*) &value);
    return result;
}
