#include <stdio.h>
#include <stdlib.h>
#include "object.h"

const char *tyobj_kind_to_string(TyObject_Kind k)
{
  const char *res;
  switch (k)
  {
#define X(NAME) case TYOBJ_##NAME : { res = #NAME ; } break;
    #include "defs/object-kind.def"
#undef X

  default: { res = NULL; } break;
  }
  return res;
}

void tyobj_inspect(const TyObj *obj)
{
  switch (obj->kind)
  {
  // NOTE(HS): this does nothing and is intentional - for `Var_Statements` which
  // don't themselves yield an object as the result of binding a variable
  case TYOBJ_NONE:
  {} break;

  case TYOBJ_INT:
  {
    printf("%I64i\n", obj->o.integer.value);
  } break;

  case TYOBJ_STRING:
  {
    printf("\"%.*s\"\n", (int) obj->o.string.len, obj->o.string.value);
  } break;

  default:
  {
    printf("[TypeError] unrepresentable type found");
  } break;
  }
}

void tyobj_delete(TyObj *obj)
{
  switch (obj->kind)
  {
  case TYOBJ_INT:
  case TYOBJ_NONE:
  {
    obj->o = (uTyObj) {0};
  } break;

  case TYOBJ_STRING:
  {
    if (obj->o.string.value) { free((void*) obj->o.string.value); }
    obj->o.string.value = NULL;
    obj->o.string.len = 0;
  } break;
  }

  obj->kind = TYOBJ_NONE;
}
