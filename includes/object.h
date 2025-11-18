#ifndef TYGER_OBJECT_H_
#define TYGER_OBJECT_H_
#include <stdint.h>

typedef enum tyobject_kind
{
#define X(NAME) TYOBJ_##NAME,
  #include "defs/object-kind.def"
#undef X
} TyObject_Kind;

typedef struct tyinteger
{
  int64_t value;
} TyInteger;

typedef union utyobj
{
  TyInteger integer;
} uTyObj;

typedef struct tyobj
{
  TyObject_Kind kind;
  uTyObj o;
} TyObj;

const char *tyobj_kind_to_string(TyObject_Kind k);
void tyobj_inspect(const TyObj *obj);


#endif  // TYGER_OBJECT_H_
