#ifndef TYOBJECT_H_
#define TYOBJECT_H_

typedef enum
{
#define X(NAME) OBJ_ ## NAME,
    #include "defs/object-type.def"
#undef X
    TYOBJECT_TYPE_COUNT
} TyObject_Type;

typedef struct
{
    TyObject_Type type;
    union {
        int integer;
    } as;
} TyObject;

const char *tyobject_type_to_string(TyObject_Type type);

TyObject *tyobject_create(TyObject_Type type, void *data);

#endif  /* TYOBJECT_H_ */
