#include "stringlib.h"

String_View sv_from_cstring(const char *str, int len)
{
    String_View sv;
    sv.str = (char*) str;
    sv.len = len;
    return sv;
}
