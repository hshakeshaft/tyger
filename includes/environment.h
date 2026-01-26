#ifndef TYGER_ENVIRONMENT_H_
#define TYGER_ENVIRONMENT_H_
#include <stddef.h>
#include "object.h"

typedef struct tyenv_var TyEnv_Var;

struct tyenv_var
{
  const char *ident;
  size_t ident_len;
  TyObj *object;
  TyEnv_Var *next;
};

typedef struct tyenv
{
  TyEnv_Var *vars;
  size_t capacity;
} TyEnv;

int tyenv_init(TyEnv *env, size_t capacity);
int tyenv_free(TyEnv *env);
int tyenv_insert(TyEnv *env, const char *ident, TyObj *obj);
int tyenv_update(TyEnv *env, const char *ident, TyObj *obj);
TyObj *tyenv_get(TyEnv *env, const char *ident);
int tyenv_delete(TyEnv *env, const char *ident);
#endif  // TYGER_ENVIRONMENT_H_
