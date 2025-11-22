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

void tyenv_insert(TyEnv *env, const char *ident, TyObj *obj);
TyObj *tyenv_get(const TyEnv *env, const char *ident);

#endif  // TYGER_ENVIRONMENT_H_
