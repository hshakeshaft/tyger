#ifndef TOKEN_H_
#define TOKEN_H_
#include "stringlib.h"

typedef enum token_type
{
#define X(NAME) TT_ ## NAME,
    #include "defs/token-type.def"
#undef X
    TOKEN_TYPE_COUNT
} Token_Type;

typedef struct token
{
    Token_Type type;
    String_View literal;
    const char *file;
    int line;
    int col;
    int offset;
} Token;


#ifdef __cplusplus
extern "C" {
#endif

const char *token_type_to_string(Token_Type type);

#ifdef __cplusplus
}
#endif

#endif  /* TOKEN_H_ */
