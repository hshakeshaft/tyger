#include <stdlib.h>
#include <stdio.h>

#include "vm.h"

void tyvm_init(TyVM *vm)
{
    vm->objects = malloc(sizeof(*vm->objects) * VM_MAX_OBJECTS);
    vm->object_count = 0;
}

void tyvm_deinit(TyVM *vm)
{
    if (vm->objects)
    {
        free(vm->objects);
    }
    vm->object_count = 0;
}

TyObject *vm_create_object(TyVM *vm, TyObject_Type type, void *data)
{
    TyObject *object;
    object = NULL;

    if (vm->object_count + 1 >= VM_MAX_OBJECTS)
    {
        fprintf(
            stderr,
            "[ERROR] VM attempted to allocate an object beyond the maximum capacity: %i\n",
            VM_MAX_OBJECTS
        );
        exit(1);
    }

    object = &vm->objects[vm->object_count++];

    object->type = type;

    switch (object->type)
    {
        case OBJ_INTEGER: {
            object->as.integer = * (int*) data;
        } break;

        default:;
    }

    return object;
}
