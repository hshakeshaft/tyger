#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "trace.h"
#include "tstrings.h"

#define TRACE_YAML_SPACES_PER_INDENT_LEVEL 4

static void yaml_print_header(const Program *p, String_Builder *sb);
static void yaml_print_statement(
  const Program *prog, const Statement *stmt, String_Builder *sb, int *indent_level
);
static void yaml_print_expression(
  const Program *prog, const Expression *expr, String_Builder *sb, int *indent_level
);

static void sexpr_print_statement(const Program *prog, const Statement *stmt, String_Builder *sb);
static void sexpr_print_expression(const Program *prog, const Expression *expr, String_Builder *sb);

const char *program_to_string(const Program *p, Trace_Format kind)
{
  assert(p);
  String_Builder sb;
  string_builder_init(&sb);

  switch (kind)
  {
  case TRACE_YAML:
  {
    yaml_print_header(p, &sb);
    for (size_t i = 0; i < p->statements.len; ++i)
    {
      int indent_level = 1;
      Statement *stmt = &(p->statements.elems[i]);
      yaml_print_statement(p, stmt, &sb, &indent_level);
    }
  } break;

  case TRACE_SEXPR:
  {
    for (size_t i = 0; i < p->statements.len; ++i)
    {
      Statement *stmt = &(p->statements.elems[i]);
      sexpr_print_statement(p, stmt, &sb);
    }
  } break;
  }

  const char *buffer = string_builder_to_cstring(&sb);
  return buffer;
}

static void check_buffer_and_resize(char *buffer, size_t *capacity, size_t len, size_t bytes_to_write)
{
  if ((len + bytes_to_write) > *capacity)
  {
    size_t new_capacity = *capacity * 2;
    char *new_buffer = realloc(buffer, new_capacity);
    if (buffer != new_buffer)
    {
      buffer = new_buffer;
    }
    *capacity = new_capacity;
  }
}

static void yaml_print_indent(String_Builder *sb, int indent_level)
{
  int num_spaces_to_indent = indent_level * TRACE_YAML_SPACES_PER_INDENT_LEVEL;
  check_buffer_and_resize(sb->buffer, &sb->capacity, sb->len, num_spaces_to_indent);
  memset(&(sb->buffer[sb->len]), ' ', num_spaces_to_indent);
  sb->len += num_spaces_to_indent;
}

static void yaml_print_header(const Program *p, String_Builder *sb)
{
  #define TRACE_YAML_HEADER_FMT "---\nErrors: %zu\nStatements: %zu\n---\nprogram:\n"
  string_builder_append_fmt(sb, TRACE_YAML_HEADER_FMT, p->errors.len, p->statements.len);
}


// TODO(HS): improve num indent spaces calculation
static void yaml_print_statement(
  const Program *prog, const Statement *stmt, String_Builder *sb, int *indent_level
)
{
  yaml_print_indent(sb, *indent_level);
  string_builder_append_fmt(sb, "- kind: %s\n", statement_kind_to_string(stmt->kind));

  switch (stmt->kind)
  {
  case STMT_VAR:
  {
    const Var_Statement *vs = &stmt->statement.var_statement;
    const char *ident = ident_handle_to_ident(prog, vs->ident_handle);
    const Expression *expr = expression_handle_to_expression(prog, vs->expression_handle);

    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "  ident: %s\n", ident);
    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "  expression:\n");
    *indent_level += 1;
    yaml_print_expression(prog, expr, sb, indent_level);
    *indent_level -= 1;
  } break;

  case STMT_EXPRESSION:
  {
    const Expression *expr = expression_handle_to_expression(
      prog, stmt->statement.expression_statement.expression_handle
    );

    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "  expression:\n");
    *indent_level += 1;
    yaml_print_expression(prog, expr, sb, indent_level);
    *indent_level -= 1;
  } break;

  default:
  {
    fprintf(
      stderr, "[ERROR] unhandles statement kind: %s (%i)\n",
      statement_kind_to_string(stmt->kind), stmt->kind
    );
    assert(0);
  } break;
  }
}

