#ifndef TYGER_EVAL_H_
#define TYGER_EVAL_H_
#include "object.h"
#include "parser.h"
#include "environment.h"

TyObj eval(TyEnv__ *global_env, const Program *prog);

#endif  // TYGER_EVAL_H_
