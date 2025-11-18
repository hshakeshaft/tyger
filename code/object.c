#include <stdio.h>
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
  const char *tstr = tyobj_kind_to_string(obj->kind);
  printf("  TyObj( type=%s, value=", tstr);
  switch (obj->kind)
  {
  case TYOBJ_INT:
  {
    printf("%I64i", obj->o.integer.value);
  } break;

  default:
  {
    printf("<Unrepresentable Value>");
  } break;
  }
  printf(")\n");
}
