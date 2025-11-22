#include <stdlib.h>
#include "environment.h"

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
  (void) env, (void) ident, (void) obj;
}

TyObj *tyenv_get(const TyEnv *env, const char *ident)
{
  (void) env, (void) ident;
  return NULL;
}
