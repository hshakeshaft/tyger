#ifndef STRINGLIB_H_
#define STRINGLIB_H_

typedef struct string_view
{
    char *str;
    int len;
} String_View;

#define SV_FMT "%.*s"
#define SV_ARGS(SV) (SV).len, (SV).str

String_View sv_from_cstring(const char *str, int len);

#endif  /* STRINGLIB_H_ */
