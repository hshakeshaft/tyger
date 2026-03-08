#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "environment.h"

#define TODO(MSG)                                                       \
  do {                                                                  \
    printf("TODO: %s:%s:%i - %s\n", __FILE__, __func__, __LINE__, MSG); \
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


static void tyenv__free_entry(TyEnv_Var *var)
{
  if (var->next) { tyenv__free_entry(var->next); }
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

static TyEnv_Var *tyvar_new(const char *ident, size_t ident_len, TyObj *obj)
{
  assert(ident && "Cannot copy [NULL] identifier");
  assert(ident_len > 0 && "Cannot copy [0] lengthed identifier");
  assert(obj && "Cannot coppy pointer to object which has value [NULL]");

  TyEnv_Var *var = malloc(sizeof(struct tyenv_var));
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

static int tyenv__handle_collision(TyEnv_Var *cur, const char *ident, size_t ident_len, TyObj *obj)
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
      res = tyenv__handle_collision(cur->next, ident, ident_len, obj);
    }
    else
    {
      TyEnv_Var *next = tyvar_new(ident, ident_len, obj);
      assert(next && "[next] insertion value in TyEnv was NULL");
      cur->next = next;
      res = 1;
    }
  }

  return res;
}

static TyObj *tyenv__handle_get(TyEnv_Var *cur, const char *ident, size_t ident_len)
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
      res = tyenv__handle_get(cur->next, ident, ident_len);
    }
    // else => return NULL
  }

  return res;
}

static int tyenv__handle_update(TyEnv_Var *cur, const char *ident, size_t ident_len, TyObj *obj)
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
      res = tyenv__handle_update(cur->next, ident, ident_len, obj);
    }
    // else => return 0  <-- name lookup error
  }

  return res;
}

static int tyenv__handle_delete(TyEnv_Var *cur, const char *ident, size_t ident_len)
{
  int res = 0;
  if (tyenv__compare_keys_eq(cur->ident, cur->ident_len, ident, ident_len))
  {
    tyobj_delete(cur->object);

    if (cur->next)
    {
      *cur = *(cur->next);
    }

    res = 1;
  }
  else if (cur->next)
  {
    tyenv__handle_delete(cur->next, ident, ident_len);
  }
  return res;
}


int tyenv_init(TyEnv *env, size_t capacity)
{
  int res = 0;
  env->vars = calloc(capacity, sizeof(struct tyenv_var));
  if (!env->vars) { return res; }
  env->capacity = capacity;
  res = 1;
  return res;
}

int tyenv_free(TyEnv *env)
{
  for (size_t i = 0; i < env->capacity; ++i)
  {
    TyEnv_Var *var = &(env->vars[i]);
    if (var->next)
    {
      tyenv__free_entry(var->next);
    }
    if (var->ident) { free((void*) var->ident); }
    var->ident_len = 0;
    if (var->object) { tyobj_delete(var->object); };
  }
  free(env->vars);
  env->capacity = 0;
  return 1;
}

int tyenv_insert(TyEnv *env, const char *ident, TyObj *obj)
{
  int res = 0;
  size_t ident_len = strlen(ident);
  uint32_t hash = fnv1a_u32(ident, ident_len);
  hash %= env->capacity;

  TyEnv_Var *var = &(env->vars[hash]);
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
        res = tyenv__handle_collision(var->next, ident, ident_len, obj);
      }
      else
      {
        TyEnv_Var *next = tyvar_new(ident, ident_len, obj);
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

int tyenv_update(TyEnv *env, const char *ident, TyObj *obj)
{
  int res = 0;

  size_t ident_len = strlen(ident);
  uint32_t hash = fnv1a_u32(ident, ident_len);
  hash %= env->capacity;

  // NOTE(HS): if returned bucket does not contain an ident, this is likely a failure
  // in lookup as key is non-existent
  TyEnv_Var *var = &(env->vars[hash]);
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
        res = tyenv__handle_update(var->next, ident, ident_len, obj);
      }
      // else => return  <-- name lookup error
    }
  }

  return res;
}

