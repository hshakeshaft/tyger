#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "eval.h"

// TODO(HS): implement error handling/reporting here at evaluation

///
/// Internal forward declarations
///
static TyObj *eval_int(const Int_Expression *expr);
static TyObj *eval_infix(TyEnv *env, const Infix_Expression *expr, const Parser_Context *ctx);
static TyObj *eval_expression(TyEnv *env, const Expression *expr, const Parser_Context *ctx);


///
/// Implementations
///
static int eval__do_add(TyObj *out, const TyObj *lhs, const TyObj *rhs)
{
  int success = 0;

  switch (lhs->kind)
  {
  case TYOBJ_INT:
  {
    int64_t lval = lhs->o.integer.value;
    int64_t rval = rhs->o.integer.value;
    out->kind = TYOBJ_INT;
    out->o.integer.value = lval + rval;
  } break;

  default:
  {
    return success;
  } break;
  }

  success = 1;
  return success;
}

static int eval__do_sub(TyObj *out, const TyObj *lhs, const TyObj *rhs)
{
  int success = 0;

  switch (lhs->kind)
  {
  case TYOBJ_INT:
  {
    int64_t lval = lhs->o.integer.value;
    int64_t rval = rhs->o.integer.value;
    out->kind = TYOBJ_INT;
    out->o.integer.value = lval - rval;
  } break;

  default:
  {
    return success;
  } break;
  }

  success = 1;
  return success;
}

static int eval__do_mul(TyObj *out, const TyObj *lhs, const TyObj *rhs)
{
  int success = 0;

  switch (lhs->kind)
  {
  case TYOBJ_INT:
  {
    int64_t lval = lhs->o.integer.value;
    int64_t rval = rhs->o.integer.value;
    out->kind = TYOBJ_INT;
    out->o.integer.value = lval * rval;
  } break;

  default:
  {
    return success;
  } break;
  }

  success = 1;
  return success;
}

static int eval__do_div(TyObj *out, const TyObj *lhs, const TyObj *rhs)
{
  int success = 0;

  switch (lhs->kind)
  {
  case TYOBJ_INT:
  {
    int64_t lval = lhs->o.integer.value;
    int64_t rval = rhs->o.integer.value;
    out->kind = TYOBJ_INT;
    out->o.integer.value = lval / rval;
  } break;

  default:
  {
    return success;
  } break;
  }

  success = 1;
  return success;
}

static int eval__do_op(TyObj **out, Operator op, const TyObj *lhs, const TyObj *rhs)
{
  // NOTE(HS): `success` is set to 1 by succss of result of operator functions
  int success = 0;

  // TODO(HS): better "zero" initialise function
  (*out) = tyobj_new(TYOBJ_NONE, NULL);
  assert((*out) && "failed to create new object");

  if (lhs->kind == rhs->kind)
  {
    switch (op)
    {
    case OP_PLUS:     { success = eval__do_add((*out), lhs, rhs); } break;
    case OP_MINUS:    { success = eval__do_sub((*out), lhs, rhs); } break;
    case OP_ASTERISK: { success = eval__do_mul((*out), lhs, rhs); } break;
    case OP_SLASH:    { success = eval__do_div((*out), lhs, rhs); } break;
    default: { assert(0 && "unreachable"); }
    }
  }
  else
  {
    return success;
  }
  return success;
}

static TyObj *eval_int(const Int_Expression *expr)
{
  TyObj *res = tyobj_new(TYOBJ_INT, (void*) &expr->value);
  return res;
}

// TODO(HS): string allocation should be pooled in VM memory, not randomly here
static TyObj *eval_string(const String_Expression *expr, const Parser_Context *ctx)
{
  char *buffer = malloc(sizeof(char) * (expr->len + 1));
  assert(buffer && "failed to malloc string memory");

  const char *string = &(ctx->strings.elems[expr->string_handle]);
  strncpy(buffer, string, expr->len);
  buffer[expr->len] = '\0';

  TyObj *res = malloc(sizeof(TyObj));
  assert(res && "failed to allocate space for [TyObj::String]");
  res->kind = TYOBJ_STRING;
  res->o.string = (TyString) {
    .value = buffer,
    .len = expr->len,
  };

  return res;
}

static TyObj *eval_ident(TyEnv *env, const Ident_Expression *expr, const Parser_Context *ctx)
{
  const char *ident = &(ctx->evaluated_identifiers.elems[expr->ident_handle]);
  assert(ident && "ident lookup from parser failed (returned NULL)");
  TyObj *res = tyenv_get(env, ident);
  assert(res && "Invalid ident used in env lookup");
  return res;
}

static int compare_function_names_eq(const char *exp, const char *act)
{
  size_t exp_len = strlen(exp);
  size_t act_len = strlen(act);
  if (exp_len != act_len) { return 0; }
  return strncmp(exp, act, exp_len) == 0;
}

