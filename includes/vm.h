#ifndef VM_H_
#define VM_H_
#include <stddef.h>

#include "ht.h"
#include "object.h"

#ifndef VM_STRESS_OBJECT_ALLOCATION
#define VM_MAX_OBJECTS 4096
#else
#define VM_MAX_OBJECTS 16
#endif

/* TODO(HS): consider moving to handles for objects in `HT` slots & identifiers */
/* TODO(HS): functions require a local "environment" which contains new name bindings
(read hash table)
*/
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

/*==============================================================================
                            VM Intrinsic functions
==============================================================================*/

TyObject *vm_object_binop(
    TyVM *vm, TyObject *lhs, TyObject *rhs,
    TyObject *(*do_binop_fn)(TyVM *vm, TyObject *lhs, TyObject *rhs)
);

TyObject *vm_intrinsic__object_add(TyVM *vm, TyObject *lhs, TyObject *rhs);
TyObject *vm_intrinsic__object_sub(TyVM *vm, TyObject *lhs, TyObject *rhs);
TyObject *vm_intrinsic__object_mul(TyVM *vm, TyObject *lhs, TyObject *rhs);
TyObject *vm_intrinsic__object_div(TyVM *vm, TyObject *lhs, TyObject *rhs);

#endif  /* VM_H_ */