TyObj *tyenv_get(TyEnv *env, const char *ident)
{
  TyObj *res = NULL;

  size_t ident_len = strlen(ident);
  uint32_t hash = fnv1a_u32(ident, ident_len);
  hash %= env->capacity;

  TyEnv_Var *var = &(env->vars[hash]);
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
        res = tyenv__handle_get(var->next, ident, ident_len);
      }
      // else => return NULL
    }
  }

  return res;
}

int tyenv_delete(TyEnv *env, const char *ident)
{
  int res = 0;
  size_t ident_len = strlen(ident);
  uint32_t hash = fnv1a_u32(ident, ident_len);
  hash %= env->capacity;

  TyEnv_Var *var = &(env->vars[hash]);
  if (var->ident)
  {
    res = tyenv__handle_delete(var, ident, ident_len);
  }

  return res;
}



/* V2 impl */

int env_init(Env *env, size_t capacity)
{
  int result = 0;
  assert(env && "cannot initialise Env as it points to NULL memory");
  assert(capacity > 0 && "cannit initialise an environment with no capacity for variables");

  env->capacity = capacity;
  env->vars = malloc(sizeof(*env->vars) * env->capacity);
  assert(env->vars && "failed to allocate memory for Env");

  for (size_t i = 0; i < capacity; ++i)
  {
    env->vars->ident = NULL;
    env->vars->ident_len = 0;
    env->vars->next = NULL;
    env->vars->object = NULL;
  }

  result = 1;
  return result;
}

void env_deinit(Env *env)
{
  assert(env && "cannot free NULL Env");
  if (env->vars)
  {
    free(env->vars);
    env->vars = NULL;
  }
  env->capacity = 0;
}

static uint32_t hash_key_and_clamp_to_slot_u32(const char *key, size_t key_len, size_t max_slot)
{
  uint32_t hash = fnv1a_u32(key, key_len);
  uint32_t slot = hash % max_slot;
  return slot;
}

static Env_Var *env_var_new(const char *key, size_t key_len, TyObj *obj, Env_Var *next)
{
  Env_Var *var;
  var = malloc(sizeof(*var));
  assert(var&& "failed to allocate space in Env for a new variable");

  var->ident = malloc(sizeof(*var->ident) * (key_len + 1));
  strncpy((char*) var->ident, key, key_len);
  ((char*) var->ident)[key_len] = '\0';

  if (next) { var->next = NULL; }

  var->object = obj;

  return var;
}


static int env__do_insert(Env_Var *cur, const char *key, size_t key_len, TyObj *obj)
{
  int result = 0;

  if (strncmp(cur->ident, key, key_len) == 0)  // keys collide => error
  {
    return result;
  }
  else if (cur->next)
  {
    result = env__do_insert(cur->next, key, key_len, obj);
  }
  else if (!cur->next)
  {
    Env_Var *next = env_var_new(key, key_len, obj, NULL);
    assert(next && "failed to allocate next");
    result = 1;
  }

  return result;
}

int env_insert(Env *env, const char *key, TyObj *obj)
{
  int result = 0;
  assert(env && "attempted to retrieve key from NULL env");
  assert(key && "cannot insert key which is NULL");
  assert(obj && "cannot insert Tyger Object which is NULL");

  size_t key_len = strlen(key);
  uint32_t slot = hash_key_and_clamp_to_slot_u32(key, key_len, env->capacity);

  Env_Var *var = &(env->vars[slot]);
  if (var->ident)   // check if collision at slot
  {
    return result;
  }
  else             // else handle insertion (recurse)
  {
    result = env__do_insert(&env->vars[slot], key, key_len, obj);
  }
  
  return result;
}

static TyObj *env__do_get(Env_Var *cur, const char *key, size_t key_len)
{
  TyObj *obj = NULL;

  if (strncmp(cur->ident, key, key_len) == 0)
  {
    obj = cur->object;
  }
  else if (cur->next)
  {
    obj = env__do_get(cur->next, key, key_len);
  }

  return obj;
}

TyObj *env_get(Env *env, const char *key)
{
  TyObj *obj = NULL;

  size_t key_len = strlen(key);
  uint32_t slot = hash_key_and_clamp_to_slot_u32(key, key_len, env->capacity);

  Env_Var *var = &(env->vars[slot]);
  if (!var->ident)
  {
    return obj;
  }
  else
  {
    obj = env__do_get(var, key, key_len);
  }

  return obj;
}


