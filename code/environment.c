#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "environment.h"

#define TODO(MSG)                                                       \
  do {                                                                  \
    printf("TODO: %s:%s:%li - %s\n", __FILE__, __func__, __LINE__, MSG);   \
    assert(0 && "TODO");                                                \
  } while (0)

static uint32_t fnv1a_u32(const char *ident, size_t ident_len)
{
  static const uint32_t OFFSET_BASIS = 0x811c9dc5;
  static const uint32_t PRIME = 0x01000193;
  uint32_t hash = OFFSET_BASIS;
  for (size_t i = 0; i < ident_len; ++i)
  {
    hash ^= ident[i];
    hash *= PRIME;
  }
  return hash;
}


static void tyenv__var_inplace_update(TyEnv_Var *var, const char *ident, size_t ident_len, TyObj *obj)
{
  char *ident_buffer = NULL;
  ident_buffer = malloc(sizeof(char) * (ident_len + 1));
  strncpy(ident_buffer, ident, ident_len);
  ident_buffer[ident_len] = '\0';

  TyObj *new = malloc(sizeof(TyObj));
  *new = *obj;

  var->ident = ident_buffer;
  var->object = new;
  var->next = NULL;
}


int tyenv_init(TyEnv *env, size_t capacity)
{
  int result = 0;
  TyEnv_Var *vars = calloc(capacity, sizeof(struct tyenv_var));
  if (!vars) { return result; }

  env->variables = vars;
  env->capacity = capacity;

  result = 1;
  return result;
}

void tyenv_insert(TyEnv *env, const char *ident, TyObj *obj)
{
  size_t ident_len = strlen(ident);
  uint32_t hash = fnv1a_u32(ident, ident_len);
  uint32_t index = hash % env->capacity;

  TyEnv_Var *existing_var = &(env->variables[index]);
  if (existing_var->ident)
  {
    TODO("Handle updates to existing entries/collisions");
  }
  else
  {
    tyenv__var_inplace_update(existing_var, ident, ident_len, obj);
  }
}

TyObj *tyenv_get(const TyEnv *env, const char *ident)
{
  size_t ident_len = strlen(ident);
  uint32_t hash = fnv1a_u32(ident, ident_len);
  uint32_t index = hash % env->capacity;

  TyEnv_Var *var = &(env->variables[index]);
  if (var->ident)
  {
    size_t var_ident_len = strlen(var->ident);
    if ((var_ident_len == ident_len) && strncmp(var->ident, ident, ident_len) == 0)
    {
      return var->object;
    }
    else
    {
      TODO("Handle collisions in lookup");
    }
  }
  else
  {
    return NULL;
  }
  return NULL;
}
