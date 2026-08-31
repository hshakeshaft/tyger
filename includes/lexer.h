#ifndef LEXER_H_
#define LEXER_H_
#include "token.h"

typedef struct lexer
{
    const char *input;
    int input_len;
    int pos;
    int read_pos;
    char ch;

    const char *file;
    int line;
    int col;
} Lexer;


#ifdef __cplusplus
extern "C" {
#endif

int lexer_init_from_buffer(Lexer *lexer, const char *input_buffer);

Token lexer_next_token(Lexer *lx);

#ifdef __cplusplus
}
#endif

#endif  /* LEXER_H_ */
