#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "eval.h"

// TODO(HS): implement error handling/reporting here at evaluation

///
/// Internal forward declarations
///
static TyObj eval_int(const Int_Expression *expr);
static TyObj eval_infix(const Infix_Expression *expr, const Parser_Context *ctx);
static TyObj eval_expression(const Expression *expr, const Parser_Context *ctx);


///
/// Implementations
///
static int eval__do_add(TyObj *out, TyObj lhs, TyObj rhs)
{
  int success = 0;

  switch (lhs.kind)
  {
  case TYOBJ_INT:
  {
    int64_t lval = lhs.o.integer.value;
    int64_t rval = rhs.o.integer.value;
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

static int eval__do_sub(TyObj *out, TyObj lhs, TyObj rhs)
{
  int success = 0;

  switch (lhs.kind)
  {
  case TYOBJ_INT:
  {
    int64_t lval = lhs.o.integer.value;
    int64_t rval = rhs.o.integer.value;
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

static int eval__do_mul(TyObj *out, TyObj lhs, TyObj rhs)
{
  int success = 0;

  switch (lhs.kind)
  {
  case TYOBJ_INT:
  {
    int64_t lval = lhs.o.integer.value;
    int64_t rval = rhs.o.integer.value;
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

static int eval__do_div(TyObj *out, TyObj lhs, TyObj rhs)
{
  int success = 0;

  switch (lhs.kind)
  {
  case TYOBJ_INT:
  {
    int64_t lval = lhs.o.integer.value;
    int64_t rval = rhs.o.integer.value;
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

static int eval__do_op(TyObj *out, Operator op, TyObj lhs, TyObj rhs)
{
  // NOTE(HS): `success` is set to 1 by succss of result of operator functions
  int success = 0;
  if (lhs.kind == rhs.kind)
  {
    switch (op)
    {
    case OP_PLUS:     { success = eval__do_add(out, lhs, rhs); } break;
    case OP_MINUS:    { success = eval__do_sub(out, lhs, rhs); } break;
    case OP_ASTERISK: { success = eval__do_mul(out, lhs, rhs); } break;
    case OP_SLASH:    { success = eval__do_div(out, lhs, rhs); } break;
    }
  }
  else
  {
    return success;
  }
  return success;
}

static TyObj eval_int(const Int_Expression *expr)
{
  TyObj res;
  res.kind = TYOBJ_INT;
  res.o.integer.value = expr->value;
  return res;
}

// NOTE(HS): this leaks
// TODO(HS): string allocation should be pooled in VM memory, not randomly here
static TyObj eval_string(const String_Expression *expr, const Parser_Context *ctx)
{
  TyObj res;

  char *buffer = malloc(sizeof(char) * (expr->len + 1));
  assert(buffer && "failed to malloc string memory");

  const char *string = &(ctx->strings.elems[expr->string_handle]);
  strncpy(buffer, string, expr->len);
  buffer[expr->len] = '\0';

  res.kind = TYOBJ_STRING;
  res.o.string = (TyString) {
    .value = buffer,
    .len = expr->len,
  };

  return res;
}

static TyObj eval_infix(const Infix_Expression *expr, const Parser_Context *ctx)
{
  TyObj res;

  const Expression *lhs = &(ctx->expressions.elems[expr->lhs]);
  const Expression *rhs = &(ctx->expressions.elems[expr->rhs]);

  TyObj lhs_res = eval_expression(lhs, ctx);
  TyObj rhs_res = eval_expression(rhs, ctx);
  assert(eval__do_op(&res, expr->op, lhs_res, rhs_res));

  return res;
}

static TyObj eval_expression(const Expression *expr, const Parser_Context *ctx)
{
  TyObj res;

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

  case EXPR_INFIX:
  {
    const Infix_Expression *ie = &(expr->expression.infix_expression);
    res = eval_infix(ie, ctx);
  } break;

  default:
  {
    res = (TyObj) {0};
    assert(0 && "Invalid expression kind");
  } break;
  }

  return res;
}

static TyObj eval_expression_statement(Expression_Handle handle, const Parser_Context *ctx)
{
  TyObj res;
  const Expression *expr = &(ctx->expressions.elems[handle]);
  res = eval_expression(expr, ctx);
  return res;
}

static TyObj eval_statement(const Statement *stmt, const Parser_Context *ctx)
{
  TyObj res;
  switch (stmt->kind)
  {
  case STMT_EXPRESSION:
  {
    Expression_Handle handle = stmt->statement.expression_statement.expression_handle;
    res = eval_expression_statement(handle, ctx);
  } break;

  default:
  {
    res = (TyObj) {0};
    assert(0 && "Invalid statement type");
  } break;
  }
  return res;
}

static TyObj eval_program_statements(const Program *prog)
{
  TyObj res;
  
  for (size_t i = 0; i < prog->statements.len; ++i)
  {
    const Statement *stmt = &(prog->statements.elems[i]);
    const Parser_Context *ctx = &(prog->context);
    res = eval_statement(stmt, ctx);
  }

  return res;
}

TyObj eval(const Program *prog)
{
  TyObj obj;
  obj = eval_program_statements(prog);
  return obj;
}
