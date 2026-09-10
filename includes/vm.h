#ifndef VM_H_
#define VM_H_
#include <stddef.h>

#include "ht.h"
#include "object.h"

#define VM_MAX_OBJECTS 4096

typedef struct
{
    TyObject **objects;
    size_t object_count;

    HT identifiers;
} TyVM;

void tyvm_init(TyVM *vm);
void tyvm_deinit(TyVM *vm);

TyObject *vm_create_object(TyVM *vm, TyObject_Type type, void *data);

/* NOTE(HS): all idents need to be associated with some object */
TyObject *vm_create_ident_object(TyVM *vm, const char *ident, TyObject *value);

TyObject *vm_get_ident(TyVM *vm, const char *ident);

#endif  /* VM_H_ */
