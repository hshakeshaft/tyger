#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "trace.h"
#include "tstrings.h"

#define TRACE_YAML_SPACES_PER_INDENT_LEVEL 4

static void yaml_print_header(const Program *p, String_Builder *sb);
static void yaml_print_statement(const Statement *stmt, String_Builder *sb, int *indent_level);
static void yaml_print_expression(const Expression *expr, String_Builder *sb, int *indent_level);

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
      yaml_print_statement(stmt, &sb, &indent_level);
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
static void yaml_print_statement(const Statement *stmt, String_Builder *sb, int *indent_level)
{
  yaml_print_indent(sb, *indent_level);
  string_builder_append_fmt(sb, "- kind: %s\n", statement_kind_to_string(stmt->kind));

  // TODO(HS): write YAML body for statement kinds
  switch (stmt->kind)
  {
  case STMT_VAR:
  {
    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "  ident: %s\n", stmt->statement.var_statement.ident);
  } break;

  case STMT_EXPRESSION:
  {
    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "  expression:\n");
    const Expression *expr = stmt->statement.expression_statement.expression;
    *indent_level += 1;
    yaml_print_expression(expr, sb, indent_level);
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

static void yaml_print_expression(const Expression *expr, String_Builder *sb, int *indent_level)
{
  yaml_print_indent(sb, *indent_level);
  string_builder_append_fmt(sb, "  - kind: %s\n", expression_kind_to_string(expr->kind));

  switch (expr->kind)
  {
  case EXPR_INT:
  {
    yaml_print_indent(sb, *indent_level);
    string_builder_append_fmt(sb, "    value: %i\n", expr->expression.int_expression.value);
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
