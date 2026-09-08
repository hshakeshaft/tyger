#ifndef EVAL_H_
#define EVAL_H_

#include "ast.h"
#include "object.h"
#include "vm.h"

TyObject *eval(TyVM *vm, Program *program);

#endif  /* EVAL_H_ */
