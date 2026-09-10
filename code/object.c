#include <stddef.h>

#include "memory.h"
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

TyObject *tyobject_create(TyObject_Type type, void *data)
{
    TyObject *object;

    object = TYGER_MALLOC(sizeof(*object));
    if (!object) { return NULL; }

    object->type = type;
    switch (type)
    {
        case OBJ_NONE: {
            /* NOTE(HS): this is left blank intentionally */
        } break;

        case OBJ_INTEGER: {
            object->as.integer = * (int*) data;
        } break;

        case OBJ_STRING: {
            object->as.string.len = * (int*) data;
        } break;

        case OBJ_IDENT: {
            object->as.ident.ident = (const char *) data;
        } break;

        default:;
    }

    return object;
}
