#include <assert.h>
#include <string.h>
#include <stdio.h>
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
  assert(bytes_to_write <= buffer_len);

  int bytes_written = snprintf(buffer, bytes_to_write + 1, "%.*s", (int) sv.len, sv.str);
  assert(bytes_written == bytes_to_write);
}

