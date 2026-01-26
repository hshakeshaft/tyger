#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
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
    printf("%" PRIi64 "\n", obj->o.integer.value);
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

TyObj *tyobj_new(TyObject_Kind kind, void *datum)
{
  TyObj *res = NULL;
  res = malloc(sizeof(TyObj));
  assert(res && "failed to allocate space for new object");

  res->kind = kind;
  switch (kind)
  {
  case TYOBJ_INT:
  {
    res->o.integer.value = * (int64_t*) datum;
  } break;

  case TYOBJ_STRING:
  {
    const char *datum_s = (const char*) datum;
    size_t string_len = strlen(datum_s);
    res->o.string.value = malloc(sizeof(char) * (string_len + 1));
    assert(res->o.string.value && "failed to allocate space for string object");
    strncpy((char*) res->o.string.value, datum_s, string_len);
    ((char*) res->o.string.value)[string_len] = '\0';
    res->o.string.len = string_len;
  } break;

  case TYOBJ_NONE: {} break;
  }

  return res;
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
