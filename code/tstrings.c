#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tstrings.h"

bool string_view_eq(String_View sv1, String_View sv2)
{
  bool result = false;

  if (sv1.len != sv2.len)
  {
    return result;
  }

  for (size_t i = 0; i < sv1.len; ++i)
  {
    if (
      (sv1.str[i] != sv2.str[i])
      || (sv1.str[i] == '\0' && sv2.str[i] != '\0')
      || (sv2.str[i] == '\0' && sv1.str[i] != '\0'))
    {
      return result;
    }
  }

  result = true;
  return result;
}

bool string_view_eq_str(String_View sv, const char *str)
{
  bool result = false;

  size_t slen = strlen(str);
  if (sv.len != slen)
  {
    return result;
  }

  for (size_t i = 0; i < sv.len; ++i)
  {
    if (
      (sv.str[i] != str[i])
      || (sv.str[i] == '\0' && str[i] != '\0')
      || (sv.str[i] != '\0' && str[i] == '\0'))
    {
      return result;
    }
  }

  result = true;
  return result;
}

void string_view_format_buffer(char *buffer, size_t buffer_len, String_View sv)
{
  assert(buffer_len >= sv.len);

  int bytes_to_write = snprintf(NULL, 0, "%.*s", (int) sv.len, sv.str);
  assert(bytes_to_write <= (int) buffer_len);

  int bytes_written = snprintf(buffer, bytes_to_write + 1, "%.*s", (int) sv.len, sv.str);
  assert(bytes_written == bytes_to_write);
}

void string_builder_init(String_Builder *sb)
{
  sb->capacity = 2048;
  sb->len = 0;
  sb->buffer = malloc(sizeof(char) * sb->capacity);
}

void string_builder_append_fmt(String_Builder *sb, const char *fmt, ...)
{
  assert(sb);
  assert(sb->buffer);

  va_list args;
  va_start(args, fmt);
  int bytes_to_write = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  if ((sb->len + bytes_to_write) > sb->capacity)
  {
    size_t new_capacity = sb->capacity * 2;
    char *new_buffer = realloc(sb->buffer, new_capacity);
    if (sb->buffer != new_buffer)
    {
      sb->buffer = new_buffer;
    }
    sb->capacity = new_capacity;
  }

  va_start(args, fmt);
  int bytes_written = vsnprintf(&(sb->buffer[sb->len]), bytes_to_write + 1, fmt, args);
  va_end(args);

  assert(bytes_written == bytes_to_write);

  sb->len += bytes_written;
}

const char *string_builder_to_cstring(const String_Builder *sb)
{
  assert(sb);
  assert(sb->buffer);
  char *buffer = malloc(sizeof(char) * sb->len + 1);
  assert(buffer);
  strncpy(buffer, sb->buffer, sb->len);
  buffer[sb->len] = '\0';
  return buffer;
}
