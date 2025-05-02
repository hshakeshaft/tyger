#ifndef TYGER_TSTRINGS_H_
#define TYGER_TSTRINGS_H_
#include <stdbool.h>
#include <stddef.h>

typedef struct stirng_view
{
  char *str;
  size_t len;
} String_View;

#if defined(__cplusplus)
#define make_string_view(STR, LEN) String_View{ (char*) (STR), (LEN) }
#else
#define make_string_view(STR, LEN) (String_View) { .str = (char*) (STR), .len = (LEN) }
#endif

#define make_string_view_ex(STR, OFFSET, LEN) make_string_view(&((STR)[OFFSET]), (LEN))

#define SV_FMT "%.*s"
#define SV_ARGS(S) (int) (S).len, (S).str


#if defined(__cplusplus)
extern "C" {
#endif

bool string_view_eq(String_View sv1, String_View sv2);
bool string_view_eq_str(String_View sv, const char *str);
void string_view_format_buffer(char *buffer, size_t buffer_len, String_View sv);

#if defined(__cplusplus)
}
#endif

#endif // TYGER_TSTRINGS_H_
