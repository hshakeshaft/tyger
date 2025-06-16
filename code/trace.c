#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "trace.h"

#define TRACE_YAML_SPACES_PER_INDENT_LEVEL 4

static void yaml_print_header(const Program *p, char *buffer, size_t *buffer_len, size_t *buffer_capacity);
static void yaml_print_statement(
  const Statement *stmt, char *buffer, size_t *buffer_len, size_t *buffer_capacity, int *indent_level
);

const char *program_to_string(const Program *p, Trace_Format kind)
{
  assert(p);
  
  size_t buffer_capacity = 2048;
  size_t buffer_len = 0;
  char *buffer = malloc(sizeof(char) * buffer_capacity);
  assert(buffer);

  switch (kind)
  {
  case TRACE_YAML:
  {
    yaml_print_header(p, buffer, &buffer_len, &buffer_capacity);
    for (size_t i = 0; i < p->statements.len; ++i)
    {
      int indent_level = 1;
      Statement *stmt = &(p->statements.elems[i]);
      yaml_print_statement(stmt, buffer, &buffer_len, &buffer_capacity, &indent_level);
    }
  } break;
  }

  buffer[buffer_len] = '\0';
  return buffer;
}

static void yaml_print_header(const Program *p, char *buffer, size_t *buffer_len, size_t *buffer_capacity)
{
  #define TRACE_YAML_HEADER_FMT "---\nErrors: %zu\nStatements: %zu\n---\nprogram:\n"

  int bytes_to_write = snprintf(NULL, 0, TRACE_YAML_HEADER_FMT, p->errors.len, p->statements.len);

  if ((*buffer_len + bytes_to_write) > *buffer_capacity)
  {
    size_t new_capacity = *buffer_capacity * 2;
    char *new_buffer = realloc(buffer, new_capacity);
    if (new_buffer != buffer)
    {
      buffer = new_buffer;
    }
    *buffer_capacity = new_capacity;
  }

  int bytes_written = snprintf(
    &(buffer[*buffer_len]), bytes_to_write + 1, TRACE_YAML_HEADER_FMT,
    p->errors.len, p->statements.len);

  assert(bytes_to_write == bytes_written);

  *buffer_len += bytes_written;
}

static void yaml_print_statement(
  const Statement *stmt, char *buffer, size_t *buffer_len, size_t *buffer_capacity, int *indent_level
)
{
  int num_indent_spaces = *indent_level * TRACE_YAML_SPACES_PER_INDENT_LEVEL;

  // write base indent
  if ((*buffer_len + num_indent_spaces) > *buffer_capacity)
  {
    size_t new_capacity = *buffer_capacity * 2;
    char *new_buffer = realloc(buffer, new_capacity);
    if (new_buffer != buffer)
    {
      buffer = new_buffer;
    }
    *buffer_capacity = new_capacity;
  }
  memset(&(buffer[*buffer_len]), ' ', num_indent_spaces);
  *buffer_len += num_indent_spaces;

  // write "kind"
  int bytes_to_write = snprintf(NULL, 0, "- kind: %s\n", statement_kind_to_string(stmt->kind));
  if ((*buffer_len + bytes_to_write) > *buffer_capacity)
  {
    size_t new_capacity = *buffer_capacity * 2;
    char *new_buffer = realloc(buffer, new_capacity);
    if (new_buffer != buffer)
    {
      buffer = new_buffer;
    }
    *buffer_capacity = new_capacity;
  }
  int bytes_written = snprintf(
    &(buffer[*buffer_len]), bytes_to_write + 1, "- kind: %s\n", statement_kind_to_string(stmt->kind)
  );
  assert(bytes_written == bytes_to_write);
  *buffer_len += bytes_written;

  // TODO(HS): write YAML body for statement kinds
}
