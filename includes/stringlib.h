#ifndef STRINGLIB_H_
#define STRINGLIB_H_

typedef struct string_view
{
    char *str;
    int len;
} String_View;


#ifdef __cplusplus
extern "C" {
#endif

String_View sv_from_cstring(const char *str, int len);

#ifdef __cplusplus
}
#endif

#endif  /* STRINGLIB_H_ */
