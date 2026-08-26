#include <assert.h>

#include "token.h"

const char *token_type_to_string(Token_Type type)
{
    const char *result;

    switch (type)
    {
    #define X(NAME) case TT_ ## NAME: { result = #NAME; } break;
        #include "defs/token-type.def"
    #undef X
        default: {
            assert(0 && "TODO: unhandled token type");
        }
    }

    return result;
}