// TODO(HS): handle more than "println" builtin
static void eval_function_builtin(TyEnv *env, const Argument_List *args, const Parser_Context *ctx)
{
  // NOTE(HS): all println handling
  {
    TyObj *evaluated_args = malloc(sizeof(TyObj) * args->len);
    for (size_t i = 0; i < args->len; ++i)
    {
      const Expression *expr = &(args->elems[i]);
      TyObj *o = eval_expression(env, expr, ctx);
      memcpy(&(evaluated_args[i]), o, sizeof(TyObj));
    }

    for (size_t i = 0; i < args->len; ++i)
    {
      TyObj *obj = &(evaluated_args[i]);
      switch (obj->kind)
      {
      case TYOBJ_INT: { printf("%" PRIi64 "", obj->o.integer.value); } break;
      case TYOBJ_STRING: { printf("%s", obj->o.string.value); } break;
      case TYOBJ_NONE: { printf("<NONE>"); } break;
      }
      tyobj_delete(obj);
      if (i + 1 < args->len) { printf(", "); }
    }
    free(evaluated_args);
    printf("\n");
  }
}

static TyObj *eval_call_expression(TyEnv *env, const Call_Expression *expr, const Parser_Context *ctx)
{
  TyObj *res = tyobj_new(TYOBJ_NONE, NULL);

  const Expression *function = &(ctx->expressions.elems[expr->function]);
  assert(function->kind == EXPR_IDENT);
  const Ident_Expression *ie = &(function->expression.ident_expression);
  const char *ident = &(ctx->evaluated_identifiers.elems[ie->ident_handle]);

  // TODO(HS): hanlde more than just the "println" function
  assert(compare_function_names_eq("println", ident));
  eval_function_builtin(env, &expr->args, ctx);

  return res;
}

static TyObj *eval_infix(TyEnv *env, const Infix_Expression *expr, const Parser_Context *ctx)
{
  TyObj *res = NULL;

  const Expression *lhs = &(ctx->expressions.elems[expr->lhs]);
  const Expression *rhs = &(ctx->expressions.elems[expr->rhs]);

  TyObj *lhs_res = eval_expression(env, lhs, ctx);
  TyObj *rhs_res = eval_expression(env, rhs, ctx);
  assert(eval__do_op(&res, expr->op, lhs_res, rhs_res));

  return res;
}

static TyObj *eval_expression(TyEnv *env, const Expression *expr, const Parser_Context *ctx)
{
  TyObj *res = NULL;

  switch (expr->kind)
  {
  case EXPR_INT:
  {
    const Int_Expression *ie = &(expr->expression.int_expression);
    res = eval_int(ie);
  } break;

  case EXPR_STRING:
  {
    const String_Expression *se = &(expr->expression.string_expression);
    res = eval_string(se, ctx);
  } break;

  case EXPR_IDENT:
  {
    const Ident_Expression *ie = &(expr->expression.ident_expression);
    res = eval_ident(env, ie, ctx);
  } break;

  case EXPR_INFIX:
  {
    const Infix_Expression *ie = &(expr->expression.infix_expression);
    res = eval_infix(env, ie, ctx);
  } break;

  case EXPR_CALL:
  {
    const Call_Expression *ce = &(expr->expression.call_expression);
    res = eval_call_expression(env, ce, ctx);
  } break;

  default:
  {
    assert(0 && "Invalid expression kind");
  } break;
  }

  return res;
}

static TyObj *eval_expression_statement(TyEnv *env, Expression_Handle handle, const Parser_Context *ctx)
{
  TyObj *res;
  const Expression *expr = &(ctx->expressions.elems[handle]);
  res = eval_expression(env, expr, ctx);
  return res;
}

static TyObj *eval_var_statement(TyEnv *env, const Var_Statement *stmt, const Parser_Context *ctx)
{
  TyObj *res;

  // TODO(HS): register object in VM
  const char *ident = &(ctx->identifiers.elems[stmt->ident_handle]);
  const Expression *expr = &(ctx->expressions.elems[stmt->expression_handle]);
  TyObj *val = eval_expression(env, expr, ctx);
  tyenv_insert(env, ident, val);

  res = tyobj_new(TYOBJ_NONE, NULL);

  return res;
}

static TyObj *eval_statement(TyEnv *env, const Statement *stmt, const Parser_Context *ctx)
{
  TyObj *res = NULL;
  switch (stmt->kind)
  {
  case STMT_VAR:
  {
    const Var_Statement *vs = &(stmt->statement.var_statement);
    res = eval_var_statement(env, vs, ctx);
  } break;

  case STMT_EXPRESSION:
  {
    Expression_Handle handle = stmt->statement.expression_statement.expression_handle;
    res = eval_expression_statement(env, handle, ctx);
  } break;

  default:
  {
    assert(0 && "Invalid statement type");
  } break;
  }
  return res;
}

static TyObj *eval_program_statements(TyEnv *env, const Program *prog)
{
  TyObj *res = NULL;
  
  for (size_t i = 0; i < prog->statements.len; ++i)
  {
    const Statement *stmt = &(prog->statements.elems[i]);
    const Parser_Context *ctx = &(prog->context);
    res = eval_statement(env, stmt, ctx);
  }

  return res;
}

// TODO(HS): do I want to break current interface and return pointer?
TyObj eval(TyEnv *global_env, const Program *prog)
{
  TyObj *obj;
  obj = eval_program_statements(global_env, prog);
  return *obj;
}
