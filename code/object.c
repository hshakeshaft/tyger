#include <stddef.h>

#include "object.h"

const char *tyobject_type_to_string(TyObject_Type type)
{
    char *result;
    switch (type)
    {
    #define X(NAME) case OBJ_ ## NAME: result = #NAME; break;
        #include "defs/object-type.def"
    #undef X
        default: result = NULL;
    }
    return result;
}
