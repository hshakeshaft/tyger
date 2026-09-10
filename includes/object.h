#ifndef TYOBJECT_H_
#define TYOBJECT_H_

typedef enum
{
#define X(NAME) OBJ_ ## NAME,
    #include "defs/object-type.def"
#undef X
    TYOBJECT_TYPE_COUNT
} TyObject_Type;

typedef struct tyobject
{
    TyObject_Type type;
    union {
        int integer;
        struct {
            const char *str;
            int len;
        } string;
        struct {
            const char *ident;
            struct tyobject *value;
        } ident;
    } as;
} TyObject;

const char *tyobject_type_to_string(TyObject_Type type);

TyObject *tyobject_create(TyObject_Type type, void *data);
void      tyobject_destroy(TyObject *object);

#endif  /* TYOBJECT_H_ */
