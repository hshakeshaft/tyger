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

///
/// "New" implementations
///
static void tyenv__free_entry__(TyEnv_Var__ *var)
{
  if (var->next) { tyenv__free_entry__(var->next); }
  free((void*) var->ident);
  tyobj_delete(var->object);
}

static int tyenv__compare_keys_eq(
  const char *k1, size_t k1_len,
  const char *k2, size_t k2_len
)
{
  int res = 0;
  if (k1_len != k2_len) { return res; }
  if (strncmp(k1, k2, k2_len) == 0) { res = 1; }
  return res;
}

static TyEnv_Var__ *tyvar_new(const char *ident, size_t ident_len, TyObj *obj)
{
  assert(ident && "Cannot copy [NULL] identifier");
  assert(ident_len > 0 && "Cannot copy [0] lengthed identifier");
  assert(obj && "Cannot coppy pointer to object which has value [NULL]");

  TyEnv_Var__ *var = malloc(sizeof(struct tyenv_var__));
  assert(var && "failed to allocate space for TyEnv_Var entry");

  var->ident = malloc(sizeof(char) * (ident_len + 1));
  assert(var->ident && "failed to allocate space for TyEnv_Var ident");

  strncpy((char*) var->ident, ident, ident_len);
  ((char*) var->ident)[ident_len] = '\0';

  var->ident_len = ident_len;
  var->object = obj;
  var->next = NULL;
  return var;
}

static int tyenv__handle_collision__(TyEnv_Var__ *cur, const char *ident, size_t ident_len, TyObj *obj)
{
  (void) obj;
  int res = 0;

  if (cur->ident_len == ident_len && strncmp(cur->ident, ident, ident_len) == 0)
  {
    TODO("handle inplace updates");
  }
  else
  {
    if (cur->next)
    {
      res = tyenv__handle_collision__(cur->next, ident, ident_len, obj);
    }
    else
    {
      TyEnv_Var__ *next = tyvar_new(ident, ident_len, obj);
      assert(next && "[next] insertion value in TyEnv was NULL");
      cur->next = next;
      res = 1;
    }
  }

  return res;
}

static TyObj *tyenv__handle_get__(TyEnv_Var__ *cur, const char *ident, size_t ident_len)
{
  TyObj *res = NULL;

  if (tyenv__compare_keys_eq(cur->ident, cur->ident_len, ident, ident_len))
  {
    assert(cur->object && "[cur] points to NULL object");
    return cur->object;
  }
  else
  {
    if (cur->next)
    {
      res = tyenv__handle_get__(cur->next, ident, ident_len);
    }
    // else => return NULL
  }

  return res;
}

static int tyenv__handle_update__(TyEnv_Var__ *cur, const char *ident, size_t ident_len, TyObj *obj)
{
  int res = 0;

  if (tyenv__compare_keys_eq(cur->ident, cur->ident_len, ident, ident_len))
  {
    // TODO(HS): handle with garbage collection
    tyobj_delete(cur->object);
    cur->object = obj;
    res = 1;
  }
  else
  {
    if (cur->next)
    {
      res = tyenv__handle_update__(cur->next, ident, ident_len, obj);
    }
    // else => return 0  <-- name lookup error
  }

  return res;
}


int tyenv_init__(TyEnv__ *env, size_t capacity)
{
  int res = 0;
  env->vars = calloc(capacity, sizeof(struct tyenv_var__));
  if (!env->vars) { return res; }
  env->capacity = capacity;
  res = 1;
  return res;
}

int tyenv_free__(TyEnv__ *env)
{
  for (size_t i = 0; i < env->capacity; ++i)
  {
    TyEnv_Var__ *var = &(env->vars[i]);
    if (var->next)
    {
      tyenv__free_entry__(var->next);
    }
    if (var->ident) { free((void*) var->ident); }
    var->ident_len = 0;
    if (var->object) { tyobj_delete(var->object); };
  }
  free(env->vars);
  env->capacity = 0;
  return 1;
}

int tyenv_insert__(TyEnv__ *env, const char *ident, TyObj *obj)
{
  int res = 0;
  size_t ident_len = strlen(ident);
  uint32_t hash = fnv1a_u32(ident, ident_len);
  hash %= env->capacity;

  TyEnv_Var__ *var = &(env->vars[hash]);
  if (var->ident)
  {
    if (tyenv__compare_keys_eq(var->ident, var->ident_len, ident, ident_len))
    {
      // TODO(HS): move this into garbage collection
      tyobj_delete(var->object);
      var->object = obj;
      res = 1;
    }
    else
    {
      if (var->next)
      {
        res = tyenv__handle_collision__(var->next, ident, ident_len, obj);
      }
      else
      {
        TyEnv_Var__ *next = tyvar_new(ident, ident_len, obj);
        assert(next && "[next] insertion value in TyEnv was NULL");
        var->next = next;
        res = 1;
      }
    }
  }
  else
  {
    char *ident_buffer = malloc(sizeof(char) * (ident_len + 1));
    if (!ident_buffer) { return res; }
    strncpy(ident_buffer, ident, ident_len);
    ident_buffer[ident_len] = '\0';
    var->ident = ident_buffer;
    var->ident_len = ident_len;
    var->object = obj;
    var->next = NULL;
    res = 1;
  }

  return res;
}

int tyenv_update__(TyEnv__ *env, const char *ident, TyObj *obj)
{
  int res = 0;

  size_t ident_len = strlen(ident);
  uint32_t hash = fnv1a_u32(ident, ident_len);
  hash %= env->capacity;

  // NOTE(HS): if returned bucket does not contain an ident, this is likely a failure
  // in lookup as key is non-existent
  TyEnv_Var__ *var = &(env->vars[hash]);
  if (var->ident)
  {
    if (tyenv__compare_keys_eq(var->ident, var->ident_len, ident, ident_len))
    {
      // TODO(HS): handle this in garbage collection
      tyobj_delete(var->object);
      var->object = obj;
      res = 1;
    }
    else
    {
      if (var->next)
      {
        res = tyenv__handle_update__(var->next, ident, ident_len, obj);
      }
      // else => return  <-- name lookup error
    }
  }

  return res;
}

TyObj *tyenv_get__(TyEnv__ *env, const char *ident)
{
  TyObj *res = NULL;

  size_t ident_len = strlen(ident);
  uint32_t hash = fnv1a_u32(ident, ident_len);
  hash %= env->capacity;

  TyEnv_Var__ *var = &(env->vars[hash]);
  if (var->ident)
  {
    if (tyenv__compare_keys_eq(var->ident, var->ident_len, ident, ident_len))
    {
      assert(var->object && "object pointed to by key was [NULL]");
      return var->object;
    }
    else
    {
      if (var->next)
      {
        res = tyenv__handle_get__(var->next, ident, ident_len);
      }
      // else => return NULL
    }
  }

  return res;
}



///
/// "Original Implementations"
///

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

static void tyenv__free_var(TyEnv_Var *var)
{
  if (var->next) { tyenv__free_var(var->next); }
  free(var->ident);
  tyobj_delete(var->object);
  free(var);
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

int tyenv_free(TyEnv *env)
{
  (void) env;
  int result = 0;

  for (size_t i = 0; i < env->capacity; ++i)
  {
    TyEnv_Var *var = &(env->variables[i]);
    (void) var;
    if (var->next) { tyenv__free_var(var->next); }
    if (var->ident) { free(var->ident); }
    if (var->object) { tyobj_delete(var->object); }
  }
  free(env->variables);

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
