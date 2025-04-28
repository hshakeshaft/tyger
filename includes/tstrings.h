#ifndef TYGER_TSTRINGS_H_
#define TYGER_TSTRINGS_H_
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

#endif // TYGER_TSTRINGS_H_
