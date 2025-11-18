#include <assert.h>
#include <stddef.h>
#include "eval.h"

static TyObj eval_int(const Int_Expression *expr)
{
  TyObj res;
  res.kind = TYOBJ_INT;
  res.o.integer.value = expr->value;
  return res;
}

static TyObj eval_expression_statement(Expression_Handle handle, const Parser_Context *ctx)
{
  TyObj res;
  const Expression *expr = &(ctx->expressions.elems[handle]);
  switch (expr->kind)
  {
  case EXPR_INT:
  {
    const Int_Expression *ie = &(expr->expression.int_expression);
    res = eval_int(ie);
  } break;

  default:
  {
    res = (TyObj) {0};
    assert(0 && "Invalid expression kind");
  } break;
  }
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