static void yaml_print_expression(
  const Program *prog, const Expression *expr, String_Builder *sb, int *indent_level
)
{
  yaml_print_indent(sb, *indent_level);
  string_builder_append_fmt(sb, "  - kind: %s\n", expression_kind_to_string(expr->kind));

  switch (expr->kind)
  {
  case EXPR_INT:
  {
    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "    value: %" PRId64 "\n", expr->expression.int_expression.value);
  } break;

  case EXPR_STRING:
  {
    const String_Expression *sexpr = &(expr->expression.string_expression);
    const char *str_value = string_handle_to_cstring(prog, sexpr->string_handle);
    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "    value: \"%s\"\n", str_value);
    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "    len: %lu\n", sexpr->len);
  } break;

  case EXPR_IDENT:
  {
    const Ident_Expression *iexpr = &(expr->expression.ident_expression);
    const char *ident = ident_handle_to_evaluated_ident(prog, iexpr->ident_handle);
    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "    ident: %s\n", ident);
  } break;

  case EXPR_INFIX:
  {
    const Infix_Expression *iexpr = &expr->expression.infix_expression;
    const char *op_str = operator_to_string(iexpr->op);

    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "    op: %s\n", op_str);

    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "    lhs:\n");
    *indent_level += 1;
    const Expression *lhs = expression_handle_to_expression(prog, iexpr->lhs);
    yaml_print_expression(prog, lhs, sb, indent_level);
    *indent_level -= 1;

    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "    rhs:\n");
    *indent_level += 1;
    const Expression *rhs = expression_handle_to_expression(prog, iexpr->rhs);
    yaml_print_expression(prog, rhs, sb, indent_level);
    *indent_level -= 1;
  } break;

  case EXPR_CALL:
  {
    const Call_Expression *cexpr = &expr->expression.call_expression;
    const Expression *function = expression_handle_to_expression(prog, cexpr->function);
    assert(function->kind == EXPR_IDENT);
    Ident_Handle ident_handle = function->expression.ident_expression.ident_handle;
    const char *ident = ident_handle_to_evaluated_ident(prog, ident_handle);
    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "    name: %s\n", ident);

    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "    args:\n");

    *indent_level += 1;
    for (size_t i = 0; i < cexpr->args.len; ++i)
    {
      const Expression *arg = &(cexpr->args.elems[i]);
      yaml_print_expression(prog, arg, sb, indent_level);
    }
    *indent_level -= 1;
  } break;

  default:
  {
    fprintf(
      stderr, "[ERROR] Unhandled Expression_Kind %i (%s)\n",
      expr->kind, expression_kind_to_string(expr->kind)
    );
    assert(0);
  } break;
  }
}

static void sexpr_print_statement(const Program *prog, const Statement *stmt, String_Builder *sb)
{
  switch (stmt->kind)
  {
  case STMT_VAR:
  {
    const Var_Statement *vs = &stmt->statement.var_statement;
    const char *ident = ident_handle_to_ident(prog, vs->ident_handle);
    const Expression *expr = expression_handle_to_expression(prog, vs->expression_handle);

    string_builder_append_fmt(sb, "(var %s ", ident);
    sexpr_print_expression(prog, expr, sb);
    string_builder_append(sb, ")");
  } break;

  case STMT_EXPRESSION:
  {
    const Expression *expr = expression_handle_to_expression(
      prog, stmt->statement.expression_statement.expression_handle
    );
    string_builder_append_fmt(sb, "(");
    sexpr_print_expression(prog, expr, sb);
    string_builder_append_fmt(sb, ")");
  } break;

  default:
  {
    fprintf(
      stderr,
      "[ERROR] Unhandled Statement_Kind %s (%i)\n",
      statement_kind_to_string(stmt->kind),
      stmt->kind
    );
    assert(0);
  } break;
  }
}

static void sexpr_print_expression(const Program *prog, const Expression *expr, String_Builder *sb)
{
  switch (expr->kind)
  {
  case EXPR_INT:
  {
    string_builder_append_fmt(sb, "%" PRId64 "", expr->expression.int_expression.value);
  } break;

  case EXPR_STRING:
  {
    const char *str_value = string_handle_to_cstring(
      prog, expr->expression.string_expression.string_handle
    );
    string_builder_append_fmt(sb, "\"%s\"", str_value);
  } break;

  case EXPR_IDENT:
  {
    const char *ident = ident_handle_to_evaluated_ident(
      prog, expr->expression.ident_expression.ident_handle
    );
    string_builder_append_fmt(sb, "%s", ident);
  } break;

  case EXPR_INFIX:
  {
    string_builder_append(sb, "(");
    {
      const Infix_Expression *iexpr = &expr->expression.infix_expression;
      const Expression *lhs = expression_handle_to_expression(prog, iexpr->lhs);
      const Expression *rhs = expression_handle_to_expression(prog, iexpr->rhs);
      const char *op_str = operator_to_string(iexpr->op);
      string_builder_append_fmt(sb, "%s ", op_str);
      sexpr_print_expression(prog, lhs, sb);
      string_builder_append_fmt(sb, " ", op_str);
      sexpr_print_expression(prog, rhs, sb);
    }
    string_builder_append(sb, ")");
  } break;

  case EXPR_CALL:
  {
    const Call_Expression *cexpr = &expr->expression.call_expression;
    const Expression *function = expression_handle_to_expression(prog, cexpr->function);
    assert(function->kind == EXPR_IDENT);
    Ident_Handle ident_handle = function->expression.ident_expression.ident_handle;
    const char *ident = ident_handle_to_evaluated_ident(prog, ident_handle);

    string_builder_append_fmt(sb, "%s [", ident);

    for (size_t i = 0; i < cexpr->args.len; ++i)
    {
      const Expression *arg = &(cexpr->args.elems[i]);
      sexpr_print_expression(prog, arg, sb);
      if (!(i + 1 >= cexpr->args.len))
      {
        string_builder_append_fmt(sb, " ; ");
      }
    }

    string_builder_append(sb, "]");
  } break;
  
  default:
  {
    fprintf(
      stderr,
      "[ERROR] Unhandled Expression_Kind %s (%i)\n",
      expression_kind_to_string(expr->kind),
      expr->kind
    );
    assert(0);
  } break;
  }
}
