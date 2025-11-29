#ifndef TYGER_ENVIRONMENT_H_
#define TYGER_ENVIRONMENT_H_
#include <stddef.h>
#include "object.h"

// TODO(HS): implement freeing function(s)

typedef struct tyenv_var TyEnv_Var;

struct tyenv_var
{
  char *ident;
  TyObj *object;
  TyEnv_Var *next;
};

typedef struct tyenv
{
  TyEnv_Var *variables;
  size_t capacity;
} TyEnv;

int tyenv_init(TyEnv *env, size_t capacity);
int tyenv_free(TyEnv *env);

void tyenv_insert(TyEnv *env, const char *ident, TyObj *obj);
TyObj *tyenv_get(const TyEnv *env, const char *ident);


// NOTE(HS): "refactor" related here:
typedef struct tyenv_var__ TyEnv_Var__;

struct tyenv_var__
{
  const char *ident;
  size_t ident_len;
  TyObj *object;
  TyEnv_Var__ *next;
};

typedef struct tyenv__
{
  TyEnv_Var__ *vars;
  size_t capacity;
} TyEnv__;

int tyenv_init__(TyEnv__ *env, size_t capacity);
int tyenv_free__(TyEnv__ *env);
int tyenv_insert__(TyEnv__ *env, const char *ident, TyObj *obj);
TyObj *tyenv_get__(TyEnv__ *env, const char *ident);

#endif  // TYGER_ENVIRONMENT_H_
