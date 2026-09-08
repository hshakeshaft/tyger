#ifndef LEXER_H_
#define LEXER_H_
#include "token.h"

/* TODO(HS): I want to replace current lexical scan with string_view based window */
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


int lexer_init_from_buffer(Lexer *lexer, const char *input_buffer);

Token lexer_next_token(Lexer *lx);

#endif  /* LEXER_H_ */
