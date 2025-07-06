#ifndef TYGER_TSTRINGS_H_
#define TYGER_TSTRINGS_H_
#include <stdbool.h>
#include <stddef.h>

typedef struct string_view
{
  char *str;
  size_t len;
} String_View;

typedef struct string_builder
{
  char *buffer;
  size_t capacity;
  size_t len;
} String_Builder;

#if defined(__cplusplus)
#define make_string_view(STR, LEN) String_View{ (char*) (STR), (LEN) }
#else
#define make_string_view(STR, LEN) (String_View) { .str = (char*) (STR), .len = (LEN) }
#endif

#define make_string_view_ex(STR, OFFSET, LEN) make_string_view(&((STR)[OFFSET]), (LEN))

#define SV_FMT "%.*s"
#define SV_ARGS(S) (int) (S).len, (S).str

bool string_view_eq(String_View sv1, String_View sv2);
bool string_view_eq_str(String_View sv, const char *str);
void string_view_format_buffer(char *buffer, size_t buffer_len, String_View sv);

void string_builder_init(String_Builder *sb);
void string_builder_append(String_Builder *sb, const char *str);
void string_builder_append_fmt(String_Builder *sb, const char *fmt, ...);
const char *string_builder_to_cstring(const String_Builder *sb);

#endif // TYGER_TSTRINGS_H_
