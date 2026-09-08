#ifndef VM_H_
#define VM_H_
#include <stddef.h>

#include "object.h"

#define VM_MAX_OBJECTS 4096

typedef struct
{
    TyObject *objects;
    size_t object_count;
} TyVM;

void tyvm_init(TyVM *vm);
void tyvm_deinit(TyVM *vm);

TyObject *vm_create_object(TyVM *vm, TyObject_Type type, void *data);

#endif  /* VM_H_ */
